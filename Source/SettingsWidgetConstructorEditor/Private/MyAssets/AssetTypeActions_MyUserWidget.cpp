﻿// Copyright (c) Yevhenii Selivanov

#include "MyAssets/SWCAssetTypeActions_MyUserWidget.h"
//---
#include "UMGEditorProjectSettings.h"
#include "WidgetBlueprint.h"
#include "WidgetBlueprintEditor.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Misc/MessageDialog.h"

void USWCMyUserWidgetBlueprint::GetReparentingRules(TSet<const UClass*>& AllowedChildrenOfClasses, TSet<const UClass*>& DisallowedChildrenOfClasses) const
{
	check(ParentClass);
	AllowedChildrenOfClasses.Empty();
	AllowedChildrenOfClasses.Add(ParentClass);
}

USWCMyUserWidgetFactory::USWCMyUserWidgetFactory()
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = USWCMyUserWidgetBlueprint::StaticClass();
}

FText USWCMyUserWidgetFactory::GetDisplayName() const
{
	return NSLOCTEXT("AssetTypeActions", "MyUserWidget", "My User Widget");
}

UObject* USWCMyUserWidgetFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext)
{
	// Make sure we are trying to factory a blueprint, then create and init one
	check(Class->IsChildOf(USWCMyUserWidgetBlueprint::StaticClass()));

	const TSubclassOf<UUserWidget> ParentClass = GetWidgetClass();
	if (ParentClass == nullptr || !FKismetEditorUtilities::CanCreateBlueprintOfClass(ParentClass))
	{
		FFormatNamedArguments Args;
		Args.Add(TEXT("ClassName"), ParentClass != nullptr ? FText::FromString(ParentClass->GetName()) : NSLOCTEXT("UnrealEd", "Null", "(null)"));
		FMessageDialog::Open(EAppMsgType::Ok, FText::Format(NSLOCTEXT("UnrealEd", "CannotCreateBlueprintFromClass", "Cannot create a blueprint based on the class '{0}'."), Args));
		return nullptr;
	}

	// If the root widget selection dialog is not enabled, use a canvas panel as the root by default
	TSubclassOf<UPanelWidget> RootWidgetClass = nullptr;
	const UUMGEditorProjectSettings* UMGProjectSettings = GetDefault<UUMGEditorProjectSettings>();
	check(UMGProjectSettings);
	if (!UMGProjectSettings->bUseWidgetTemplateSelector)
	{
		RootWidgetClass = UMGProjectSettings->DefaultRootWidget;
	}

	USWCMyUserWidgetBlueprint* NewBP = CastChecked<USWCMyUserWidgetBlueprint>(FKismetEditorUtilities::CreateBlueprint(ParentClass, InParent, Name, BPTYPE_Normal, USWCMyUserWidgetBlueprint::StaticClass(), UWidgetBlueprintGeneratedClass::StaticClass(), NAME_None));

	// Create the selected root widget
	if (NewBP->WidgetTree->RootWidget == nullptr)
	{
		if (const TSubclassOf<UPanelWidget> RootWidgetPanel = RootWidgetClass)
		{
			UWidget* Root = NewBP->WidgetTree->ConstructWidget<UWidget>(RootWidgetPanel);
			NewBP->WidgetTree->RootWidget = Root;
		}
	}

	return NewBP;
}

UObject* USWCMyUserWidgetFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return FactoryCreateNew(Class, InParent, Name, Flags, Context, Warn, NAME_None);
}

TSubclassOf<UUserWidget> USWCMyUserWidgetFactory::GetWidgetClass() const
{
	return UUserWidget::StaticClass();
}

FText FAssetTypeActions_MyUserWidget::GetName() const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_MyUserWidget", "My User Widget");
}

void FAssetTypeActions_MyUserWidget::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	const EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		USWCMyUserWidgetBlueprint* Blueprint = Cast<USWCMyUserWidgetBlueprint>(*ObjIt);
		if (Blueprint && Blueprint->SkeletonGeneratedClass && Blueprint->GeneratedClass)
		{
			const TSharedRef<FWidgetBlueprintEditor> NewBlueprintEditor(new FWidgetBlueprintEditor());

			TArray<UBlueprint*> Blueprints;
			Blueprints.Add(Blueprint);
			NewBlueprintEditor->InitWidgetBlueprintEditor(Mode, EditWithinLevelEditor, Blueprints, true);
		}
		else
		{
			FMessageDialog::Open(EAppMsgType::Ok, NSLOCTEXT("AssetTypeActions", "FailedToLoadWidgetBlueprint", "Widget Blueprint could not be loaded because it derives from an invalid class.\nCheck to make sure the parent class for this blueprint hasn't been removed!"));
		}
	}
}
