// Copyright (c) Yevhenii Selivanov

#include "UI/SettingsWidget.h"
//---
#include "Data/SettingsDataAsset.h"
#include "MyUtilsLibraries/SettingsUtilsLibrary.h"
#include "UI/SettingSubWidget.h"
//---
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/SizeBox.h"
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
void USettingsWidget::UpdateSettings(const FGameplayTagContainer& SettingsToUpdate)
{
	if (SettingsToUpdate.IsEmpty()
		|| !SettingsToUpdate.IsValidIndex(0))
	{
		return;
	}

	if (SettingsTableRowsInternal.IsEmpty())
	{
		UpdateSettingsTableRows();
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
		if (!ChosenData)
		{
			continue;
		}

		UObject* Owner = Setting.PrimaryData.GetSettingOwner(this);
		if (!Owner)
		{
			continue;
		}

		// Obtain the latest value from configs and set it
		Owner->LoadConfig();

		FString Result;
		ChosenData->GetSettingValue(*this, SettingTag, /*Out*/Result);
		ChosenData->SetSettingValue(*this, SettingTag, Result);
	}
}

// Returns the name of found tag by specified function
const FSettingTag& USettingsWidget::GetTagByFunction(const FSettingFunctionPicker& FunctionPicker) const
{
	for (const TTuple<FName, FSettingsPicker>& RowIt : SettingsTableRowsInternal)
	{
		const FSettingsPrimary& PrimaryData = RowIt.Value.PrimaryData;
		if (PrimaryData.Getter == FunctionPicker
			|| PrimaryData.Setter == FunctionPicker)
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
		UpdateSettings(FoundRowPtr->PrimaryData.SettingsToUpdate);					\
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

	UpdateSettings(SettingsRowPtr->PrimaryData.SettingsToUpdate);

	PlayUIClickSFX();
}

// Toggle checkbox
void USettingsWidget::SetSettingCheckbox(const FSettingTag& CheckboxTag, bool InValue)
{
	SET_SETTING_VALUE(CheckboxTag, Checkbox, bIsSet, InValue, OnSetterBool);

	// BP implementation
	SetCheckbox(CheckboxTag, InValue);
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

	// BP implementation
	SetComboboxIndex(ComboboxTag, InValue);
}

// Set new members for a combobox
void USettingsWidget::SetSettingComboboxMembers(const FSettingTag& ComboboxTag, const TArray<FText>& InValue)
{
	if (!ComboboxTag.IsValid())
	{
		return;
	}

	FSettingsPicker* SettingsRowPtr = SettingsTableRowsInternal.Find(ComboboxTag.GetTagName());
	if (!SettingsRowPtr)
	{
		return;
	}

	SettingsRowPtr->Combobox.Members = InValue;
	SettingsRowPtr->Combobox.OnSetMembers.ExecuteIfBound(InValue);

	// BP implementation
	SetComboboxMembers(ComboboxTag, InValue);
}

// Set current value for a slider
void USettingsWidget::SetSettingSlider(const FSettingTag& SliderTag, double InValue)
{
	static constexpr double MinValue = 0.0;
	static constexpr float MaxValue = 1.0;
	const double NewValue = FMath::Clamp(InValue, MinValue, MaxValue);
	SET_SETTING_VALUE(SliderTag, Slider, ChosenValue, NewValue, OnSetterFloat);

	// BP implementation
	SetSlider(SliderTag, InValue);
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
	UpdateSettings(PrimaryRef.SettingsToUpdate);

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

		if (USettingUserInput* SettingUserInput = Cast<USettingUserInput>(SettingsRowPtr->PrimaryData.SettingSubWidget.Get()))
		{
			SettingUserInput->SetEditableText(FText::FromString(NewValueStr));
		}
	}

	UserInputRef.UserInput = InValue;
	UserInputRef.OnSetterName.ExecuteIfBound(InValue);
	UpdateSettings(SettingsRowPtr->PrimaryData.SettingsToUpdate);

	// BP implementation
	SetUserInput(UserInputTag, InValue);
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
	UpdateSettings(SettingsRowPtr->PrimaryData.SettingsToUpdate);
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
	const FMargin ScrollBoxPadding = SettingsData.GetScrollboxPadding();
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

	TryFocusOnUI();
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

	UpdateSettingsTableRows();

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

	UpdateSettings(AddedSettings);

	UpdateScrollBoxesHeight();
}

void USettingsWidget::UpdateSettingsTableRows()
{
	TMap<FName, FSettingsRow> SettingRows;
	USettingsUtilsLibrary::GetAllSettingRows(/*Out*/SettingRows);
	if (!ensureMsgf(!SettingRows.IsEmpty(), TEXT("ASSERT: 'SettingRows' are empty")))
	{
		return;
	}

	// Reset values if currently are set
	OverallColumnsNumInternal = 1;
	SettingsTableRowsInternal.Empty();

	SettingsTableRowsInternal.Reserve(SettingRows.Num());
	for (const TTuple<FName, FSettingsRow>& SettingRowIt : SettingRows)
	{
		const FSettingsPicker& SettingsPicker = SettingRowIt.Value.SettingsPicker;
		SettingsTableRowsInternal.Emplace(SettingRowIt.Key, SettingsPicker);

		// Set overall columns num by amount of rows that are marked to be started on next column
		OverallColumnsNumInternal += static_cast<int32>(SettingsPicker.PrimaryData.bStartOnNextColumn);
	}
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

// Starts adding settings on the next column
void USettingsWidget::StartNextColumn_Implementation()
{
	// BP implementation
	// ...
}

// Automatically sets the height for all scrollboxes in the Settings
void USettingsWidget::UpdateScrollBoxesHeight()
{
	ForceLayoutPrepass(); // Call it to make GetSettingsHeight work since it is called during widget construction
	const float ScrollBoxHeight = GetScrollBoxHeight();

	for (const USettingScrollBox* ScrollBoxIt : SettingScrollBoxesInternal)
	{
		USizeBox* SizeBoxWidget = ScrollBoxIt ? ScrollBoxIt->GetSizeBoxWidget() : nullptr;
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
#define BIND_SETTING(Primary, Data, GetterFunction, SetterFunction)													\
	do																												\
	{																												\
		if (UObject* OwnerObject = Primary.GetSettingOwner(this))													\
		{																											\
			const FName GetterFunctionName = Primary.Getter.FunctionName;											\
			if (Primary.OwnerFunctionList.Contains(GetterFunctionName))												\
			{																										\
				Data.GetterFunction.BindUFunction(OwnerObject, GetterFunctionName);							\
			}																										\
			const FName SetterFunctionName = Primary.Setter.FunctionName;											\
			if (Primary.OwnerFunctionList.Contains(SetterFunctionName))										\
			{																										\
				Data.SetterFunction.BindUFunction(OwnerObject, SetterFunctionName);							\
			}																										\
		}																											\
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
		UpdateSettings(ReboundSettings);
	}
}

// Add setting on UI.
void USettingsWidget::AddSetting(FSettingsPicker& Setting)
{
	FSettingsDataBase* ChosenData = Setting.GetChosenSettingsData();
	if (!ChosenData)
	{
		return;
	}

	FSettingsPrimary& PrimaryData = Setting.PrimaryData;

	if (Setting.PrimaryData.bStartOnNextColumn)
	{
		StartNextColumn();
	}

	CreateSettingSubWidget(PrimaryData, ChosenData->GetSubWidgetClass());
	ChosenData->AddSetting(*this, PrimaryData);
}
