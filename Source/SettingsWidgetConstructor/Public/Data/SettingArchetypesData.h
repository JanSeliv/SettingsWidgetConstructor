// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Data/SettingTypes.h"
//---
#include "Data/SettingFunction.h"
#include "Framework/Text/TextLayout.h"
//---
#include "SettingArchetypesData.generated.h"

struct FSettingTag;
struct FSettingsPrimary;

class USettingsWidget;
class USettingSubWidget;

/**
  * The base archetype of any setting.
  * Properties of child archetypes are used by Settings Picker select a setting.
  *	 ╔FSettingsPicker
  *	 ╚════FSettingsDataBase
  * Actively uses Function Template metas to provide a way to filter functions in the list:
  * @see FFunctionPicker
  */
USTRUCT(BlueprintType, meta = (
	FunctionContextTemplate = "/Script/SettingsWidgetConstructor.SettingFunctionTemplate::OnGetterObject__DelegateSignature"))
struct SETTINGSWIDGETCONSTRUCTOR_API FSettingsDataBase
{
	GENERATED_BODY()

	virtual ~FSettingsDataBase() = default;

#if WITH_EDITOR
	/** Validates chosen data. */
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const { return EDataValidationResult::Valid; }
#endif // WITH_EDITOR

	/*********************************************************************************************
	 * Base methods to override
	 *
	 * Since FSettingsPicker contains many types of settings, base methods are used to call them indirectly: 
	 * FSettingsPicker& Setting == ... ;
	 * FSettingsDataBase* ChosenData = Setting.GetChosenSettingsData();
	 * ChosenData-> ... ; // call any of the following methods
	 ********************************************************************************************* */

	/** Base method to get the sub-widget class of this setting type. */
	virtual TSubclassOf<USettingSubWidget> GetSubWidgetClass() const PURE_VIRTUAL(FSettingsDataBase::GetSubWidgetClass, return nullptr;);

	/** Base method to get the setting value, where appropriate Getter of Settings Widget will be called. */
	virtual void GetSettingValue(const USettingsWidget& SettingsWidget, const FSettingTag& Tag, FString& OutResult) const PURE_VIRTUAL(FSettingsDataBase::GetSettingValue,);

	/** Base method to set the setting value, where string value will be converted to the setting type and appropriate Setter of Settings Widget will be called. */
	virtual void SetSettingValue(USettingsWidget& SettingsWidget, const FSettingTag& Tag, const FString& Value) PURE_VIRTUAL(FSettingsDataBase::SetSettingValue,);

	/** Base method to Bind setting to specified in table Get/Set delegates, so both methods will be called. */
	virtual void BindSetting(USettingsWidget& SettingsWidget, const FSettingsPrimary& InOutPrimaryData) PURE_VIRTUAL(FSettingsDataBase::BindSetting,);

	/** Base method to add the setting to the Settings Widget, where appropriate Add method of Settings Widget will be called. */
	virtual void AddSetting(USettingsWidget& SettingsWidget, const FSettingsPrimary& InOutPrimaryData) PURE_VIRTUAL(FSettingsDataBase::AddSetting,);
};

/**
  * The setting button data.
  */
USTRUCT(BlueprintType, meta = (
	FunctionSetterTemplate = "/Script/SettingsWidgetConstructor.SettingFunctionTemplate::OnButtonPressed__DelegateSignature"))
struct SETTINGSWIDGETCONSTRUCTOR_API FSettingsButton : public FSettingsDataBase
{
	GENERATED_BODY()

	/*********************************************************************************************
	 * Data
	 ********************************************************************************************* */

	/** Either Header, Content, or Footer. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EMyVerticalAlignment VerticalAlignment = EMyVerticalAlignment::Content;

	/** Either Left, Right, Center, or Fill. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TEnumAsByte<EHorizontalAlignment> HorizontalAlignment = HAlign_Fill;

	/*********************************************************************************************
	 * Bindings
	 ********************************************************************************************* */

	/** Cached bound delegate, is executed on pressing this button. */
	USettingFunctionTemplate::FOnButtonPressed OnButtonPressed;

	/*********************************************************************************************
	 * Overrides
	 ********************************************************************************************* */

	/** Returns the sub-widget class of this setting type. */
	virtual TSubclassOf<USettingSubWidget> GetSubWidgetClass() const override;

	/** Calls the Get function of the Settings Widget of this setting type. */
	virtual void GetSettingValue(const USettingsWidget& SettingsWidget, const FSettingTag& Tag, FString& OutResult) const override { OutResult = FString(TEXT("")); }

	/** Calls the Set function of the Settings Widget of this setting type. */
	virtual void SetSettingValue(USettingsWidget& SettingsWidget, const FSettingTag& Tag, const FString& Value) override;

	/** Calls the Bind function of the Settings Widget of this setting type. */
	virtual void BindSetting(USettingsWidget& SettingsWidget, const FSettingsPrimary& PrimaryData) override;

	/** Calls the Add function of the Settings Widget of this setting type. */
	virtual void AddSetting(USettingsWidget& SettingsWidget, const FSettingsPrimary& PrimaryData) override;
};

/**
  * The setting checkbox data.
  */
USTRUCT(BlueprintType, meta = (
	FunctionSetterTemplate = "/Script/SettingsWidgetConstructor.SettingFunctionTemplate::OnSetterBool__DelegateSignature",
	FunctionGetterTemplate = "/Script/SettingsWidgetConstructor.SettingFunctionTemplate::OnGetterBool__DelegateSignature"))
struct SETTINGSWIDGETCONSTRUCTOR_API FSettingsCheckbox : public FSettingsDataBase
{
	GENERATED_BODY()

	/*********************************************************************************************
	 * Data
	 ********************************************************************************************* */

	/** The cached current checkbox state. */
	bool bIsSet = false;

	/*********************************************************************************************
	 * Bindings
	 ********************************************************************************************* */

	/** The cached bound delegate, is executed to get the current checkbox state. */
	USettingFunctionTemplate::FOnGetterBool OnGetterBool;

	/** The cached bound delegate, is executed to set the current checkbox state. */
	USettingFunctionTemplate::FOnSetterBool OnSetterBool;

	/*********************************************************************************************
	 * Overrides
	 ********************************************************************************************* */

	/** Returns the sub-widget class of this setting type. */
	virtual TSubclassOf<USettingSubWidget> GetSubWidgetClass() const override;

	/** Calls the Get function of the Settings Widget of this setting type. */
	virtual void GetSettingValue(const USettingsWidget& SettingsWidget, const FSettingTag& Tag, FString& OutResult) const override;

	/** Calls the Set function of the Settings Widget of this setting type. */
	virtual void SetSettingValue(USettingsWidget& SettingsWidget, const FSettingTag& Tag, const FString& Value) override;

	/** Calls the Bind function of the Settings Widget of this setting type. */
	virtual void BindSetting(USettingsWidget& SettingsWidget, const FSettingsPrimary& PrimaryData) override;

	/** Calls the Add function of the Settings Widget of this setting type. */
	virtual void AddSetting(USettingsWidget& SettingsWidget, const FSettingsPrimary& InOutPrimaryData) override;
};

/**
  * The setting combobox data.
  */
USTRUCT(BlueprintType, meta = (
	FunctionSetterTemplate = "/Script/SettingsWidgetConstructor.SettingFunctionTemplate::OnSetterInt__DelegateSignature",
	FunctionGetterTemplate = "/Script/SettingsWidgetConstructor.SettingFunctionTemplate::OnGetterInt__DelegateSignature"))
struct SETTINGSWIDGETCONSTRUCTOR_API FSettingsCombobox : public FSettingsDataBase
{
	GENERATED_BODY()

	/*********************************************************************************************
	 * Data
	 ********************************************************************************************* */

	/** The Setter function to be called to set all combobox members. */
	UPROPERTY(EditDefaultsOnly, meta = (FunctionSetterTemplate = "/Script/SettingsWidgetConstructor.SettingFunctionTemplate::OnSetMembers__DelegateSignature"))
	FSettingFunctionPicker SetMembers = FSettingFunctionPicker::EmptySettingFunction;

	/** The Setter function to be called to get all combobox members. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (FunctionGetterTemplate = "/Script/SettingsWidgetConstructor.SettingFunctionTemplate::OnGetMembers__DelegateSignature"))
	FSettingFunctionPicker GetMembers = FSettingFunctionPicker::EmptySettingFunction;

	/** Contains all combobox members. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FText> Members;

	/** Text alignment either left, center, or right. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TEnumAsByte<ETextJustify::Type> TextJustify = ETextJustify::Center;

	/** The cached chosen member index. */
	int32 ChosenMemberIndex = INDEX_NONE;

	/*********************************************************************************************
	 * Bindings
	 ********************************************************************************************* */

	/** The cached bound delegate, is executed to get the chosen member index. */
	USettingFunctionTemplate::FOnGetterInt OnGetterInt;

	/** The cached bound delegate, is executed to set the chosen member index. */
	USettingFunctionTemplate::FOnSetterInt OnSetterInt;

	/** The cached bound delegate, is executed to get all combobox members. */
	USettingFunctionTemplate::FOnGetMembers OnGetMembers;

	/** The cached bound delegate, is executed to set all combobox members. */
	USettingFunctionTemplate::FOnSetMembers OnSetMembers;

	/*********************************************************************************************
	 * Overrides
	 ********************************************************************************************* */

	/** Returns the sub-widget class of this setting type. */
	virtual TSubclassOf<USettingSubWidget> GetSubWidgetClass() const override;

	/** Calls the Get function of the Settings Widget of this setting type. */
	virtual void GetSettingValue(const USettingsWidget& SettingsWidget, const FSettingTag& Tag, FString& OutResult) const override;

	/** Calls the Set function of the Settings Widget of this setting type. */
	virtual void SetSettingValue(USettingsWidget& SettingsWidget, const FSettingTag& Tag, const FString& Value) override;

	/** Calls the Bind function of the Settings Widget of this setting type. */
	virtual void BindSetting(USettingsWidget& SettingsWidget, const FSettingsPrimary& PrimaryData) override;

	/** Calls the Add function of the Settings Widget of this setting type. */
	virtual void AddSetting(USettingsWidget& SettingsWidget, const FSettingsPrimary& InOutPrimaryData) override;
};

/**
  * The setting slider data.
  */
USTRUCT(BlueprintType, meta = (
	FunctionSetterTemplate = "/Script/SettingsWidgetConstructor.SettingFunctionTemplate::OnSetterFloat__DelegateSignature",
	FunctionGetterTemplate = "/Script/SettingsWidgetConstructor.SettingFunctionTemplate::OnGetterFloat__DelegateSignature"))
struct SETTINGSWIDGETCONSTRUCTOR_API FSettingsSlider : public FSettingsDataBase
{
	GENERATED_BODY()

	/*********************************************************************************************
	 * Data
	 ********************************************************************************************* */

	/** Cached slider value (0..1). */
	double ChosenValue = INDEX_NONE;

	/*********************************************************************************************
	 * Bindings
	 ********************************************************************************************* */

	/** The cached bound delegate, is executed to get the current slider value. */
	USettingFunctionTemplate::FOnGetterFloat OnGetterFloat;

	/** The cached bound delegate, is executed to set the current slider value. */
	USettingFunctionTemplate::FOnSetterFloat OnSetterFloat;

	/*********************************************************************************************
	 * Overrides
	 ********************************************************************************************* */

	/** Returns the sub-widget class of this setting type. */
	virtual TSubclassOf<USettingSubWidget> GetSubWidgetClass() const override;

	/** Calls the Get function of the Settings Widget of this setting type. */
	virtual void GetSettingValue(const USettingsWidget& SettingsWidget, const FSettingTag& Tag, FString& OutResult) const override;

	/** Calls the Set function of the Settings Widget of this setting type. */
	virtual void SetSettingValue(USettingsWidget& SettingsWidget, const FSettingTag& Tag, const FString& Value) override;

	/** Calls the Bind function of the Settings Widget of this setting type. */
	virtual void BindSetting(USettingsWidget& SettingsWidget, const FSettingsPrimary& PrimaryData) override;

	/** Calls the Add function of the Settings Widget of this setting type. */
	virtual void AddSetting(USettingsWidget& SettingsWidget, const FSettingsPrimary& InOutPrimaryData) override;
};

/**
  * The setting text line data.
  */
USTRUCT(BlueprintType, meta = (
	FunctionSetterTemplate = "/Script/SettingsWidgetConstructor.SettingFunctionTemplate::OnSetterText__DelegateSignature",
	FunctionGetterTemplate = "/Script/SettingsWidgetConstructor.SettingFunctionTemplate::OnGetterText__DelegateSignature"))
struct SETTINGSWIDGETCONSTRUCTOR_API FSettingsTextLine : public FSettingsDataBase
{
	GENERATED_BODY()

	/*********************************************************************************************
	 * Data
	 ********************************************************************************************* */

	/** Either Header, Content, or Footer. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EMyVerticalAlignment VerticalAlignment = EMyVerticalAlignment::Content;

	/** Either Left, Right, Center, or Fill. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TEnumAsByte<EHorizontalAlignment> HorizontalAlignment = HAlign_Fill;

	/*********************************************************************************************
	 * Bindings
	 ********************************************************************************************* */

	/** The cached bound delegate, is executed to set the text caption. */
	USettingFunctionTemplate::FOnGetterText OnGetterText;

	/** The cached bound delegate, is executed to get the text caption. */
	USettingFunctionTemplate::FOnSetterText OnSetterText;

	/*********************************************************************************************
	 * Overrides
	 ********************************************************************************************* */

	/** Returns the sub-widget class of this setting type. */
	virtual TSubclassOf<USettingSubWidget> GetSubWidgetClass() const override;

	/** Calls the Get function of the Settings Widget of this setting type. */
	virtual void GetSettingValue(const USettingsWidget& SettingsWidget, const FSettingTag& Tag, FString& OutResult) const override;

	/** Calls the Set function of the Settings Widget of this setting type. */
	virtual void SetSettingValue(USettingsWidget& SettingsWidget, const FSettingTag& Tag, const FString& Value) override;

	/** Calls the Bind function of the Settings Widget of this setting type. */
	virtual void BindSetting(USettingsWidget& SettingsWidget, const FSettingsPrimary& PrimaryData) override;

	/** Calls the Add function of the Settings Widget of this setting type. */
	virtual void AddSetting(USettingsWidget& SettingsWidget, const FSettingsPrimary& InOutPrimaryData) override;
};

/**
  * The setting user input data.
  */
USTRUCT(BlueprintType, meta = (
	FunctionSetterTemplate = "/Script/SettingsWidgetConstructor.SettingFunctionTemplate::OnSetterName__DelegateSignature",
	FunctionGetterTemplate = "/Script/SettingsWidgetConstructor.SettingFunctionTemplate::OnGetterName__DelegateSignature"))
struct SETTINGSWIDGETCONSTRUCTOR_API FSettingsUserInput : public FSettingsDataBase
{
	GENERATED_BODY()

	/*********************************************************************************************
	 * Data
	 ********************************************************************************************* */

	/** The maximal length of the player input that is allowed to type.
	 * Set 0 to do not limit number of characters. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 MaxCharactersNumber = 0;

	/** The cached text shown left of the input box. */
	FName UserInput = NAME_None;

	/*********************************************************************************************
	 * Bindings
	 ********************************************************************************************* */

	/** The cached bound delegate, is executed to set the input text. */
	USettingFunctionTemplate::FOnGetterName OnGetterName;

	/** The cached bound delegate, is executed to get the input text. */
	USettingFunctionTemplate::FOnSetterName OnSetterName;

	/*********************************************************************************************
	 * Overrides
	 ********************************************************************************************* */

	/** Returns the sub-widget class of this setting type. */
	virtual TSubclassOf<USettingSubWidget> GetSubWidgetClass() const override;

	/** Calls the Get function of the Settings Widget of this setting type. */
	virtual void GetSettingValue(const USettingsWidget& SettingsWidget, const FSettingTag& Tag, FString& OutResult) const override;

	/** Calls the Set function of the Settings Widget of this setting type. */
	virtual void SetSettingValue(USettingsWidget& SettingsWidget, const FSettingTag& Tag, const FString& Value) override;

	/** Calls the Bind function of the Settings Widget of this setting type. */
	virtual void BindSetting(USettingsWidget& SettingsWidget, const FSettingsPrimary& PrimaryData) override;

	/** Calls the Add function of the Settings Widget of this setting type. */
	virtual void AddSetting(USettingsWidget& SettingsWidget, const FSettingsPrimary& InOutPrimaryData) override;
};

/**
  * The setting user input data.
  */
USTRUCT(BlueprintType, meta = (
	FunctionSetterTemplate = "/Script/SettingsWidgetConstructor.SettingFunctionTemplate::OnSetterWidget__DelegateSignature",
	FunctionGetterTemplate = "/Script/SettingsWidgetConstructor.SettingFunctionTemplate::OnGetterWidget__DelegateSignature"))
struct SETTINGSWIDGETCONSTRUCTOR_API FSettingsCustomWidget : public FSettingsDataBase
{
	GENERATED_BODY()

	/*********************************************************************************************
	 * Data
	 ********************************************************************************************* */

	/** Contains created custom widget of the setting. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ShowOnlyInnerProperties))
	TSubclassOf<class USettingCustomWidget> CustomWidgetClass = nullptr;

	/*********************************************************************************************
	 * Bindings
	 ********************************************************************************************* */

	/** The cached bound delegate, is executed to set the custom widget. */
	USettingFunctionTemplate::FOnGetterWidget OnGetterWidget;

	/** The cached bound delegate, is executed to get the custom widget. */
	USettingFunctionTemplate::FOnSetterWidget OnSetterWidget;

	/*********************************************************************************************
	 * Overrides
	 ********************************************************************************************* */

	/** Returns the sub-widget class of this setting type. */
	virtual TSubclassOf<USettingSubWidget> GetSubWidgetClass() const override;

	/** Calls the Get function of the Settings Widget of this setting type. */
	virtual void GetSettingValue(const USettingsWidget& SettingsWidget, const FSettingTag& Tag, FString& OutResult) const override;

	/** Calls the Set function of the Settings Widget of this setting type. */
	virtual void SetSettingValue(USettingsWidget& SettingsWidget, const FSettingTag& Tag, const FString& Value) override {}

	/** Calls the Bind function of the Settings Widget of this setting type. */
	virtual void BindSetting(USettingsWidget& SettingsWidget, const FSettingsPrimary& PrimaryData) override;

	/** Calls the Add function of the Settings Widget of this setting type. */
	virtual void AddSetting(USettingsWidget& SettingsWidget, const FSettingsPrimary& InOutPrimaryData) override;
};
