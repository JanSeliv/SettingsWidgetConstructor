// Copyright (c) Yevhenii Selivanov

#include "MyDataTable/SWCMyDataTable.h"
//---
#if WITH_EDITOR
#include "EditorFramework/AssetImportData.h"
#include "Misc/FileHelper.h"
#include "UObject/ObjectSaveContext.h"
#endif // WITH_EDITOR
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(SWCMyDataTable)

// Default constructor to set RowStruct structure inherit from FSWCMyTableRow
USWCMyDataTable::USWCMyDataTable()
{
	RowStruct = FSWCMyTableRow::StaticStruct();
}

#if WITH_EDITOR
// Called on every change in this this row
void FSWCMyTableRow::OnDataTableChanged(const UDataTable* InDataTable, const FName InRowName)
{
	if (const USWCMyDataTable* SWCMyDataTable = Cast<USWCMyDataTable>(InDataTable))
	{
		const uint8& ThisRowPtr = reinterpret_cast<uint8&>(*this);
		USWCMyDataTable* DataTable = const_cast<USWCMyDataTable*>(SWCMyDataTable);
		DataTable->OnThisDataTableChanged(InRowName, ThisRowPtr);
	}
}

// Is called on saving the data table
void USWCMyDataTable::PostSaveRoot(FObjectPostSaveRootContext ObjectSaveContext)
{
	Super::PostSaveRoot(ObjectSaveContext);

	ReexportToJson();
}

// Reexports this table to .json
void USWCMyDataTable::ReexportToJson()
{
	if (!AssetImportData)
	{
		return;
	}

	const FString CurrentFilename = AssetImportData->GetFirstFilename();
	if (!CurrentFilename.IsEmpty())
	{
		const FString TableAsJSON = GetTableAsJSON(EDataTableExportFlags::UseJsonObjectsForStructs);
		FFileHelper::SaveStringToFile(TableAsJSON, *CurrentFilename);
	}
}
#endif // WITH_EDITOR
