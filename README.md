# Animation Production Pipeline Using Unreal Engine

## Preface
- This is a tool for automating FBX import and Sequence creation for animation production in Unreal Engine.
- It is developed as an Editor Toolbar Extension Plugin.

<img src="./Images/image_preface1.png" width="100%"/>
<img src="./Images/image_preface2.png" width="100%"/>

## FBX Importer
```cpp
void TubaNMenuTool::ImportFBX(TArray<FString> path)
{
	//`Get all fbx files from specific dir
	Params::FAssetData assetData;
	//assetData.GetFBXPath();

	//GetAllPathsFromDir(assetData);

	//`Import fbx files
	assetData.OutPathArray = path;
	bool isAssetExist = assetData.OutPathArray.Num() > 0 ? true : false;
	if (isAssetExist == true)
	{
		InitAssetInfoArray(assetData.OutPathArray);
		TArray<UAssetImportTask*> importTaskArray = CreateImportTask(AssetInfoArray, true);
		if (importTaskArray.Num() == 0)
		{
			LOG(Warning, TEXT("Import task array is empty"));
			LOG_SERVER(Warning, TEXT("Import task array is empty"));
			return;
		}

		TArray<FString> applyStateArray;
		for (auto& importTask : importTaskArray)
		{
			bool isImportSucceed = IsValid(importTask);
			if (isImportSucceed == true)
			{
				LOG(Display, TEXT("%s : Import succeeded"), *importTask->DestinationName);

				applyStateArray.Add(importTask->DestinationName + Common::ExtensionFbx);
			}
			else
			{
				LOG(Warning, TEXT("%s : Import failed"), *importTask->DestinationName);
				LOG_SERVER(Warning, TEXT("%s : Import failed"), *importTask->DestinationName);
			}
		}
		assetTools->ImportAssetTasks(importTaskArray);
		OnFBXImported.ExecuteIfBound(applyStateArray);
	}
}
```

## Sequence Generator

## Packet Manager for Web Server

## Usage Examples
<img src="./Images/image_toolUI1.png" width="100%"/>
<img src="./Images/image_toolUI2.png" width="100%"/>
