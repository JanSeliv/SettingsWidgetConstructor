// Copyright (c) Yevhenii Selivanov

#include "MyUtilsLibraries/SettingsUtilsLibrary.h"
//---
#include "MyUtilsLibraries/SWCWidgetUtilsLibrary.h"
#include "GameFramework/GameUserSettings.h"
#include "UI/SettingsWidget.h"
#include "Data/SettingsDataAsset.h"
#include "Data/SettingsDataTable.h"
#include "Data/SettingsRow.h"
//---
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "DataRegistrySource_DataTable.h"
#include "DataRegistrySubsystem.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(SettingsUtilsLibrary)

// Returns the Settings widget from viewport
USettingsWidget* USettingsUtilsLibrary::GetSettingsWidget(const UObject* WorldContextObject)
{
	UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull) : nullptr;
	return FSWCWidgetUtilsLibrary::FindWidgetOfClass<USettingsWidget>(World);
}

// Returns the Game User Settings object
UGameUserSettings* USettingsUtilsLibrary::GetGameUserSettings(const UObject* OptionalWorldContext/* = nullptr*/)
{
	return GEngine ? GEngine->GetGameUserSettings() : nullptr;
}

// Returns all Settings Rows from project's Settings Data Table and all other additional Data Tables from 'SettingsDataTable' Data Registry
void USettingsUtilsLibrary::GetAllSettingRows(TMap<FName, FSettingsRow>& OutSettingRows)
{
	if (!OutSettingRows.IsEmpty())
	{
		OutSettingRows.Empty();
	}

	TSet<const USettingsDataTable*> OutDataTables;
	GetAllSettingDataTables(OutDataTables);

	if (!ensureMsgf(!OutDataTables.IsEmpty(), TEXT("ASSERT: [%i] %s:\n'Settings Data Table' is not set in the 'Project Settings', can't retrieve any settings!"), __LINE__, *FString(__FUNCTION__)))
	{
		return;
	}

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
		checkf(TableIt, TEXT("ERROR: [%i] %s:\n'TableIt' is null!"), __LINE__, *FString(__FUNCTION__));

		TMap<FName, FSettingsRow> TableRows;
		TableIt->GetSettingRows(TableRows);

		TArray<FSettingsRow> CurrentOverrideBlock;
		FSettingTag CurrentOverrideTag;

		for (const TTuple<FName, FSettingsRow>& Pair : TableRows)
		{
			const FSettingsRow& Row = Pair.Value;
			const FSettingTag& OverrideTag = Row.SettingsPicker.PrimaryData.ShowNextToSettingOverride;

			if (OverrideTag.IsValid())
			{
				// Store the previous block if any, then start a new block
				if (CurrentOverrideBlock.Num() > 0)
				{
					OverrideBlocks.Emplace(CurrentOverrideTag, MoveTemp(CurrentOverrideBlock));
				}
				CurrentOverrideBlock.Empty();
				CurrentOverrideTag = OverrideTag;
			}

			if (CurrentOverrideTag.IsValid())
			{
				CurrentOverrideBlock.Emplace(Row);
			}
			else
			{
				OrderedSettings.Emplace(Row);
			}
		}

		// Add the last block if any
		if (CurrentOverrideBlock.Num() > 0)
		{
			OverrideBlocks.Emplace(CurrentOverrideTag, MoveTemp(CurrentOverrideBlock));
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
 * Multiple Data Tables support
 ********************************************************************************************* */

// Registers the Settings Data Table with the Data Registry
void USettingsUtilsLibrary::RegisterDataTable(const TSoftObjectPtr<const USettingsDataTable> SettingsDataTable)
{
	UDataRegistrySubsystem* DataRegistrySubsystem = UDataRegistrySubsystem::Get();
	checkf(DataRegistrySubsystem, TEXT("ERROR: [%i] %s:\n'DataRegistrySubsystem' is null!"), __LINE__, *FString(__FUNCTION__));

	const TSoftObjectPtr<const UDataRegistry>& SettingsDataRegistry = USettingsDataAsset::Get().GetSettingsDataRegistrySoft();
	if (ensureMsgf(!SettingsDataRegistry.IsNull(), TEXT("ASSERT: 'SettingsDataRegistry' is null, it has to be set automatically, something went wrong!")))
	{
		// Initialize the Settings Data Registry
		DataRegistrySubsystem->LoadRegistryPath(SettingsDataRegistry.ToSoftObjectPath());
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

// Returns all Settings Data Tables added to 'SettingsDataTable' Data Registry
void USettingsUtilsLibrary::GetAllSettingDataTables(TSet<const USettingsDataTable*>& OutDataTables)
{
	if (!OutDataTables.IsEmpty())
	{
		OutDataTables.Empty();
	}

	const UDataRegistry* SettingsDataRegistry = USettingsDataAsset::Get().GetSettingsDataRegistry();
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
		if (!DataTableSource || DataTableSource->SourceTable.IsNull())
		{
			continue;
		}

		const USettingsDataTable* DataTable = Cast<USettingsDataTable>(DataTableSource->SourceTable.LoadSynchronous());
		if (ensureMsgf(DataTable, TEXT("ASSERT: [%i] %s:\nNext Settings Data Table is found, but can't be loaded: %s"), __LINE__, *FString(__FUNCTION__), *DataTableSource->SourceTable.GetAssetName()))
		{
			OutDataTables.Add(DataTable);
		}
	}
}
