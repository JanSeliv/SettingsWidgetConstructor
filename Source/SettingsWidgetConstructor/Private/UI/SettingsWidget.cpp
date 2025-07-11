﻿// Copyright (c) Yevhenii Selivanov

#include "UI/SettingsWidget.h"
//---
#include "Data/SettingsDataAsset.h"
#include "MyUtilsLibraries/SettingsUtilsLibrary.h"
#include "MyUtilsLibraries/SWCWidgetUtilsLibrary.h"
#include "UI/SettingCombobox.h"
#include "UI/SettingSubWidget.h"
//---
#include "DataRegistry.h"
#include "DataRegistryTypes.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/SizeBox.h"
#include "Components/Viewport.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/Texture.h"
#include "GameFramework/GameUserSettings.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(SettingsWidget)

/* ---------------------------------------------------
 *		Public functions
 * --------------------------------------------------- */

// Try to find the setting row
const FSettingsPicker& USettingsWidget::FindSettingRow(FName PotentialTagName) const
{
	if (PotentialTagName.IsNone())
	{
		return FSettingsPicker::Empty;
	}

	const FSettingsPicker* FoundRow = &FSettingsPicker::Empty;

	// Find row by specified substring
	const FString TagSubString(PotentialTagName.ToString());
	for (const TTuple<FName, FSettingsPicker>& RowIt : SettingsTableRowsInternal)
	{
		const FString TagStringIt(RowIt.Key.ToString());
		if (TagStringIt.Contains(TagSubString))
		{
			FoundRow = &RowIt.Value;
			break;
		}
	}

	return *FoundRow;
}

// Returns the found row by specified tag
const FSettingsPicker& USettingsWidget::GetSettingRow(const FSettingTag& SettingTag) const
{
	if (!SettingTag.IsValid())
	{
		return FSettingsPicker::Empty;
	}

	const FSettingsPicker* FoundRow = SettingsTableRowsInternal.Find(SettingTag.GetTagName());
	return FoundRow ? *FoundRow : FSettingsPicker::Empty;
}

// Save all settings into their configs
void USettingsWidget::SaveSettings()
{
	ApplySettings();

	for (const TTuple<FName, FSettingsPicker>& RowIt : SettingsTableRowsInternal)
	{
		if (UObject* ContextObject = RowIt.Value.PrimaryData.GetSettingOwner(this))
		{
			ContextObject->SaveConfig();
		}
	}
}

// Apply all current settings on device
void USettingsWidget::ApplySettings()
{
	UGameUserSettings* GameUserSettings = USettingsUtilsLibrary::GetGameUserSettings();
	if (!GameUserSettings)
	{
		return;
	}

	constexpr bool bCheckForCommandLineOverrides = false;
	GameUserSettings->ApplySettings(bCheckForCommandLineOverrides);
}

// Update settings on UI
void USettingsWidget::UpdateSettingsByTags(const FGameplayTagContainer& SettingsToUpdate, bool bLoadFromConfig/* = false*/)
{
	if (SettingsToUpdate.IsEmpty()
		|| !SettingsToUpdate.IsValidIndex(0))
	{
		return;
	}

	if (SettingsTableRowsInternal.IsEmpty())
	{
		CacheTable();
	}

	for (const TTuple<FName, FSettingsPicker>& RowIt : SettingsTableRowsInternal)
	{
		const FSettingsPicker& Setting = RowIt.Value;
		const FSettingTag& SettingTag = Setting.PrimaryData.Tag;
		if (!SettingTag.IsValid()
			|| !SettingTag.MatchesAny(SettingsToUpdate))
		{
			continue;
		}

		FSettingsDataBase* ChosenData = Setting.GetChosenSettingsData();
		if (!ChosenData
			|| !ChosenData->CanUpdateSetting())
		{
			continue;
		}

		UObject* Owner = Setting.PrimaryData.GetSettingOwner(this);
		if (!Owner)
		{
			continue;
		}

		if (bLoadFromConfig)
		{
			// Obtain the latest value from configs and set it
			Owner->LoadConfig();
		}

		FString Result;
		ChosenData->GetSettingValue(*this, SettingTag, /*Out*/Result);
		ChosenData->SetSettingValue(*this, SettingTag, Result);
	}
}

// Update all existing settings on UI
void USettingsWidget::UpdateAllSettings(bool bLoadFromConfig)
{
	FGameplayTagContainer AllSettingTags;
	for (const TTuple<FName, FSettingsPicker>& RowIt : SettingsTableRowsInternal)
	{
		AllSettingTags.AddTagFast(RowIt.Value.PrimaryData.Tag);
	}
	UpdateSettingsByTags(AllSettingTags, bLoadFromConfig);
}

// Returns the name of found tag by specified function
const FSettingTag& USettingsWidget::GetTagByFunction(const FSettingFunctionPicker& SettingFunction) const
{
	for (const TTuple<FName, FSettingsPicker>& RowIt : SettingsTableRowsInternal)
	{
		const FSettingsPrimary& PrimaryData = RowIt.Value.PrimaryData;
		if (PrimaryData.Getter == SettingFunction
			|| PrimaryData.Setter == SettingFunction)
		{
			return PrimaryData.Tag;
		}
	}

	return FSettingTag::EmptySettingTag;
}

/* ---------------------------------------------------
 *		Setters by setting types
 * --------------------------------------------------- */

// Set value to the option by tag
void USettingsWidget::SetSettingValue(FName TagName, const FString& Value)
{
	const FSettingsPicker& FoundRow = FindSettingRow(TagName);
	if (!FoundRow.IsValid())
	{
		return;
	}

	FSettingsDataBase* ChosenData = FoundRow.GetChosenSettingsData();
	if (!ChosenData)
	{
		return;
	}

	const FSettingTag& Tag = FoundRow.PrimaryData.Tag;
	if (Tag.IsValid())
	{
		ChosenData->SetSettingValue(*this, Tag, Value);
	}
}

/** Executes the common pattern of setting a value, executing if bound, and updating the settings.
 * @param Tag The tag used to find the setting row.
 * @param DataMember The member that holds the desired value.
 * @param MemberValue The specific member to set the value to.
 * @param Value The new value to set.
 * @param SetterExpression The expression to update the setter delegate. */
#define SET_SETTING_VALUE(Tag, DataMember, MemberValue, Value, SetterExpression)	\
	do {																			\
		if (!Tag.IsValid())															\
		{																			\
			return;																	\
		}																			\
		FSettingsPicker* FoundRowPtr = SettingsTableRowsInternal.Find(Tag.GetTagName());\
		if (!FoundRowPtr)															\
		{																			\
			return;																	\
		}																			\
		auto& Data = FoundRowPtr->DataMember;										\
		if (Data.MemberValue == Value)												\
		{																			\
			return;																	\
		}																			\
		Data.MemberValue = Value;													\
		Data.SetterExpression.ExecuteIfBound(Value);								\
		UpdateSettingsByTags(FoundRowPtr->PrimaryData.SettingsToUpdate);			\
	} while (0)

// Press button
void USettingsWidget::SetSettingButtonPressed(const FSettingTag& ButtonTag)
{
	if (!ButtonTag.IsValid())
	{
		return;
	}

	const FSettingsPicker* SettingsRowPtr = SettingsTableRowsInternal.Find(ButtonTag.GetTagName());
	if (!SettingsRowPtr)
	{
		return;
	}

	SettingsRowPtr->Button.OnButtonPressed.ExecuteIfBound();

	UpdateSettingsByTags(SettingsRowPtr->PrimaryData.SettingsToUpdate);

	OnAnySettingSet(SettingsRowPtr->PrimaryData);

	PlayUIClickSFX();
}

// Toggle checkbox
void USettingsWidget::SetSettingCheckbox(const FSettingTag& CheckboxTag, bool InValue)
{
	SET_SETTING_VALUE(CheckboxTag, Checkbox, bIsSet, InValue, OnSetterBool);

	if (USettingCheckbox* SettingCheckbox = GetSettingSubWidget<USettingCheckbox>(CheckboxTag))
	{
		SettingCheckbox->SetCheckboxValue(InValue);
		OnAnySettingSet(SettingCheckbox->GetSettingPrimaryRow());
	}

	PlayUIClickSFX();
}

// Set chosen member index for a combobox
void USettingsWidget::SetSettingComboboxIndex(const FSettingTag& ComboboxTag, int32 InValue)
{
	if (InValue == INDEX_NONE)
	{
		return;
	}

	SET_SETTING_VALUE(ComboboxTag, Combobox, ChosenMemberIndex, InValue, OnSetterInt);

	if (USettingCombobox* SettingCombobox = GetSettingSubWidget<USettingCombobox>(ComboboxTag))
	{
		SettingCombobox->SetComboboxIndex(InValue);
		OnAnySettingSet(SettingCombobox->GetSettingPrimaryRow());
	}
}

// Set current value for a slider
void USettingsWidget::SetSettingSlider(const FSettingTag& SliderTag, double InValue)
{
	static constexpr double MinValue = 0.0;
	static constexpr float MaxValue = 1.0;
	const double NewValue = FMath::Clamp(InValue, MinValue, MaxValue);
	SET_SETTING_VALUE(SliderTag, Slider, ChosenValue, NewValue, OnSetterFloat);

	if (USettingSlider* SettingSlider = GetSettingSubWidget<USettingSlider>(SliderTag))
	{
		SettingSlider->SetSliderValue(NewValue);
		OnAnySettingSet(SettingSlider->GetSettingPrimaryRow());
	}
}

// Set new text
void USettingsWidget::SetSettingTextLine(const FSettingTag& TextLineTag, const FText& InValue)
{
	if (!TextLineTag.IsValid())
	{
		return;
	}

	FSettingsPicker* SettingsRowPtr = SettingsTableRowsInternal.Find(TextLineTag.GetTagName());
	if (!SettingsRowPtr)
	{
		return;
	}

	FSettingsPrimary& PrimaryRef = SettingsRowPtr->PrimaryData;
	FText& CaptionRef = PrimaryRef.Caption;
	if (CaptionRef.EqualTo(InValue))
	{
		return;
	}

	CaptionRef = InValue;
	SettingsRowPtr->TextLine.OnSetterText.ExecuteIfBound(InValue);
	UpdateSettingsByTags(PrimaryRef.SettingsToUpdate);

	if (USettingTextLine* SettingTextLine = Cast<USettingTextLine>(PrimaryRef.SettingSubWidget))
	{
		SettingTextLine->SetCaptionText(InValue);
	}
}

// Set new text for an input box
void USettingsWidget::SetSettingUserInput(const FSettingTag& UserInputTag, FName InValue)
{
	if (!UserInputTag.IsValid())
	{
		return;
	}

	FSettingsPicker* SettingsRowPtr = SettingsTableRowsInternal.Find(UserInputTag.GetTagName());
	if (!SettingsRowPtr)
	{
		return;
	}

	FSettingsUserInput& UserInputRef = SettingsRowPtr->UserInput;
	if (UserInputRef.UserInput.IsEqual(InValue)
		|| InValue.IsNone())
	{
		// Is not needed to update
		return;
	}

	if (UserInputRef.MaxCharactersNumber > 0)
	{
		// Limit the length of the string
		const FString NewValueStr = InValue.ToString().Left(UserInputRef.MaxCharactersNumber);
		InValue = *NewValueStr;

		if (USettingUserInput* SettingUserInput = GetSettingSubWidget<USettingUserInput>(UserInputTag))
		{
			SettingUserInput->SetUserInputValue(InValue);
			OnAnySettingSet(SettingUserInput->GetSettingPrimaryRow());
		}
	}

	UserInputRef.UserInput = InValue;
	UserInputRef.OnSetterName.ExecuteIfBound(InValue);
	UpdateSettingsByTags(SettingsRowPtr->PrimaryData.SettingsToUpdate);

	PlayUIClickSFX();
}

// Set new custom widget for setting by specified tag
void USettingsWidget::SetSettingCustomWidget(const FSettingTag& CustomWidgetTag, USettingCustomWidget* InCustomWidget)
{
	if (!CustomWidgetTag.IsValid())
	{
		return;
	}

	FSettingsPicker* SettingsRowPtr = SettingsTableRowsInternal.Find(CustomWidgetTag.GetTagName());
	if (!SettingsRowPtr)
	{
		return;
	}

	TWeakObjectPtr<USettingSubWidget>& CustomWidgetRef = SettingsRowPtr->PrimaryData.SettingSubWidget;
	if (CustomWidgetRef == InCustomWidget)
	{
		return;
	}

	CustomWidgetRef.Reset();
	CustomWidgetRef = InCustomWidget;
	SettingsRowPtr->CustomWidget.OnSetterWidget.ExecuteIfBound(InCustomWidget);
	UpdateSettingsByTags(SettingsRowPtr->PrimaryData.SettingsToUpdate);

	OnAnySettingSet(SettingsRowPtr->PrimaryData);
}

// Is called after any setting is changed
void USettingsWidget::OnAnySettingSet_Implementation(const FSettingsPrimary& SettingPrimaryRow)
{
	if (SettingPrimaryRow.bApplyImmediately)
	{
		ApplySettings();
	}
}

/* ---------------------------------------------------
 *		Getters by setting types
 * --------------------------------------------------- */

/** Retrieve a specific setting row using a given tag.
 * @param Tag The tag used to find the setting row.
 * @param DataMember The member that holds the desired value. */
#define GET_SETTING_ROW(Tag, DataMember)					\
	const FSettingsPicker& FoundRow = GetSettingRow(Tag);	\
	if (!FoundRow.IsValid()) { return; }					\
	const auto& Data = FoundRow.DataMember;

/** Executes the common pattern of getting a value from a data structure.
 * @param Tag The tag used to find the setting row.
 * @param DataMember The member that holds the desired value.
 * @param ValueType The type of value to retrieve.
 * @param ValueExpression The expression to retrieve the value.
 * @param GetterExpression The expression to retrieve the getter delegate.
 * @param DefaultValue The default value to return if no value is found. */
#define GET_SETTING_VALUE(Tag, DataMember, ValueType, ValueExpression, GetterExpression, DefaultValue) \
	{															\
		const FSettingsPicker& FoundRow = GetSettingRow(Tag);	\
		ValueType Value = DefaultValue;							\
		if (FoundRow.IsValid())									\
		{														\
			const auto& Data = FoundRow.DataMember;				\
			Value = ValueExpression;							\
			const auto& Getter = GetterExpression;				\
			if (Getter.IsBound())								\
			{													\
				Value = Getter.Execute();						\
			}													\
		}														\
		return Value; \
	}

// Returns is a checkbox toggled
bool USettingsWidget::GetCheckboxValue(const FSettingTag& CheckboxTag) const
{
	GET_SETTING_VALUE(CheckboxTag, Checkbox, bool, Data.bIsSet, Data.OnGetterBool, false);
}

// Returns chosen member index of a combobox
int32 USettingsWidget::GetComboboxIndex(const FSettingTag& ComboboxTag) const
{
	GET_SETTING_VALUE(ComboboxTag, Combobox, int32, Data.ChosenMemberIndex, Data.OnGetterInt, 0);
}

// Get all members of a combobox
void USettingsWidget::GetComboboxMembers(const FSettingTag& ComboboxTag, TArray<FText>& OutMembers) const
{
	GET_SETTING_ROW(ComboboxTag, Combobox)
	OutMembers = Data.Members;
	Data.OnGetMembers.ExecuteIfBound(OutMembers);
}

// Get current value of a slider [0...1]
double USettingsWidget::GetSliderValue(const FSettingTag& SliderTag) const
{
	GET_SETTING_VALUE(SliderTag, Slider, double, Data.ChosenValue, Data.OnGetterFloat, 0.f);
}

// Get current text of a simple text widget
void USettingsWidget::GetTextLineValue(const FSettingTag& TextLineTag, FText& OutText) const
{
	GET_SETTING_ROW(TextLineTag, PrimaryData)
	OutText = Data.Caption;
	FoundRow.TextLine.OnGetterText.ExecuteIfBound(OutText);
}

// Get current input name of the text input
FName USettingsWidget::GetUserInputValue(const FSettingTag& UserInputTag) const
{
	GET_SETTING_VALUE(UserInputTag, UserInput, FName, Data.UserInput, Data.OnGetterName, NAME_None);
}

// Get custom widget of the setting by specified tag
USettingCustomWidget* USettingsWidget::GetCustomWidget(const FSettingTag& CustomWidgetTag) const
{
	GET_SETTING_VALUE(CustomWidgetTag, CustomWidget, USettingCustomWidget*, Cast<USettingCustomWidget>(FoundRow.PrimaryData.SettingSubWidget.Get()), Data.OnGetterWidget, nullptr);
}

// Get setting widget object by specified tag
USettingSubWidget* USettingsWidget::GetSettingSubWidget(const FSettingTag& SettingTag) const
{
	const FSettingsPrimary& PrimaryData = GetSettingRow(SettingTag).PrimaryData;
	return PrimaryData.IsValid() ? PrimaryData.SettingSubWidget.Get() : nullptr;
}

/* ---------------------------------------------------
 *		Style
 * --------------------------------------------------- */

// Returns the size of the Settings widget on the screen
FVector2D USettingsWidget::GetSettingsSize() const
{
	const FVector2D PercentSize = USettingsDataAsset::Get().GetSettingsPercentSize();

	UObject* WorldContextObject = GetOwningPlayer();
	const FVector2D ViewportSize = UWidgetLayoutLibrary::GetViewportSize(WorldContextObject);
	const float ViewportScale = UWidgetLayoutLibrary::GetViewportScale(WorldContextObject);

	const FVector2D NewViewportSize = ViewportSize * PercentSize;
	return ViewportScale ? NewViewportSize / ViewportScale : NewViewportSize;
}

// Returns the size of specified category on the screen
FVector2D USettingsWidget::GetSubWidgetsSize(int32 SectionsBitmask) const
{
	if (!SectionsBitmask)
	{
		return FVector2D::ZeroVector;
	}

	TArray<const UWidget*> DesiredWidgets;

	constexpr int32 HeaderAlignment = static_cast<int32>(EMyVerticalAlignment::Header);
	if (HeaderAlignment & SectionsBitmask)
	{
		DesiredWidgets.Emplace(HeaderVerticalBox);
	}

	constexpr int32 ContentAlignment = static_cast<int32>(EMyVerticalAlignment::Content);
	if (ContentAlignment & SectionsBitmask)
	{
		DesiredWidgets.Emplace(ContentHorizontalBox);
	}

	constexpr int32 FooterAlignment = static_cast<int32>(EMyVerticalAlignment::Footer);
	if (FooterAlignment & SectionsBitmask)
	{
		DesiredWidgets.Emplace(FooterVerticalBox);
	}

	FVector2D SubWidgetsHeight = FVector2D::ZeroVector;
	for (const UWidget* DesiredWidgetIt : DesiredWidgets)
	{
		if (DesiredWidgetIt)
		{
			const FVector2D SubWidgetHeight = DesiredWidgetIt->GetDesiredSize();
			ensureAlwaysMsgf(!SubWidgetHeight.IsZero(), TEXT("ASSERT: 'SubWidgetHeight' is zero, can't get the size of subwidget, most likely widget is not initialized yet, call ForceLayoutPrepass()"));
			SubWidgetsHeight += SubWidgetHeight;
		}
	}
	return SubWidgetsHeight;
}

// Returns the height of a setting scrollbox on the screen
float USettingsWidget::GetScrollBoxHeight() const
{
	const USettingsDataAsset& SettingsData = USettingsDataAsset::Get();

	// The widget size
	const FVector2D SettingsSize = GetSettingsSize();

	// Margin size
	constexpr int32 Margins = static_cast<int32>(EMyVerticalAlignment::Margins);
	const FVector2D MarginsSize = GetSubWidgetsSize(Margins);

	// Additional padding sizes
	float Paddings = 0.f;
	const FMargin SettingsPadding = SettingsData.GetSettingsPadding();
	Paddings += SettingsPadding.Top + SettingsPadding.Bottom;
	const FMargin ScrollBoxPadding = SettingsData.GetColumnPadding();
	Paddings += ScrollBoxPadding.Top + ScrollBoxPadding.Bottom;

	const float ScrollBoxHeight = (SettingsSize - MarginsSize).Y - Paddings;

	// Scale scrollbox
	const float PercentSize = FMath::Clamp(SettingsData.GetScrollboxPercentHeight(), 0.f, 1.f);
	const float ScaledHeight = ScrollBoxHeight * PercentSize;

	return ScaledHeight;
}

// Is blueprint-event called that returns the style brush by specified button state
FSlateBrush USettingsWidget::GetButtonBrush(ESettingsButtonState State)
{
	const USettingsDataAsset& SettingsDataAsset = USettingsDataAsset::Get();
	const FMiscThemeData& MiscThemeData = SettingsDataAsset.GetMiscThemeData();
	const FButtonThemeData& ButtonThemeData = SettingsDataAsset.GetButtonThemeData();

	FSlateColor SlateColor;
	switch (State)
	{
	case ESettingsButtonState::Normal:
		SlateColor = MiscThemeData.ThemeColorNormal;
		break;
	case ESettingsButtonState::Hovered:
		SlateColor = MiscThemeData.ThemeColorHover;
		break;
	case ESettingsButtonState::Pressed:
		SlateColor = MiscThemeData.ThemeColorExtra;
		break;
	default:
		SlateColor = FLinearColor::White;
	}

	FSlateBrush SlateBrush;
	SlateBrush.TintColor = SlateColor;
	SlateBrush.DrawAs = ButtonThemeData.DrawAs;
	SlateBrush.Margin = ButtonThemeData.Margin;
	SlateBrush.SetImageSize(ButtonThemeData.Size);
	SlateBrush.SetResourceObject(ButtonThemeData.Texture);

	return SlateBrush;
}

/* ---------------------------------------------------
 *		Protected functions
 * --------------------------------------------------- */

// Called after the underlying slate widget is constructed
void USettingsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (USettingsDataAsset::Get().IsAutoConstruct())
	{
		TryConstructSettings();
	}

	BindOnSettingsDataRegistryChanged();
}

// Called when the widget is removed from the viewport
void USettingsWidget::NativeDestruct()
{
	Super::NativeDestruct();

	RemoveAllSettings();
}

// Is called right after the game was started and windows size is set to construct settings
void USettingsWidget::OnViewportResizedWhenInit(FViewport* Viewport, uint32 Index)
{
	if (FViewport::ViewportResizedEvent.IsBoundToObject(this))
	{
		FViewport::ViewportResizedEvent.RemoveAll(this);
	}

	ConstructSettings();
}

// Construct all settings from the settings data table
void USettingsWidget::ConstructSettings()
{
	if (IsSettingsWidgetConstructed())
	{
		// Settings are already constructed
		return;
	}

	CacheTable();

	// BP implementation to cache some data before creating subwidgets
	OnConstructSettings();

	FGameplayTagContainer AddedSettings;
	for (TTuple<FName, FSettingsPicker>& RowIt : SettingsTableRowsInternal)
	{
		FSettingsPicker& SettingRef = RowIt.Value;
		BindSetting(SettingRef);
		AddSetting(SettingRef);
		AddedSettings.AddTag(SettingRef.PrimaryData.Tag);
	}

	UpdateSettingsByTags(AddedSettings, /*bLoadFromConfig*/true);

	UpdateScrollBoxesHeight();

	ApplySettings();
}

// Internal function to cache setting rows from Settings Data Table
void USettingsWidget::CacheTable()
{
	TMap<FName, FSettingsPicker> SettingRows;
	USettingsUtilsLibrary::GenerateAllSettingRows(/*Out*/SettingRows);
	if (!ensureMsgf(!SettingRows.IsEmpty(), TEXT("ASSERT: 'SettingRows' are empty")))
	{
		return;
	}

	// Reset values if currently are set
	SettingsTableRowsInternal.Empty();
	SettingsTableRowsInternal.Reserve(SettingRows.Num());
	for (const TTuple<FName, FSettingsPicker>& SettingRowIt : SettingRows)
	{
		const FSettingsPicker& SettingsPicker = SettingRowIt.Value;
		SettingsTableRowsInternal.Emplace(SettingRowIt.Key, SettingsPicker);
	}
}

// Clears all added settings
void USettingsWidget::RemoveAllSettings()
{
	for (TTuple<FName, FSettingsPicker>& RowIt : SettingsTableRowsInternal)
	{
		USettingSubWidget* SubWidget = RowIt.Value.PrimaryData.SettingSubWidget.Get();
		if (ensureMsgf(SubWidget, TEXT("ASSERT: [%i] %s:\n'SubWidget' is not valid!"), __LINE__, *FString(__FUNCTION__)))
		{
			FSWCWidgetUtilsLibrary::DestroyWidget(*SubWidget);
		}
	}
	SettingsTableRowsInternal.Empty();

	for (USettingColumn* ColumnIt : ColumnsInternal)
	{
		if (ensureMsgf(ColumnIt, TEXT("ASSERT: [%i] %s:\n'ColumnIt' is not valid!"), __LINE__, *FString(__FUNCTION__)))
		{
			FSWCWidgetUtilsLibrary::DestroyWidget(*ColumnIt);
		}
	}
	ColumnsInternal.Empty();
}

// Is called when In-Game menu became opened or closed
void USettingsWidget::OnToggleSettings(bool bIsVisible)
{
	PlayUIClickSFX();

	if (OnToggledSettings.IsBound())
	{
		OnToggledSettings.Broadcast(bIsVisible);
	}
}

// Bind and set static object delegate
bool USettingsWidget::TryBindOwner(FSettingsPrimary& Primary)
{
	const UObject* FoundContextObj = nullptr;
	const FSettingFunctionPicker& Owner = Primary.Owner;
	if (Owner.IsValid())
	{
		Primary.OwnerFunc.BindUFunction(Owner.FunctionClass->GetDefaultObject(), Owner.FunctionName);
		FoundContextObj = Primary.GetSettingOwner(this);
	}

	if (!FoundContextObj)
	{
		if (Owner.IsValid())
		{
			// Static context function is set, but returning object is null,
			// most likely such object is not initialized yet,
			// defer binding to try to rebind it later
			DeferredBindingsInternal.AddTag(Primary.Tag);
		}

		return false;
	}

	const UClass* ContextClass = FoundContextObj->GetClass();
	checkf(ContextClass, TEXT("ERROR: [%i] %s:\n'ContextClass' is null!"), __LINE__, *FString(__FUNCTION__));

	// Cache all functions that are contained in returned object
	for (TFieldIterator<UFunction> It(ContextClass, EFieldIteratorFlags::IncludeSuper); It; ++It)
	{
		const UFunction* FunctionIt = *It;
		if (!FunctionIt)
		{
			continue;
		}

		const FName FunctionNameIt = FunctionIt->GetFName();
		if (!FunctionNameIt.IsNone())
		{
			Primary.OwnerFunctionList.Emplace(FunctionNameIt);
		}
	}

	return true;
}

// Creates new widget based on specified setting class and sets it to specified primary data
USettingSubWidget* USettingsWidget::CreateSettingSubWidget(FSettingsPrimary& InOutPrimary, const TSubclassOf<USettingSubWidget> SettingSubWidgetClass)
{
	if (!SettingSubWidgetClass)
	{
		return nullptr;
	}

	USettingSubWidget* SettingSubWidget = CreateWidget<USettingSubWidget>(this, SettingSubWidgetClass);
	InOutPrimary.SettingSubWidget = SettingSubWidget;
	SettingSubWidget->SetSettingsWidget(this);
	SettingSubWidget->SetSettingPrimaryRow(InOutPrimary);
	SettingSubWidget->SetLineHeight(InOutPrimary.LineHeight);
	SettingSubWidget->SetCaptionText(InOutPrimary.Caption);

	return SettingSubWidget;
}

// Automatically sets the height for all scrollboxes in the Settings
void USettingsWidget::UpdateScrollBoxesHeight()
{
	ForceLayoutPrepass(); // Call it to make GetSettingsHeight work since it is called during widget construction
	const float ScrollBoxHeight = GetScrollBoxHeight();

	for (const USettingColumn* ColumnIt : ColumnsInternal)
	{
		USizeBox* SizeBoxWidget = ColumnIt ? ColumnIt->GetSizeBoxWidget() : nullptr;
		if (SizeBoxWidget)
		{
			SizeBoxWidget->SetMaxDesiredHeight(ScrollBoxHeight);
		}
	}
}

// Constructs settings if viewport is ready otherwise Wait until viewport become initialized
void USettingsWidget::TryConstructSettings()
{
	auto IsViewportInitialized = []()-> bool
	{
		UGameViewportClient* GameViewport = GEngine ? GEngine->GameViewport : nullptr;
		FViewport* Viewport = GameViewport ? GameViewport->Viewport : nullptr;
		if (!Viewport)
		{
			return false;
		}

		auto IsZeroViewportSize = [Viewport] { return Viewport->GetSizeXY() == FIntPoint::ZeroValue; };

		if (IsZeroViewportSize())
		{
			// Try update its value by mouse enter event
			GameViewport->MouseEnter(Viewport, FIntPoint::ZeroValue.X, FIntPoint::ZeroValue.Y);
			return !IsZeroViewportSize();
		}

		return true;
	};

	if (IsViewportInitialized())
	{
		ConstructSettings();
	}
	else if (!FViewport::ViewportResizedEvent.IsBoundToObject(this))
	{
		FViewport::ViewportResizedEvent.AddUObject(this, &ThisClass::OnViewportResizedWhenInit);
	}
}

// Display settings on UI
void USettingsWidget::OpenSettings()
{
	if (IsVisible())
	{
		// Is already shown
		return;
	}

	TryConstructSettings();

	TryRebindDeferredContexts();

	UpdateAllSettings();

	SetVisibility(ESlateVisibility::Visible);

	OnToggleSettings(true);

	TryFocusOnUI();

	OnOpenSettings();
}

// Save and close the settings widget
void USettingsWidget::CloseSettings()
{
	if (!IsVisible()
		&& !IsHovered())
	{
		// Widget is already closed
		return;
	}

	SetVisibility(ESlateVisibility::Collapsed);

	SaveSettings();

	OnToggleSettings(false);

	OnCloseSettings();
}

// Flip-flop opens and closes the Settings menu
void USettingsWidget::ToggleSettings()
{
	if (IsVisible())
	{
		CloseSettings();
	}
	else
	{
		OpenSettings();
	}
}

// Is called on opening to focus the widget on UI if allowed
void USettingsWidget::TryFocusOnUI()
{
	if (!USettingsDataAsset::Get().IsAutoFocusOnOpen())
	{
		return;
	}

	APlayerController* PlayerController = GetOwningPlayer();
	if (!ensureMsgf(PlayerController, TEXT("ASSERT: [%i] %s:\n'PlayerController' is not valid!"), __LINE__, *FString(__FUNCTION__)))
	{
		return;
	}

	static const FInputModeGameAndUI GameAndUI{};
	PlayerController->SetInputMode(GameAndUI);
	PlayerController->SetShowMouseCursor(true);
	PlayerController->bEnableClickEvents = true;
	PlayerController->bEnableMouseOverEvents = true;
}

/* ---------------------------------------------------
 *		Bind by setting types
 * --------------------------------------------------- */

// Bind setting to specified Get/Set delegates, so both methods will be called
bool USettingsWidget::BindSetting(FSettingsPicker& Setting)
{
	FSettingsDataBase* ChosenData = Setting.GetChosenSettingsData();
	if (!ChosenData)
	{
		return false;
	}

	if (TryBindOwner(Setting.PrimaryData))
	{
		ChosenData->BindSetting(*this, Setting.PrimaryData);
		return true;
	}

	return false;
}

/**
 * Macro to create and bind a UI widget.
 * @param Primary				Primary settings for the widget
 * @param Data					Data structure containing widget properties
 * @param GetterFunction		The getter function to bind
 * @param SetterFunction		The setter function to bind
 * @param AdditionalFunctionCalls Any additional function calls needed for specific widgets
 */
#define BIND_SETTING(Primary, Data, GetterFunction, SetterFunction)					\
	do																				\
	{																				\
		if (UObject* OwnerObject = Primary.GetSettingOwner(this))					\
		{																			\
			const FName GetterFunctionName = Primary.Getter.FunctionName;			\
			if (Primary.OwnerFunctionList.Contains(GetterFunctionName))				\
			{																		\
				Data.GetterFunction.BindUFunction(OwnerObject, GetterFunctionName);	\
			}																		\
			const FName SetterFunctionName = Primary.Setter.FunctionName;			\
			if (Primary.OwnerFunctionList.Contains(SetterFunctionName))				\
			{																		\
				Data.SetterFunction.BindUFunction(OwnerObject, SetterFunctionName);	\
			}																		\
		}																			\
	} while (0)

// Bind button to own Get/Set delegates
void USettingsWidget::BindButton(const FSettingsPrimary& Primary, FSettingsButton& Data)
{
	BIND_SETTING(Primary, Data, OnButtonPressed, OnButtonPressed);
}

// Bind checkbox to own Get/Set delegates
void USettingsWidget::BindCheckbox(const FSettingsPrimary& Primary, FSettingsCheckbox& Data)
{
	BIND_SETTING(Primary, Data, OnGetterBool, OnSetterBool);
}

// Bind combobox to own Get/Set delegates
void USettingsWidget::BindCombobox(const FSettingsPrimary& Primary, FSettingsCombobox& Data)
{
	BIND_SETTING(Primary, Data, OnGetterInt, OnSetterInt);

	if (UObject* OwnerObject = Primary.GetSettingOwner(this))
	{
		const FName GetMembersFunctionName = Data.GetMembers.FunctionName;
		if (Primary.OwnerFunctionList.Contains(GetMembersFunctionName))
		{
			Data.OnGetMembers.BindUFunction(OwnerObject, GetMembersFunctionName);
			Data.OnGetMembers.ExecuteIfBound(Data.Members);
		}

		const FName SetMembersFunctionName = Data.SetMembers.FunctionName;
		if (Primary.OwnerFunctionList.Contains(SetMembersFunctionName))
		{
			Data.OnSetMembers.BindUFunction(OwnerObject, SetMembersFunctionName);
			Data.OnSetMembers.ExecuteIfBound(Data.Members);
		}
	}
}

// Bind slider to own Get/Set delegates
void USettingsWidget::BindSlider(const FSettingsPrimary& Primary, FSettingsSlider& Data)
{
	BIND_SETTING(Primary, Data, OnGetterFloat, OnSetterFloat);
}

// Bind simple text to own Get/Set delegates
void USettingsWidget::BindTextLine(const FSettingsPrimary& Primary, FSettingsTextLine& Data)
{
	BIND_SETTING(Primary, Data, OnGetterText, OnSetterText);
}

// Bind text input to own Get/Set delegates
void USettingsWidget::BindUserInput(const FSettingsPrimary& Primary, FSettingsUserInput& Data)
{
	BIND_SETTING(Primary, Data, OnGetterName, OnSetterName);
}

// Bind custom widget to own Get/Set delegates
void USettingsWidget::BindCustomWidget(const FSettingsPrimary& Primary, FSettingsCustomWidget& Data)
{
	BIND_SETTING(Primary, Data, OnGetterWidget, OnSetterWidget);
}

// Attempts to rebind those Settings that failed to bind their Getter/Setter functions on initial construct
void USettingsWidget::TryRebindDeferredContexts()
{
	if (DeferredBindingsInternal.IsEmpty())
	{
		// Nothing to rebind, we are done
		return;
	}

	FGameplayTagContainer ReboundSettings;
	for (const FGameplayTag& TagIt : DeferredBindingsInternal)
	{
		FSettingsPicker* FoundRowPtr = TagIt.IsValid() ? SettingsTableRowsInternal.Find(TagIt.GetTagName()) : nullptr;
		if (FoundRowPtr
			&& BindSetting(*FoundRowPtr))
		{
			ReboundSettings.AddTagFast(TagIt);
		}
	}

	if (!ReboundSettings.IsEmpty())
	{
		// Some settings were successfully rebound, remove them from the deferred list and update them
		DeferredBindingsInternal.RemoveTags(ReboundSettings);
		UpdateSettingsByTags(ReboundSettings, /*bLoadFromConfig*/true);
	}
}

// Add setting on UI.
void USettingsWidget::AddSetting(FSettingsPicker& Setting)
{
	const FSettingsDataBase* ChosenData = Setting.GetChosenSettingsData();
	if (!ChosenData)
	{
		return;
	}

	FSettingsPrimary& PrimaryData = Setting.PrimaryData;

	if (Setting.PrimaryData.bStartOnNextColumn)
	{
		AddColumn(GetColumnIndexBySetting(PrimaryData.Tag));
	}

	USettingSubWidget* SettingSubWidget = CreateSettingSubWidget(PrimaryData, ChosenData->GetSubWidgetClass());
	checkf(SettingSubWidget, TEXT("ERROR: [%i] %s:\n'SettingSubWidget' is null!"), __LINE__, *FString(__FUNCTION__));
	SettingSubWidget->OnAddSetting(Setting);
}

/*********************************************************************************************
 * Columns builder
 ********************************************************************************************* */

// Returns the index of column for a Setting by specified tag or -1 if not found
int32 USettingsWidget::GetColumnIndexBySetting(const FSettingTag& SettingTag) const
{
	int32 ColumnIndex = 0;
	for (const TTuple<FName, FSettingsPicker>& RowIt : SettingsTableRowsInternal)
	{
		const FSettingsPrimary& PrimaryData = RowIt.Value.PrimaryData;
		if (PrimaryData.bStartOnNextColumn)
		{
			++ColumnIndex;
		}

		if (PrimaryData.Tag == SettingTag)
		{
			return ColumnIndex;
		}
	}

	return INDEX_NONE;
}

// Creates new column on specified index
void USettingsWidget::AddColumn(int32 ColumnIndex)
{
	USettingColumn* NewColumn = CreateWidget<USettingColumn>(this, USettingsDataAsset::Get().GetColumnClass());
	NewColumn->SetSettingsWidget(this);
	ColumnIndex = FMath::Clamp(ColumnIndex, 0, ColumnsInternal.Num());
	ColumnsInternal.Insert(NewColumn, ColumnIndex);
	NewColumn->OnAddSetting(FSettingsPicker());
}

/*********************************************************************************************
 * Multiple Data Tables support
 ********************************************************************************************* */

// Is called when the Settings Data Registry is changed
void USettingsWidget::OnSettingsDataRegistryChanged_Implementation(class UDataRegistry* SettingsDataRegistry)
{
	const UWorld* World = GetWorld();
	const APlayerController* PC = GetOwningPlayer();
	if (!World || World->bIsTearingDown
		|| !PC
		|| !IsInViewport()
		|| !SettingsDataRegistry || SettingsDataRegistry->GetLowestAvailability() == EDataRegistryAvailability::DoesNotExist)
	{
		// The game was ended or no data registry is set
		return;
	}

	// Perfectly, we should insert new settings here,
	// But inserting anything in between to scrollbox is not supported by UE at all
	// So, clear all settings first
	RemoveAllSettings();
	ConstructSettings();
}

void USettingsWidget::BindOnSettingsDataRegistryChanged()
{
	UDataRegistry* SettingsDataRegistry = USettingsDataAsset::Get().GetSettingsDataRegistry();
	checkf(SettingsDataRegistry, TEXT("ERROR: [%i] %s:\n'SettingsDataRegistry' is not set in Project Settings!"), __LINE__, *FString(__FUNCTION__));
	FDataRegistryCacheVersionCallback& SettingsDataRegistryDelegate = SettingsDataRegistry->OnCacheVersionInvalidated();
	if (!SettingsDataRegistryDelegate.IsBoundToObject(this))
	{
		SettingsDataRegistryDelegate.AddUObject(this, &ThisClass::OnSettingsDataRegistryChanged);
	}
}
