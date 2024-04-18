// Copyright (c) 2022 YDrive Inc. All rights reserved.

#include "TextureStyles/SemanticCsvInterface.h"

#include "Serialization/Csv/CsvParser.h"

#include "EasySynth.h"
#include "IDesktopPlatform.h"
#include "DesktopPlatformModule.h"
#include "PathUtils.h"
#include "TextureStyles/TextureMappingAsset.h"
#include "TextureStyles/TextureStyleManager.h"


#define LOCTEXT_NAMESPACE "FSemanticCsvInterface"

bool FSemanticCsvInterface::ExportSemanticClasses(const FString& OutputDir, UTextureMappingAsset* TextureMappingAsset)
{
	TArray<FString> Lines;

	for (auto Element : TextureMappingAsset->SemanticClasses)
	{
		const FSemanticClass& Class = Element.Value;
		Lines.Add(FString::Printf(TEXT("%s,%d,%d,%d"), *Class.Name, Class.Color.R, Class.Color.G, Class.Color.B));
	}

	// Save the file
	const FString SaveFilePath = FPathUtils::SemanticClassesFilePath(OutputDir);
	if (!FFileHelper::SaveStringArrayToFile(
		Lines,
		*SaveFilePath,
		FFileHelper::EEncodingOptions::AutoDetect,
		&IFileManager::Get(),
		EFileWrite::FILEWRITE_None))
	{
		UE_LOG(LogEasySynth, Error, TEXT("%s: Failed while saving the file %s"), *FString(__FUNCTION__), *SaveFilePath)
		return false;
	}

	return true;
}

#undef LOCTEXT_NAMESPACE
