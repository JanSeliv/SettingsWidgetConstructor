﻿// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Engine/DeveloperSettings.h"
//---
#include "Data/SettingsThemeData.h"
//---
#include "Templates/SubclassOf.h"
//---
#include "SettingsDataAsset.generated.h"

class USettingsDataTable;
class UDataRegistry;

/**
 * Contains common settings data of the Constructor Widget plugin.
 * Is set up in 'Project Settings' -> "Plugins" -> "Settings Widget Constructor".
 */
UCLASS(Config = SettingsWidgetConstructor, DefaultConfig, DisplayName = "Settings Widget Constructor")
class SETTINGSWIDGETCONSTRUCTOR_API USettingsDataAsset : public UDeveloperSettings
{
	GENERATED_BODY()

	/*********************************************************************************************
	 * Getters
	 ********************************************************************************************* */
public:
	/** Returns Project Settings Data of the Settings Widget Constructor plugin. */
	static const FORCEINLINE USettingsDataAsset& Get() { return *GetDefault<ThisClass>(); }

	/** Returns Project Settings Data of the Settings Widget Constructor plugin. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor")
	static const FORCEINLINE USettingsDataAsset* GetSettingsDataAsset() { return &Get(); }

	/** Gets the settings container name for the settings, either Project or Editor */
	virtual FName GetContainerName() const override { return TEXT("Project"); }

	/** Gets the category for the settings, some high level grouping like, Editor, Engine, Game...etc. */
	virtual FName GetCategoryName() const override { return TEXT("Plugins"); }

	/** Returns the project's main Settings Data Table, it has to be set manually. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor")
	const USettingsDataTable* GetSettingsDataTable() const;

	/** Returns the sub-widget of Button settings. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor")
	FORCEINLINE TSubclassOf<class USettingButton> GetButtonClass() const { return ButtonClassInternal; }

	/** Returns the sub-widget of Checkbox settings. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor")
	FORCEINLINE TSubclassOf<class USettingCheckbox> GetCheckboxClass() const { return CheckboxClassInternal; }

	/** Returns the sub-widget of Combobox settings. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor")
	FORCEINLINE TSubclassOf<class USettingCombobox> GetComboboxClass() const { return ComboboxClassInternal; }

	/** Returns the widget class that represents each option in the Setting Combobox. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor")
	FORCEINLINE TSubclassOf<class USettingComboitem> GetComboitemClass() const { return ComboitemClassInternal; }

	/** Returns the sub-widget of Slider settings. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor")
	FORCEINLINE TSubclassOf<class USettingSlider> GetSliderClass() const { return SliderClassInternal; }

	/** Returns the sub-widget of Text Line settings. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor")
	FORCEINLINE TSubclassOf<class USettingTextLine> GetTextLineClass() const { return TextLineClassInternal; }

	/** Returns the sub-widget of User Input settings. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor")
	FORCEINLINE TSubclassOf<class USettingUserInput> GetUserInputClass() const { return UserInputClassInternal; }

	/** Returns the tooltip class applied to each setting. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor")
	FORCEINLINE TSubclassOf<class USettingTooltip> GetTooltipClass() const { return TooltipClassInternal; }

	/** Returns the column class which holds scrollbox and sub-settings inside. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor")
	FORCEINLINE TSubclassOf<class USettingColumn> GetColumnClass() const { return ColumnClassInternal; }

	/** Returns true, when USettingsWidget::TryConstructSettings is automatically called whenever the Settings Widget becomes constructed (e.g: on UUserWidget::AddToViewport call). */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor")
	FORCEINLINE bool IsAutoConstruct() const { return bAutoConstructInternal; }

	/** Returns true if should automatically focus the Settings Widget on UI each time it is opened. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor")
	FORCEINLINE bool IsAutoFocusOnOpen() const { return bAutoFocusOnOpenInternal; }

	/** Returns the width and height of the settings widget in percentages of an entire screen. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor")
	const FORCEINLINE FVector2D& GetSettingsPercentSize() const { return SettingsPercentSizeInternal; }

	/** Returns the height of the scrollbox widget in percentages of the entire settings widget. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor")
	FORCEINLINE float GetScrollboxPercentHeight() const { return ScrollboxPercentHeightInternal; }

	/** Returns the padding of the settings widget. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor")
	const FORCEINLINE FMargin& GetSettingsPadding() const { return SettingsPaddingInternal; }

	/** Returns the padding of the scrollbox widget. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor")
	const FORCEINLINE FMargin& GetColumnPadding() const { return ColumnPaddingInternal; }

	/** Return the button theme data. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor")
	const FORCEINLINE FButtonThemeData& GetButtonThemeData() const { return ButtonThemeDataInternal; }

	/** Returns the checkbox theme data. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor")
	const FORCEINLINE FCheckboxThemeData& GetCheckboxThemeData() const { return CheckboxThemeDataInternal; }

	/** Returns the combobox theme data. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor")
	const FORCEINLINE FComboboxThemeData& GetComboboxThemeData() const { return ComboboxThemeDataInternal; }

	/** Returns the slider theme data. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor")
	const FORCEINLINE FSliderThemeData& GetSliderThemeData() const { return SliderThemeDataInternal; }

	/** Returns the user input theme data. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor")
	const FORCEINLINE FSettingsThemeData& GetUserInputThemeData() const { return UserInputThemeDataInternal; }

	/** Returns the misc theme data. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor")
	const FORCEINLINE FMiscThemeData& GetMiscThemeData() const { return MiscThemeDataInternal; }

	/** Returns the Settings Data Registry asset, is automatically set by default to which 'Settings Data Table' is added by itself. */
	UFUNCTION(BlueprintPure, Category = "Settings Widget Constructor")
	UDataRegistry* GetSettingsDataRegistry() const;
	const TSoftObjectPtr<UDataRegistry>& GetSettingsDataRegistrySoft() const { return SettingsDataRegistryInternal; }

	/*********************************************************************************************
	 * Protected properties
	 ********************************************************************************************* */
protected:
	/** The project's main Settings Data Table, is config property and has to be set manually. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, Category = "Settings Widget Constructor", meta = (BlueprintProtected, DisplayName = "Settings Data Table", ShowOnlyInnerProperties))
	TSoftObjectPtr<const USettingsDataTable> SettingsDataTableInternal;

	/** The sub-widget class of Button settings, is config property. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, Category = "Settings Widget Constructor", meta = (BlueprintProtected, DisplayName = "Button Class", ShowOnlyInnerProperties))
	TSubclassOf<class USettingButton> ButtonClassInternal;

	/** The sub-widget class of Checkbox settings, is config property. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, Category = "Settings Widget Constructor", meta = (BlueprintProtected, DisplayName = "Checkbox Class", ShowOnlyInnerProperties))
	TSubclassOf<class USettingCheckbox> CheckboxClassInternal;

	/** The sub-widget class of Combobox settings, is config property. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, Category = "Settings Widget Constructor", meta = (BlueprintProtected, DisplayName = "Combobox Class", ShowOnlyInnerProperties))
	TSubclassOf<class USettingCombobox> ComboboxClassInternal;

	/** The widget class that represents each option in the Setting Combobox. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, Category = "Settings Widget Constructor", meta = (BlueprintProtected, DisplayName = "Comboitem Class", ShowOnlyInnerProperties))
	TSubclassOf<class USettingComboitem> ComboitemClassInternal;

	/** The sub-widget class of Slider settings, is config property. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, Category = "Settings Widget Constructor", meta = (BlueprintProtected, DisplayName = "Slider Class", ShowOnlyInnerProperties))
	TSubclassOf<class USettingSlider> SliderClassInternal;

	/** The sub-widget class of Text Line settings, is config property. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, Category = "Settings Widget Constructor", meta = (BlueprintProtected, DisplayName = "Text Line Class", ShowOnlyInnerProperties))
	TSubclassOf<class USettingTextLine> TextLineClassInternal;

	/** The sub-widget class of User Input settings, is config property. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, Category = "Settings Widget Constructor", meta = (BlueprintProtected, DisplayName = "User Input Class", ShowOnlyInnerProperties))
	TSubclassOf<class USettingUserInput> UserInputClassInternal;

	/** The tooltip class applied to all settings, is config property. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, Category = "Settings Widget Constructor", meta = (BlueprintProtected, DisplayName = "Tooltip Class", ShowOnlyInnerProperties))
	TSubclassOf<class USettingTooltip> TooltipClassInternal;

	/** The column class which holds scrollbox and sub-settings inside, is config property. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, Category = "Settings Widget Constructor", meta = (BlueprintProtected, DisplayName = "Column Class", ShowOnlyInnerProperties))
	TSubclassOf<class USettingColumn> ColumnClassInternal;

	/** If true, it will automatically call USettingsWidget::TryConstructSettings whenever the Settings Widget becomes constructed (e.g: on UUserWidget::AddToViewport call). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, Category = "Settings Widget Constructor", meta = (BlueprintProtected, DisplayName = "Auto Construct", ShowOnlyInnerProperties))
	bool bAutoConstructInternal;

	/** If true, the Setting Widget will be automatically focused on UI each time it is opened. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, Category = "Settings Widget Constructor", meta = (BlueprintProtected, DisplayName = "Auto Focus On Open", ShowOnlyInnerProperties))
	bool bAutoFocusOnOpenInternal;

	/** The width and height of the settings widget in percentages of an entire screen. Is clamped between 0 and 1, is config property. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, Category = "Settings Widget Constructor", meta = (BlueprintProtected, DisplayName = "Settings Percent Size", ClampMin = "0", ClampMax = "1", ShowOnlyInnerProperties))
	FVector2D SettingsPercentSizeInternal;

	/** The padding of the settings widget, is config property. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, Category = "Settings Widget Constructor", meta = (BlueprintProtected, DisplayName = "Settings Padding", ShowOnlyInnerProperties))
	FMargin SettingsPaddingInternal;

	/** The height of the scrollbox widget in percentages relatively to the content section, where 1 means fill all space under settings content section, is config property. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, Category = "Settings Widget Constructor", meta = (BlueprintProtected, DisplayName = "Scrollbox Percent Height", ClampMin = "0", ClampMax = "1", ShowOnlyInnerProperties))
	float ScrollboxPercentHeightInternal;

	/** The padding of the Column, is config property. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, Category = "Settings Widget Constructor", meta = (BlueprintProtected, DisplayName = "Scrollbox Padding", ShowOnlyInnerProperties))
	FMargin ColumnPaddingInternal;

	/** The button theme data, is config property. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, Category = "Settings Widget Constructor", meta = (BlueprintProtected, DisplayName = "Button Theme Data"))
	FButtonThemeData ButtonThemeDataInternal;

	/** The checkbox theme data, is config property. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, Category = "Settings Widget Constructor", meta = (BlueprintProtected, DisplayName = "Checkbox Theme Data"))
	FCheckboxThemeData CheckboxThemeDataInternal;

	/** The combobox theme data, is config property. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, Category = "Settings Widget Constructor", meta = (BlueprintProtected, DisplayName = "Combobox Theme Data"))
	FComboboxThemeData ComboboxThemeDataInternal;

	/** The slider theme data, is config property. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, Category = "Settings Widget Constructor", meta = (BlueprintProtected, DisplayName = "Slider Theme Data"))
	FSliderThemeData SliderThemeDataInternal;

	/** The user input theme data, is config property. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, Category = "Settings Widget Constructor", meta = (BlueprintProtected, DisplayName = "User Input Theme Data"))
	FSettingsThemeData UserInputThemeDataInternal;

	/** The misc theme data, is config property. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, Category = "Settings Widget Constructor", meta = (BlueprintProtected, DisplayName = "Misc Theme Data"))
	FMiscThemeData MiscThemeDataInternal;

	/** The Settings Data Registry asset, is automatically set by default to which 'Settings Data Table' is added by itself. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Config, Category = "Settings Widget Constructor", meta = (BlueprintProtected, DisplayName = "Settings Data Registry", ShowOnlyInnerProperties))
	TSoftObjectPtr<UDataRegistry> SettingsDataRegistryInternal;

	/*********************************************************************************************
	 * Internal
	 ********************************************************************************************* */
protected:
	/** Overrides post init to register Settings Data Table by default on startup. */
	virtual void PostInitProperties() override;

	/** Is called once Engine is initialized, so we can register Settings Data Table by default on startup. */
	void OnPostEngineInit();

#if WITH_EDITOR
	/** Overrides property change events to handle picking Settings Data Table. */
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
