// Copyright (c) Yevhenii Selivanov

#include "UI/SettingSubWidget.h"

// SWC
#include "Data/SettingsDataAsset.h"
#include "MyUtilsLibraries/SWCWidgetUtilsLibrary.h"
#include "UI/SettingsWidget.h"

// UE
#include "Components/Button.h"
#include "Components/CheckBox.h"
#include "Components/EditableTextBox.h"
#include "Components/HorizontalBox.h"
#include "Components/ScrollBox.h"
#include "Components/SizeBox.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Engine/Texture.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SSlider.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SettingSubWidget)

// Set the new setting tag for this widget
void USettingSubWidget::SetSettingPrimaryRow(const FSettingsPrimary& InSettingPrimaryRow)
{
	PrimaryDataInternal = InSettingPrimaryRow;
}

// Returns the main setting widget (the outer of this subwidget)
USettingsWidget* USettingSubWidget::GetSettingsWidget() const
{
	if (!SettingsWidgetInternal)
	{
		// Try to find the parent widget
		return FSWCWidgetUtilsLibrary::GetParentWidgetOfClass<USettingsWidget>(this);
	}
	return SettingsWidgetInternal;
}

// Returns the main setting widget checked
USettingsWidget& USettingSubWidget::GetSettingsWidgetChecked() const
{
	USettingsWidget* SettingsWidget = GetSettingsWidget();
	checkf(SettingsWidget, TEXT("%s: 'SettingsWidgetInternal' is null"), *FString(__FUNCTION__));
	return *SettingsWidget;
}

// Sets the main settings widget for this subwidget
void USettingSubWidget::SetSettingsWidget(USettingsWidget* InSettingsWidget)
{
	SettingsWidgetInternal = InSettingsWidget;
}

// Sets the parent widget element in hierarchy of this subwidget
UPanelSlot* USettingSubWidget::Attach()
{
	if (ParentSlotInternal)
	{
		// is already attached
		return ParentSlotInternal;
	}

	const FSettingsDataBase* SettingData = GetSettingData();
	const EMyVerticalAlignment Alignment = SettingData ? SettingData->GetVerticalAlignment() : EMyVerticalAlignment::None;
	if (!ensureMsgf(Alignment != EMyVerticalAlignment::None, TEXT("ASSERT: [%i] %s:\n'This widget '%s' can not be attached to the parent widget, because it has no alignment!"), __LINE__, *FString(__FUNCTION__), *GetName()))
	{
		return nullptr;
	}

	UPanelWidget* ParentWidget = nullptr;
	switch (Alignment)
	{
		case EMyVerticalAlignment::Header:
			ParentWidget = GetSettingsWidgetChecked().GetHeaderVerticalBox();
			break;
		case EMyVerticalAlignment::Content:
			if (const USettingColumn* Column = GetSettingsWidgetChecked().GetColumnBySetting(GetSettingTag()))
			{
				ParentWidget = Column->GetVerticalHolderBox();
			}
			break;
		case EMyVerticalAlignment::Footer:
			ParentWidget = GetSettingsWidgetChecked().GetFooterVerticalBox();
			break;
		default: break;
	}

	if (!ensureMsgf(ParentWidget, TEXT("ASSERT: [%i] %s:\n'ParentWidget' is not found for the setting '%s'"), __LINE__, *FString(__FUNCTION__), *GetSettingTag().ToString()))
	{
		return nullptr;
	}

	ParentSlotInternal = ParentWidget->AddChild(this);

	ensureMsgf(ParentSlotInternal, TEXT("ASSERT: [%i] %s:\nFailed to attached the Setting subwidget with the next tag: '%s'"), __LINE__, *FString(__FUNCTION__), *GetSettingTag().ToString());
	return ParentSlotInternal;
}

// Adds given widget as tooltip to this setting
void USettingSubWidget::AddTooltipWidget()
{
	if (PrimaryDataInternal.Tooltip.IsEmpty()
	    || PrimaryDataInternal.Tooltip.EqualToCaseIgnored(FCoreTexts::Get().None))
	{
		return;
	}

	USettingTooltip* CreatedWidget = CreateWidget<USettingTooltip>(GetOwningPlayer(), USettingsDataAsset::Get().GetTooltipClass());
	checkf(CreatedWidget, TEXT("ERROR: [%i] %s:\n'CreatedWidget' is null!"), __LINE__, *FString(__FUNCTION__));

	SetToolTip(CreatedWidget);
	CreatedWidget->SetToolTipText(PrimaryDataInternal.Tooltip);
	CreatedWidget->ApplyTheme();
}

// Base method that is called when the underlying slate widget is constructed
void USettingSubWidget::OnAddSetting(const FSettingsPicker& Setting)
{
	BPOnAddSetting();

	AddTooltipWidget();

	Attach();

	ApplyTheme();
}

// Returns the custom line height for this setting
float USettingSubWidget::GetLineHeight() const
{
	return SizeBoxWidget ? SizeBoxWidget->GetMinDesiredHeight() : 0.f;
}

// Set custom line height for this setting
void USettingSubWidget::SetLineHeight(float NewLineHeight)
{
	if (SizeBoxWidget)
	{
		SizeBoxWidget->SetMinDesiredHeight(NewLineHeight);
	}
}

// Returns the caption text that is shown on UI
void USettingSubWidget::GetCaptionText(FText& OutCaptionText) const
{
	if (CaptionWidget)
	{
		OutCaptionText = CaptionWidget->GetText();
	}
}

// Set the new caption text on UI for this widget
void USettingSubWidget::SetCaptionText(const FText& NewCaptionText)
{
	if (CaptionWidget)
	{
		CaptionWidget->SetText(NewCaptionText);
	}
}

// Set the new button setting data for this widget
void USettingButton::SetButtonData(const FSettingsButton& InButtonData)
{
	ButtonDataInternal = InButtonData;
}

// Called after the underlying slate widget is constructed
void USettingButton::NativeConstruct()
{
	Super::NativeConstruct();

	if (ButtonWidget)
	{
		ButtonWidget->SetClickMethod(EButtonClickMethod::PreciseClick);
		ButtonWidget->OnClicked.AddUniqueDynamic(this, &ThisClass::USettingButton::OnButtonPressed);

		SlateButtonInternal = FSWCWidgetUtilsLibrary::GetSlateWidget<SButton>(ButtonWidget);
		check(SlateButtonInternal.IsValid());
	}
}

// Is overridden to construct the button
void USettingButton::OnAddSetting(const FSettingsPicker& Setting)
{
	ButtonDataInternal = Setting.Button;

	Super::OnAddSetting(Setting);
}

// Called when the Button Widget is pressed
void USettingButton::OnButtonPressed()
{
	if (!SettingsWidgetInternal)
	{
		return;
	}

	SettingsWidgetInternal->SetSettingButtonPressed(GetSettingTag());
}

// Set the new checkbox setting data for this widget
void USettingCheckbox::SetCheckboxData(const FSettingsCheckbox& InCheckboxData)
{
	CheckboxDataInternal = InCheckboxData;
}

// Internal function to change the value of this subwidget
void USettingCheckbox::SetCheckboxValue(bool InValue)
{
	if (ensureMsgf(CheckboxWidget, TEXT("ASSERT: [%i] %hs:\n'CheckboxWidget' condition is FALSE"), __LINE__, __FUNCTION__))
	{
		CheckboxWidget->SetCheckedState(InValue ? ECheckBoxState::Checked : ECheckBoxState::Unchecked);
	}

	K2_OnSetCheckboxValue(InValue);
}

// Called after the underlying slate widget is constructed
void USettingCheckbox::NativeConstruct()
{
	Super::NativeConstruct();

	if (CheckboxWidget)
	{
		CheckboxWidget->OnCheckStateChanged.AddUniqueDynamic(this, &ThisClass::OnCheckStateChanged);

		SlateCheckboxInternal = FSWCWidgetUtilsLibrary::GetSlateWidget<SCheckBox>(CheckboxWidget);
		check(SlateCheckboxInternal.IsValid());
	}
}

// Called when the checked state has changed
void USettingCheckbox::OnCheckStateChanged_Implementation(bool bIsChecked)
{
	if (!SettingsWidgetInternal)
	{
		return;
	}

	SettingsWidgetInternal->SetSettingCheckbox(GetSettingTag(), bIsChecked);
}

// Is overridden to construct the checkbox
void USettingCheckbox::OnAddSetting(const FSettingsPicker& Setting)
{
	CheckboxDataInternal = Setting.Checkbox;

	Super::OnAddSetting(Setting);
}

// Set the new slider setting data for this widget
void USettingSlider::SetSliderData(const FSettingsSlider& InSliderData)
{
	SliderDataInternal = InSliderData;
}

// Internal function to change the value of this subwidget
void USettingSlider::SetSliderValue(double InValue)
{
	if (ensureMsgf(SliderWidget, TEXT("ERROR: [%i] %hs:\n'SliderWidget' is null!"), __LINE__, __FUNCTION__))
	{
		SliderWidget->SetValue(InValue);
	}

	K2_OnSetSliderValue(InValue);
}

// Called after the underlying slate widget is constructed
void USettingSlider::NativeConstruct()
{
	Super::NativeConstruct();

	if (SliderWidget)
	{
		SliderWidget->OnValueChanged.AddUniqueDynamic(this, &ThisClass::OnValueChanged);
		SliderWidget->OnMouseCaptureEnd.AddUniqueDynamic(this, &ThisClass::OnMouseCaptureEnd);

		SlateSliderInternal = FSWCWidgetUtilsLibrary::GetSlateWidget<SSlider>(SliderWidget);
		check(SlateSliderInternal.IsValid());
	}
}

// Invoked when the mouse is released and a capture ends
void USettingSlider::OnMouseCaptureEnd()
{
	// Play the sound
	if (SettingsWidgetInternal)
	{
		SettingsWidgetInternal->PlayUIClickSFX();
	}
}

// Called when the value is changed by slider or typing
void USettingSlider::OnValueChanged(float Value)
{
	if (!SettingsWidgetInternal)
	{
		return;
	}

	SettingsWidgetInternal->SetSettingSlider(GetSettingTag(), Value);
}

// Is overridden to construct the slider
void USettingSlider::OnAddSetting(const FSettingsPicker& Setting)
{
	SliderDataInternal = Setting.Slider;

	Super::OnAddSetting(Setting);
}

// Set the new Text Line setting data for this widget
void USettingTextLine::SetTextLineData(const FSettingsTextLine& InTextLineData)
{
	TextLineDataInternal = InTextLineData;
}

// Is overridden to construct the text line
void USettingTextLine::OnAddSetting(const FSettingsPicker& Setting)
{
	TextLineDataInternal = Setting.TextLine;

	Super::OnAddSetting(Setting);
}

// Returns current text set in the Editable Text Box
void USettingUserInput::GetEditableText(FText& OutText) const
{
	if (EditableTextBox)
	{
		OutText = EditableTextBox->GetText();
	}
}

// Set the new user input setting data for this widget
void USettingUserInput::SetUserInputData(const FSettingsUserInput& InUserInputData)
{
	UserInputDataInternal = InUserInputData;
}

// Internal function to change the value of this subwidget
void USettingUserInput::SetUserInputValue(FName InValue)
{
	if (!ensureMsgf(EditableTextBox, TEXT("ERROR: [%i] %hs:\n'EditableTextBox' is null!"), __LINE__, __FUNCTION__)
	    || FText::FromName(InValue).EqualTo(EditableTextBox->GetText()))
	{
		return;
	}

	EditableTextBox->SetText(FText::FromName(InValue));

	K2_OnSetUserInputValue(InValue);
}

// Is overridden to apply theme to unique parts of this widget
void USettingUserInput::ApplyTheme_Implementation()
{
	Super::ApplyTheme_Implementation();

	SEditableTextBox* InSlateEditableTextBox = SlateEditableTextBoxInternal.Pin().Get();
	if (!ensureMsgf(InSlateEditableTextBox, TEXT("ASSERT: [%i] %hs:\n'SlateEditableTextBoxInternal' is not valid!"), __LINE__, __FUNCTION__)
	    || !ensureMsgf(EditableTextBox, TEXT("ASSERT: [%i] %hs:\n'EditableTextBox' is not valid!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	const USettingsDataAsset& SettingsDataAsset = USettingsDataAsset::Get();
	const FSettingsThemeData& UserInputThemeData = SettingsDataAsset.GetUserInputThemeData();
	const FMiscThemeData& MiscThemeData = SettingsDataAsset.GetMiscThemeData();

	FSlateBrush NormalBrush;
	NormalBrush.SetResourceObject(UserInputThemeData.Texture);
	NormalBrush.ImageSize = UserInputThemeData.Size;
	NormalBrush.DrawAs = UserInputThemeData.DrawAs;
	NormalBrush.Margin = UserInputThemeData.Margin;
	NormalBrush.TintColor = MiscThemeData.ThemeColorNormal;

	FSlateBrush HoveredBrush = NormalBrush;
	HoveredBrush.TintColor = MiscThemeData.ThemeColorHover;

	EditableTextBox->WidgetStyle.BackgroundImageNormal = NormalBrush;
	EditableTextBox->WidgetStyle.BackgroundImageHovered = HoveredBrush;
	EditableTextBox->WidgetStyle.BackgroundImageFocused = NormalBrush;
	EditableTextBox->WidgetStyle.BackgroundImageReadOnly = NormalBrush;
	EditableTextBox->WidgetStyle.Padding = UserInputThemeData.Padding;
	EditableTextBox->WidgetStyle.TextStyle.Font = MiscThemeData.TextAndCaptionFont;
	EditableTextBox->WidgetStyle.TextStyle.ColorAndOpacity = MiscThemeData.TextAndCaptionColor;
	EditableTextBox->WidgetStyle.ForegroundColor = MiscThemeData.ThemeColorNormal;
	EditableTextBox->WidgetStyle.ReadOnlyForegroundColor = MiscThemeData.ThemeColorExtra;
	EditableTextBox->WidgetStyle.ScrollBarStyle.HorizontalBackgroundImage = NormalBrush;
	EditableTextBox->WidgetStyle.ScrollBarStyle.VerticalBackgroundImage = NormalBrush;
	InSlateEditableTextBox->SetStyle(&EditableTextBox->WidgetStyle);
}

// Called after the underlying slate widget is constructed
void USettingUserInput::NativeConstruct()
{
	Super::NativeConstruct();

	if (EditableTextBox)
	{
		EditableTextBox->OnTextChanged.AddUniqueDynamic(this, &ThisClass::OnTextChanged);

		SlateEditableTextBoxInternal = FSWCWidgetUtilsLibrary::GetSlateWidget<SEditableTextBox>(EditableTextBox);
		check(SlateEditableTextBoxInternal.IsValid());
	}
}

// Called whenever the text is changed programmatically or interactively by the user
void USettingUserInput::OnTextChanged(const FText& Text)
{
	if (!SettingsWidgetInternal)
	{
		return;
	}

	const FName MewValue(Text.ToString());
	SettingsWidgetInternal->SetSettingUserInput(GetSettingTag(), MewValue);
}

// Is overridden to construct the user input
void USettingUserInput::OnAddSetting(const FSettingsPicker& Setting)
{
	UserInputDataInternal = Setting.UserInput;

	Super::OnAddSetting(Setting);
}

// Set the new custom widget setting data for this widget
void USettingCustomWidget::SetCustomWidgetData(const FSettingsCustomWidget& InCustomWidgetData)
{
	CustomWidgetDataInternal = InCustomWidgetData;
}

// Is overridden to construct the custom widget
void USettingCustomWidget::OnAddSetting(const FSettingsPicker& Setting)
{
	CustomWidgetDataInternal = Setting.CustomWidget;

	Super::OnAddSetting(Setting);
}

// Is overridden to attach the column to the Settings Widget
UPanelSlot* USettingColumn::Attach()
{
	if (ParentSlotInternal)
	{
		// is already attached
		return ParentSlotInternal;
	}

	UHorizontalBox* ContentHorizontalBox = GetSettingsWidgetChecked().GetContentHorizontalBox();
	checkf(ContentHorizontalBox, TEXT("ERROR: [%i] %s:\n'ContentHorizontalBox' is null!"), __LINE__, *FString(__FUNCTION__));
	ParentSlotInternal = ContentHorizontalBox->AddChild(this);
	return ParentSlotInternal;
}

// Called after the underlying slate widget is constructed
void USettingColumn::NativeConstruct()
{
	Super::NativeConstruct();

	if (ScrollBoxWidget)
	{
		SlateScrollBoxInternal = FSWCWidgetUtilsLibrary::GetSlateWidget<SScrollBox>(ScrollBoxWidget);
		check(SlateScrollBoxInternal.IsValid());
	}
}
