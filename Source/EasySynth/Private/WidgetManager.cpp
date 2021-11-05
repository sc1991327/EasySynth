// Copyright Ydrive 2021

#include "WidgetManager.h"

#include "LevelSequence.h"
#include "PropertyCustomizationHelpers.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"


const FText FWidgetManager::StartRenderingErrorMessageBoxTitle = FText::FromString(TEXT("Could not start rendering"));

TSharedRef<SDockTab> FWidgetManager::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SScrollBox)
			+SScrollBox::Slot()
			[
				SNew(STextBlock)
				.Text(FText::FromString("Pick sequencer"))
			]
			+SScrollBox::Slot()
			[
				SNew(SObjectPropertyEntryBox)
				.AllowedClass(ULevelSequence::StaticClass())
				.ObjectPath_Raw(this, &FWidgetManager::GetSequencerPath)
				.OnObjectChanged_Raw(this, &FWidgetManager::OnSequencerSelected)
				.AllowClear(true)
				.DisplayUseSelected(true)
				.DisplayBrowse(true)
			]
			+SScrollBox::Slot()
			[
				SNew(STextBlock)
				.Text(FText::FromString("Chose targets to be rendered"))
			]
			+SScrollBox::Slot()
			[
				SNew(SCheckBox)
				.OnCheckStateChanged_Raw(this, &FWidgetManager::OnRenderColorImagesChanged)
				[
					SNew(STextBlock)
					.Text(FText::FromString("Color images"))
				]
			]
			+SScrollBox::Slot()
			[
				SNew(SCheckBox)
				.OnCheckStateChanged_Raw(this, &FWidgetManager::OnRenderDepthImagesChanged)
				[
					SNew(STextBlock)
					.Text(FText::FromString("Depth images"))
				]
			]
			+SScrollBox::Slot()
			[
				SNew(SCheckBox)
				.OnCheckStateChanged_Raw(this, &FWidgetManager::OnRenderNormalImagesChanged)
				[
					SNew(STextBlock)
					.Text(FText::FromString("Normal images"))
				]
			]
			+SScrollBox::Slot()
			[
				SNew(SCheckBox)
				.OnCheckStateChanged_Raw(this, &FWidgetManager::OnRenderSemanticImagesChanged)
				[
					SNew(STextBlock)
					.Text(FText::FromString("Semantic images"))
				]
			]
			+SScrollBox::Slot()
			[
				SNew(SButton)
				.OnClicked_Raw(this, &FWidgetManager::OnRenderImagesClicked)
				.Content()
				[
					SNew(STextBlock)
					.Text(FText::FromString("Render Images"))
				]
			]
		];
}

void FWidgetManager::OnSequencerSelected(const FAssetData& AssetData)
{
	LevelSequenceAssetData = AssetData;
}

FString FWidgetManager::GetSequencerPath() const
{
	if (LevelSequenceAssetData.IsValid())
	{
		return LevelSequenceAssetData.ObjectPath.ToString();
	}
	return "";
}

void FWidgetManager::OnRenderColorImagesChanged(ECheckBoxState NewState)
{
	SequenceRendererTargets.bColorImages = (NewState == ECheckBoxState::Checked);
}

void FWidgetManager::OnRenderDepthImagesChanged(ECheckBoxState NewState)
{
	SequenceRendererTargets.bDepthImages = (NewState == ECheckBoxState::Checked);
}

void FWidgetManager::OnRenderNormalImagesChanged(ECheckBoxState NewState)
{
	SequenceRendererTargets.bNormalImages = (NewState == ECheckBoxState::Checked);
}

void FWidgetManager::OnRenderSemanticImagesChanged(ECheckBoxState NewState)
{
	SequenceRendererTargets.bSemanticImages = (NewState == ECheckBoxState::Checked);
}

FReply FWidgetManager::OnRenderImagesClicked()
{
	ULevelSequence* LevelSequence = Cast<ULevelSequence>(LevelSequenceAssetData.GetAsset());
	// Make a copy of the SequenceRendererTargets to avoid
	// them being changed through the UI during rendering
	if (!SequenceRenderer.RenderSequence(LevelSequence, SequenceRendererTargets))
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			FText::FromString(*SequenceRenderer.GetErrorMessage()),
			&StartRenderingErrorMessageBoxTitle);
	}
	return FReply::Handled();
}
