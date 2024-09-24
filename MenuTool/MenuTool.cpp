// Fill out your copyright notice in the Description page of Project Settings.

#include "MenuTool/MenuTool.h"

#include "AssetToolsModule.h"									//`Load asset tools module
#include "AssetImportTask.h"									//`Access UAssetImportTask
#include "AssetRegistry/AssetRegistryModule.h"					//`Register created bp asset
#include "CinematicCamera/Public/CineCameraActor.h"				//`Create new cinecamera
#include "Channels/MovieSceneChannelProxy.h"					//`Access transform channel
#include "ComponentAssetBroker.h"								//`Access componentAssetBrokerage
#include "DesktopPlatformModule.h"								//`Access desktop content browser asset
#include "EditorUtilityWidgetBlueprint.h"						//`Access editor utility widget
#include "EditorUtilitySubsystem.h"								//`Access open editor utility widget
#include "Engine/SimpleConstructionScript.h"					//`Access blueprint construction script
#include "Engine/SCS_Node.h"									//`Access blueprint component
#include "EditorReimportHandler.h"								//`Access reimport method
#include "Factories/FbxImportUI.h"								//`Access Unreal fbx import UI
#include "Factories/FbxSkeletalMeshImportData.h"				//`Access fbx import settings(skeletal mesh)
#include "FbxImporter.h"										//'Access Unreal fbx importer
#include "KismetCompilerModule.h"								//`Access kismetCompiler module
#include "Kismet2/KismetEditorUtilities.h"						// create bp using asset
#include "LevelSequenceActor.h"									//`Create Levelsequence
#include "MovieSceneTrack.h"									//`Access master track
#include "Sections/MovieSceneSubSection.h"						//`Access LS subseq
#include "Sections/MovieSceneCinematicShotSection.h"			//`Access CS shots
#include "Tracks/MovieSceneCameraCutTrack.h"					//`Access CC track
#include "Tracks/MovieSceneCinematicShotTrack.h"				//`Access CS track
#include "Tracks/MovieSceneSpawnTrack.h"						//`Access spawnable track
#include "Tracks/MovieScene3DTransformTrack.h"					//`Access 3D transform track
#include "ILevelSequenceEditorToolkit.h"						//`Access sequence editor
#include "SequencerTools.h"										//`Access level sequence importer

//#include "LevelSequence/Private/LevelSequenceModule.h"			//`Access object spawners
//#include "Sequencer/Private/SequencerCommonHelpers.h"				//`Aceess camera importer
//#include "ISequencer.h"											//`Access sequencer
//#include "ISequencerModule.h"
//#include "CineCameraComponent.h"
//#include "Tracks/MovieSceneFloatTrack.h"
//#include "MovieSceneCommonHelpers.h"
//#include "MovieSceneToolHelpers.h"

TubaNMenuTool::TubaNMenuTool()
{
	assetTools = &FModuleManager::GetModuleChecked<FAssetToolsModule>(TEXT("assetTools")).Get();
}

void TubaNMenuTool::OnStartupModule()
{
	commandList = MakeShareable(new FUICommandList);

	MenuCommand::Register();
	MapCommands();

	// 중요..! EditorMenuTool.cpp의 FillPulldownMenu 함수 내에 AddMenuSeparator에서 추가한 이름(Common::CategoryPipeline)과 동일 해야 함.
	FEditorMenuTool::Get().AddMenuExtension(FMenuExtensionDelegate::CreateRaw(this, &TubaNMenuTool::MakeMenuEntry), FName(Common::CategoryPipeline), commandList);

	LOG_S(Display);
}

void TubaNMenuTool::OnShutdownModule()
{
	MenuCommand::Unregister();

	LOG_S(Display);
}

void TubaNMenuTool::MakeMenuEntry(FMenuBuilder& menuBuilder)
{
	// 코드 순서에 따라 메뉴 순서 변경 됨.
	menuBuilder.AddSubMenu(
		FText::FromString("Import Menu"),
		FText::FromString("This is Import menu"),
		FNewMenuDelegate::CreateSP(this, &TubaNMenuTool::MakeSubMenu)
	);

	menuBuilder.AddMenuEntry(MenuCommand::Get().CreateWidget);

	LOG_S(Display);
}

void TubaNMenuTool::MakeSubMenu(FMenuBuilder& menuBuilder)
{
	const MenuCommand& commands = MenuCommand::Get();

	menuBuilder.AddMenuEntry(commands.TestFunction);
	menuBuilder.AddMenuSeparator();

	menuBuilder.AddMenuEntry(commands.ImportFBX);
	menuBuilder.AddMenuEntry(commands.SequenceGenerator);
	menuBuilder.AddMenuEntry(commands.AllTasks);
}

void TubaNMenuTool::InitAssetInfoArray(TArray<FString>& pathArray)
{
	//** Init */
	if (AssetInfoArray.Num() > 0 == true)
	{
		AssetInfoArray.Empty();
	}

	for (FString& animPath : pathArray)
	{

		TSharedRef<AssetInfo> assetInfo = MakeShared<AssetInfo>();
		assetInfo->Initialize(animPath);
		AssetInfoArray.Add(assetInfo);
	}

	//** Sort */
	AssetInfoArray.Sort([](const TSharedRef<AssetInfo>& lhs, const TSharedRef<AssetInfo>& rhs)
		{
			if (lhs->GetEpisodeNumberStr() == rhs->GetEpisodeNumberStr())
			{
				if (lhs->GetSceneNumberStr() == rhs->GetSceneNumberStr())
				{
					if (lhs->GetCutNumberStr() == rhs->GetCutNumberStr())
					{
						return lhs->GetName() < rhs->GetName();
					}
					else
					{
						return lhs->GetCutNumberStr() < rhs->GetCutNumberStr();
					}
				}
				else
				{
					return lhs->GetSceneNumberStr() < rhs->GetSceneNumberStr();
				}
			}
			else
			{
				return lhs->GetEpisodeNumberStr() < rhs->GetEpisodeNumberStr();
			}
		});
}

void TubaNMenuTool::GetAllPathsFromDir(Params::FAssetData& assetData)
{
	//`testing FindFiles function->
	//if (FPaths::DirectoryExists(InAssetDir))
	//{
	//	FString newPath = InAssetDir + InAssetExtension;
	//	IFileManager::Get().FindFiles(OutPathArray, *newPath, true, false);
	//}

	IFileManager& fileManager = IFileManager::Get();
	const TCHAR* directory = *assetData.InAssetDir;
	DirVisitor visitor;

	if (fileManager.IterateDirectoryRecursively(directory, visitor) == true)
	{
		assetData.OutDirArray = visitor.OutDirArray;

		for (FString& path : visitor.OutPathArray)
		{
			if (path.Contains(assetData.InAssetExtension) == true)
			{
				if (assetData.IsCameraFBX == true)
				{
					if (path.Contains(TEXT("CAM")) == true)
					{
						assetData.OutPathArray.Add(path);
					}
				}
				else
				{
					if (path.Contains(TEXT("CAM")) == false)
					{
						if (assetData.EpisodeNum == "")
						{
							assetData.OutPathArray.Add(path);
						}
						else
						{
							if (path.Contains(assetData.EpisodeNum) == true)
							{
								assetData.OutPathArray.Add(path);
							}
						}
					}
				}
			}
		}
		//`Deprecated.
		//if (assetData.OutDirArray.Num() == 0)
		//{
		//	LOG(Warning, TEXT("Do not exist. Dir : %s"), directory);
		//}
		//else
		//{
		//	LOG(Warning, TEXT("Dir found. Dir : %s"), directory);
		//}
	}
	else
	{
		LOG(Warning, TEXT("Do not exist or visitor returned false. Dir : %s"), directory);
	}
}

void TubaNMenuTool::OnFBXImporter(TArray<FString> pathArray)
{
	FString serverDir = Common::DirServerAsset;

	bool isConnected = CheckServerDir(serverDir);
	if (isConnected == true)
	{
		ImportFBX(pathArray);
	}
	else
	{
		return;
	}
}

void TubaNMenuTool::OnFBXReimporter(TArray<FString> pathArray)
{
	//`Get all files from filtered directory
	//Params::FAssetData assetData;
	//assetData.GetUAssetPath();
	//assetData.InAssetDir = Common::DirContentAnimation + animDir;

	//GetAllPathsFromDir(assetData);

	//InitAssetInfoArray(assetData.OutPathArray);

	InitAssetInfoArray(pathArray);
	ReimportFBX();
}

void TubaNMenuTool::OnFBXImporterDialog()
{
	ImportFBXDialog();
}

void TubaNMenuTool::OnSequenceGenerator()
{
	//`Deprecated
	//`Get all files(anim, cam) from UE contents dir
	Params::FAssetData assetData;
	assetData.GetUAssetPath();

	GetAllPathsFromDir(assetData);

	InitAssetInfoArray(assetData.OutPathArray);
	GenerateSubSequence();
	GenerateMainSequence();

	PrintMessage(EMessageSeverity::Info, TEXT("Generate sequence task completely done. Either succeed or not."));
}

void TubaNMenuTool::OnSequenceGenerator(FString animDir, FString camDir)
{
	//`Get all files from filtered directory
	Params::FAssetData assetData;
	assetData.GetUAssetPath();
	assetData.InAssetDir = Common::DirContentAnimation + animDir;
	cameraDir = camDir;

	GetAllPathsFromDir(assetData);

	if (assetData.OutPathArray.Num() > 0)
	{
		InitAssetInfoArray(assetData.OutPathArray);
		GenerateSubSequence();
		GenerateMainSequence();

		PrintMessage(EMessageSeverity::Info, TEXT("Generate sequence task completely done. Either succeed or not."));
	}
	else
	{
		PrintMessage(EMessageSeverity::Error, TEXT("There are no animation in current directory. ") + assetData.InAssetDir);
	}
}



void TubaNMenuTool::MapCommands()
{
	const MenuCommand& commands = MenuCommand::Get();

	commandList->MapAction(
		commands.CreateWidget,
		FExecuteAction::CreateSP(this, &TubaNMenuTool::OnCreateWidget),
		FCanExecuteAction());

	commandList->MapAction(
		commands.TestFunction,
		FExecuteAction::CreateSP(this, &TubaNMenuTool::TestFunc),
		FCanExecuteAction());

	commandList->MapAction(
		commands.ImportFBX,
		FExecuteAction::CreateSP(this, &TubaNMenuTool::OnFBXImporterDialog),
		FCanExecuteAction());

	commandList->MapAction(
		commands.SequenceGenerator,
		FExecuteAction::CreateSP(this, &TubaNMenuTool::OnSequenceGenerator),
		FCanExecuteAction());

	commandList->MapAction(
		commands.AllTasks,
		FExecuteAction::CreateSP(this, &TubaNMenuTool::OnAllTasks),
		FCanExecuteAction());
}

void TubaNMenuTool::OnCreateWidget()
{
	FString assetPath = Common::EUWBPPath();
	OpenEditorUtilityWidgetBP(assetPath);
}

void TubaNMenuTool::OnCreateBP(TSharedRef<AssetInfo>& inAsset)
{
	FString assetPrefix = inAsset->GetPrefixAsset();

	bool isMaster = assetPrefix == Common::PrefixSKEL ? true : false;
	if (isMaster == true)
	{
		CreateBP(inAsset, false);
	}
}

void TubaNMenuTool::OnCreateBPDialog(TSharedRef<AssetInfo>& inAsset)
{
	CreateBP(inAsset, true);
}

bool TubaNMenuTool::CheckServerDir(FString& serverDir)
{
	if (FPaths::DirectoryExists(serverDir) == true)
	{
		return true;
	}
	else
	{
		LOG(Warning, TEXT("%s : Can not access to server directory"), *serverDir);
		return false;
	}
}

void TubaNMenuTool::OnAllTasks()
{
	//OnFBXImporter();
	OnSequenceGenerator();
}



void TubaNMenuTool::CreateDirectory(FString& directory)
{
	IPlatformFile& platformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (platformFile.DirectoryExists(*directory) == false)
	{
		platformFile.CreateDirectory(*directory);
	}
}

void TubaNMenuTool::CreateBP(TSharedRef<AssetInfo>& inAsset, bool withDialog)
{
	UObject* newAsset = nullptr;

	FString assetDir;
	FString assetName;
	FString assetPath;

	Tie(assetDir, assetName) = inAsset->GetMasterDestinationPath();
	assetPath = assetDir + assetName;

	newAsset = UEditorAssetLibrary::DoesAssetExist(assetPath) == true ? UEditorAssetLibrary::LoadAsset(assetPath) : nullptr;
	CHECK_EXIT(newAsset);

	if (withDialog == true)
	{
		UBlueprint* newBP = FKismetEditorUtilities::CreateBlueprintUsingAsset(newAsset, false);
	}
	else
	{
		// ** Convert mesh asset to actor blueprint(without dialog) */
		TSubclassOf<UActorComponent> componentClass = FComponentAssetBrokerage::GetPrimaryComponentForAsset(newAsset->GetClass());
		if (componentClass != nullptr)
		{
			UClass* blueprintClass = nullptr;
			UClass* blueprintGeneratedClass = nullptr;

			IKismetCompilerInterface& kismetCompilerModule = FModuleManager::LoadModuleChecked<IKismetCompilerInterface>("KismetCompiler");
			kismetCompilerModule.GetBlueprintTypesForClass(AActor::StaticClass(), blueprintClass, blueprintGeneratedClass);

			Tie(assetDir, assetName) = inAsset->GetActorBlueprintPath();
			FString actorPath = assetDir + assetName + Common::SuffixTest;
			if (UEditorAssetLibrary::DoesAssetExist(actorPath) == true)
			{
				UEditorAssetLibrary::DeleteAsset(actorPath);
			}

			FName newBPName(*FPackageName::GetLongPackageAssetName(actorPath));
			UPackage* package = CreatePackage(*actorPath);
			CHECK_EXIT(package);

			UBlueprint* newBP = FKismetEditorUtilities::CreateBlueprint(AActor::StaticClass(), package, newBPName, BPTYPE_Normal, blueprintClass, blueprintGeneratedClass, FName("LevelEditorActions"));
			CHECK_EXIT(newBP);

			//`Register (If we do not register it causes warning.)
			FAssetRegistryModule::AssetCreated(newBP);
			package->MarkPackageDirty();

			//`Create a new SCS node
			CHECK_EXIT(newBP->SimpleConstructionScript);
			USCS_Node* newNode = newBP->SimpleConstructionScript->CreateNode(componentClass);

			//`Assign the asset to the template
			FComponentAssetBrokerage::AssignAssetToComponent(newNode->ComponentTemplate, newAsset);

			//`Add node to the SCS
			newBP->SimpleConstructionScript->AddNode(newNode);
			newNode->SetVariableName(*FPackageName::GetLongPackageAssetName(assetPath));

			//`Recompile skeleton because of the new component we added (and 
			//`broadcast the change to those that care, like the BP node database)
			//`Can not use for now.
			//FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(newBP);

			UEditorAssetLibrary::SaveAsset(actorPath);
		}
	}
}

void TubaNMenuTool::OpenEditorUtilityWidgetBP(FString& assetPath)
{
	UEditorUtilityWidgetBlueprint* WBP = LoadObject<UEditorUtilityWidgetBlueprint>(nullptr, *assetPath); //Reference to your EditorUtilityWidget
	CHECK_EXIT(WBP);

	UEditorUtilityWidget* toolEditor = GEditor->GetEditorSubsystem<UEditorUtilitySubsystem>()->SpawnAndRegisterTab(WBP);
	//toolEditor->SetToolTipText(FText::FromString(""));

	// Modelled on the  UEditorUtilityWidgetBlueprint::CreateUtilityWidget()  Do it again 
	//TSharedRef<SWidget> TabWidget = SNullWidget::NullWidget;
	//{
	//
	//	UEditorUtilityWidget* createdUMGWidget = nullptr;// Created UMG Control 
	//
	//	UClass* blueprintClass = WBP->GeneratedClass;
	//	TSubclassOf<UEditorUtilityWidget> WidgetClass = blueprintClass;
	//	UWorld* world = GEditor->GetEditorWorldContext().World();
	//	if (world)
	//	{
	//
	//		if (createdUMGWidget)
	//		{
	//
	//			createdUMGWidget->Rename(nullptr, GetTransientPackage());
	//		}
	//		createdUMGWidget = CreateWidget<UEditorUtilityWidget>(world, WidgetClass);
	//	}
	//	if (createdUMGWidget)
	//	{
	//
	//		TabWidget = SNew(SVerticalBox)
	//			+ SVerticalBox::Slot()
	//			.HAlign(HAlign_Fill)
	//			[
	//				createdUMGWidget->TakeWidget()
	//			];
	//	}
	//}
}

TArray<UAssetImportTask*> TubaNMenuTool::CreateImportTask(TArray<TSharedRef<AssetInfo>>& inAssetArray, bool willSave)
{
	TArray<UAssetImportTask*> importTaskArray;

	for (TSharedRef<AssetInfo>& inAsset : inAssetArray)
	{
		if (inAsset->IsValidInfo() == false)
		{
			if (inAsset->GetType() == Common::PrefixCAM)
			{
				continue;
			}
			else
			{
				LOG(Warning, TEXT("%s >> This format is not supported"), *inAsset->GetBaseFilename());
				LOG_SERVER(Warning, TEXT("%s >> This format is not supported"), *inAsset->GetBaseFilename());
				continue;
			}
		}
		else
		{
			if (Common::ExtensionAbc.Contains(inAsset->GetExtension()))
			{
				LOG(Display, TEXT("Call abc importer"));

				//todo : It only support selected asset with dialog. Fix destination directory.
				//TArray<FString> array;
				//array.Add(inAsset->GetOriginPath());

				//FString dir;
				//FString name;

				//Tie(dir, name) = inAsset->GetAnimationGamePath();
				//FString path = dir + name;

				//assetTools->ImportAssets(array, path);

				continue;
			}
		}

		FString assetDir;
		FString assetName;
		FString assetPrefix = inAsset->GetPrefixAsset();
		bool isMaster = assetPrefix == Common::PrefixSKEL ? true : false;

		UAssetImportTask* task = NewObject<UAssetImportTask>();
		task->Options = CreateImportOptions(inAsset, isMaster);
		task->bSave = willSave;
		task->bAutomated = true;
		task->bReplaceExisting = true;
		task->DestinationName = TEXT_EMPTY;
		task->Filename = inAsset->GetOriginPath();

		if (isMaster == true)
		{
			Tie(assetDir, assetName) = inAsset->GetMasterDestinationPath();
			task->DestinationPath = assetDir;
			task->DestinationName = assetName;

			FString outStr = isMaster ? TEXT("isMain") : TEXT("isSub");
			LOG(Warning, TEXT("%s = %s"), *assetName, *outStr);
		}
		else
		{
			Tie(assetDir, assetName) = inAsset->GetAnimationContentPath();
			task->DestinationPath = assetDir;
			task->DestinationName = assetName;
		}

		bool isOptionCreated = IsValid(task->Options);
		if (isOptionCreated == false)
		{
			LOG(Error, TEXT("%s : Import option did not created"), *task->DestinationName);
			LOG_SERVER(Error, TEXT("%s : Import option did not created"), *task->DestinationName);
			continue;
		}

		bool isFileExist = FPlatformFileManager::Get().GetPlatformFile().FileExists(*task->Filename);
		if (isFileExist == false)
		{
			LOG(Error, TEXT("%s : File does not exist"), *task->DestinationName);
			LOG_SERVER(Error, TEXT("%s : File does not exist"), *task->DestinationName);
			continue;
		}

		importTaskArray.Add(task);
	}

	return importTaskArray;
}

UFbxImportUI* TubaNMenuTool::CreateImportOptions(TSharedRef<AssetInfo>& inAsset, bool isMaster)
{
	UFbxImportUI* importUI = NewObject<UFbxImportUI>();

	importUI->bAutomatedImportShouldDetectType = false;

	if (inAsset->GetPrefixAsset() == Common::PrefixSKEL)
	{
		importUI->MeshTypeToImport = EFBXImportType::FBXIT_SkeletalMesh;
	}
	else if (inAsset->GetPrefixAsset() == Common::PrefixSM)
	{
		importUI->MeshTypeToImport = EFBXImportType::FBXIT_StaticMesh;
	}
	else if (inAsset->GetPrefixAsset() == Common::PrefixAS || inAsset->GetPrefixAsset() == Common::PrefixHIGH)
	{
		importUI->MeshTypeToImport = EFBXImportType::FBXIT_Animation;

		FString assetType = inAsset->GetType();
		if (assetType == Common::PrefixCH || assetType == Common::PrefixPROP)
		{
			FString assetDir;
			FString assetName;

			Tie(assetDir, assetName) = inAsset->GetMasterSkeletonPath();
			FString assetPath = assetDir + assetName;
			if (UEditorAssetLibrary::DoesAssetExist(assetPath))
			{
				importUI->Skeleton = (USkeleton*)UEditorAssetLibrary::LoadAsset(assetPath);
			}
			else
			{
				LOG(Error, TEXT("%s : Master skeleton does not exist"), *assetPath);
				LOG_SERVER(Warning, TEXT("%s : Master skeleton does not exist"), *assetPath);
				return nullptr;
			}
		}
	}
	else
	{
		LOG(Error, TEXT("Filename : %s import failed"), *inAsset->GetBaseFilename());
		LOG_SERVER(Error, TEXT("Filename : %s import failed"), *inAsset->GetBaseFilename());
		return nullptr;
	}

	//`These options are determined by FBX type.
	//importUI->bImportAnimations = true;
	//importUI->bImportMesh = false;
	//importUI->bImportMaterials = false;
	//importUI->bCreatePhysicsAsset = false;
	//importUI->bImportTextures = false;

	//`Texture options
	importUI->TextureImportData->MaterialSearchLocation = EMaterialSearchLocation::AllAssets;

	//`Skeletal options
	importUI->SkeletalMeshImportData->bUseT0AsRefPose = true;
	importUI->SkeletalMeshImportData->bImportMorphTargets = true;

	//`Animation options
	//importUI->AnimSequenceImportData->ImportTranslation = FVector(0.0, 0.0, 0.0);
	//importUI->AnimSequenceImportData->ImportRotation = FRotator(0.0, 0.0, 0.0);
	//importUI->AnimSequenceImportData->ImportUniformScale = 1.0f;
	//importUI->AnimSequenceImportData->bRemoveRedundantKeys = false;
	importUI->AnimSequenceImportData->AnimationLength = EFBXAnimationLengthImportType::FBXALIT_AnimatedKey;
	importUI->AnimSequenceImportData->bSnapToClosestFrameBoundary = true;

	return importUI;
}

UMovieSceneUserImportFBXSettings* TubaNMenuTool::CreateImportCameraOptions()
{
	UMovieSceneUserImportFBXSettings* importFBXSettings = GetMutableDefault<UMovieSceneUserImportFBXSettings>();
	//`Default options.
	//bMatchByNameOnly = true;
	//bForceFrontXAxis = false;
	//bCreateCameras = true;
	//bReplaceTransformTrack = true;
	//bReduceKeys = true;
	//ReduceKeysTolerance = 0.001f;
	//bConvertSceneUnit = true;
	//ImportUniformScale = 1.0f;

	importFBXSettings->bMatchByNameOnly = false;
	importFBXSettings->bForceFrontXAxis = false;
	importFBXSettings->bConvertSceneUnit = false;
	importFBXSettings->bCreateCameras = false;
	importFBXSettings->bReplaceTransformTrack = true;

	return importFBXSettings;
}

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

void TubaNMenuTool::ReimportFBX()
{
	LOG_SERVER_S(Display);

	TArray<UObject*> animSeqArray;
	for (TSharedRef<AssetInfo>& assetInfo : AssetInfoArray)
	{
		FString dir;
		FString name;
		Tie(dir, name) = assetInfo->GetAnimationGamePath();

		FString path = dir + name;
		UObject* animAsset = UEditorAssetLibrary::LoadAsset(path);
		animSeqArray.Add(animAsset);

		LOG(Display, TEXT("Path : %s"), *path);
	}

	bool isReimportSucceed = FReimportManager::Instance()->ReimportMultiple(animSeqArray, false, true, L"", nullptr, -1, false, false);
	if (isReimportSucceed == true)
	{
		//LOG(Display, TEXT("Reimport succeed"));
	}
	else
	{
		LOG(Warning, TEXT("Reimport failed"));
		LOG_SERVER(Warning, TEXT("Reimport failed"));
	}
}

void TubaNMenuTool::ImportFBXDialog()
{
	//`Get file name by using dialog.
	Params::FAssetData assetData;

	IDesktopPlatform* desktopPlatform = FDesktopPlatformModule::Get();
	desktopPlatform->OpenFileDialog(
		NULL,
		TEXT("Select FBX file"),
		TEXT_EMPTY,
		TEXT_EMPTY,
		TEXT("FBX File(*.fbx)|*.fbx"),
		EFileDialogFlags::Multiple,
		assetData.OutPathArray
	);

	bool isAssetExist = assetData.OutPathArray.Num() > 0 ? true : false;
	if (isAssetExist == true)
	{
		InitAssetInfoArray(assetData.OutPathArray);
		TArray<UAssetImportTask*> importTaskArray = CreateImportTask(AssetInfoArray, true);
		assetTools->ImportAssetTasks(importTaskArray);

		//`Deprecated.
		//for (TSharedRef<AssetInfo>& assetInfo : AssetInfoArray)
		//{
		//	OnCreateBP(assetInfo);
		//}
	}
}

void TubaNMenuTool::ImportFBXCamera(bool withNotify)
{
	/// todo : Testing fbxImporter. `It will be useful for remove notify slate.
	//UnFbx::FFbxImporter* fbxImporter = UnFbx::FFbxImporter::GetInstance();
	//TMap<FGuid, FString> newBindingMap;
	//for (FSequencerBindingProxy& binding : bindingArray)
	//{
	//	FString newCameraName = movieScene->GetObjectDisplayName(binding.BindingID).ToString();
	//	newBindingMap.Add(binding.BindingID, newCameraName);
	//}
	//if (!fbxImporter->ImportFromFile(*importFilename, fileExtension, true))
	//{
	//	fbxImporter->ReleaseScene();
	//	return;
	//}
	//fbxsdk::FbxScene* scene = fbxImporter->Scene;
	//if (scene == nullptr)
	//{
	//	fbxImporter->ReleaseScene();
	//	return;
	//}
	//FFBXInOutParameters inOutParams;
	//if (!MovieSceneToolHelpers::ReadyFBXForImport(importFilename, importFBXSettings, inOutParams))
	//{
	//	return;
	//}
	//ALevelSequenceActor* outActor;
	//FMovieSceneSequencePlaybackSettings playerSettings;
	//FLevelSequenceCameraSettings cameraSettings;
	//ULevelSequencePlayer* player = ULevelSequencePlayer::CreateLevelSequencePlayer(currentWorld, masterSequence, playerSettings, outActor);
	//player->Initialize(masterSequence, currentWorld->GetLevel(0), playerSettings, cameraSettings);
	//player->State.AssignSequence(MovieSceneSequenceID::Root, *masterSequence, *player);
	//currentWorld->DestroyActor(outActor);
	//MovieSceneToolHelpers::ImportFBXWithDialog(masterSequence, *sharedSequencer, newBindingMap, false);
	//MovieSceneToolHelpers::ImportFBXCameraToExisting(fbxImporter, masterSequence, sharedSequencer.Get(), sharedSequencer->GetFocusedTemplateID(), newBindingMap, false, withNotify);
	//MovieSceneToolHelpers::ImportFBXCameraToExisting(fbxImporter, masterSequence, player, MovieSceneSequenceID::Root, newBindingMap, importFBXSettings->bMatchByNameOnly, withNotify);
	//MovieSceneToolHelpers::ImportFBXIfReady(currentWorld, masterSequence, sharedSequencer.Get(), MovieSceneSequenceID::Root, newBindingMap, importFBXSettings, inOutParams);
}

void TubaNMenuTool::ModifyAnimSequence(UMovieSceneSkeletalAnimationTrack* animTrack, UAnimSequence* animSequence)
{
	/** Check duplicates */
	//int32 spawnableCnt = movieScene->GetSpawnableCount();
	//if (spawnableCnt > 0 == true)
	//{
	//	TArray<FString> spawnableNameArray;
	//	FMovieSceneSpawnable* spawnable = nullptr;
	//	for (int32 spawnableIndex = 0; spawnableIndex < spawnableCnt; spawnableIndex++)
	//	{
	//		spawnable = &movieScene->GetSpawnable(spawnableIndex);
	//		spawnableNameArray.Add(spawnable->GetName());
	//	}

	//	FString newActorName = spawnedActor->GetActorLabel();
	//	if (spawnableNameArray.Contains(newActorName) == true)
	//	{
	//		newGuid = spawnable->GetGuid();
	//		animTrack = movieScene->FindTrack<UMovieSceneSkeletalAnimationTrack>(newGuid);
	//	}
	//	else
	//	{
	//		newGuid = CreateSpawnableToSequence(sequenceAsset, spawnedActor);
	//		animTrack = movieScene->AddTrack<UMovieSceneSkeletalAnimationTrack>(newGuid);
	//	}
	//}
	//else
	//{
	//	newGuid = CreateSpawnableToSequence(sequenceAsset, spawnedActor);
	//	animTrack = movieScene->AddTrack<UMovieSceneSkeletalAnimationTrack>(newGuid);
	//}

	/** Refrence to existing */
	//UMovieSceneSkeletalAnimationSection* animSection = CastChecked<UMovieSceneSkeletalAnimationSection>(animTrack->CreateNewSection());
	//FFrameTime animLength = animSequence->GetPlayLength() * Common::GetTickResolution();
	//int32 frameNumber = animLength.FrameNumber.Value + (int)(animLength.GetSubFrame() + 0.5f) + 1;
	//TArray<UMovieSceneSection*> animSections = animTrack->GetAllSections();
	//if (animSections.Num())
	//{
	//	LOG_S(Error);
	//	for (auto& section : animSections)
	//	{
	//		UMovieSceneSkeletalAnimationSection* newSection = Cast<UMovieSceneSkeletalAnimationSection>(section);
	//		newSection->Params.Animation = animSequence;
	//	}
	//}
	//else
	//{
	//	animSection->InitialPlacement(animSections, 0, frameNumber, true);
	//	animTrack->AddSection(*animSection);
	//	//animSection->Params.Animation = animSequnece;
	//}
}

void TubaNMenuTool::CreateAniLevelSequence(TSharedRef<AssetInfo>& inAsset)
{
	UObject* levelSequence = nullptr;

	FString assetDir;
	FString assetName;
	FString assetPath;

	Tie(assetDir, assetName) = inAsset->GetAniLevelSequencePath();
	assetPath = assetDir + assetName;

	levelSequence = UEditorAssetLibrary::DoesAssetExist(assetPath) == true ? UEditorAssetLibrary::LoadAsset(assetPath) : nullptr;
	CHECK_EXIT(levelSequence);

	//`Spawn blueprint actor
	Tie(assetDir, assetName) = inAsset->GetActorBlueprintPath();
	AActor* spawnedActor = SpawnActorFromPath(assetDir, assetName);
	ENSURE_EXIT(spawnedActor, *assetName);

	//`Make spawnable & Destroy spawned actor
	UMovieSceneSkeletalAnimationTrack* animTrack = CreateSpawnableToAniSequence(levelSequence, spawnedActor);
	CHECK_EXIT(animTrack);

	UWorld* currentWorld = GCurrentLevelEditingViewportClient ? GCurrentLevelEditingViewportClient->GetWorld() : nullptr;
	CHECK_EXIT(currentWorld);

	currentWorld->EditorDestroyActor(spawnedActor, false);

	//`Load anim sequence asset & Add anim
	Tie(assetDir, assetName) = inAsset->GetAnimationGamePath();
	assetPath = assetDir + assetName;

	UObject* animSequenceAsset = UEditorAssetLibrary::LoadAsset(assetPath);
	CHECK_EXIT(animSequenceAsset);

	UAnimSequence* animSequence = Cast<UAnimSequence>(animSequenceAsset);
	CHECK_EXIT(animSequence);

	FFrameNumber startKeyTime = Common::GetStartKeyTime();
	UMovieSceneSection* animSection = animTrack->AddNewAnimation(startKeyTime, animSequence);
	TRange<FFrameNumber> animRange = animSection->GetRange();

	//`Set start time & playback range
	ULevelSequence* masterSequence = CastChecked<ULevelSequence>(levelSequence);
	masterSequence->GetMovieScene()->SetPlaybackRange(Common::AdjPlaybackStart, animRange.GetUpperBoundValue().Value + startKeyTime.Value);
}

int32 TubaNMenuTool::AddSubSeqToCutSequence(UMovieScene* movieScene, TSharedRef<AssetInfo>& inAsset, FString subSeqType)
{
	int32 shotDuration = 0;
	FString assetDir;
	FString assetName;
	FString assetPath;

	if (subSeqType == Common::SuffixEffect)
	{
		Tie(assetDir, assetName) = inAsset->GetEffectLevelSequencePath();
	}
	else if (subSeqType == Common::SuffixBack)
	{
		Tie(assetDir, assetName) = inAsset->GetBackLevelSequencePath();
	}
	else if (subSeqType == Common::SuffixLight)
	{
		Tie(assetDir, assetName) = inAsset->GetLightLevelSequencePath();
	}
	else if (subSeqType == Common::SuffixAni)
	{
		Tie(assetDir, assetName) = inAsset->GetAniLevelSequencePath();
	}

	assetPath = assetDir + assetName;

	UObject* levelSequence = UEditorAssetLibrary::DoesAssetExist(assetPath) == true ? UEditorAssetLibrary::LoadAsset(assetPath) : nullptr;
	if (levelSequence != nullptr)
	{
		UMovieSceneSubTrack* newTrack = movieScene->AddMasterTrack<UMovieSceneSubTrack>();
		UMovieSceneSequence* newSequence = CastChecked<UMovieSceneSequence>(levelSequence);

		shotDuration = ConvertFrameTime(
			newSequence->GetMovieScene()->GetPlaybackRange().Size<FFrameNumber>(),
			newSequence->GetMovieScene()->GetTickResolution(),
			movieScene->GetTickResolution()).FloorToFrame().Value;

		newTrack->AddSequence(newSequence, Common::AdjGeneralStart, shotDuration);
		newTrack->SetDisplayName(FText::FromString(subSeqType));
	}
	else
	{
		UMovieSceneSubTrack* newTrack = movieScene->AddMasterTrack<UMovieSceneSubTrack>();
		newTrack->SetDisplayName(FText::FromString(subSeqType));
	}

	return shotDuration;
}

void TubaNMenuTool::CreateCutLevelSequence(TSharedRef<AssetInfo>& inAsset)
{
	//`Create New Level Sequence
	UObject* levelSequence = nullptr;

	FString assetDir;
	FString assetName;
	FString assetPath;

	Tie(assetDir, assetName) = inAsset->GetCutLevelSequencePath();
	levelSequence = CreateEmptyLevelSequence(assetDir, assetName);
	CHECK_EXIT(levelSequence);

	/** `Add tracks */
	ULevelSequence* masterSequence = CastChecked<ULevelSequence>(levelSequence);
	UMovieScene* movieScene = masterSequence->GetMovieScene();

	const TArray<UMovieSceneTrack*>& masterTrackArray = movieScene->GetMasterTracks();
	if (masterTrackArray.IsEmpty() == true)
	{
		int32 shotDuration = 0;

		//`Add LS_ANI
		shotDuration = AddSubSeqToCutSequence(movieScene, inAsset, Common::SuffixAni);
		movieScene->SetPlaybackRange(Common::AdjPlaybackStart, shotDuration);
		//`Add LS_FX
		AddSubSeqToCutSequence(movieScene, inAsset, Common::SuffixEffect);
		//`Add LS_LGT
		AddSubSeqToCutSequence(movieScene, inAsset, Common::SuffixLight);

		//`Create Cine Camera Actor
		UWorld* currentWorld = GCurrentLevelEditingViewportClient ? GCurrentLevelEditingViewportClient->GetWorld() : nullptr;
		CHECK_EXIT(currentWorld);

		ACineCameraActor* newCamera = currentWorld->SpawnActor<ACineCameraActor>();
		CHECK_EXIT(newCamera);

		FString newCameraName = FName::NameToDisplayString(ACineCameraActor::StaticClass()->GetFName().ToString(), false);
		newCameraName = inAsset->GetCameraFBXPath().Value.LeftChop(4).RightChop(4);
		newCamera->SetActorLabel(newCameraName);		//`Highly important func for determining new spawned asset name.

		FGuid newGuid = CreateSpawnableToSequence(levelSequence, newCamera);
		currentWorld->EditorDestroyActor(newCamera, false);

		MovieSceneToolHelpers::CreateCameraCutSectionForCamera(movieScene, newGuid, 0);

		/// todo : Get cam fbx file from server till now. may deprecated
		Tie(assetDir, assetName) = inAsset->GetCameraFBXPath();
		assetDir = cameraDir;
		assetPath = assetDir + assetName;

		//bool isConnected = CheckServerDir(assetDir);
		bool isFileExist = FPlatformFileManager::Get().GetPlatformFile().FileExists(*assetPath);
		if (isFileExist == false)
		{
			LOG(Error, TEXT("Can not find %s"), *assetPath);
			LOG_SERVER(Error, TEXT("Can not find %s"), *assetPath);
			return;
		}

		TArray<FMovieSceneBindingProxy> bindingArray;
		bindingArray.Add(FMovieSceneBindingProxy(newGuid, masterSequence));
		UMovieSceneUserImportFBXSettings* importFBXSettings = CreateImportCameraOptions();

		USequencerToolsFunctionLibrary::ImportLevelSequenceFBX(currentWorld, masterSequence, bindingArray, importFBXSettings, assetPath);

		AdjustKeyTimes(movieScene, newGuid);
	}
}

void TubaNMenuTool::CreateSceneLevelSequence(Params::FSequenceData& sequenceData)
{
	FString assetDir = sequenceData.LevelSequenceDir;
	FString assetName = sequenceData.LevelSequenceName;

	CreateEmptyLevelSequence(assetDir, assetName);
}

void TubaNMenuTool::CreateEpisodeLevelSequence(Params::FSequenceData& sequenceData)
{
	// ** `Test create master sequence using LevelSequenceEditor */
	//TSharedPtr<ULevelSequenceMasterSequenceSettings> LevelSequenceSettings;
	//
	//FString MasterSequenceAssetName = LevelSequenceSettings->MasterSequenceName;
	//FString MasterSequencePackagePath = LevelSequenceSettings->MasterSequenceBasePath.Path;
	//MasterSequencePackagePath /= MasterSequenceAssetName;
	//MasterSequenceAssetName += LevelSequenceSettings->MasterSequenceSuffix;
	//
	//UObject* MasterSequenceAsset = LevelSequenceEditorHelpers::CreateLevelSequenceAsset(MasterSequenceAssetName, MasterSequencePackagePath);
	//ULevelSequenceMasterSequenceSettings
	//TSharedPtr<LevelSequenceEditorHelpers> TEST;
	//TEST->CreateLevelSequenceAsset(MasterSequenceAssetName, MasterSequencePackagePath);

	FString assetDir = sequenceData.LevelSequenceDir;
	FString assetName = sequenceData.LevelSequenceName;

	CreateEmptyLevelSequence(assetDir, assetName);
}

void TubaNMenuTool::AddShotToLevelSequence(Params::FSequenceData& sequenceData)
{
	FString assetPath = sequenceData.LevelSequenceDir + sequenceData.LevelSequenceName;

	UObject* mainLevelSequence = UEditorAssetLibrary::DoesAssetExist(assetPath) == true ? UEditorAssetLibrary::LoadAsset(assetPath) : nullptr;
	CHECK_EXIT(mainLevelSequence);

	ULevelSequence* masterSequence = CastChecked<ULevelSequence>(mainLevelSequence);
	UMovieScene* movieScene = masterSequence->GetMovieScene();

	//`Load sub level sequences
	assetPath = sequenceData.LoadAssetDir + sequenceData.LoadAssetName;
	UObject* subLevelSequence = UEditorAssetLibrary::DoesAssetExist(assetPath) == true ? UEditorAssetLibrary::LoadAsset(assetPath) : nullptr;
	CHECK_EXIT(subLevelSequence);

	UMovieSceneSequence* shotSequence = Cast<UMovieSceneSequence>(subLevelSequence);
	TRange<FFrameNumber> shotRange = shotSequence->GetMovieScene()->GetPlaybackRange();
	int32 shotDuration = shotRange.GetUpperBoundValue().Value;
	UMovieSceneSequence* preShotSequence = shotSequence;
	TRange<FFrameNumber> preShotRange = shotRange;
	int32 preShotDuration = 0;

	assetPath = sequenceData.PreAssetDir + sequenceData.PreAssetName;
	if (assetPath.IsEmpty() == false)
	{
		UObject* preLevelSequence = UEditorAssetLibrary::DoesAssetExist(assetPath) == true ? UEditorAssetLibrary::LoadAsset(assetPath) : nullptr;
		CHECK_EXIT(preLevelSequence);

		preShotSequence = CastChecked<UMovieSceneSequence>(preLevelSequence);
		preShotRange = preShotSequence->GetMovieScene()->GetPlaybackRange();
		preShotDuration = preShotRange.GetUpperBoundValue().Value;
	}

	UMovieSceneCinematicShotTrack* cinematicShotTrack = movieScene->FindMasterTrack<UMovieSceneCinematicShotTrack>();
	if (cinematicShotTrack == nullptr)
	{
		cinematicShotTrack = movieScene->AddMasterTrack<UMovieSceneCinematicShotTrack>();
	}

	if (sequenceData.HasEnded == true)
	{
		preShotDuration = 0;
	}

	cinematicShotTrack->AddSequence(shotSequence, preShotDuration, shotDuration);

	SortSectionsInShotTrack(cinematicShotTrack);

	movieScene->SetPlaybackRange(Common::AdjPlaybackStart, preShotDuration + shotDuration);
}

void TubaNMenuTool::GenerateSubSequence()
{
	LOG_SERVER_S(Display);

	for (int32 idx = 0; idx < AssetInfoArray.Num(); idx++)
	{
		Params::FSequenceData sequenceData;

		//`Initialize num info
		Tie(sequenceData.CurEpisodeNumStr, sequenceData.CurSceneNumStr, sequenceData.CurCutNumStr) = AssetInfoArray[idx]->GetNumInfoStr();
		if (idx >= 1)
		{
			Tie(sequenceData.PreEpisodeNumStr, sequenceData.PreSceneNumStr, sequenceData.PreCutNumStr) = AssetInfoArray[idx - 1]->GetNumInfoStr();
		}

		//`Create LS_ANI
		bool isSameEpisode = sequenceData.PreEpisodeNumStr == sequenceData.CurEpisodeNumStr;
		bool isSameScene = sequenceData.PreSceneNumStr == sequenceData.CurSceneNumStr;
		bool isSameCut = sequenceData.PreCutNumStr == sequenceData.CurCutNumStr;
		if (isSameEpisode == true && isSameScene == true && isSameCut == true)
		{
			if (idx >= 1)
			{
				LOG(Display, TEXT(">> It's in a same CUT as before. Pre : %s, Cur : %s"), *AssetInfoArray[idx - 1]->GetBaseFilename(), *AssetInfoArray[idx]->GetBaseFilename());
			}
			else
			{
				LOG(Display, TEXT(">> It's in a same CUT as before. Cur : %s"), *AssetInfoArray[idx]->GetBaseFilename());
			}
			CreateAniLevelSequence(AssetInfoArray[idx]);

			continue;
		}

		Tie(sequenceData.LevelSequenceDir, sequenceData.LevelSequenceName) = AssetInfoArray[idx]->GetAniLevelSequencePath();
		CreateEmptyLevelSequence(sequenceData.LevelSequenceDir, sequenceData.LevelSequenceName);
		CreateAniLevelSequence(AssetInfoArray[idx]);

		//`Create LS_CUT
		CreateCutLevelSequence(AssetInfoArray[idx]);

		//`Create LS_SCENE
		Tie(sequenceData.LevelSequenceDir, sequenceData.LevelSequenceName) = AssetInfoArray[idx]->GetSceneLevelSequencePath();
		Tie(sequenceData.LoadAssetDir, sequenceData.LoadAssetName) = AssetInfoArray[idx]->GetCutLevelSequencePath();
		if (idx >= 1)
		{
			Tie(sequenceData.PreAssetDir, sequenceData.PreAssetName) = AssetInfoArray[idx - 1]->GetSceneLevelSequencePath();
		}

		if (isSameEpisode == true && isSameScene == true)
		{
			if (idx >= 1)
			{
				LOG(Display, TEXT(">> It's in a same SCENE as before. Pre : %s, Cur : %s"), *AssetInfoArray[idx - 1]->GetBaseFilename(), *AssetInfoArray[idx]->GetBaseFilename());
			}
			else
			{
				LOG(Display, TEXT(">> It's in a same SCENE as before. Cur : %s"), *AssetInfoArray[idx]->GetBaseFilename());
			}
			AddShotToLevelSequence(sequenceData);

			continue;
		}

		//`Set start point
		if (isSameEpisode == false || isSameScene == false)
		{
			sequenceData.HasEnded = true;
		}
		CreateSceneLevelSequence(sequenceData);
		AddShotToLevelSequence(sequenceData);
	}

	FString assetDir;
	FString assetName;
	Tie(assetDir, assetName) = AssetInfoArray[0]->GetSceneLevelSequencePath();

	FString assetPath = assetDir + assetName;
	UObject* levelSequence = UEditorAssetLibrary::DoesAssetExist(assetPath) == true ? UEditorAssetLibrary::LoadAsset(assetPath) : nullptr;
	CHECK_EXIT(levelSequence);

	GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(levelSequence);
}

void TubaNMenuTool::GenerateMainSequence()
{
	LOG_SERVER_S(Display);

	for (int32 idx = 0; idx < AssetInfoArray.Num(); idx++)
	{
		Params::FSequenceData sequenceData;

		//`Initialize num info
		Tie(sequenceData.CurEpisodeNum, sequenceData.CurSceneNum, sequenceData.CurCutNum) = AssetInfoArray[idx]->GetNumInfo();
		if (idx >= 1)
		{
			Tie(sequenceData.PreEpisodeNum, sequenceData.PreSceneNum, sequenceData.PreCutNum) = AssetInfoArray[idx - 1]->GetNumInfo();
		}

		//`Create LS_Episode
		Tie(sequenceData.LevelSequenceDir, sequenceData.LevelSequenceName) = AssetInfoArray[idx]->GetEpisodeLevelSequencePath();
		Tie(sequenceData.LoadAssetDir, sequenceData.LoadAssetName) = AssetInfoArray[idx]->GetSceneLevelSequencePath();
		if (idx >= 1)
		{
			Tie(sequenceData.PreAssetDir, sequenceData.PreAssetName) = AssetInfoArray[idx - 1]->GetSceneLevelSequencePath();
		}

		bool isSameEpisode = sequenceData.PreEpisodeNum == sequenceData.CurEpisodeNum;
		bool isSameScene = sequenceData.PreSceneNum == sequenceData.CurSceneNum;
		bool isSameCut = sequenceData.PreCutNum == sequenceData.CurCutNum;
		if (isSameEpisode == true && isSameScene == false)
		{
			if (idx >= 1)
			{
				LOG(Display, TEXT(">> It's in a same EPISODE as before. Pre : %s, Cur : %s"), *AssetInfoArray[idx - 1]->GetBaseFilename(), *AssetInfoArray[idx]->GetBaseFilename());
			}
			else
			{
				LOG(Display, TEXT(">> It's in a same EPISODE as before. Cur : %s"), *AssetInfoArray[idx]->GetBaseFilename());
			}
			AddShotToLevelSequence(sequenceData);

			continue;
		}

		//`Set start point
		if (isSameEpisode == false)
		{
			sequenceData.HasEnded = true;
			CreateEpisodeLevelSequence(sequenceData);
			AddShotToLevelSequence(sequenceData);
		}
	}
}

AActor* TubaNMenuTool::SpawnActorFromPath(FString& assetDir, FString& assetName)
{
	AActor* newActor = nullptr;

	UActorFactory* actorFactory = GEditor->FindActorFactoryForActorClass(AActor::StaticClass());
	if (ensure(actorFactory) == false)
	{
		return nullptr;
	}

	FString assetPath = assetDir + assetName;
	if (UEditorAssetLibrary::DoesAssetExist(assetPath) == true)
	{
		UObject* newAsset = UEditorAssetLibrary::LoadAsset(assetPath);
		actorFactory = GEditor->FindActorFactoryForActorClass(newAsset->StaticClass());
		newActor = GEditor->UseActorFactory(actorFactory, FAssetData(newAsset), &FTransform::Identity);
	}
	else
	{
		LOG(Error, TEXT("%s : Does not exist"), *assetPath);
		LOG_SERVER(Error, TEXT("%s : Does not exist"), *assetPath);
	}

	//`Either move infront of the camera or focus camera on it (depending on the viewport)
	//if (GCurrentLevelEditingViewportClient != nullptr && GCurrentLevelEditingViewportClient->IsPerspective() == true)
	//{
	//	GEditor->MoveViewportCamerasToActor(*newActor, false);
	//	GEditor->MoveActorInFrontOfCamera(*newActor, GCurrentLevelEditingViewportClient->GetViewLocation(), GCurrentLevelEditingViewportClient->GetViewRotation().Vector());
	//}

	return newActor;
}

UObject* TubaNMenuTool::CreateAsset(FString& assetDir, FString& assetName, UClass* assetClass)
{
	//`Check performance
	UObject* newAsset = nullptr;

	for (TObjectIterator<UClass> iter; iter; ++iter)
	{
		UClass* currentClass = *iter;
		bool isFactory = currentClass->IsChildOf(UFactory::StaticClass());
		bool isAbstract = currentClass->HasAnyClassFlags(CLASS_Abstract);
		if (isFactory == true && isAbstract == false)
		{
			UFactory* assetFactory = CastChecked<UFactory>(currentClass->GetDefaultObject());
			if (assetFactory->CanCreateNew() == true && assetFactory->SupportedClass == assetClass)
			{
				newAsset = assetTools->CreateAsset(assetName, assetDir, assetClass, assetFactory);
				break;
			}
		}
	}

	if (newAsset == nullptr)
	{
		LOG(Error, TEXT("%s Failed to create"), *assetName);
	}

	return newAsset;
}

UObject* TubaNMenuTool::CreateEmptyLevelSequence(FString& levelSequenceDir, FString& levelSequenceName)
{
	//`Createl level sequence
	FString assetDir = levelSequenceDir;
	FString assetName = levelSequenceName;
	FString assetPath = assetDir + assetName;

	if (UEditorAssetLibrary::DoesAssetExist(assetPath) == true)
	{
		UEditorAssetLibrary::DeleteAsset(assetPath);
		LOG(Error, TEXT("```%s already exist"), *assetPath);
	}

	UObject* levelSequence = CreateAsset(assetDir, assetName, ULevelSequence::StaticClass());
	if (levelSequence != nullptr)
	{
		ULevelSequence* masterSequence = CastChecked<ULevelSequence>(levelSequence);
		masterSequence->GetMovieScene()->SetDisplayRate(Common::GetDisplayRate());
	}

	return levelSequence;
}

FGuid TubaNMenuTool::CreateSpawnableToSequence(UObject* sequenceAsset, AActor* spawnedActor)
{
#pragma region Create spawnable 1 - <F>		
	//TArray<TSharedRef<IMovieSceneObjectSpawner>> objectSpawners;
	//FGuid newGuid;
	//
	////`In order to create a spawnable, we have to instantiate all the relevant object spawners for level sequences, and try to create a spawnable from each
	//FLevelSequenceModule& levelSequenceModule = FModuleManager::LoadModuleChecked<FLevelSequenceModule>("LevelSequence");
	//levelSequenceModule.GenerateObjectSpawners(objectSpawners);
	//
	////`The first object spawner to return a valid result
	//for (TSharedRef<IMovieSceneObjectSpawner> spawner : objectSpawners)
	//{
	//	TValueOrError<FNewSpawnable, FText> result = spawner->CreateNewSpawnableType(*newCamera, *movieScene, nullptr);
	//	if (result.IsValid())
	//	{
	//		//`Add spawnable cine camera actor
	//		FNewSpawnable& newSpawnable = result.GetValue();
	//		newSpawnable.Name = MovieSceneHelpers::MakeUniqueSpawnableName(movieScene, newCameraName);
	//		newSpawnable.ObjectTemplate = MovieSceneHelpers::MakeSpawnableTemplateFromInstance(*newCamera, movieScene, *newCameraName);
	//		newGuid = movieScene->AddSpawnable(*newSpawnable.Name, *newSpawnable.ObjectTemplate);
	//		//`Add cince camera component property track
	//		// ** test * /
	//		UCineCameraComponent* ccc = newCamera->GetCineCameraComponent();
	//		
	//		int32 uid = ccc->GetUniqueID();
	//		FGuid possessableGuid = movieScene->AddPossessable(ccc->GetName(), ccc->GetClass());
	//
	//		FMovieScenePossessable* childPossessable = movieScene->FindPossessable(possessableGuid);
	//		if (ensure(childPossessable))
	//		{
	//			childPossessable->SetParent(newGuid);
	//		}
	//
	//		FMovieSceneSpawnable* parentSpawnable = movieScene->FindSpawnable(newGuid);
	//		if (parentSpawnable)
	//		{
	//			parentSpawnable->AddChildPossessable(possessableGuid);
	//		}
	//		parentSpawnable->GetChildPossessables();
	//
	//		FMovieSceneBinding* bind = movieScene->FindBinding(possessableGuid);
	//		UMovieSceneFloatTrack* floatTrack1 = movieScene->AddTrack<UMovieSceneFloatTrack>(possessableGuid);
	//		floatTrack1->SetPropertyNameAndPath("Current Aperture", "CurrentAperture");
	//		UMovieSceneFloatTrack* floatTrack2 = movieScene->AddTrack<UMovieSceneFloatTrack>(possessableGuid);
	//		UMovieSceneFloatTrack* floatTrack3 = movieScene->AddTrack<UMovieSceneFloatTrack>(possessableGuid);
	//		// ** ~test */
	//
	//		//`Add spawn track
	//		UMovieSceneSpawnTrack* newSpawnTrack = movieScene->AddTrack<UMovieSceneSpawnTrack>(newGuid);
	//		if (newSpawnTrack)
	//		{
	//			newSpawnTrack->AddSection(*newSpawnTrack->CreateNewSection());
	//		}
	//
	//		//`Add transform track
	//		UMovieScene3DTransformTrack* newTransformTrack = movieScene->AddTrack<UMovieScene3DTransformTrack>(newGuid);
	//		if (newTransformTrack)
	//		{
	//			newTransformTrack->SetDisplayName(FText::FromString(TEXT("Transform")));
	//			newTransformTrack->SetPropertyNameAndPath(FName(TEXT("Transform"), false), TEXT("Transform"));
	//		}
	//
	//		//`Lock the viewport to this camera. uncalled-for
	//		if (newCamera && newCamera->GetLevel())
	//		{
	//			GCurrentLevelEditingViewportClient->SetCinematicActorLock(nullptr);
	//			GCurrentLevelEditingViewportClient->SetActorLock(newCamera);
	//			GCurrentLevelEditingViewportClient->bLockedCameraView = true;
	//			GCurrentLevelEditingViewportClient->UpdateViewForLockedActor();
	//			GCurrentLevelEditingViewportClient->Invalidate();
	//		}
	//		MovieSceneToolHelpers::CreateCameraCutSectionForCamera(movieScene, newGuid, 0);
	//	}
	//}
#pragma endregion

#pragma region Create spawnable 2 - <F>
		/*
		//`this code cause create infinite spawnable camera actor. Delete it.
		FString newName = MovieSceneHelpers::MakeUniqueSpawnableName(movieScene, FName::NameToDisplayString(ACineCameraActor::StaticClass()->GetFName().ToString(), false));

		FGuid newCameraGuid = movieScene->AddSpawnable(newName, *newCamera);
		FMovieSceneSpawnable* spawnable = movieScene->FindSpawnable(newCameraGuid);
		if (ensure(spawnable))
		{
			spawnable->SetName(newName);
		}

		currentWorld->EditorDestroyActor(newCamera, false);

		newCamera->SetActorLabel(newName, false);
		newCamera->SetActorLocation(GCurrentLevelEditingViewportClient->GetViewLocation(), false);
		newCamera->SetActorRotation(GCurrentLevelEditingViewportClient->GetViewRotation());

		//TArray<FMovieSceneSequenceID> activeTemplateIDs;
		//activeTemplateIDs.Add(MovieSceneSequenceID::Root);

		//IMovieScenePlayer* player = nullptr;
		//for (TWeakObjectPtr<UObject>& Object : player->FindBoundObjects(newCameraGuid, activeTemplateIDs.Top()))
		//{
		//	newCamera = Cast<ACineCameraActor>(Object.Get());
		//	if (newCamera)
		//	{
		//		break;
		//	}
		//}
		//ensure(newCamera);


		//`Add camera component track
		//UCineCameraComponent* newCameraComponentTrack = movieScene->AddTrack<UCineCameraComponent>(newCameraGuid);
		//if (newCameraComponentTrack)
		//{

		//}

		//`Add spawn track
		UMovieSceneSpawnTrack* newSpawnTrack = nullptr;
		newSpawnTrack = movieScene->AddTrack<UMovieSceneSpawnTrack>(newCameraGuid);
		if (newSpawnTrack)
		{

			newSpawnTrack->AddSection(*newSpawnTrack->CreateNewSection());
		}

		//`Add transform track
		UMovieSceneTransformTrack* newTransformTrack = movieScene->AddTrack<UMovieSceneTransformTrack>(newCameraGuid);
		if (newTransformTrack)
		{
			UMovieSceneSection* newSection = newTransformTrack->CreateNewSection();
			newTransformTrack->AddSection(*newSection);
			newTransformTrack->SetDisplayName(FText::FromString(TEXT("Transform")));
			//newTransformTrack->SetPropertyNameAndPath(FName(TEXT("Transform"),false), "");
		}
		*/
#pragma endregion

#pragma region Create spawanble 3 - <T>
		/*
		GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(levelSequence);

		IAssetEditorInstance* assetEditor = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->FindEditorForAsset(levelSequence, false);
		ILevelSequenceEditorToolkit* levelSequenceEditor = static_cast<ILevelSequenceEditorToolkit*>(assetEditor);
		TSharedPtr<ISequencer> sequencerShared = levelSequenceEditor ? levelSequenceEditor->GetSequencer() : nullptr;
		FGuid newGuid = sequencerShared.Get()->MakeNewSpawnable(*newCamera);

		levelSequenceEditor->CloseWindow();
		*/
#pragma endregion

	GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(sequenceAsset);

	IAssetEditorInstance* assetEditor = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->FindEditorForAsset(sequenceAsset, false);
	ILevelSequenceEditorToolkit* sequenceEditor = static_cast<ILevelSequenceEditorToolkit*>(assetEditor);
	TSharedPtr<ISequencer> sharedSequencer = sequenceEditor ? sequenceEditor->GetSequencer() : nullptr;
	FGuid newGuid = sharedSequencer->MakeNewSpawnable(*spawnedActor);

	//sequenceEditor->CloseWindow();

	return newGuid;
}

UMovieSceneSkeletalAnimationTrack* TubaNMenuTool::CreateSpawnableToAniSequence(UObject* sequenceAsset, AActor* spawnedActor)
{
	UMovieSceneSkeletalAnimationTrack* animTrack = nullptr;

	ULevelSequence* masterSequence = CastChecked<ULevelSequence>(sequenceAsset);
	UMovieScene* movieScene = masterSequence->GetMovieScene();
	FGuid newGuid = CreateSpawnableToSequence(sequenceAsset, spawnedActor);
	if (newGuid.IsValid() == true)
	{
		animTrack = movieScene->AddTrack<UMovieSceneSkeletalAnimationTrack>(newGuid);
	}

	return animTrack;
}

void TubaNMenuTool::AdjustKeyTimes(UMovieScene* movieScene, FGuid cameraGuid)
{
	TArray<UMovieSceneTrack*> cameraTrackArray;
	TArray<UMovieSceneTrack*> componentTrackArray;

	TArray<FMovieSceneBinding> bindingArray = movieScene->GetBindings();
	for (FMovieSceneBinding& bind : bindingArray)
	{
		LOG(Display, TEXT("%s"), *bind.GetName());
		if (bind.GetObjectGuid() == cameraGuid)
		{
			cameraTrackArray = bind.GetTracks();
		}
		else
		{
			componentTrackArray = bind.GetTracks();
		}
	}

	SetKeyTimesInternal(cameraTrackArray, Common::GetStartKeyTime());
	SetKeyTimesInternal(componentTrackArray, Common::GetStartKeyTime());
}

void TubaNMenuTool::SetKeyTimesInternal(TArray<UMovieSceneTrack*> inTrackArray, FFrameNumber startKeyTime)
{
	for (auto& track : inTrackArray)
	{
		for (UMovieSceneSection* const& section : track->GetAllSections())
		{
			for (const FMovieSceneChannelEntry& entry : section->GetChannelProxy().GetAllEntries())	//`return 2 entries - double, float
			{
				for (FMovieSceneChannel* channel : entry.GetChannels())	//`return 9 channels - { Location, Rotation, Scale } * { x,y,z }
				{
					TRange<FFrameNumber> range;
					TArray<FFrameNumber> keyTimeArray;
					TArray<FKeyHandle> keyHandleArray;
					channel->GetKeys(range, &keyTimeArray, &keyHandleArray);

					bool hasKey = keyHandleArray.Num() > 0;
					if (hasKey == true)
					{
						FFrameNumber diff = keyTimeArray[0] - startKeyTime;
						for (FFrameNumber& keyTime : keyTimeArray)
						{
							keyTime -= diff;	//`Adjust key time for motion blur. Always start on -5 key.
						}

						channel->SetKeyTimes(keyHandleArray, keyTimeArray);
					}
				}
			}
		}
	}
}

void TubaNMenuTool::SortSectionsInShotTrack(UMovieSceneCinematicShotTrack* shotTrack)
{
	//`Set row index of loaded cut sequence.
	bool isUpLine = true;
	for (UMovieSceneSection* section : shotTrack->GetAllSections())
	{
		if (isUpLine == true)
		{
			section->SetRowIndex(0);
			isUpLine = false;
		}
		else
		{
			section->SetRowIndex(1);
			isUpLine = true;
		}
	}
}

void TubaNMenuTool::PrintMessage(EMessageSeverity::Type serverity, FString message)
{
	// todo : Deprecated. Could display message on tool widget
	FMessageLog("MYLOG").Open(serverity, true);
	FMessageLog("MYLOG").Message(serverity, FText::FromString(message));
}

void TubaNMenuTool::TestFunc()
{
	//TArray<FString> pathArray;
	//pathArray.Add("R:/PROJECT_RND2022/05_ANIMATION/00_EPISODES/EP001_Racing/01_Scene/05_FBX/LOW_EP00_S01_C001sc_CAM_Camera_001.fbx");
	//InitAssetInfoArray(pathArray);
	//GenerateSubSequence();

	/// ** testing ptr * /
	//----------------------------------------------
	FString origin = FPaths::EngineContentDir();
	FString& originRef = origin;
	FString* originPtr = &origin;
	originRef.Append("1");
	originPtr->Append("1");

	LOG_S(Display);
	LOG_S(Warning);
	//LOG_S(Error);

	APacketManager::Instance()->SendTableName();

	//t.FactoryCreateFile();
	//IFactoryInterface* t;

	//assetTools->ImportAssets();

	AActor* aiTest = nullptr;
	CHECK_EXIT(aiTest);

	//----------------------------------------------

	/** Save current level func test */
	//UEditorLoadingAndSavingUtils::SaveMap(CurrentWorld, TEXT("/Game/Untitled"));
}