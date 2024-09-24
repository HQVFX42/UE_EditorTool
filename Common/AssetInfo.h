// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Common/Common.h"

class AssetInfo
{
public:
	/** Constructor */
	AssetInfo();
	/** Destructor */
	~AssetInfo();

public:
	/**
	 * Convert arguments into each asset info elements
	 * 
	 * @param filename	file path we are trying to convert
	 */
	void Initialize(const FString& filename);

	/**
	 * Check filename is valid
	 * 
	 * @return [true] If is valid info
	 */
	bool IsValidInfo();

	/** Elements getter */

	FString GetPrefixAsset() const;
	int32 GetEpisodeNumber() const;
	int32 GetSceneNumber() const;
	int32 GetCutNumber() const;
	FString GetEpisodeNumberStr() const;
	FString GetSceneNumberStr() const;
	FString GetCutNumberStr() const;
	FString GetType() const;
	FString GetName() const;
	FString GetDisplayName() const;
	FString GetExtension() const;
	FString GetOriginPath() const;
	FString GetBaseFilename() const;

	/** Paths getter */

	TPair<FString, FString> GetEpisodeLevelSequencePath() const;
	TPair<FString, FString> GetSceneLevelSequencePath() const;
	TPair<FString, FString> GetCutLevelSequencePath() const;
	TPair<FString, FString> GetAniLevelSequencePath() const;
	TPair<FString, FString> GetBackLevelSequencePath() const;
	TPair<FString, FString> GetEffectLevelSequencePath() const;
	TPair<FString, FString> GetLightLevelSequencePath() const;
	TPair<FString, FString> GetCameraFBXPath() const;
	TPair<FString, FString> GetActorBlueprintPath() const;
	TPair<FString, FString> GetAnimationGamePath() const;
	TPair<FString, FString> GetMasterSkeletonPath() const;
	TPair<FString, FString> GetAnimationContentPath() const;
	TPair<FString, FString> GetMasterDestinationPath() const;

	/**
	 * Get asset number info
	 * 
	 * @return	Episode
	 * @return	Scene
	 * @return	Cut
	 * 
	 */
	TTuple<int32, int32, int32> GetNumInfo() const;
	TTuple<FString, FString, FString> GetNumInfoStr() const;



private:
	/** Asset info elements */

	FString prefixAsset;

	FString episodeNumber;
	FString sceneNumber;
	FString cutNumber;
	bool isExtraCut;

	FString type;
	FString categoryName;
	FString originName;
	FString name;
	FString version;
	FString extension;

	/** Paths */

	FString episodeDir;
	FString episodeName;
	FString sceneDir;
	FString sceneName;
	FString cutDir;
	FString cutName;

	FString originPath;
	FString baseFilename;

private:
	void SetPaths();
};
