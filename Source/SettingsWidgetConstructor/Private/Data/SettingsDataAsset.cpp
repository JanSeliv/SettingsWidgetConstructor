// Copyright (c) Yevhenii Selivanov

#include "Data/SettingsDataAsset.h"
//---
#include "Data/SettingsDataTable.h"
#include "MyUtilsLibraries/SettingsUtilsLibrary.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(SettingsDataAsset)

// Returns the data table, it has to be set manually
const USettingsDataTable* USettingsDataAsset::GetSettingsDataTable() const
{
	return SettingsDataTableInternal.LoadSynchronous();
}

// Returns the Settings Data Registry asset, is automatically set by default to which 'Settings Data Table' is added by itself
const UDataRegistry* USettingsDataAsset::GetSettingsDataRegistry() const
{
	return SettingsDataRegistryInternal.LoadSynchronous();
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
	USettingsUtilsLibrary::RegisterDataTable(SettingsDataTableInternal);
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
		USettingsUtilsLibrary::RegisterDataTable(SettingsDataTableInternal);
	}
}
#endif // WITH_EDITOR
