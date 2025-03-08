// Copyright (c) Yevhenii Selivanov

#include "UI/SettingCombobox.h"
//---
#include "Data/SettingsDataAsset.h"
#include "MyUtilsLibraries/SWCWidgetUtilsLibrary.h"
#include "UI/SettingsWidget.h"
//---
#include "Components/Border.h"
#include "Components/ComboBoxString.h"
#include "Components/TextBlock.h"
#include "Widgets/Input/SComboBox.h"
//---
#include UE_INLINE_GENERATED_CPP_BY_NAME(SettingCombobox)

// Set the new combobox setting data for this widget
void USettingCombobox::SetComboboxData(const FSettingsCombobox& InComboboxData)
{
	ComboboxDataInternal = InComboboxData;
}

// Internal function to change the value of this subwidget
void USettingCombobox::SetComboboxIndex(int32 InValue)
{
	if (ensureMsgf(ComboboxWidget, TEXT("ERROR: [%i] %hs:\n'ComboboxWidget' is null!"), __LINE__, __FUNCTION__))
	{
		ComboboxWidget->SetSelectedIndex(InValue);
	}

	K2_OnSetComboboxIndex(InValue);
}

// Is the earliest point where the BindWidget properties are constructed
TSharedRef<SWidget> USettingCombobox::RebuildWidget()
{
	if (ComboboxWidget && !ComboboxWidget->OnGenerateWidgetEvent.IsBoundToObject(this))
	{
		ComboboxWidget->OnGenerateWidgetEvent.BindDynamic(this, &ThisClass::OnConstructComboitem);
	}

	return Super::RebuildWidget();
}

/*********************************************************************************************
 * Events and overrides
 ********************************************************************************************* */


// Called after the underlying slate widget is constructed
void USettingCombobox::NativeConstruct()
{
	Super::NativeConstruct();

	if (ComboboxWidget)
	{
		ComboboxWidget->OnSelectionChanged.AddUniqueDynamic(this, &ThisClass::OnSelectionChanged);

		SlateComboboxInternal = FSWCWidgetUtilsLibrary::GetSlateWidget<SComboboxString>(ComboboxWidget);
		check(SlateComboboxInternal.IsValid());
	}
}

// Is executed every tick when widget is enabled
void USettingCombobox::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!ComboboxWidget)
	{
		return;
	}

	const bool bIsComboboxOpenedLast = bIsComboboxOpenedInternal;
	bIsComboboxOpenedInternal = ComboboxWidget->IsOpen();

	if (bIsComboboxOpenedLast != bIsComboboxOpenedInternal)
	{
		OnMenuOpenChanged();
	}
}

// Called when a new item is selected in the combobox
void USettingCombobox::OnSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	if (!SettingsWidgetInternal
		|| !ComboboxWidget)
	{
		return;
	}

	const int32 SelectedIndex = ComboboxWidget->GetSelectedIndex();
	SettingsWidgetInternal->SetSettingComboboxIndex(GetSettingTag(), SelectedIndex);
}

// Called when the combobox is opened or closed/
void USettingCombobox::OnMenuOpenChanged()
{
	// Play the sound
	if (SettingsWidgetInternal)
	{
		SettingsWidgetInternal->PlayUIClickSFX();
	}
}

// Assigns the text to the text widget
void USettingComboitem::SetItemTextValue(const FText& InText)
{
	checkf(ItemTextWidget, TEXT("ERROR: [%i] %hs:\n'ItemTextWidget' is null!"), __LINE__, __FUNCTION__);
	ItemTextWidget->SetText(InText);
}

// Returns the text value of this comboitem
FText USettingComboitem::GetItemTextValue() const
{
	return ItemTextWidget ? ItemTextWidget->GetText() : FText::GetEmpty();
}

// Assigns the text to the text widget
void USettingComboitem::ApplyTheme_Implementation(const FSettingsCombobox& ComboboxData)
{
	checkf(ItemTextWidget, TEXT("ERROR: [%i] %hs:\n'ItemTextWidget' is null!"), __LINE__, __FUNCTION__);
	ItemTextWidget->SetJustification(ComboboxData.TextJustify);

	const USettingsDataAsset& SettingsDataAsset = USettingsDataAsset::Get();
	const FMiscThemeData& MiscThemeData = SettingsDataAsset.GetMiscThemeData();
	ItemTextWidget->SetFont(MiscThemeData.TextElementFont);
	ItemTextWidget->SetColorAndOpacity(MiscThemeData.TextElementColor);

	checkf(ItemBackgroundWidget, TEXT("ERROR: [%i] %hs:\n'ItemBackgroundWidget' is null!"), __LINE__, __FUNCTION__);
	const FComboboxThemeData& ComboboxThemeData = SettingsDataAsset.GetComboboxThemeData();
	ItemBackgroundWidget->SetBrushColor(ComboboxThemeData.ItemBackgroundColor.GetSpecifiedColor());
}

// Is overridden to construct the combobox
void USettingCombobox::OnAddSetting(const FSettingsPicker& Setting)
{
	ComboboxDataInternal = Setting.Combobox;

	// Add new items
	for (const FText& It : ComboboxDataInternal.Members)
	{
		CreateComboitem(It);
	}

	// Choose the selected item
	const int32 SelectedIndex = GetSettingsWidgetChecked().GetComboboxIndex(GetSettingTag());
	if (SelectedIndex != INDEX_NONE)
	{
		ComboboxWidget->SetSelectedIndex(SelectedIndex);
	}

	Super::OnAddSetting(Setting);
}

// Prespawn a new comboitem widget (is not added to the combobox yet)
void USettingCombobox::CreateComboitem(const FText& ItemTextValue)
{
	// Engine's combobox items are in FString without localization support, to solve it:
	// LOC-1. Prespawn own comboitem widget with TextBlock widget inside (which stores the FText value)
	// LOC-2. Init options with the FText key instead of value directly, e.g: 2503BD4742C4
	// LOC-3. in OnConstructComboitem event (UComboBoxString::OnGenerateWidgetEvent), provide to the engine own comboitem widget by given Text Id.
	// In this way, engine FString items are only used for keys (associations) while own comboitem widgets store localized FText values

	const TSubclassOf<USettingComboitem> ComboitemClass = USettingsDataAsset::Get().GetComboitemClass();
	if (!ensureMsgf(ComboitemClass, TEXT("ASSERT: [%i] %hs:\n'ComboitemClass' is not selected in the Settings Data Asset!"), __LINE__, __FUNCTION__)
		|| !ensureMsgf(!ItemTextValue.IsEmpty(), TEXT("ASSERT: [%i] %hs:\n'!ItemTextValue' is empty, can not construct comboitem!"), __LINE__, __FUNCTION__))
	{
		return;
	}

	// LOC-1. Prespawn own comboitem widget
	USettingComboitem* ComboitemWidget = CreateWidget<USettingComboitem>(this, ComboitemClass);
	checkf(ComboitemWidget, TEXT("ERROR: [%i] %hs:\n'ComboitemWidget' is null!"), __LINE__, __FUNCTION__);
	ComboitemWidget->ApplyTheme(ComboboxDataInternal);
	ComboitemWidget->SetItemTextValue(ItemTextValue);
	ComboitemWidgets.Add(ComboitemWidget);

	// LOC-2. Init Text ID strings; ID may be null if Text is not localized, then use the string value directly
	const FTextKey FoundTextId = FTextInspector::GetTextId(ItemTextValue).GetKey();
	const FString FinalTextId = !FoundTextId.IsEmpty() ? FoundTextId.ToString() : ItemTextValue.ToString();
	checkf(!FinalTextId.IsEmpty(), TEXT("ERROR: [%i] %hs:\n'!FinalTextId' is empty, can not construct comboitem from '%s' value!"), __LINE__, __FUNCTION__, *ItemTextValue.ToString());
	ComboboxWidget->AddOption(FinalTextId);
}

// Is called by an engine on attempting to add own comboitem widget to the combobox
UWidget* USettingCombobox::OnConstructComboitem(FString ItemTextId)
{
	if (!ensureMsgf(!ItemTextId.IsEmpty(), TEXT("ASSERT: [%i] %hs:\n'!ItemTextId' is empty, can not construct comboitem!"), __LINE__, __FUNCTION__)
		|| !ensureMsgf(!ComboitemWidgets.IsEmpty(), TEXT("ASSERT: [%i] %hs:\n'ComboitemWidgets' are empty, can not construct comboitem!"), __LINE__, __FUNCTION__))
	{
		return nullptr;
	}

	// LOC-3. Return to the engine own comboitem widget by given Text Id
	const TObjectPtr<USettingComboitem>* FoundComboitemPtr = ComboitemWidgets.FindByPredicate([&ItemTextId](const USettingComboitem* It)
	{
		const FText ItemText = It ? It->GetItemTextValue() : FText::GetEmpty();
		const FTextKey FoundTextId = FTextInspector::GetTextId(ItemText).GetKey();
		const bool bIsFoundKey = !FoundTextId.IsEmpty() && FoundTextId.ToString() == ItemTextId;
		return bIsFoundKey
			|| It->GetItemTextValue().ToString() == ItemTextId; // ID may be null if Text is not localized, so compare the string value directly
	});

	ensureMsgf(FoundComboitemPtr, TEXT("ASSERT: [%i] %hs:\nFailed to find the comboitem widget by the given Text Id: '%s'; default widget without styling will be created!"), __LINE__, __FUNCTION__, *ItemTextId);
	return FoundComboitemPtr ? *FoundComboitemPtr : nullptr;
}
