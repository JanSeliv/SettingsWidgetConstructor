// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
//---
#include "SettingsUtilsLibrary.generated.h"

/**
 * The common functions library for Settings Widget Constructor.
 */
UCLASS()
class SETTINGSWIDGETCONSTRUCTOR_API USettingsUtilsLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Returns the Settings widget from viewport. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor", meta = (WorldContext = "WorldContextObject"))
	static class USettingsWidget* GetSettingsWidget(const UObject* WorldContextObject);

	/** Returns the Game User Settings object. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor", meta = (WorldContext = "OptionalWorldContext"))
	static class UGameUserSettings* GetGameUserSettings(const UObject* OptionalWorldContext = nullptr);

	/** Returns all Settings Rows from project's Settings Data Table and all other additional Data Tables from 'SettingsDataTable' Data Registry.
	 * Note: Is expensive function, prefer to cache the result once! */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor")
	static void GetAllSettingRows(TMap<FName, struct FSettingsRow>& OutSettingRows);

	/*********************************************************************************************
	 * Multiple Data Tables support
	 * Allows to register additional Settings Data Tables if needed
	 * https://docs.google.com/document/d/1IXnOqrgaXTClP-0cIo28a9f6GHc9N1BCgTNnMk-X9VQ
	 ********************************************************************************************* */
public:
	/** Registers given Settings Data Table to the Settings Data Registry. Alternatively Game Feature Action can be used to register it.
	 * It's automatically called on startup for the 'Settings Data Table' set in the Project Settings.
	 * Can be called manually to register additional Settings Data Tables. */
	UFUNCTION(BlueprintCallable, Category = "C++")
	static void RegisterDataTable(const TSoftObjectPtr<const class USettingsDataTable> SettingsDataTable);

	/** Returns all Settings Data Tables added to 'SettingsDataTable' Data Registry including Project's one. */
	static void GetAllSettingDataTables(TSet<const class USettingsDataTable*>& OutDataTables);
};
