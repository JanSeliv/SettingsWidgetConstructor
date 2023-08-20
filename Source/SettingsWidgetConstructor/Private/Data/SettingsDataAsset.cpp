// Copyright (c) Yevhenii Selivanov

#include "Data/SettingsDataAsset.h"
//---
#include "Data/SettingsDataTable.h"
#include "Data/SettingsRow.h"
//---
#include "DataRegistrySource_DataTable.h"
#include "DataRegistrySubsystem.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(SettingsDataAsset)

// Returns the data table, it has to be set manually
const USettingsDataTable* USettingsDataAsset::GetSettingsDataTable() const
{
	return SettingsDataTableInternal.LoadSynchronous();
}

// Returns all Settings Rows from project's Settings Data Table and all other additional Data Tables from 'SettingsDataTable' Data Registry
void USettingsDataAsset::GetAllSettingRows(TMap<FName, FSettingsRow>& OutSettingRows) const
{
	if (!OutSettingRows.IsEmpty())
	{
		OutSettingRows.Empty();
	}

	TSet<const USettingsDataTable*> OutDataTables;
	GetAllSettingDataTables(OutDataTables);

	/**
	 * Sort Setting Rows based on the FSettingsPrimary::ShowNextToSettingOverride property.
	 * All the next settings after ShowNextToSettingOverride in the same table will be also shown next to it.
	 * Sort is needed since setting can be shown based on another setting in different Settings Data Table, so we want to fix the order.
	 */

	TArray<FSettingsRow> OrderedSettings;
	TMap<FSettingTag, TArray<FSettingsRow>> OverrideBlocks;

	// Collect settings and override blocks
	for (const USettingsDataTable* TableIt : OutDataTables)
	{
		TMap<FName, FSettingsRow> TableRows;
		TableIt->GetSettingRows(TableRows);

		bool bCollectingOverrideBlock = false;
		TArray<FSettingsRow> CurrentOverrideBlock;
		FSettingTag CurrentOverrideTag;

		for (const TTuple<FName, FSettingsRow>& Pair : TableRows)
		{
			const FSettingsRow& Row = Pair.Value;
			const FSettingTag& OverrideTag = Row.SettingsPicker.PrimaryData.ShowNextToSettingOverride;

			if (bCollectingOverrideBlock)
			{
				CurrentOverrideBlock.Emplace(Row);
			}
			else if (OverrideTag.IsValid())
			{
				bCollectingOverrideBlock = true;
				CurrentOverrideTag = OverrideTag;
				CurrentOverrideBlock.Emplace(Row);
			}
			else
			{
				OrderedSettings.Emplace(Row);
			}
		}

		if (bCollectingOverrideBlock)
		{
			OverrideBlocks.Add(CurrentOverrideTag, MoveTemp(CurrentOverrideBlock));
		}
	}

	// Build the final map, handling the override blocks
	for (const FSettingsRow& Row : OrderedSettings)
	{
		const FName Tag = Row.SettingsPicker.PrimaryData.Tag.GetTagName();
		OutSettingRows.Add(Tag, Row);

		// Check if there's an override block for this tag
		const FSettingTag SettingTag = Row.SettingsPicker.PrimaryData.Tag;
		TArray<FSettingsRow>* OverrideBlock = OverrideBlocks.Find(SettingTag);
		if (OverrideBlock)
		{
			// Add the override block next to the current setting
			for (const FSettingsRow& OverrideRow : *OverrideBlock)
			{
				const FName OverrideTag = OverrideRow.SettingsPicker.PrimaryData.Tag.GetTagName();
				OutSettingRows.Add(OverrideTag, OverrideRow);
			}
		}
	}
}

/*********************************************************************************************
 * Data Registry
 ********************************************************************************************* */

// Returns the Settings Data Registry asset, is automatically set by default to which 'Settings Data Table' is added by itself
const UDataRegistry* USettingsDataAsset::GetSettingsDataRegistry() const
{
	return SettingsDataRegistryInternal.LoadSynchronous();
}

// Returns all Settings Data Tables added to 'SettingsDataTable' Data Registry
void USettingsDataAsset::GetAllSettingDataTables(TSet<const USettingsDataTable*>& OutDataTables) const
{
	if (!OutDataTables.IsEmpty())
	{
		OutDataTables.Empty();
	}

	const UDataRegistry* SettingsDataRegistry = GetSettingsDataRegistry();
	if (!ensureMsgf(SettingsDataRegistry, TEXT("ASSERT: 'SettingsDataRegistry' is not loaded, can't retrieve any settings!")))
	{
		return;
	}

	const UScriptStruct* SettingStruct = FSettingsRow::StaticStruct();
	TMap<FDataRegistryId, const uint8*> OutItemMap;
	SettingsDataRegistry->GetAllCachedItems(OutItemMap, SettingStruct);

	// Obtain all Settings Data Tables from the registry
	for (const TTuple<FDataRegistryId, const unsigned char*>& ItemIt : OutItemMap)
	{
		FDataRegistryLookup Lookup;
		SettingsDataRegistry->ResolveDataRegistryId(/*out*/Lookup, ItemIt.Key);

		FName ResolvedName;
		constexpr int32 LookupIndex = 0;
		UDataRegistrySource* LookupSource = SettingsDataRegistry->LookupSource(ResolvedName, Lookup, LookupIndex);

		const UDataRegistrySource_DataTable* DataTableSource = Cast<UDataRegistrySource_DataTable>(LookupSource);
		if (DataTableSource && !DataTableSource->SourceTable.IsNull())
		{
			const USettingsDataTable* DataTable = Cast<USettingsDataTable>(DataTableSource->SourceTable.LoadSynchronous());
			OutDataTables.Add(DataTable);
		}
	}
}

// Registers the Settings Data Table with the Data Registry
void USettingsDataAsset::RegisterDataTable(const TSoftObjectPtr<const USettingsDataTable> SettingsDataTable)
{
	UDataRegistrySubsystem* DataRegistrySubsystem = UDataRegistrySubsystem::Get();
	checkf(DataRegistrySubsystem, TEXT("ERROR: [%i] %s:\n'DataRegistrySubsystem' is null!"), __LINE__, *FString(__FUNCTION__));

	if (ensureMsgf(!SettingsDataRegistryInternal.IsNull(), TEXT("ASSERT: 'SettingsDataRegistry' is null, it has to be set automatically, something went wrong!")))
	{
		// Initialize the Settings Data Registry
		DataRegistrySubsystem->LoadRegistryPath(SettingsDataRegistryInternal.ToSoftObjectPath());
	}

	// If set, add the Settings Data Table to the Settings Data Registry
	const FSoftObjectPath DataTablePath = SettingsDataTable.ToSoftObjectPath();
	if (!DataTablePath.IsNull())
	{
		TMap<FDataRegistryType, TArray<FSoftObjectPath>> AssetMap;
		static const FDataRegistryType RegistryToAddTo{TEXT("SettingsDataTable")};
		TArray<FSoftObjectPath>& AssetList = AssetMap.Add(RegistryToAddTo);
		AssetList.Emplace(DataTablePath);
		DataRegistrySubsystem->PreregisterSpecificAssets(AssetMap);
	}
}

// Overrides post init to register Settings Data Table by default on startup
void USettingsDataAsset::PostInitProperties()
{
	Super::PostInitProperties();

	if (!GEngine || !GEngine->IsInitialized())
	{
		FCoreDelegates::OnPostEngineInit.AddUObject(this, &ThisClass::OnPostEngineInit);
	}
	else
	{
		OnPostEngineInit();
	}
}

// Is called once Engine is initialized, so we can register Settings Data Table by default on startup
void USettingsDataAsset::OnPostEngineInit()
{
	RegisterDataTable(SettingsDataTableInternal);
}

#if WITH_EDITOR
// Overrides property change events to handle picking Settings Data Table
void USettingsDataAsset::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	static const FName PropertyName = GET_MEMBER_NAME_CHECKED(USettingsDataAsset, SettingsDataTableInternal);
	const FProperty* Property = PropertyChangedEvent.Property;
	if (Property && Property->GetFName() == PropertyName)
	{
		// The Settings Data Table has been changed, so we have to register it
		RegisterDataTable(SettingsDataTableInternal);
	}
}
#endif // WITH_EDITOR
