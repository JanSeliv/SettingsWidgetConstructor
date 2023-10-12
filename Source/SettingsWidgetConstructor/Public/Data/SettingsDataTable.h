// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "SettingsWidgetConstructor/Private/MyDataTable/SWCMyDataTable.h"
//---
#include "SettingsDataTable.generated.h"

/**
 * Settings data table with FSettingsRow members.
 * Provides additional in-editor functionality like automatic set the key name by specified setting tag.
 */
UCLASS(BlueprintType)
class SETTINGSWIDGETCONSTRUCTOR_API USettingsDataTable : public USWCMyDataTable
{
	GENERATED_BODY()

public:
	/** Default constructor to set members as FSettingsRow. */
	USettingsDataTable();

	/** Returns the table rows.
	 * @see USettingsDataAsset::SettingsDataTableInternal */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor")
	void GetSettingRows(TMap<FName, struct FSettingsRow>& OutRows) const { GetRows(OutRows); }

protected:
#if WITH_EDITOR
	/** Called on every change in this data table to automatic set the key name by specified setting tag. */
	virtual void OnThisDataTableChanged(FName RowKey, const uint8& RowData) override;

	/** Is called to validate the data table setup. */
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif // WITH_EDITOR
};
