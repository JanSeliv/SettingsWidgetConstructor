// Copyright (c) Yevhenii Selivanov

#pragma once

#include "UI/SettingSubWidget.h"
//---
#include "SettingCombobox.generated.h"

template <typename T>
class SComboBox;

/**
 * The sub-widget of Combobox settings.
 */
UCLASS()
class SETTINGSWIDGETCONSTRUCTOR_API USettingCombobox : public USettingSubWidget
{
	GENERATED_BODY()

public:
	/** Returns the actual combobox widget of this setting. */
	UFUNCTION(BlueprintPure, Category = "SettingSubWidget")
	FORCEINLINE class UComboBoxString* GetComboboxWidget() const { return ComboboxWidget; }

	typedef SComboBox<TSharedPtr<FString>> SComboboxString;

	/** Returns the slate combobox. */
	FORCEINLINE TSharedPtr<SComboboxString> GetSlateCombobox() const { return SlateComboboxInternal.Pin(); }

	/** Returns true if combobox is opened. */
	UFUNCTION(BlueprintPure, Category = "SettingSubWidget")
	FORCEINLINE bool IsComboboxOpened() const { return bIsComboboxOpenedInternal; }

	/** Returns the combobox setting data. */
	UFUNCTION(BlueprintPure, Category = "SettingSubWidget")
	const FORCEINLINE FSettingsCombobox& GetComboboxData() const { return ComboboxDataInternal; }

	/** Set the new combobox setting data for this widget. */
	UFUNCTION(BlueprintCallable, Category = "SettingSubWidget", meta = (AutoCreateRefTerm = "InComboboxData"))
	void SetComboboxData(const FSettingsCombobox& InComboboxData);

	/** Internal function to change the value of this subwidget.
	 * @warning is not blueprintable, don't call it directly, but use Setter function from the Settings Widget. */
	void SetComboboxIndex(int32 InValue);

protected:
	/** Blueprint event called when the the subwidget value is changed. */
	UFUNCTION(BlueprintImplementableEvent, Category = "SettingSubWidget", meta = (BlueprintProtected, DisplayName = "On Set Combobox Index"))
	void K2_OnSetComboboxIndex(int32 InValue);

	/*********************************************************************************************
	 * Data
	 ********************************************************************************************* */
protected:
	/** The slate combobox.*/
	TWeakPtr<SComboboxString> SlateComboboxInternal = nullptr;

	/** The actual combobox widget of this setting. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "SettingSubWidget", meta = (BlueprintProtected, BindWidget))
	TObjectPtr<class UComboBoxString> ComboboxWidget = nullptr;

	/** Is true if combobox is currently opened in Settings. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "SettingSubWidget", meta = (BlueprintProtected, DisplayName = "Is Combobox Opened"))
	bool bIsComboboxOpenedInternal = false;

	/** The combobox setting data. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "SettingSubWidget", meta = (BlueprintProtected, DisplayName = "Combobox Data"))
	FSettingsCombobox ComboboxDataInternal;

	/** All created comboitem widgets. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "SettingSubWidget", meta = (BlueprintProtected, DisplayName = "Comboitem Widgets"))
	TArray<TObjectPtr<class USettingComboitem>> ComboitemWidgets;

	/*********************************************************************************************
	 * Events and overrides
	 ********************************************************************************************* */
public:
	/** Is overridden to return the combobox data of this widget. */
	virtual const FSettingsDataBase* GetSettingData() const override { return &ComboboxDataInternal; }

protected:
	/** Is the earliest point where the BindWidget properties are constructed. */
	virtual TSharedRef<SWidget> RebuildWidget() override;

	/** Called after the underlying slate widget is constructed.
	 * May be called multiple times due to adding and removing from the hierarchy. */
	virtual void NativeConstruct() override;

	/** Is executed every tick when widget is enabled. */
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/** Called when a new item is selected in the combobox. */
	UFUNCTION(BlueprintCallable, Category = "SettingSubWidget", meta = (BlueprintProtected))
	void OnSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	/** Called when the combobox is opened or closed. */
	UFUNCTION(BlueprintCallable, Category = "SettingSubWidget", meta = (BlueprintProtected))
	void OnMenuOpenChanged();

	/** Is overridden to construct the combobox.*/
	virtual void OnAddSetting(const FSettingsPicker& Setting) override;

	/** Prespawn a new comboitem widget (is not added to the combobox yet). */
	UFUNCTION(BlueprintCallable, Category = "SettingSubWidget", meta = (BlueprintProtected))
	void CreateComboitem(const FText& ItemTextValue);

	/** Is called by an engine on attempting to add own comboitem widget to the combobox.
	 * Is bound to UComboBoxString::OnGenerateWidgetEvent event. 
	 * @param ItemTextId The ID of the member that is being constructed, e.g: 2503BD4742C4
	 * @return Must return the comboitem widget that will be added to the combobox, otherwise engine will create a default one. */
	UFUNCTION(BlueprintCallable, Category = "SettingSubWidget", meta = (BlueprintProtected))
	UWidget* OnConstructComboitem(FString ItemTextId);
};

/**
 * Represents each option in the Setting Combobox.
 * Is exposed for localization support to bypass limitations of UComboBoxString, which works only with FString.
 */
UCLASS()
class SETTINGSWIDGETCONSTRUCTOR_API USettingComboitem : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Returns the widget that completely wraps this comboitem. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "SettingSubWidget")
	class UBorder* GetItemBackgroundWidget() const { return ItemBackgroundWidget; }

	/** Returns the widget that shows the caption text of this comboitem. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "SettingSubWidget")
	class UTextBlock* GetItemTextWidget() const { return ItemTextWidget; }

	/** Assigns the text to the text widget. */
	UFUNCTION(BlueprintCallable, Category = "SettingSubWidget", meta = (AutoCreateRefTerm = "InText"))
	void SetItemTextValue(const FText& InText);

	/** Returns the text value of this comboitem. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "SettingSubWidget")
	FText GetItemTextValue() const;

	/** Applies 'Style', paddings, colors, etc. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "SettingSubWidget|Theme")
	void ApplyTheme(const FSettingsCombobox& ComboboxData);

protected:
	/** The widget that completely wraps this comboitem. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "SettingSubWidget", meta = (BlueprintProtected, BindWidget))
	TObjectPtr<class UBorder> ItemBackgroundWidget = nullptr;

	/** The widget that shows the caption text of this comboitem. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "SettingSubWidget", meta = (BlueprintProtected, BindWidget))
	TObjectPtr<class UTextBlock> ItemTextWidget = nullptr;
};
