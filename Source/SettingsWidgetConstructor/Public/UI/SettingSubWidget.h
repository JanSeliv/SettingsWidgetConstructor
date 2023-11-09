// Copyright (c) Yevhenii Selivanov

#pragma once

#include "Blueprint/UserWidget.h"
//---
#include "Data/SettingsRow.h"
//---
#include "SettingSubWidget.generated.h"

/**
 * The base class of specific setting like button, checkbox, combobox, slider, text line, user input etc.
 */
UCLASS()
class SETTINGSWIDGETCONSTRUCTOR_API USettingSubWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Returns the widget that shows the caption text of this setting. */
	UFUNCTION(BlueprintPure, Category = "SettingSubWidget")
	FORCEINLINE class UTextBlock* GetCaptionWidget() const { return CaptionWidget; }

	/** Returns the Size Box widget . */
	UFUNCTION(BlueprintPure, Category = "SettingSubWidget")
	FORCEINLINE class USizeBox* GetSizeBoxWidget() const { return SizeBoxWidget; }

	/** Returns the custom line height for this setting. */
	UFUNCTION(BlueprintPure, Category = "SettingSubWidget")
	float GetLineHeight() const;

	/** Set custom line height for this setting. */
	UFUNCTION(BlueprintCallable, Category = "SettingSubWidget")
	void SetLineHeight(float NewLineHeight);

	/** Returns the caption text that is shown on UI. */
	UFUNCTION(BlueprintPure, Category = "SettingSubWidget")
	void GetCaptionText(FText& OutCaptionText) const;

	/** Set the new caption text on UI for this widget. */
	UFUNCTION(BlueprintCallable, Category = "SettingSubWidget", meta = (AutoCreateRefTerm = "NewCaptionText"))
	void SetCaptionText(const FText& NewCaptionText);

	/** Returns the setting tag of this widget. */
	UFUNCTION(BlueprintPure, Category = "SettingSubWidget")
	const FORCEINLINE FSettingTag& GetSettingTag() const { return PrimaryDataInternal.Tag; }

	/** Returns the setting primary row of this widget. */
	UFUNCTION(BlueprintPure, Category = "SettingSubWidget")
	const FORCEINLINE FSettingsPrimary& GetSettingPrimaryRow() const { return PrimaryDataInternal; }

	/** Set the new setting tag for this widget. */
	UFUNCTION(BlueprintCallable, Category = "SettingSubWidget", meta = (AutoCreateRefTerm = "InSettingPrimaryRow"))
	void SetSettingPrimaryRow(const FSettingsPrimary& InSettingPrimaryRow);

	/** Returns the main setting widget (the outer of this subwidget). */
	UFUNCTION(BlueprintPure, Category = "SettingSubWidget")
	USettingsWidget* GetSettingsWidget() const;
	USettingsWidget& GetSettingsWidgetChecked() const;

	/** Sets the main settings widget for this subwidget. */
	UFUNCTION(BlueprintCallable, Category = "SettingSubWidget")
	void SetSettingsWidget(USettingsWidget* InSettingsWidget);

	/** Returns parent widget element in hierarchy of this subwidget: it could be a header/footer vertical box or column. */
	UFUNCTION(BlueprintPure, Category = "SettingSubWidget")
	FORCEINLINE UPanelSlot* GetParentSlot() const { return ParentSlotInternal; }

	/** Sets the parent widget element in hierarchy of this subwidget.
	 * @param InPanelWidget header/footer vertical box or column.
	 * @return The slot where this widget was added, or null if the add failed. */
	UFUNCTION(BlueprintCallable, Category = "SettingSubWidget")
	UPanelSlot* AttachTo(UPanelWidget* InPanelWidget);

	/** Adds given widget as tooltip to this setting. */
	UFUNCTION(BlueprintCallable, Category = "SettingSubWidget")
	void AddTooltipWidget();

	/*********************************************************************************************
	 * Events
	 ********************************************************************************************* */
public:
	/** Base method that is called when the underlying slate widget is constructed.
	 * May be called multiple times due to adding and removing from the hierarchy. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Settings Widget Constructor|Adders", meta = (BlueprintProtected, DisplayName = "On Add Setting"))
	void BPOnAddSetting();
	virtual void OnAddSetting(const FSettingsPicker& Setting);

	/** Applies 'Style', paddings, colors, etc. */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "SettingSubWidget|Theme")
	void ApplyTheme();

	/*********************************************************************************************
	 * Data
	 ********************************************************************************************* */
protected:
	/** The Size Box widget. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "SettingSubWidget", meta = (BlueprintProtected, BindWidget))
	TObjectPtr<class USizeBox> SizeBoxWidget = nullptr;

	/** The widget that shows the caption text of this setting. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "SettingSubWidget", meta = (BlueprintProtected, BindWidget))
	TObjectPtr<class UTextBlock> CaptionWidget = nullptr;

	/** The setting primary row of this widget. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "SettingSubWidget", meta = (BlueprintProtected, DisplayName = "Primary Data"))
	FSettingsPrimary PrimaryDataInternal = FSettingsPrimary::EmptyPrimary;

	/** The main settings widget. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "SettingSubWidget", meta = (BlueprintProtected, DisplayName = "Settings Widget"))
	TObjectPtr<USettingsWidget> SettingsWidgetInternal = nullptr;

	/** The parent widget element in hierarchy of this subwidget: it could be a header/footer vertical box or column. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "SettingSubWidget", meta = (BlueprintProtected, DisplayName = "Panel Widget"))
	TObjectPtr<UPanelSlot> ParentSlotInternal = nullptr;
};

/**
 * The sub-widget of Button settings.
 */
UCLASS()
class SETTINGSWIDGETCONSTRUCTOR_API USettingButton : public USettingSubWidget
{
	GENERATED_BODY()

public:
	/** Returns the actual button widget of this setting. */
	UFUNCTION(BlueprintPure, Category = "SettingSubWidget")
	FORCEINLINE class UButton* GetButtonWidget() const { return ButtonWidget; }

	/** Returns the slate button. */
	FORCEINLINE TSharedPtr<class SButton> GetSlateButton() const { return SlateButtonInternal.Pin(); }

	/** Returns the button setting data. */
	UFUNCTION(BlueprintPure, Category = "SettingSubWidget")
	const FORCEINLINE FSettingsButton& GetButtonData() const { return ButtonDataInternal; }

	/** Set the new button setting data for this widget. */
	UFUNCTION(BlueprintCallable, Category = "SettingSubWidget", meta = (AutoCreateRefTerm = "InButtonData"))
	void SetButtonData(const FSettingsButton& InButtonData);

	/*********************************************************************************************
	 * Data
	 ********************************************************************************************* */
protected:
	/** The slate button.*/
	TWeakPtr<class SButton> SlateButtonInternal = nullptr;

	/** The actual button widget of this setting. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "SettingSubWidget", meta = (BlueprintProtected, BindWidget))
	TObjectPtr<class UButton> ButtonWidget = nullptr;

	/** The button setting data. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "SettingSubWidget", meta = (BlueprintProtected, DisplayName = "Button Data"))
	FSettingsButton ButtonDataInternal;

	/*********************************************************************************************
	 * Events
	 ********************************************************************************************* */
protected:
	/** Called after the underlying slate widget is constructed.
	 * May be called multiple times due to adding and removing from the hierarchy. */
	virtual void NativeConstruct() override;

	/** Is overridden to construct the button. */
	virtual void OnAddSetting(const FSettingsPicker& Setting) override;

	/** Called when the Button Widget is pressed.
	 * @see USettingButton::OnSettingButtonPressed */
	UFUNCTION(BlueprintCallable, Category = "SettingSubWidget", meta = (BlueprintProtected))
	void OnButtonPressed();
};

/**
 * The sub-widget of Checkbox settings.
 */
UCLASS()
class SETTINGSWIDGETCONSTRUCTOR_API USettingCheckbox : public USettingSubWidget
{
	GENERATED_BODY()

public:
	/** Returns the actual checkbox widget of this setting. */
	UFUNCTION(BlueprintPure, Category = "SettingSubWidget")
	FORCEINLINE class UCheckBox* GetCheckboxWidget() const { return CheckboxWidget; }

	/** Returns the slate checkbox. */
	FORCEINLINE TSharedPtr<class SCheckBox> GetSlateCheckbox() const { return SlateCheckboxInternal.Pin(); }

	/** Returns the checkbox setting data. */
	UFUNCTION(BlueprintPure, Category = "SettingSubWidget")
	const FORCEINLINE FSettingsCheckbox& GetCheckboxData() const { return CheckboxDataInternal; }

	/** Set the new checkbox setting data for this widget. */
	UFUNCTION(BlueprintCallable, Category = "SettingSubWidget", meta = (AutoCreateRefTerm = "InCheckboxData"))
	void SetCheckboxData(const FSettingsCheckbox& InCheckboxData);

	/*********************************************************************************************
	 * Data
	 ********************************************************************************************* */
protected:
	/** The slate checkbox.*/
	TWeakPtr<class SCheckBox> SlateCheckboxInternal = nullptr;

	/** The actual checkbox widget of this setting. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "SettingSubWidget", meta = (BlueprintProtected, BindWidget))
	TObjectPtr<class UCheckBox> CheckboxWidget = nullptr;

	/** The checkbox setting data. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "SettingSubWidget", meta = (BlueprintProtected, DisplayName = "Checkbox Data"))
	FSettingsCheckbox CheckboxDataInternal;

	/*********************************************************************************************
	 * Events
	 ********************************************************************************************* */
public:
	/** Called after the underlying slate widget is constructed.
 	 * May be called multiple times due to adding and removing from the hierarchy. */
	virtual void NativeConstruct() override;

	/** Called when the checked state has changed.
	 * @see USettingCheckbox::CheckboxWidgetInternal */
	UFUNCTION(BlueprintCallable, Category = "SettingSubWidget", meta = (BlueprintProtected))
	void OnCheckStateChanged(bool bIsChecked);

	/** Is overridden to construct the checkbox. */
	virtual void OnAddSetting(const FSettingsPicker& Setting) override;
};

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

	/*********************************************************************************************
	 * Data
	 ********************************************************************************************* */
protected:
	/** The slate combobox.*/
	TWeakPtr<SComboboxString> SlateComboboxInternal = nullptr;

	/** The actual combobox widget of this setting. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "SettingSubWidget", meta = (BlueprintProtected, BindWidget))
	TObjectPtr<class UComboBoxString> ComboboxWidget = nullptr;

	/** Is true if combobox is currently opened in Settings. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "SettingSubWidget", meta = (BlueprintProtected, DisplayName = "Is Combobox Opened"))
	bool bIsComboboxOpenedInternal = false;

	/** The combobox setting data. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "SettingSubWidget", meta = (BlueprintProtected, DisplayName = "Combobox Data"))
	FSettingsCombobox ComboboxDataInternal;

	/*********************************************************************************************
	 * Data
	 ********************************************************************************************* */
protected:
	/** Called after the underlying slate widget is constructed.
	 * May be called multiple times due to adding and removing from the hierarchy. */
	virtual void NativeConstruct() override;

	/** Is executed every tick when widget is enabled. */
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/*********************************************************************************************
	 * Events
	 ********************************************************************************************* */
protected:
	/** Called when a new item is selected in the combobox
	 * @see USettingCheckbox::ComboboxWidgetInternal */
	UFUNCTION(BlueprintCallable, Category = "SettingSubWidget", meta = (BlueprintProtected))
	void OnSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	/** Called when the combobox is opened or closed. */
	UFUNCTION(BlueprintCallable, Category = "SettingSubWidget", meta = (BlueprintProtected))
	void OnMenuOpenChanged();

	/** Is overridden to construct the combobox. */
	virtual void OnAddSetting(const FSettingsPicker& Setting) override;
};

/**
 * The sub-widget of Slider settings.
 */
UCLASS()
class SETTINGSWIDGETCONSTRUCTOR_API USettingSlider : public USettingSubWidget
{
	GENERATED_BODY()

public:
	/** Returns the actual slider widget of this setting. */
	UFUNCTION(BlueprintPure, Category = "SettingSubWidget")
	FORCEINLINE class USlider* GetSliderWidget() const { return SliderWidget; }

	/** Returns the slate slider. */
	FORCEINLINE TSharedPtr<class SSlider> GetSlateSlider() const { return SlateSliderInternal.Pin(); }

	/** Returns the slider setting data. */
	UFUNCTION(BlueprintPure, Category = "SettingSubWidget")
	const FORCEINLINE FSettingsSlider& GetSliderData() const { return SliderDataInternal; }

	/** Set the new slider setting data for this widget. */
	UFUNCTION(BlueprintCallable, Category = "SettingSubWidget", meta = (AutoCreateRefTerm = "InSliderData"))
	void SetSliderData(const FSettingsSlider& InSliderData);

	/*********************************************************************************************
	 * Data
	 ********************************************************************************************* */
protected:
	/** The slate slider.*/
	TWeakPtr<class SSlider> SlateSliderInternal = nullptr;

	/** The actual slider widget of this setting. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "SettingSubWidget", meta = (BlueprintProtected, BindWidget))
	TObjectPtr<class USlider> SliderWidget = nullptr;

	/** The slider setting data. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "SettingSubWidget", meta = (BlueprintProtected, DisplayName = "Slider Data"))
	FSettingsSlider SliderDataInternal;

	/*********************************************************************************************
	 * Events
	 ********************************************************************************************* */
protected:
	/** Called after the underlying slate widget is constructed.
	 * May be called multiple times due to adding and removing from the hierarchy. */
	virtual void NativeConstruct() override;

	/** Invoked when the mouse is released and a capture ends. */
	UFUNCTION(BlueprintCallable, Category = "SettingSubWidget", meta = (BlueprintProtected))
	void OnMouseCaptureEnd();

	/** Called when the value is changed by slider or typing.
	 * @see USettingCheckbox::SliderWidgetInternal */
	UFUNCTION(BlueprintCallable, Category = "SettingSubWidget", meta = (BlueprintProtected))
	void OnValueChanged(float Value);

	/** Is overridden to construct the slider. */
	virtual void OnAddSetting(const FSettingsPicker& Setting) override;
};

/**
 * The sub-widget of Text Line settings.
 */
UCLASS()
class SETTINGSWIDGETCONSTRUCTOR_API USettingTextLine : public USettingSubWidget
{
	GENERATED_BODY()

public:
	/** Returns the Text Line setting data. */
	UFUNCTION(BlueprintPure, Category = "SettingSubWidget")
	const FORCEINLINE FSettingsTextLine& GetTextLineData() const { return TextLineDataInternal; }

	/** Set the new Text Line setting data for this widget. */
	UFUNCTION(BlueprintCallable, Category = "SettingSubWidget", meta = (AutoCreateRefTerm = "InTextLineData"))
	void SetTextLineData(const FSettingsTextLine& InTextLineData);

protected:
	/** The text line setting data. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "SettingSubWidget", meta = (BlueprintProtected, DisplayName = "Text Line Data"))
	FSettingsTextLine TextLineDataInternal;

	/** Is overridden to construct the text line. */
	virtual void OnAddSetting(const FSettingsPicker& Setting) override;
};

/**
 * The sub-widget of User Input settings.
 */
UCLASS()
class SETTINGSWIDGETCONSTRUCTOR_API USettingUserInput : public USettingSubWidget
{
	GENERATED_BODY()

public:
	/** Returns the actual Editable Text Box widget of this setting. */
	UFUNCTION(BlueprintPure, Category = "SettingSubWidget")
	FORCEINLINE class UEditableTextBox* GetEditableTextBox() const { return EditableTextBox; }

	/** Returns current text set in the Editable Text Box. */
	UFUNCTION(BlueprintPure, Category = "SettingSubWidget")
	void GetEditableText(FText& OutText) const;

	/** Set new text programmatically instead of by the user. */
	UFUNCTION(BlueprintCallable, Category = "SettingSubWidget", meta = (AutoCreateRefTerm = "InText"))
	void SetEditableText(const FText& InText);

	/** Returns the slate editable text box. */
	FORCEINLINE TSharedPtr<class SEditableTextBox> GetSlateEditableTextBox() const { return SlateEditableTextBoxInternal.Pin(); }

	/** Returns the user input setting data. */
	UFUNCTION(BlueprintPure, Category = "SettingSubWidget")
	const FORCEINLINE FSettingsUserInput& GetUserInputData() const { return UserInputDataInternal; }

	/** Set the new user input setting data for this widget. */
	UFUNCTION(BlueprintCallable, Category = "SettingSubWidget", meta = (AutoCreateRefTerm = "InUserInputData"))
	void SetUserInputData(const FSettingsUserInput& InUserInputData);

	/*********************************************************************************************
	 * Data
	 ********************************************************************************************* */
protected:
	/** The slate editable text box.*/
	TWeakPtr<class SEditableTextBox> SlateEditableTextBoxInternal = nullptr;

	/** The actual Editable Text Box widget of this setting. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "SettingSubWidget", meta = (BlueprintProtected, BindWidget))
	TObjectPtr<class UEditableTextBox> EditableTextBox = nullptr;

	/** The user input setting data. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "SettingSubWidget", meta = (BlueprintProtected, DisplayName = "User Input Data"))
	FSettingsUserInput UserInputDataInternal;

	/*********************************************************************************************
	 * Events
	 ********************************************************************************************* */
protected:
	/** Called after the underlying slate widget is constructed.
	* May be called multiple times due to adding and removing from the hierarchy. */
	virtual void NativeConstruct() override;

	/** Called whenever the text is changed programmatically or interactively by the user.
	 * @see USettingCheckbox::EditableTextBoxInternal */
	UFUNCTION(BlueprintCallable, Category = "SettingSubWidget", meta = (BlueprintProtected, AutoCreateRefTerm = "Text"))
	void OnTextChanged(const FText& Text);

	/** Is overridden to construct the user input. */
	virtual void OnAddSetting(const FSettingsPicker& Setting) override;
};

/**
 * The sub-widget of the custom widget settings.
 */
UCLASS()
class SETTINGSWIDGETCONSTRUCTOR_API USettingCustomWidget : public USettingSubWidget
{
	GENERATED_BODY()

public:
	/** Returns the custom widget setting data. */
	UFUNCTION(BlueprintPure, Category = "SettingSubWidget")
	const FORCEINLINE FSettingsCustomWidget& GetCustomWidgetData() const { return CustomWidgetDataInternal; }

	/** Set the new custom widget setting data for this widget. */
	UFUNCTION(BlueprintCallable, Category = "SettingSubWidget", meta = (AutoCreateRefTerm = "InCustomWidgetData"))
	void SetCustomWidgetData(const FSettingsCustomWidget& InCustomWidgetData);

protected:
	/** The custom widget setting data. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "SettingSubWidget", meta = (BlueprintProtected, DisplayName = "Custom Widget Data"))
	FSettingsCustomWidget CustomWidgetDataInternal;

	/** Is overridden to construct the custom widget. */
	virtual void OnAddSetting(const FSettingsPicker& Setting) override;
};

/**
* The sub-widget of the Scrollbox widget settings.
 */
UCLASS()
class SETTINGSWIDGETCONSTRUCTOR_API USettingScrollBox : public USettingSubWidget
{
	GENERATED_BODY()

public:
	/** Returns the actual ScrollBox widget of this setting. */
	UFUNCTION(BlueprintPure, Category = "SettingSubWidget")
	FORCEINLINE class UScrollBox* GetScrollBoxWidget() const { return ScrollBoxWidget; }

	/** Returns the slate ScrollBox. */
	FORCEINLINE TSharedPtr<class SScrollBox> GetSlateScrollBox() const { return SlateScrollBoxInternal.Pin(); }

protected:
	/** The slate ScrollBox.*/
	TWeakPtr<class SScrollBox> SlateScrollBoxInternal = nullptr;

	/** The actual ScrollBox widget of this setting. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "SettingSubWidget", meta = (BlueprintProtected, BindWidget))
	TObjectPtr<class UScrollBox> ScrollBoxWidget = nullptr;

	/** Called after the underlying slate widget is constructed.
	 * May be called multiple times due to adding and removing from the hierarchy. */
	virtual void NativeConstruct() override;
};

/**
 * Represents the ToolTip widget that is shown when the mouse is over the setting.
 */
UCLASS()
class SETTINGSWIDGETCONSTRUCTOR_API USettingTooltip : public USettingSubWidget
{
	GENERATED_BODY()

public:
};
