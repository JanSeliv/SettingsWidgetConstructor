// Copyright (c) Yevhenii Selivanov.

#include "FunctionPickerData/SWCFunctionPicker.h"
//---
#if WITH_EDITOR
#include "Misc/DataValidation.h" // IsDataValid func
#endif
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(SWCFunctionPicker)

// Empty settings function
const FSWCFunctionPicker FSWCFunctionPicker::Empty = FSWCFunctionPicker();

// Custom constructor to set all members values
FSWCFunctionPicker::FSWCFunctionPicker(UClass* InFunctionClass, FName InFunctionName)
	: FunctionClass(InFunctionClass)
	  , FunctionName(InFunctionName) {}

// Returns the function pointer based on set data to this structure
UFunction* FSWCFunctionPicker::GetFunction() const
{
	if (CachedFunctionInternal.IsValid())
	{
		return CachedFunctionInternal.Get();
	}

	if (FunctionClass
		&& !FunctionName.IsNone())
	{
		UFunction* FoundFunction = FunctionClass->FindFunctionByName(FunctionName, EIncludeSuperFlag::ExcludeSuper);
		CachedFunctionInternal = FoundFunction;
		return FoundFunction;
	}

	return nullptr;
}

// Returns string in text format: Class::Function
FString FSWCFunctionPicker::ToDisplayString() const
{
	return IsValid() ? FString::Printf(TEXT("%s::%s"), *FunctionClass->GetName(), *FunctionName.ToString()) : FString();
}

// Creates a hash value
uint32 GetTypeHash(const FSWCFunctionPicker& Other)
{
	const uint32 FunctionClassHash = GetTypeHash(Other.FunctionClass);
	const uint32 FunctionNameHash = GetTypeHash(Other.FunctionName);
	return HashCombine(FunctionClassHash, FunctionNameHash);
}

#if WITH_EDITOR
// Validates chosen data
EDataValidationResult FSWCFunctionPicker::IsDataValid(FDataValidationContext& Context) const
{
	if (!FunctionClass)
	{
		Context.AddError(FText::FromString(TEXT("Function class is not set")));

		// Don't process next
		return EDataValidationResult::Invalid;
	}

	if (FunctionName.IsNone())
	{
		static const FText ErrorText = FText::FromString(TEXT("Function name is not set while the class '{0}' is chosen!"));
		Context.AddError(FText::Format(ErrorText, FText::FromString(FunctionClass->GetName())));

		// Don't process next
		return EDataValidationResult::Invalid;
	}

	// Check UFunction is set
	if (!GetFunction())
	{
		static const FText ErrorText = FText::FromString(TEXT("Function '{0}' does not exist in the class '{1}'!"));
		Context.AddError(FText::Format(ErrorText, FText::FromName(FunctionName), FText::FromString(FunctionClass->GetName())));

		// Don't process next
		return EDataValidationResult::Invalid;
	}

	return EDataValidationResult::Valid;
}
#endif // WITH_EDITOR
