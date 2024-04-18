// Copyright (c) 2022 YDrive Inc. All rights reserved.

#include "Widgets/WidgetManager.h"

#include "EasySynth.h"
#include "TextureStyles/TextureStyleManager.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "EditorAssetLibrary.h"
#include "LevelSequence.h"
#include "PropertyCustomizationHelpers.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Input/SDirectoryPicker.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/WidgetStateAsset.h"


const FString FWidgetManager::TextureStyleColorName(TEXT("Original color textures"));
const FString FWidgetManager::TextureStyleSemanticName(TEXT("Semantic color textures"));
const FIntPoint FWidgetManager::DefaultOutputImageResolution(1920, 1080);

#define LOCTEXT_NAMESPACE "FWidgetManager"

FWidgetManager::FWidgetManager()
{
	// Create the texture style manager and add it to the root to avoid garbage collection
	TextureStyleManager = NewObject<UTextureStyleManager>();
	check(TextureStyleManager);
	TextureStyleManager->AddToRoot();
	// Register the semantic classes updated callback
	TextureStyleManager->OnSemanticClassesUpdated().AddRaw(this, &FWidgetManager::OnSemanticClassesUpdated);

	// No need to ever release the TextureStyleManager and the SequenceRenderer,
	// as the FWidgetManager lives as long as the plugin inside the editor

	// Prepare content of the texture style checkout combo box
	TextureStyleNames.Add(MakeShared<FString>(TextureStyleColorName));
	TextureStyleNames.Add(MakeShared<FString>(TextureStyleSemanticName));

	// Initialize SemanticClassesWidgetManager
	SemanticsWidget.SetTextureStyleManager(TextureStyleManager);
}

TSharedRef<SDockTab> FWidgetManager::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	// Bind events now that the editor has finished starting up
	TextureStyleManager->BindEvents();

	// Update combo box semantic class names
	OnSemanticClassesUpdated();

	// Generate the UI
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		.ContentPadding(2)
		[
			SNew(SScrollBox)
			+SScrollBox::Slot()
			.Padding(0, 2, 0, 2)
			[
				SNew(SSeparator)
			]
			+SScrollBox::Slot()
			.Padding(2)
			[
				SNew(SButton)
				.OnClicked_Raw(&SemanticsWidget, &FSemanticClassesWidgetManager::OnManageSemanticClassesClicked)
				.Content()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("ManageSemanticClassesButtonText", "Manage Semantic Classes"))
				]
			]
			+SScrollBox::Slot()
			.Padding(2)
			[
				SNew(SButton)
				.OnClicked_Raw(this, &FWidgetManager::OnPickSemanticByTagsClicked)
				.Content()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("PickSemanticByTagsButtonText", "Pick semantic by tags"))
				]
			]
			+ SScrollBox::Slot()
			.Padding(2)
			[
				SNew(SObjectPropertyEntryBox)
					.AllowedClass(UDataTable::StaticClass())
					.ObjectPath_Raw(this, &FWidgetManager::GetDataTablePath)
					.OnObjectChanged_Raw(this, &FWidgetManager::OnDataTableSelected)
					.AllowClear(true)
					.DisplayUseSelected(true)
					.DisplayBrowse(true)
			]
			+SScrollBox::Slot()
			.Padding(2)
			[
				SNew(SButton)
				.OnClicked_Raw(this, &FWidgetManager::OnPickSemanticByDataTableClicked)
				.Content()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("PickSemanticByDataTableButtonText", "Pick semantic by data table"))
				]
			]
			+SScrollBox::Slot()
			.Padding(2)
			[
				SAssignNew(SemanticClassComboBox, SComboBox<TSharedPtr<FString>>)
				.OptionsSource(&SemanticClassNames)
				.ContentPadding(2)
				.OnGenerateWidget_Lambda(
					[](TSharedPtr<FString> StringItem)
					{ return SNew(STextBlock).Text(FText::FromString(*StringItem)); })
				.OnSelectionChanged_Raw(this, &FWidgetManager::OnSemanticClassComboBoxSelectionChanged)
				.Content()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("PickSemanticClassComboBoxText", "Pick a semantic class"))
				]
			]
			+SScrollBox::Slot()
			.Padding(2)
			[
				SNew(SComboBox<TSharedPtr<FString>>)
				.OptionsSource(&TextureStyleNames)
				.ContentPadding(2)
				.OnGenerateWidget_Lambda(
					[](TSharedPtr<FString> StringItem)
					{ return SNew(STextBlock).Text(FText::FromString(*StringItem)); })
				.OnSelectionChanged_Raw(this, &FWidgetManager::OnTextureStyleComboBoxSelectionChanged)
				.Content()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("PickMeshTextureStyleComboBoxText", "Pick a mesh texture style"))
				]
			]
		];
}

void FWidgetManager::OnSemanticClassComboBoxSelectionChanged(
	TSharedPtr<FString> StringItem,
	ESelectInfo::Type SelectInfo)
{
	if (StringItem.IsValid())
	{
		UE_LOG(LogEasySynth, Log, TEXT("%s: Semantic class selected: %s"), *FString(__FUNCTION__), **StringItem)
		TextureStyleManager->ApplySemanticClassToSelectedActors(*StringItem);
		SemanticClassComboBox->ClearSelection();
	}
}

void FWidgetManager::OnTextureStyleComboBoxSelectionChanged(
	TSharedPtr<FString> StringItem,
	ESelectInfo::Type SelectInfo)
{
	if (StringItem.IsValid())
	{
		UE_LOG(LogEasySynth, Log, TEXT("%s: Texture style selected: %s"), *FString(__FUNCTION__), **StringItem)
		if (*StringItem == TextureStyleColorName)
		{
			TextureStyleManager->CheckoutTextureStyle(ETextureStyle::COLOR);
		}
		else if (*StringItem == TextureStyleSemanticName)
		{
			TextureStyleManager->CheckoutTextureStyle(ETextureStyle::SEMANTIC);
		}
		else
		{
			UE_LOG(LogEasySynth, Error, TEXT("%s: Got unexpected texture style: %s"),
				*FString(__FUNCTION__), **StringItem);
		}
	}
}

FString FWidgetManager::GetDataTablePath() const
{
	if (DataTableAssetData.IsValid())
	{
		return DataTableAssetData.ObjectPath.ToString();
	}
	return "";
}

FReply FWidgetManager::OnPickSemanticByTagsClicked()
{

	UE_LOG(LogEasySynth, Log, TEXT("%s: pick tag button clicked"), *FString(__FUNCTION__))
	TextureStyleManager->ApplySemanticClassToTagedActors();
	SemanticClassComboBox->ClearSelection();

	return FReply::Handled();
}

FReply FWidgetManager::OnPickSemanticByDataTableClicked()
{
	UE_LOG(LogEasySynth, Log, TEXT("%s: pick data table button clicked"), *FString(__FUNCTION__))


	// Update the data table.
	UDataTable* CurrDataTable = Cast<UDataTable>(DataTableAssetData.GetAsset());
	if (CurrDataTable && CurrDataTable->GetRowStruct()->IsChildOf(FMeshSemanticTableRowBase::StaticStruct()))
	{
		FString ContextString(TEXT("FWidgetManager::OnPickSemanticByDataTableClicked"));
		TArray<FMeshSemanticTableRowBase*> MeshSemanticTableRows;
		CurrDataTable->GetAllRows(ContextString, MeshSemanticTableRows);

		for (const FMeshSemanticTableRowBase* MeshSemanticRow : MeshSemanticTableRows)
		{
			TextureStyleManager->ApplySemanticClassToDataTableActors(*MeshSemanticRow);
		}
	}
	SemanticClassComboBox->ClearSelection();

	return FReply::Handled();
}

void FWidgetManager::OnSemanticClassesUpdated()
{
	// Refresh the list of semantic classes
	SemanticClassNames.Reset();
	TArray<FString> ClassNames = TextureStyleManager->SemanticClassNames();
	for (const FString& ClassName : ClassNames)
	{
		SemanticClassNames.Add(MakeShared<FString>(ClassName));
	}

	// Refresh the combo box
	if (SemanticClassComboBox.IsValid())
	{
		SemanticClassComboBox->RefreshOptions();
	}
	else
	{
		UE_LOG(LogEasySynth, Error, TEXT("%s: Semantic class picker is invalid, could not refresh"),
			*FString(__FUNCTION__));
	}
}

#undef LOCTEXT_NAMESPACE
