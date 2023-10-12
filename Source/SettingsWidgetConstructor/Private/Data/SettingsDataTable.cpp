// Copyright (c) Yevhenii Selivanov.

#include "Data/SettingsDataTable.h"
//---
#include "Data/SettingsRow.h"
//---
#if WITH_EDITOR
#include "DataTableEditorUtils.h" // FDataTableEditorUtils::RenameRow
#include "Misc/DataValidation.h" // IsDataValid func
#endif
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(SettingsDataTable)

// Default constructor to set members as FSettingsRow
USettingsDataTable::USettingsDataTable()
{
	RowStruct = FSettingsRow::StaticStruct();
}

#if WITH_EDITOR
// Called on every change in this data table to automatic set the key name by specified setting tag
void USettingsDataTable::OnThisDataTableChanged(FName RowKey, const uint8& RowData)
{
	Super::OnThisDataTableChanged(RowKey, RowData);

	// Set row name by specified tag
	const FSettingsRow& Row = reinterpret_cast<const FSettingsRow&>(RowData);
	const FName RowValueTag = Row.SettingsPicker.PrimaryData.Tag.GetTagName();
	if (!RowValueTag.IsNone() // Tag is not empty
		&& RowKey != RowValueTag // New tag name
		&& !RowMap.Contains(RowValueTag)) // Unique key
	{
		FDataTableEditorUtils::RenameRow(this, RowKey, RowValueTag);
	}
}

// Is called to validate the data table setup
EDataValidationResult USettingsDataTable::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);

	int32 RowIndex = 1; // Tables indexing starts from 1
	TMap<FName, FSettingsRow> SettingsRows;
	GetSettingRows(SettingsRows);
	for (const TTuple<FName, FSettingsRow>& RowIt : SettingsRows)
	{
		const EDataValidationResult RowResult = RowIt.Value.SettingsPicker.IsDataValid(Context);
		Result = CombineDataValidationResults(Result, RowResult);
		if (RowResult == EDataValidationResult::Invalid)
		{
			Context.AddError(FText::FromString(FString::Printf(TEXT("ERROR: Next setting row is invalid: index [%d], name: '%s'"), RowIndex, *RowIt.Key.ToString())));
		}

		++RowIndex;
	}

	return Result;
}
#endif // WITH_EDITOR
