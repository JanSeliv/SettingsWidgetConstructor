﻿// Copyright (c) Yevhenii Selivanov

#include "Data/SettingArchetypesData.h"
//---
#include "Data/SettingsDataAsset.h"
#include "Data/SettingTag.h"
#include "UI/SettingCombobox.h"
#include "UI/SettingSubWidget.h"
#include "UI/SettingsWidget.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(SettingArchetypesData)

/*********************************************************************************************
 * FSettingsButton
 ********************************************************************************************* */

// Returns the sub-widget class of this setting type
TSubclassOf<USettingSubWidget> FSettingsButton::GetSubWidgetClass() const
{
	return USettingsDataAsset::Get().GetButtonClass();
}

// Calls the Set function of the Settings Widget of this setting type
void FSettingsButton::SetSettingValue(USettingsWidget& SettingsWidget, const FSettingTag& Tag, const FString& Value)
{
	SettingsWidget.SetSettingButtonPressed(Tag);
}

// Calls the Bind function of the Settings Widget of this setting type
void FSettingsButton::BindSetting(USettingsWidget& SettingsWidget, const FSettingsPrimary& PrimaryData)
{
	SettingsWidget.BindButton(PrimaryData, *this);
}

/*********************************************************************************************
 * FSettingsCheckbox
 ********************************************************************************************* */

// Returns the sub-widget class of this setting type
TSubclassOf<USettingSubWidget> FSettingsCheckbox::GetSubWidgetClass() const
{
	return USettingsDataAsset::Get().GetCheckboxClass();
}

// Calls the Get function of the Settings Widget of this setting type
void FSettingsCheckbox::GetSettingValue(const USettingsWidget& SettingsWidget, const FSettingTag& Tag, FString& OutResult) const
{
	const bool Value = SettingsWidget.GetCheckboxValue(Tag);
	OutResult = Value ? TEXT("true") : TEXT("false");
}

// Calls the Set function of the Settings Widget of this setting type
void FSettingsCheckbox::SetSettingValue(USettingsWidget& SettingsWidget, const FSettingTag& Tag, const FString& Value)
{
	const bool NewValue = Value.ToBool();
	SettingsWidget.SetSettingCheckbox(Tag, NewValue);
}

// Calls the Bind function of the Settings Widget of this setting type
void FSettingsCheckbox::BindSetting(USettingsWidget& SettingsWidget, const FSettingsPrimary& PrimaryData)
{
	SettingsWidget.BindCheckbox(PrimaryData, *this);
}

/*********************************************************************************************
 * FSettingsCombobox
 ********************************************************************************************* */

// Returns the sub-widget class of this setting type
TSubclassOf<USettingSubWidget> FSettingsCombobox::GetSubWidgetClass() const
{
	const TSubclassOf<USettingCombobox> ComboboxClass = USettingsDataAsset::Get().GetComboboxClass();
	return ComboboxClass;
}

// Calls the Get function of the Settings Widget of this setting type
void FSettingsCombobox::GetSettingValue(const USettingsWidget& SettingsWidget, const FSettingTag& Tag, FString& OutResult) const
{
	const int32 Value = SettingsWidget.GetComboboxIndex(Tag);
	OutResult = FString::Printf(TEXT("%d"), Value);
}

// Calls the Set function of the Settings Widget of this setting type
void FSettingsCombobox::SetSettingValue(USettingsWidget& SettingsWidget, const FSettingTag& Tag, const FString& Value)
{
	if (Value.IsNumeric())
	{
		const int32 NewValue = FCString::Atoi(*Value);
		SettingsWidget.SetSettingComboboxIndex(Tag, NewValue);
	}
}

// Calls the Bind function of the Settings Widget of this setting type
void FSettingsCombobox::BindSetting(USettingsWidget& SettingsWidget, const FSettingsPrimary& PrimaryData)
{
	SettingsWidget.BindCombobox(PrimaryData, *this);
}

/*********************************************************************************************
 * FSettingsSlider
 ********************************************************************************************* */

// Returns the sub-widget class of this setting type
TSubclassOf<USettingSubWidget> FSettingsSlider::GetSubWidgetClass() const
{
	return USettingsDataAsset::Get().GetSliderClass();
}

// Calls the Get function of the Settings Widget of this setting type
void FSettingsSlider::GetSettingValue(const USettingsWidget& SettingsWidget, const FSettingTag& Tag, FString& OutResult) const
{
	const double Value = SettingsWidget.GetSliderValue(Tag);
	OutResult = FString::Printf(TEXT("%f"), Value);
}

// Calls the Set function of the Settings Widget of this setting type
void FSettingsSlider::SetSettingValue(USettingsWidget& SettingsWidget, const FSettingTag& Tag, const FString& Value)
{
	const double NewValue = FCString::Atod(*Value);
	SettingsWidget.SetSettingSlider(Tag, NewValue);
}

// Calls the Bind function of the Settings Widget of this setting type
void FSettingsSlider::BindSetting(USettingsWidget& SettingsWidget, const FSettingsPrimary& PrimaryData)
{
	SettingsWidget.BindSlider(PrimaryData, *this);
}

/*********************************************************************************************
 * FSettingsTextLine
 ********************************************************************************************* */

// Returns the sub-widget class of this setting type
TSubclassOf<USettingSubWidget> FSettingsTextLine::GetSubWidgetClass() const
{
	return USettingsDataAsset::Get().GetTextLineClass();
}

// Calls the Get function of the Settings Widget of this setting type
void FSettingsTextLine::GetSettingValue(const USettingsWidget& SettingsWidget, const FSettingTag& Tag, FString& OutResult) const
{
	FText OutText;
	SettingsWidget.GetTextLineValue(Tag, OutText);
	OutResult = OutText.ToString();
}

// Calls the Set function of the Settings Widget of this setting type
void FSettingsTextLine::SetSettingValue(USettingsWidget& SettingsWidget, const FSettingTag& Tag, const FString& Value)
{
	const FText NewValue = FText::FromString(Value);
	SettingsWidget.SetSettingTextLine(Tag, NewValue);
}

// Calls the Bind function of the Settings Widget of this setting type
void FSettingsTextLine::BindSetting(USettingsWidget& SettingsWidget, const FSettingsPrimary& PrimaryData)
{
	SettingsWidget.BindTextLine(PrimaryData, *this);
}

/*********************************************************************************************
 * FSettingsUserInput
 ********************************************************************************************* */

// Returns the sub-widget class of this setting type
TSubclassOf<USettingSubWidget> FSettingsUserInput::GetSubWidgetClass() const
{
	return USettingsDataAsset::Get().GetUserInputClass();
}

// Calls the Get function of the Settings Widget of this setting type
void FSettingsUserInput::GetSettingValue(const USettingsWidget& SettingsWidget, const FSettingTag& Tag, FString& OutResult) const
{
	const FName Value = SettingsWidget.GetUserInputValue(Tag);
	OutResult = Value.ToString();
}

// Calls the Set function of the Settings Widget of this setting type
void FSettingsUserInput::SetSettingValue(USettingsWidget& SettingsWidget, const FSettingTag& Tag, const FString& Value)
{
	const FName NewValue = *Value;
	SettingsWidget.SetSettingUserInput(Tag, NewValue);
}

// Calls the Bind function of the Settings Widget of this setting type
void FSettingsUserInput::BindSetting(USettingsWidget& SettingsWidget, const FSettingsPrimary& PrimaryData)
{
	SettingsWidget.BindUserInput(PrimaryData, *this);
}

/*********************************************************************************************
 * FSettingsCustomWidget
 ********************************************************************************************* */

// Returns the sub-widget class of this setting type
TSubclassOf<USettingSubWidget> FSettingsCustomWidget::GetSubWidgetClass() const
{
	return CustomWidgetClass;
}

// Calls the Get function of the Settings Widget of this setting type
void FSettingsCustomWidget::GetSettingValue(const USettingsWidget& SettingsWidget, const FSettingTag& Tag, FString& OutResult) const
{
	const TSoftObjectPtr<USettingCustomWidget> CustomWidget = SettingsWidget.GetCustomWidget(Tag);
	OutResult = CustomWidget.IsValid() ? CustomWidget.ToSoftObjectPath().ToString() : TEXT("");
}

// Calls the Bind function of the Settings Widget of this setting type
void FSettingsCustomWidget::BindSetting(USettingsWidget& SettingsWidget, const FSettingsPrimary& PrimaryData)
{
	SettingsWidget.BindCustomWidget(PrimaryData, *this);
}
