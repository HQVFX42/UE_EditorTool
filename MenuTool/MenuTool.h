// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorMenuTool.h"
#include "Common/Common.h"
#include "Common/AssetInfo.h"

#include "IAssetTools.h"										//`Access asset import task
#include "EditorAssetLibrary.h"									//`Access Unreal content browser asset
#include "Tracks/MovieSceneSkeletalAnimationTrack.h"			//`Access SK anim track
#include "MovieSceneToolsUserSettings.h"						//`Access import camera settings
#include "Factories/FbxImportUI.h"								//`Access UFBXImportUI
#include "C:/Program Files/Epic Games/UE_5.1/Engine/Plugins/Importers/AlembicImporter/Source/AlembicImporter/Classes/AlembicImportFactory.h"
//#include "C:/Program Files/Epic Games/UE_5.1/Engine/Plugins/Importers/AlembicImporter/Source/AlembicImporter/Private/AlembicImportFactory.cpp"

/** Asset params */
namespace Params
{
	struct FAssetData
	{
	public:
		FString InAssetDir = TEXT_EMPTY;
		//FString InAssetPrefix = TEXT_EMPTY;
		FString InAssetExtension = TEXT_EMPTY;
		FString EpisodeNum = TEXT_EMPTY;
		FString SceneNum = TEXT_EMPTY;
		FString CutNum = TEXT_EMPTY;
		bool IsCameraFBX = false;
		TArray<FString> OutDirArray;
		TArray<FString> OutPathArray;

	public:
		void GetFBXPath()
		{
			InAssetDir = Common::DirServerAsset;
			//InAssetPrefix = Common::FrontAS();
			InAssetExtension = Common::ExtensionFbx;
			EpisodeNum = TEXT_EMPTY;
			IsCameraFBX = false;
		}

		void GetUAssetPath()
		{
			InAssetDir = Common::DirContentAnimation;
			//InAssetPrefix = Common::FrontAS();
			InAssetExtension = Common::ExtensionUasset;
			EpisodeNum = TEXT_EMPTY;
			IsCameraFBX = false;
		}
	};

	struct FSequenceData
	{
	public:
		bool HasEnded = false;

		int32 CurEpisodeNum = 0;
		int32 CurSceneNum = 0;
		int32 CurCutNum = 0;
		int32 PreEpisodeNum = 0;
		int32 PreSceneNum = 0;
		int32 PreCutNum = 0;

		FString CurEpisodeNumStr = TEXT_EMPTY;
		FString CurSceneNumStr = TEXT_EMPTY;
		FString CurCutNumStr = TEXT_EMPTY;
		FString PreEpisodeNumStr = TEXT_EMPTY;
		FString PreSceneNumStr = TEXT_EMPTY;
		FString PreCutNumStr = TEXT_EMPTY;

		FString LevelSequenceDir = TEXT_EMPTY;
		FString LevelSequenceName = TEXT_EMPTY;
		FString LoadAssetDir = TEXT_EMPTY;
		FString LoadAssetName = TEXT_EMPTY;
		FString PreAssetDir = TEXT_EMPTY;
		FString PreAssetName = TEXT_EMPTY;
	};
}

DECLARE_DELEGATE_OneParam(FOnFBXImported, TArray<FString>);

// todo : Change class name to suitable
class TubaNMenuTool : public IModuleListenerInterface, public TSharedFromThis<TubaNMenuTool>
{
public:
	TArray<TSharedRef<AssetInfo>> AssetInfoArray;
	FOnFBXImported OnFBXImported;
	FString cameraDir;

public:
	/** Constructor */
	TubaNMenuTool();
	/** Destructor */
	virtual ~TubaNMenuTool() {}

	virtual void OnStartupModule() override;
	virtual void OnShutdownModule() override;

	void MakeMenuEntry(FMenuBuilder& menuBuilder);
	void MakeSubMenu(FMenuBuilder& menuBuilder);

	/**
	 * Initialize asset info array
	 *
	 * @param pathArray
	 */
	void InitAssetInfoArray(TArray<FString>& pathArray);

	/**
	 * Get all files from specific dir.
	 *
	 * @param InAssetDir		Input specific dir
	 * @param InAssetPrefix		Input specific prefix
	 * @param InAssetExtension	Input specific extension
	 * @param IsCameraFBX		Check camera fbx -> evitable
	 * @param OutDirArray		Return all dirs
	 * @param OutPathArray		Return all paths
	 */
	void GetAllPathsFromDir(Params::FAssetData& assetData);

	/**
	 * FBX Import automation without dialog
	 */
	void OnFBXImporter(TArray<FString> pathArray);

	/**
	 * Call reimport fbx
	 *
	 * @param pathArray animation directory in /Game/
	 */
	void OnFBXReimporter(TArray<FString> pathArray);

	/**
	 * FBX Import automation with dialog
	 */
	void OnFBXImporterDialog();

	/**
	 * Deprecated
	 * Generate master & sub sequence
	 */
	void OnSequenceGenerator();

	/**
	 * Generate master & sub sequence from specific dir
	 */
	void OnSequenceGenerator(FString animDir, FString camDir);

	/**
	 * Function for test
	 *
	 */
	void TestFunc();



protected:
	TSharedPtr<FUICommandList> commandList;
	IAssetTools* assetTools;

protected:
	/**
	 * Register command list
	 */
	void MapCommands();

	/**
	 * Create editor utility widget
	 */
	void OnCreateWidget();

	/**
	 * Call CreateBP without dialog
	 *
	 * @param inAsset	Input asset info
	 */
	void OnCreateBP(TSharedRef<AssetInfo>& inAsset);

	/**
	 * Call CreateBP with dialog
	 *
	 * @param inAsset	Input asset info
	 */
	void OnCreateBPDialog(TSharedRef<AssetInfo>& inAsset);

	/**
	 * Check server dir connected
	 *
	 * @param serverDir
	 * @return true if connected
	 */
	bool CheckServerDir(FString& serverDir);

	/**
	 * Initiate all tasks.
	 * Import automation & Generate sequence
	 */
	void OnAllTasks();



private:
	/**
	 * Deprecated.
	 * Create directory to specific path
	 *
	 * @param directory	Absolute path
	 */
	void CreateDirectory(FString& directory);

	/**
	 * Convert high polygon mesh to blueprint actor
	 *
	 * @param inAsset		AssetInfo
	 * @param withDialog	[true] with dialog, [false] without dialog
	 */
	void CreateBP(TSharedRef<AssetInfo>& inAsset, bool withDialog);

	/**
	 * Load editor utility widget & Spawn and register tab
	 *
	 * @param assetPath
	 */
	void OpenEditorUtilityWidgetBP(FString& assetPath);

	/**
	 * Create FBX import task
	 *
	 * @param inAssetArray	Input asset info array
	 * @param wiilSave		Save mesh
	 */
	TArray<UAssetImportTask*> CreateImportTask(TArray<TSharedRef<AssetInfo>>& inAssetArray, bool willSave);

	/*
	 * Set FBX import options
	 * [skeleton]
	 * [material]
	 * [T0AsRefPose]
	 *
	 * @param inAsset	Input asset info
	 * @param isMaster	If it's master asset import type will be SkeletalMesh
	 * @return			Import options
	 */
	UFbxImportUI* CreateImportOptions(TSharedRef<AssetInfo>& inAsset, bool isMaster);

	/**
	 * Set import camera FBX options
	 *
	 * @return
	 */
	UMovieSceneUserImportFBXSettings* CreateImportCameraOptions();

	/**
	 * FBX Import automation without dialog
	 */
	void ImportFBX(TArray<FString> path);

	/**
	 * Automated reimport
	 */
	void ReimportFBX();

	/**
	 * FBX Import automation with dialog
	 */
	void ImportFBXDialog();

	/**
	 * Test...
	 * import camera without notify
	 *
	 * @param	[true]	Import with notify
	 *			[false]	Import without notify
	 */
	void ImportFBXCamera(bool withNotify);

	/**
	 * Test...
	 * Modify existing animation sequence data
	 * Deprecated
	 *
	 * @param animTrack
	 */
	void ModifyAnimSequence(UMovieSceneSkeletalAnimationTrack* animTrack, UAnimSequence* animSequence);

	/**
	 * Create LS_ANI
	 *
	 * @param inAsset
	 */
	void CreateAniLevelSequence(TSharedRef<AssetInfo>& inAsset);

	/**
	 * Add sub sequence into LS_CUT
	 *
	 * @param movieScene
	 * @param inAsset
	 * @param subSeqTyp
	 * @return sequence's shot duration
	 */
	int32 AddSubSeqToCutSequence(UMovieScene* movieScene, TSharedRef<AssetInfo>& inAsset, FString subSeqTyp);

	/**
	 * Create LS_CUT
	 *
	 * @param inAsset
	 */
	void CreateCutLevelSequence(TSharedRef<AssetInfo>& inAsset);

	/**
	 * Create LS_SCENE
	 *
	 * @param sceneDir
	 * @param sceneName
	 */
	void CreateSceneLevelSequence(Params::FSequenceData& sequenceData);

	/**
	 * Create LS_MASTER
	 *
	 * @param episodeDir
	 * @param episodeName
	 */
	void CreateEpisodeLevelSequence(Params::FSequenceData& sequenceData);

	/**
	 * Add shot into level sequence
	 *
	 * @param sequenceData
	 */
	void AddShotToLevelSequence(Params::FSequenceData& sequenceData);

	/**
	 * Generate scene & cut sequences
	 *
	 */
	void GenerateSubSequence();

	/**
	 * Generate master sequence
	 *
	 */
	void GenerateMainSequence();

private:
	/**
	 * Spawn actor into current level
	 *
	 * @param assetDir
	 * @param assetName
	 * @return actor
	 */
	AActor* SpawnActorFromPath(FString& assetDir, FString& assetName);

	/**
	 * Create asset by class
	 *
	 * @param assetDir
	 * @param assetName
	 * @param assetClass
	 * @return
	 */
	UObject* CreateAsset(FString& assetDir, FString& assetName, UClass* assetClass);

	/**
	 * Create empty level sequence uasset
	 *
	 * @param levelSequenceDir
	 * @param levelSequenceName
	 * @return
	 */
	UObject* CreateEmptyLevelSequence(FString& levelSequenceDir, FString& levelSequenceName);

	/**
	 * Make new spawnable into level sequence
	 *
	 * @param sourceAsset
	 * @param spawnedActor
	 * @return spawnable guid
	 */
	FGuid CreateSpawnableToSequence(UObject* sequenceAsset, AActor* spawnedActor);

	/**
	 * Make new spawnable into LS_ANI
	 *
	 * @param sourceAsset
	 * @param spawnedActor
	 * @return spawnable animation track
	 */
	UMovieSceneSkeletalAnimationTrack* CreateSpawnableToAniSequence(UObject* sequenceAsset, AActor* spawnedActor);

	/**
	 * Adjust camera key times from specific start time.
	 *
	 * @param movieScene
	 * @param cameraGuid
	 * @param startKeyTime
	 */
	void AdjustKeyTimes(UMovieScene* movieScene, FGuid cameraGuid);

	/**
	 * Set camera key times in tracks.
	 *
	 * @param inTrackArray
	 */
	void SetKeyTimesInternal(TArray<UMovieSceneTrack*> inTrackArray, FFrameNumber startKeyTime);

	/**
	 * Set row index of loaded sub sequences.
	 *
	 * @param shotTrack
	 */
	void SortSectionsInShotTrack(UMovieSceneCinematicShotTrack* shotTrack);

	/**
	 * Print message log on editor screen.
	 * 
	 * @param serverity
	 * @param message
	 */
	void PrintMessage(EMessageSeverity::Type serverity, FString message);
};