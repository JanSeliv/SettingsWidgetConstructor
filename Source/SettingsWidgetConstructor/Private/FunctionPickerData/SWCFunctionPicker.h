// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "SWCFunctionPicker.generated.h"

/**
  * Allows designer to choose the function in the list.
  *
  * It's possible to set the template by specifying the function or delegate in the UPROPERTY's meta.
  * It will filter the list of all functions by its return type and type of all function arguments.
  *	If none meta is set, all functions will appear in the list without any filtering.
  *
  * Meta keys:
  * FunctionContextTemplate (only to show static functions or Blueprint Library functions)
  * FunctionGetterTemplate
  * FunctionSetterTemplate
  *
  * Meta value (without prefixes):
  * /Script/ModuleName.ClassName::FunctionName
  *
  * Example:
  * UPROPERTY(EditDefaultsOnly, meta = (FunctionSetterTemplate = "/Script/FunctionPicker.FunctionPickerTemplate::OnSetMembers__DelegateSignature"))
  * FSWCFunctionPicker SetMembers = FSWCFunctionPicker::Empty;
  */
USTRUCT(BlueprintType)
struct SETTINGSWIDGETCONSTRUCTOR_API FSWCFunctionPicker
{
	GENERATED_BODY()

	/** Empty function data. */
	static const FSWCFunctionPicker Empty;

	/** Default constructor. */
	FSWCFunctionPicker() = default;

	/** Custom constructor to set all members values. */
	FSWCFunctionPicker(UClass* InFunctionClass, FName InFunctionName);

	/** The class where function can be found. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (DisplayName = "Class", AllowAbstract, ShowOnlyInnerProperties))
	TObjectPtr<UClass> FunctionClass = nullptr;

	/** The function name to choose for specified class.*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (DisplayName = "Function", ShowOnlyInnerProperties))
	FName FunctionName = NAME_None;

	/** Returns true if is valid. */
	FORCEINLINE bool IsValid() const { return FunctionClass && !FunctionName.IsNone(); }

	/** Returns the function pointer based on set data to this structure. */
	UFunction* GetFunction() const;

	/** Returns string in text format: Class::Function. */
	FString ToDisplayString() const;

	/** Compares for equality.
	  * @param Other The other object being compared. */
	FORCEINLINE bool operator==(const FSWCFunctionPicker& Other) const { return GetTypeHash(*this) == GetTypeHash(Other); }

	/** Creates a hash value.
	  * @param Other the other object to create a hash value for. */
	friend SETTINGSWIDGETCONSTRUCTOR_API uint32 GetTypeHash(const FSWCFunctionPicker& Other);

	/** bool operator */
	FORCEINLINE operator bool() const { return IsValid(); }

	/** FName operator */
	FORCEINLINE operator FName() const { return FunctionName; }

#if WITH_EDITOR
	/** Returns error if function is not valid.
	 * To make this validation work, override IsDataValid(Context) in any UObject-derived class and call this function.
	 * See example: https://github.com/JanSeliv/Bomber/commit/a29b933 */
	EDataValidationResult IsDataValid(class FDataValidationContext& Context) const;
#endif // WITH_EDITOR

protected:
	/** Contains cached function ptr for performance reasons. */
	mutable TWeakObjectPtr<UFunction> CachedFunctionInternal = nullptr;
};
