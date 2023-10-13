// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "Data/SettingArchetypesData.h"
#include "Data/SettingFunction.h"
#include "Data/SettingTag.h"
#include "SettingsWidgetConstructor/Private/MyDataTable/SWCMyDataTable.h"
//---
#include "SettingsRow.generated.h"

#define TEXT_NONE FCoreTexts::Get().None

/* --- Settings Data Table dependencies ---
 *
 *	╔USettingsDataTable
 *	╚═══╦FSettingsRow
 *		╚═══╦FSettingsPicker
 *			╠═══╦FSettingsPrimary
 *			║	╠════FSettingTag
 *			║	╚════FSettingFunctionPicker (Owner, Setter, Getter)
 *			╚════FSettingsDataBase
 */

/**
  * The primary data of any setting.
  * Does not contain a default states for its value, because it should be set in the DefaultGameUserSettings.ini
  */
USTRUCT(BlueprintType)
struct SETTINGSWIDGETCONSTRUCTOR_API FSettingsPrimary
{
	GENERATED_BODY()

	/** Empty settings primary row. */
	static const FSettingsPrimary EmptyPrimary;

	/** The tag of the setting. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSettingTag Tag = FSettingTag::EmptySettingTag;

	/** The static function to obtain object to call Setters and Getters.
	  * The FunctionContextTemplate meta will contain a name of one UFunctionPickerTemplate delegate. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (FunctionContextTemplate))
	FSettingFunctionPicker Owner = FSettingFunctionPicker::EmptySettingFunction;

	/** The Setter function to be called to set the setting value for the Owner object.
	  * The FunctionSetterTemplate meta will contain a name of one UFunctionPickerTemplate delegate. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (FunctionSetterTemplate))
	FSettingFunctionPicker Setter = FSettingFunctionPicker::EmptySettingFunction;

	/** The Getter function to be called to get the setting value from the Owner object.
	  * The FunctionGetterTemplate meta will contain a name of one UFunctionPickerTemplate delegate. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (FunctionGetterTemplate))
	FSettingFunctionPicker Getter = FSettingFunctionPicker::EmptySettingFunction;

	/** The setting name. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (MultiLine = "true"))
	FText Caption = TEXT_NONE;

	/** The description to be shown as tooltip. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (MultiLine = "true"))
	FText Tooltip = TEXT_NONE;

	/** The padding of this setting. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMargin Padding = 0.f;

	/** The custom line height for this setting. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LineHeight = 48.f;

	/** Set true to add new column starting from this setting. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bStartOnNextColumn = false;

	/** Contains tags of settings which are needed to update after change of this setting. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Categories = "Settings"))
	FGameplayTagContainer SettingsToUpdate = FGameplayTagContainer::EmptyContainer;

	/** If set, it will override own position to be shown after specified setting.
	 * Is useful when should be shown after setting that is created in another Settings Data Table.
	 * All the next settings after ShowNextToSettingOverride in the same table will be also shown next to it.
	 * @see 'Data Registr'y category of 'Settings Data Asset'. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSettingTag ShowNextToSettingOverride = FSettingTag::EmptySettingTag;

	/** Created widget of the chosen setting (button, checkbox, combobox, slider, text line, user input). */
	TWeakObjectPtr<class USettingSubWidget> SettingSubWidget = nullptr;

	/** Contains all cached functions of the Owner object. */
	TArray<FName> OwnerFunctionList;

	/** Returns true if is valid. */
	FORCEINLINE bool IsValid() const { return Tag.IsValid(); }

	/** Compares for equality.
	* @param Other The other object being compared. */
	bool operator==(const FSettingsPrimary& Other) const;

	/** Creates a hash value.
	* @param Other the other object to create a hash value for. */
	friend SETTINGSWIDGETCONSTRUCTOR_API uint32 GetTypeHash(const FSettingsPrimary& Other);

#if WITH_EDITOR
	/** Validates chosen data. */
	EDataValidationResult IsDataValid(class FDataValidationContext& Context) const;
#endif // WITH_EDITOR

	/*********************************************************************************************
	 * Setting Owner
	 * Is living object that contains specific Setter and Getter functions.
	 ********************************************************************************************* */
public:
	/** Is executed to obtain holding object. */
	UObject* GetSettingOwner(const UObject* WorldContext) const;

	/** The cached bound delegate that returns holding object. */
	USettingFunctionTemplate::FOnGetterObject OwnerFunc;
};

/**
  * Allows to select in editor only one setting archetype property at once within this struct.
  * Is customizable struct, its members were created under FSettingsPicker ...
  * (instead of FSettingsRow which implements table row struct and can't be customized),
  * to have possibility to be property-customized by FSettingsPickerCustomization.
  * @see FSettingsDataBase
  */
USTRUCT(BlueprintType)
struct SETTINGSWIDGETCONSTRUCTOR_API FSettingsPicker
{
	GENERATED_BODY()

	/** Nothing picked. */
	static const FSettingsPicker Empty;

	/** Contains a in-game settings type to be used - the name of one of these members. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SettingsType = NAME_None;

	/** The common setting data. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSettingsPrimary PrimaryData;

	/** The button setting data. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSettingsButton Button;

	/** The checkbox setting data. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSettingsCheckbox Checkbox;

	/** The combobox setting data. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSettingsCombobox Combobox;

	/** The slider setting data. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSettingsSlider Slider;

	/** The text line setting data. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSettingsTextLine TextLine;

	/** The user input setting data. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSettingsUserInput UserInput;

	/** The custom widget setting data. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSettingsCustomWidget CustomWidget;

	/** Returns the pointer to one of the chosen in-game type.
	  * It searches the member property of this struct by a value of SettingsType.
	  * @see FSettingsPicker::SettingsType */
	FSettingsDataBase* GetChosenSettingsData() const;

	/** Returns true if row is valid. */
	FORCEINLINE bool IsValid() const { return !(*this == Empty); }

	/** Compares for equality.
	  * @param Other The other object being compared. */
	bool operator==(const FSettingsPicker& Other) const;

	/** Creates a hash value.
	  * @param Other the other object to create a hash value for. */
	friend SETTINGSWIDGETCONSTRUCTOR_API uint32 GetTypeHash(const FSettingsPicker& Other);

#if WITH_EDITOR
	/** Validates chosen data. */
	EDataValidationResult IsDataValid(class FDataValidationContext& Context) const;
#endif // WITH_EDITOR
};

/**
  * Row of the settings data table.
  * In a row can be specified all UI values, chosen any getter/setter in the list.
  * Main features:
  * By this row, new setting will be automatically added on UI.
  * Executing UI getters/setters will call automatically bounded chosen functions.
  */
USTRUCT(BlueprintType)
struct SETTINGSWIDGETCONSTRUCTOR_API FSettingsRow : public FSWCMyTableRow
{
	GENERATED_BODY()

	/** The setting row to be customised. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FSettingsPicker SettingsPicker = FSettingsPicker::Empty;
};
