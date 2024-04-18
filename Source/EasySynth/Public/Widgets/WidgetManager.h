// Copyright (c) 2022 YDrive Inc. All rights reserved.

#pragma once

#include "CoreMinimal.h"

#include "TextureStyles/SemanticCsvInterface.h"
#include "Widgets/SemanticClassesWidgetManager.h"

class ULevelSequence;

class UTextureStyleManager;
class UWidgetStateAsset;


/**
 * Class that manages main UI widget interaction
*/
class FWidgetManager
{
public:
	FWidgetManager();

	/** Handles the UI tab creation when requested */
	TSharedRef<SDockTab> OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs);

private:
	/**
	 * Main plugin widget handlers
	*/

	/** Handles manage semantic classes button click */
	FReply OnManageSemanticClassesClicked();

	/** Handles render images button click */
	FReply OnPickSemanticByTagsClicked();

	FReply OnPickSemanticByDataTableClicked();

	/** Callback function handling the choosing of the semantic class inside the combo box */
	void OnSemanticClassComboBoxSelectionChanged(TSharedPtr<FString> StringItem, ESelectInfo::Type SelectInfo);

	/** Callback function handling the choosing of the texture style inside the combo box */
	void OnTextureStyleComboBoxSelectionChanged(TSharedPtr<FString> StringItem, ESelectInfo::Type SelectInfo);

	/** Callback function handling the update of the selected datatable */
	void OnDataTableSelected(const FAssetData& AssetData) { DataTableAssetData = AssetData; }

	FString GetDataTablePath() const;

	/** Callback function handling the update of the output directory */
	void OnOutputDirectoryChanged(const FString& Directory) { OutputDirectory = Directory; }

	/** Handles the semantic classes updated event */
	void OnSemanticClassesUpdated();

	/**
	 * Local members
	*/

	/** Interface that handles importing semantic classes from CSV */
	FSemanticCsvInterface SemanticCsvInterface;

	/** Manager that handles semantic class widget */
	FSemanticClassesWidgetManager SemanticsWidget;

	/** FStrings semantic class names referenced by the combo box */
	TArray<TSharedPtr<FString>> SemanticClassNames;

	/** Semantic class combo box */
	TSharedPtr<SComboBox<TSharedPtr<FString>>> SemanticClassComboBox;

	/** FStrings texture style names referenced by the combo box */
	TArray<TSharedPtr<FString>> TextureStyleNames;

	/** FStrings output image format names referenced by the combo box */
	TArray<TSharedPtr<FString>> OutputFormatNames;

	/** Currently selected sequencer asset data */
	FAssetData LevelSequenceAssetData;

	FAssetData DataTableAssetData;

	/** Output image resolution, with the image size always being an even number */
	FIntPoint OutputImageResolution;

	/** Currently selected output directory */
	FString OutputDirectory;

	/**
	 * Module that manages default color and semantic texture styles,
	 * must be added to the root to avoid garbage collection
	*/
	UTextureStyleManager* TextureStyleManager;

	/** The name of the texture style representing original colors */
	static const FString TextureStyleColorName;

	/** The name of the texture style representing semantic colors */
	static const FString TextureStyleSemanticName;

	/** The name of the JPEG output format */
	static const FString JpegFormatName;

	/** The name of the PNG output format */
	static const FString PngFormatName;

	/** The name of the EXR output format */
	static const FString ExrFormatName;

	/** Default output image resolution */
	static const FIntPoint DefaultOutputImageResolution;
};
