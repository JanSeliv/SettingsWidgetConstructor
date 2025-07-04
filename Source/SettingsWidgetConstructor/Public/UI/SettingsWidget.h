﻿// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Blueprint/UserWidget.h"
//---
#include "Data/SettingsRow.h"
//---
#include "SettingsWidget.generated.h"

/**
 * The UI widget of settings.
 * It generates and manages settings specified in rows of the Settings Data Table.
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class SETTINGSWIDGETCONSTRUCTOR_API USettingsWidget : public UUserWidget
{
	GENERATED_BODY()

	/* ---------------------------------------------------
	 *		Public properties
	 * --------------------------------------------------- */
public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnToggledSettings, bool, bIsVisible);

	/** Is called to notify listeners the Settings widget is opened or closed. */
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "Settings Widget Constructor")
	FOnToggledSettings OnToggledSettings;

	/* ---------------------------------------------------
	 *		Public functions
	 * --------------------------------------------------- */
public:
	/** Constructs settings if viewport is ready otherwise wait until viewport become initialized. */
	UFUNCTION(BlueprintCallable, Category = "Settings Widget Constructor")
	void TryConstructSettings();

	/** Display settings on UI. */
	UFUNCTION(BlueprintCallable, Category = "Settings Widget Constructor")
	void OpenSettings();

	/** Is called on displayed settings on UI. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Settings Widget Constructor")
	void OnOpenSettings();

	/** Save and close the settings widget. */
	UFUNCTION(BlueprintCallable, Category = "Settings Widget Constructor")
	void CloseSettings();

	/** Is called on closed settings on UI. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Settings Widget Constructor")
	void OnCloseSettings();

	/** Flip-flop opens and closes the Settings menu. */
	UFUNCTION(BlueprintCallable, Category = "Settings Widget Constructor")
	void ToggleSettings();

	/** Is called on opening to focus the widget on UI if allowed. */
	UFUNCTION(BlueprintCallable, Category = "Settings Widget Constructor")
	void TryFocusOnUI();

	/** Returns true when this widget is fully constructed and ready to be used. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE bool IsSettingsWidgetConstructed() const { return !SettingsTableRowsInternal.IsEmpty(); }

	/** Is called to player sound effect on any setting click. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Settings Widget Constructor")
	void PlayUIClickSFX();

	/** Returns the amount of settings rows. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor")
	FORCEINLINE int32 GetSettingsTableRowsNum() const { return SettingsTableRowsInternal.Num(); }

	/** Try to find the setting row.
	* @param PotentialTagName The probable tag name by which the row will be found (for 'VSync' will find a row with 'Settings.Checkbox.VSync' tag).
	* @see UMyGameUserSettings::SettingsTableRowsInternal */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor")
	const FSettingsPicker& FindSettingRow(FName PotentialTagName) const;

	/** Returns the found row by specified tag.
	* @param SettingTag The gameplay tag by which the row will be found. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor", meta = (AutoCreateRefTerm = "SettingTag"))
	const FSettingsPicker& GetSettingRow(const FSettingTag& SettingTag) const;

	/** Save all settings into their configs. */
	UFUNCTION(BlueprintCallable, Category = "Settings Widget Constructor")
	void SaveSettings();

	/** Apply all current settings on device. */
	UFUNCTION(BlueprintCallable, Category = "Settings Widget Constructor")
	void ApplySettings();

	/** Update specific settings on UI by tags.
	 * Alternative, in code `UPDATE_SETTING_BY_FUNCTION(SettingsWidget, ThisClass, SetFullscreenMode)` can be used.
	 * @param SettingsToUpdate Contains tags of settings that are needed to update.
	 * @param bLoadFromConfig If true, then load settings from config file, otherwise just update UI. */
	UFUNCTION(BlueprintCallable, Category = "Settings Widget Constructor", meta = (AutoCreateRefTerm = "SettingsToUpdate"))
	void UpdateSettingsByTags(
		UPARAM(meta = (Categories = "Settings")) const FGameplayTagContainer& SettingsToUpdate,
		bool bLoadFromConfig = false);

	/** Update all existing settings on UI.
	 * @param bLoadFromConfig If true, then load settings from config file, otherwise just update UI. */
	UFUNCTION(BlueprintCallable, Category = "Settings Widget Constructor")
	void UpdateAllSettings(bool bLoadFromConfig = false);

	/** Returns the name of found tag by specified function. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor", meta = (AutoCreateRefTerm = "SettingFunction"))
	const FSettingTag& GetTagByFunction(const FSettingFunctionPicker& SettingFunction) const;

	/* ---------------------------------------------------
	 *		Style
	 * --------------------------------------------------- */
public:
	/** Returns the size of the Settings widget on the screen. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor")
	FVector2D GetSettingsSize() const;

	/** Returns the size of specified sections on the screen. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor|Style")
	FVector2D GetSubWidgetsSize(
		UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/SettingsWidgetConstructor.EMyVerticalAlignment")) int32 SectionsBitmask) const;

	/** Returns the height of a setting scrollbox on the screen. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor|Style")
	float GetScrollBoxHeight() const;

	/** Is blueprint-event called that returns the style brush by specified button state. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor|Style")
	static FSlateBrush GetButtonBrush(ESettingsButtonState State);

	/* ---------------------------------------------------
	 *		Setters by setting types
	 * --------------------------------------------------- */
public:
	/**
   	  * Set value to the option by tag.
   	  * Common function to set setting of an any type by the string.
   	  * Used by cheat manager to override any setting.
	  *	@param TagName The key by which the row will be find.
	  * @param Value The value in a string format.
	  */
	UFUNCTION(BlueprintCallable, Category = "Settings Widget Constructor|Setters", meta = (AutoCreateRefTerm = "Value"))
	void SetSettingValue(FName TagName, const FString& Value);

	/** Press button. */
	UFUNCTION(BlueprintCallable, Category = "Settings Widget Constructor|Setters", meta = (AutoCreateRefTerm = "ButtonTag"))
	void SetSettingButtonPressed(const FSettingTag& ButtonTag);

	/** Toggle checkbox. */
	UFUNCTION(BlueprintCallable, Category = "Settings Widget Constructor|Setters", meta = (AutoCreateRefTerm = "CheckboxTag"))
	void SetSettingCheckbox(const FSettingTag& CheckboxTag, bool InValue);

	/** Set chosen member index for a combobox. */
	UFUNCTION(BlueprintCallable, Category = "Settings Widget Constructor|Setters", meta = (AutoCreateRefTerm = "ComboboxTag"))
	void SetSettingComboboxIndex(const FSettingTag& ComboboxTag, int32 InValue);

	/** Set current value for a slider [0...1]. */
	UFUNCTION(BlueprintCallable, Category = "Settings Widget Constructor|Setters", meta = (AutoCreateRefTerm = "SliderTag"))
	void SetSettingSlider(const FSettingTag& SliderTag, double InValue);

	/** Set new text. */
	UFUNCTION(BlueprintCallable, Category = "Settings Widget Constructor|Setters", meta = (AutoCreateRefTerm = "TextLineTag,InValue"))
	void SetSettingTextLine(const FSettingTag& TextLineTag, const FText& InValue);

	/** Set new text for an input box. */
	UFUNCTION(BlueprintCallable, Category = "Settings Widget Constructor|Setters", meta = (AutoCreateRefTerm = "UserInputTag"))
	void SetSettingUserInput(const FSettingTag& UserInputTag, FName InValue);

	/** Set new custom widget for setting by specified tag. */
	UFUNCTION(BlueprintCallable, Category = "Settings Widget Constructor|Setters", meta = (AutoCreateRefTerm = "CustomWidgetTag"))
	void SetSettingCustomWidget(const FSettingTag& CustomWidgetTag, class USettingCustomWidget* InCustomWidget);

	/** Is called after any setting is changed. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "C++", meta = (AutoCreateRefTerm = "SettingPrimaryRow"))
	void OnAnySettingSet(const FSettingsPrimary& SettingPrimaryRow);

	/** Creates setting sub-widget (like button, checkbox etc.) based on specified setting class and sets it to specified primary data.
	 * @param InOutPrimary The Data that should contain created setting class.
	 * @param SettingSubWidgetClass The setting widget class to create. */
	UFUNCTION(BlueprintCallable, Category = "Settings Widget Constructor", meta = (AutoCreateRefTerm = "InOutPrimary"))
	USettingSubWidget* CreateSettingSubWidget(UPARAM(ref) FSettingsPrimary& InOutPrimary, const TSubclassOf<USettingSubWidget> SettingSubWidgetClass);

	/** Creates setting sub-widget (like button, checkbox etc.) based on specified setting class and sets it to specified primary data. */
	template <typename T = USettingSubWidget>
	FORCEINLINE T* CreateSettingSubWidget(FSettingsPrimary& InOutPrimary, const TSubclassOf<USettingSubWidget> SettingSubWidgetClass) { return Cast<T>(CreateSettingSubWidget(InOutPrimary, SettingSubWidgetClass)); }

	/* ---------------------------------------------------
	 *		Getters by setting types
	 * --------------------------------------------------- */
public:
	/** Returns is a checkbox toggled. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor|Getters", meta = (AutoCreateRefTerm = "CheckboxTag"))
	bool GetCheckboxValue(const FSettingTag& CheckboxTag) const;

	/** Returns chosen member index of a combobox. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor|Getters", meta = (AutoCreateRefTerm = "ComboboxTag"))
	int32 GetComboboxIndex(const FSettingTag& ComboboxTag) const;

	/** Get all members of a combobox. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor|Getters", meta = (AutoCreateRefTerm = "ComboboxTag"))
	void GetComboboxMembers(const FSettingTag& ComboboxTag, TArray<FText>& OutMembers) const;

	/** Get current value of a slider [0...1]. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor|Getters", meta = (AutoCreateRefTerm = "SliderTag"))
	double GetSliderValue(const FSettingTag& SliderTag) const;

	/** Get current text of the text line setting. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor|Getters", meta = (AutoCreateRefTerm = "TextLineTag"))
	void GetTextLineValue(const FSettingTag& TextLineTag, FText& OutText) const;

	/** Get current input name of the text input setting. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor|Getters", meta = (AutoCreateRefTerm = "UserInputTag"))
	FName GetUserInputValue(const FSettingTag& UserInputTag) const;

	/** Get custom widget of the setting by specified tag.  */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor|Getters", meta = (AutoCreateRefTerm = "CustomWidgetTag"))
	class USettingCustomWidget* GetCustomWidget(const FSettingTag& CustomWidgetTag) const;

	/** Get setting widget object by specified tag. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor|Getters", meta = (AutoCreateRefTerm = "SettingTag"))
	class USettingSubWidget* GetSettingSubWidget(const FSettingTag& SettingTag) const;

	template <typename T = USettingSubWidget>
	FORCEINLINE T* GetSettingSubWidget(const FSettingTag& SettingTag) const { return Cast<T>(GetSettingSubWidget(SettingTag)); }

	/* ---------------------------------------------------
	 *		Protected properties
	 * --------------------------------------------------- */
protected:
	/** Contains all settings. */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Transient, Category = "Settings Widget Constructor", meta = (BlueprintProtected, DisplayName = "Settings Table Rows"))
	TMap<FName/*Tag*/, FSettingsPicker/*Row*/> SettingsTableRowsInternal;

	/** Contains all Setting tags that failed to bind their Getter/Setter functions on initial construct, so it's stored to be rebound later.
	 * @see USettingsWidget::TryRebindDeferredContexts */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "Settings Widget Constructor", meta = (BlueprintProtected, DisplayName = "DeferredBindings"))
	FGameplayTagContainer DeferredBindingsInternal;

	/* ---------------------------------------------------
	 *		Bound widget properties
	 * --------------------------------------------------- */
public:
	/** Returns the widget of the header section. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor|Widgets")
	FORCEINLINE class UVerticalBox* GetHeaderVerticalBox() const { return HeaderVerticalBox; }

	/** Returns the widget of the content section. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor|Widgets")
	FORCEINLINE class UHorizontalBox* GetContentHorizontalBox() const { return ContentHorizontalBox; }

	/** Returns the widget of the footer section. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor|Widgets")
	FORCEINLINE class UVerticalBox* GetFooterVerticalBox() const { return FooterVerticalBox; }

protected:
	/** The section in the top margin of Settings, usually contains a title. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "Settings Widget Constructor|Widgets", meta = (BlueprintProtected, BindWidget))
	TObjectPtr<class UVerticalBox> HeaderVerticalBox = nullptr;

	/** The main section in the middle of Settings, contains all in-game settings. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "Settings Widget Constructor|Widgets", meta = (BlueprintProtected, BindWidget))
	TObjectPtr<class UHorizontalBox> ContentHorizontalBox = nullptr;

	/** The section in the bottom margin of Settings, usually contains the Back button*/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "Settings Widget Constructor|Widgets", meta = (BlueprintProtected, BindWidget))
	TObjectPtr<class UVerticalBox> FooterVerticalBox = nullptr;

	/* ---------------------------------------------------
	*		Protected functions
	* --------------------------------------------------- */
protected:
	/** Called after the underlying slate widget is constructed.
	* May be called multiple times due to adding and removing from the hierarchy. */
	virtual void NativeConstruct() override;

	/** Called when the widget is removed from the viewport. */
	virtual void NativeDestruct() override;

	/** Is called right after the game was started and windows size is set to construct settings. */
	void OnViewportResizedWhenInit(class FViewport* Viewport, uint32 Index);

	/** Is blueprint-event called on settings construct to cache some data before creating subwidgets. */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Settings Widget Constructor", meta = (BlueprintProtected))
	void OnConstructSettings();
	void ConstructSettings();

	/** Internal function to cache setting rows from Settings Data Table. */
	UFUNCTION(BlueprintCallable, Category = "Settings Widget Constructor", meta = (BlueprintProtected))
	void CacheTable();

	/** Clears all added settings. */
	UFUNCTION(BlueprintCallable, Category = "Settings Widget Constructor", meta = (BlueprintProtected))
	void RemoveAllSettings();

	/** Is called when In-Game menu became opened or closed. */
	UFUNCTION(BlueprintCallable, Category = "Settings Widget Constructor", meta = (BlueprintProtected))
	void OnToggleSettings(bool bIsVisible);

	/** Automatically sets the height for all scrollboxes in the Settings. */
	UFUNCTION(BlueprintCallable, Category = "Settings Widget Constructor", meta = (BlueprintProtected))
	void UpdateScrollBoxesHeight();

	/* ---------------------------------------------------
	 *		Bind by setting types
	 * --------------------------------------------------- */
public:
	/** Bind setting to specified in table Get/Set delegates, so both methods will be called. */
	UFUNCTION(BlueprintCallable, Category = "Settings Widget Constructor", meta = (BlueprintProtected))
	bool BindSetting(UPARAM(ref)FSettingsPicker& Setting);

	/** Bind button to own Get/Set delegates. */
	UFUNCTION(BlueprintCallable, Category = "Settings Widget Constructor|Binders", meta = (BlueprintProtected, AutoCreateRefTerm = "Primary,Data"))
	void BindButton(const FSettingsPrimary& Primary, FSettingsButton& Data);

	/** Bind checkbox to own Get/Set delegates. */
	UFUNCTION(BlueprintCallable, Category = "Settings Widget Constructor|Binders", meta = (BlueprintProtected, AutoCreateRefTerm = "Primary,Data"))
	void BindCheckbox(const FSettingsPrimary& Primary, FSettingsCheckbox& Data);

	/** Bind combobox to own Get/Set delegates. */
	UFUNCTION(BlueprintCallable, Category = "Settings Widget Constructor|Binders", meta = (BlueprintProtected, AutoCreateRefTerm = "Primary,Data"))
	void BindCombobox(const FSettingsPrimary& Primary, FSettingsCombobox& Data);

	/** Bind slider to own Get/Set delegates. */
	UFUNCTION(BlueprintCallable, Category = "Settings Widget Constructor|Binders", meta = (BlueprintProtected, AutoCreateRefTerm = "Primary,Data"))
	void BindSlider(const FSettingsPrimary& Primary, FSettingsSlider& Data);

	/** Bind simple text to own Get/Set delegates. */
	UFUNCTION(BlueprintCallable, Category = "Settings Widget Constructor|Binders", meta = (BlueprintProtected, AutoCreateRefTerm = "Primary,Data"))
	void BindTextLine(const FSettingsPrimary& Primary, FSettingsTextLine& Data);

	/** Bind text input to own Get/Set delegates. */
	UFUNCTION(BlueprintCallable, Category = "Settings Widget Constructor|Binders", meta = (BlueprintProtected, AutoCreateRefTerm = "Primary,Data"))
	void BindUserInput(const FSettingsPrimary& Primary, FSettingsUserInput& Data);

	/** Bind custom widget to own Get/Set delegates.  */
	UFUNCTION(BlueprintCallable, Category = "Settings Widget Constructor|Binders", meta = (BlueprintProtected, AutoCreateRefTerm = "Primary,Data"))
	void BindCustomWidget(const FSettingsPrimary& Primary, FSettingsCustomWidget& Data);

protected:
	/** Bind and set static object delegate.
	* @see FSettingsPrimary::OwnerFunc */
	UFUNCTION(BlueprintCallable, Category = "Settings Widget Constructor", meta = (BlueprintProtected))
	bool TryBindOwner(UPARAM(ref)FSettingsPrimary& Primary);

	/** Attempts to rebind those Settings that failed to bind their Getter/Setter functions on initial construct.
	 * @see USettingsWidget::DeferredBindingsInternal */
	UFUNCTION(BlueprintCallable, Category = "Settings Widget Constructor", meta = (BlueprintProtected))
	void TryRebindDeferredContexts();

	/** Add setting on UI. */
	UFUNCTION(BlueprintCallable, Category = "Settings Widget Constructor|Adders", meta = (BlueprintProtected))
	void AddSetting(UPARAM(ref)FSettingsPicker& Setting);

	/*********************************************************************************************
	 * Columns builder
	 ********************************************************************************************* */
public:
	/** Returns the index of column for a Setting by specified tag or -1 if not found. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor|Columns", meta = (AutoCreateRefTerm = "SettingTag"))
	int32 GetColumnIndexBySetting(const FSettingTag& SettingTag) const;

	/** Returns a column by specified index. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor|Columns")
	class USettingColumn* GetColumnByIndex(int32 ColumnIndex) const { return ColumnsInternal.IsValidIndex(ColumnIndex) ? ColumnsInternal[ColumnIndex] : nullptr; }

	/** Returns a column by specified setting tag. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor|Columns", meta = (AutoCreateRefTerm = "SettingTag"))
	FORCEINLINE USettingColumn* GetColumnBySetting(const FSettingTag& SettingTag) const { return GetColumnByIndex(GetColumnIndexBySetting(SettingTag)); }

protected:
	/** Contains all setting columns. */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Transient, Category = "Settings Widget Constructor|Columns", meta = (BlueprintProtected, DisplayName = "Columns"))
	TArray<TObjectPtr<class USettingColumn>> ColumnsInternal;

protected:
	/** Creates new column on specified index. */
	UFUNCTION(BlueprintCallable, Category = "Settings Widget Constructor|Columns", meta = (BlueprintProtected))
	void AddColumn(int32 ColumnIndex);

	/*********************************************************************************************
	 * Multiple Data Tables support
	 ********************************************************************************************* */
protected:
	/** Is called when the Settings Data Registry is changed. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Settings Widget Constructor", meta = (BlueprintProtected))
	void OnSettingsDataRegistryChanged(class UDataRegistry* SettingsDataRegistry);
	void BindOnSettingsDataRegistryChanged();
};

/** Helper to update setting by specified function.
 * E.g: UPDATE_SETTING(SettingsWidget, ThisClass, SetFullscreenMode);
 * @param SettingsWidget The widget responsible for managing settings.
 * @param InClass The class that declares the function.
 * @param InFunctionName The name of the function used to identify the setting tag. */
#define UPDATE_SETTING_BY_FUNCTION(SettingsWidget, InClass, InFunctionName)						\
	do {																			\
		if (!SettingsWidget)														\
		{																			\
			break;																	\
		}																			\
		static FGameplayTagContainer InFunctionName##SettingTag = FGameplayTagContainer::EmptyContainer; \
		if (InFunctionName##SettingTag.IsEmpty())										\
		{																			\
			static const FSettingFunctionPicker FunctionPicker(						\
				GetClass(),															\
				GET_FUNCTION_NAME_CHECKED(InClass, InFunctionName));				\
			InFunctionName##SettingTag.AddTag(SettingsWidget->GetTagByFunction(FunctionPicker)); \
		}																			\
		SettingsWidget->UpdateSettingsByTags(InFunctionName##SettingTag);				\
	} while (0)
