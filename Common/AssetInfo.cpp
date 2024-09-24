// Fill out your copyright notice in the Description page of Project Settings.

#include "Common/AssetInfo.h"

AssetInfo::AssetInfo()
	: prefixAsset()
	, episodeNumber()
	, sceneNumber()
	, cutNumber()
	, isExtraCut(false)
	, type()
	, name()
	, extension()
{
}

AssetInfo::~AssetInfo()
{
}

void AssetInfo::Initialize(const FString& filename)
{
	originPath = filename;
	FPaths::NormalizeFilename(originPath);
	baseFilename = FPaths::GetBaseFilename(originPath);

	FString rightStr = TEXT_EMPTY;
	FString leftStr = TEXT_EMPTY;

	baseFilename.Split(TEXT_UNDERSCORE, &leftStr, &rightStr, ESearchCase::IgnoreCase, ESearchDir::FromStart);
	prefixAsset = leftStr;

	rightStr.Split(TEXT_UNDERSCORE, &leftStr, &rightStr, ESearchCase::IgnoreCase, ESearchDir::FromStart);
	episodeNumber = leftStr;

	rightStr.Split(TEXT_UNDERSCORE, &leftStr, &rightStr, ESearchCase::IgnoreCase, ESearchDir::FromStart);
	sceneNumber = leftStr;

	rightStr.Split(TEXT_UNDERSCORE, &leftStr, &rightStr, ESearchCase::IgnoreCase, ESearchDir::FromStart);
	cutNumber = leftStr;

	rightStr.Split(TEXT_UNDERSCORE, &leftStr, &rightStr, ESearchCase::IgnoreCase, ESearchDir::FromStart);
	type = leftStr;
	originName = rightStr;

	rightStr.Split(TEXT_MINUS_SIGN, &leftStr, &rightStr, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
	name = rightStr;

	rightStr.Split(TEXT_UNDERSCORE, &leftStr, &rightStr, ESearchCase::IgnoreCase, ESearchDir::FromStart);
	categoryName = leftStr;
	version = rightStr;

	extension = FPaths::GetExtension(originPath);
	if (cutNumber.Len() >= 5)
	{
		isExtraCut = true;
	}
	SetPaths();
}

bool AssetInfo::IsValidInfo()
{
	/*if (episodeNumber.Contains(Common::PrefixEP) == true &&
		sceneNumber.Contains(Common::PrefixS) == true &&
		cutNumber.Contains(Common::PrefixC) == true &&
		type == Common::PrefixCH || type == Common::PrefixPROP)*/
	if (episodeNumber.Contains(Common::PrefixEP) == true &&
		sceneNumber.Contains(Common::PrefixS) == true &&
		cutNumber.Contains(Common::PrefixC) == true &&
		type != Common::PrefixCAM)
	{
		LOG(Display, TEXT("%s"), *baseFilename);
		return true;
	}
	else if (baseFilename.Contains(Common::SuffixEpisode) == true)
	{
		LOG(Display, TEXT("%s"), *baseFilename);
		return true;
	}

	return false;
}

void AssetInfo::SetPaths()
{
	episodeDir = episodeNumber + TEXT_SLASH;
	episodeName = episodeNumber + TEXT_UNDERSCORE;
	sceneDir = sceneNumber + TEXT_SLASH;
	sceneName = sceneNumber + TEXT_UNDERSCORE;
	cutDir = cutNumber + TEXT_SLASH;
	cutName = cutNumber + TEXT_UNDERSCORE;
}

FString AssetInfo::GetOriginPath() const
{
	return originPath;
}

FString AssetInfo::GetBaseFilename() const
{
	return baseFilename;
}

FString AssetInfo::GetPrefixAsset() const
{
	return prefixAsset;
}

int32 AssetInfo::GetEpisodeNumber() const
{
	int32 number = FCString::Strtoi(*episodeNumber + Common::PrefixEP.Len(), nullptr, 10);
	return number;
}

int32 AssetInfo::GetSceneNumber() const
{
	int32 number = FCString::Strtoi(*sceneNumber + Common::PrefixS.Len(), nullptr, 10);
	return number;
}

int32 AssetInfo::GetCutNumber() const
{
	int32 number = FCString::Strtoi(*cutNumber + Common::PrefixC.Len(), nullptr, 10);
	return number;
}

FString AssetInfo::GetEpisodeNumberStr() const
{
	return episodeNumber;
}

FString AssetInfo::GetSceneNumberStr() const
{
	return sceneNumber;
}

FString AssetInfo::GetCutNumberStr() const
{
	return cutNumber;
}

FString AssetInfo::GetType() const
{
	return type;
}

FString AssetInfo::GetName() const
{
	return name;
}

FString AssetInfo::GetDisplayName() const
{
	return FPaths::GetBaseFilename(originName);
}

FString AssetInfo::GetExtension() const
{
	return extension;
}

TPair<FString, FString> AssetInfo::GetEpisodeLevelSequencePath() const
{
	FString levelSequenceDir = Common::DirGameEpisode + episodeDir;
	FString levelSequenceName = Common::PrefixLS + TEXT_UNDERSCORE + episodeName + Common::SuffixEpisode + Common::SuffixTest;

	return TPair<FString, FString>(levelSequenceDir, levelSequenceName);
}

TPair<FString, FString> AssetInfo::GetSceneLevelSequencePath() const
{
	FString levelSequenceDir = Common::DirGameEpisode + episodeDir + sceneDir;
	FString levelSequenceName = Common::PrefixLS + TEXT_UNDERSCORE + episodeName + sceneName + Common::SuffixScene + Common::SuffixTest;

	return TPair<FString, FString>(levelSequenceDir, levelSequenceName);
}

TPair<FString, FString> AssetInfo::GetCutLevelSequencePath() const
{
	FString levelSequenceDir = Common::DirGameEpisode + episodeDir + sceneDir + cutDir;
	FString levelSequenceName = Common::PrefixLS + TEXT_UNDERSCORE + episodeName + sceneName + cutName + Common::SuffixCut + Common::SuffixTest;

	return TPair<FString, FString>(levelSequenceDir, levelSequenceName);
}

TPair<FString, FString> AssetInfo::GetAniLevelSequencePath() const
{
	FString levelSequenceDir = Common::DirGameEpisode + episodeDir + sceneDir + cutDir + Common::SuffixAni + TEXT_SLASH;
	FString levelSequenceName = Common::PrefixLS + TEXT_UNDERSCORE + episodeName + sceneName + cutName + Common::SuffixAni + Common::SuffixTest;

	return TPair<FString, FString>(levelSequenceDir, levelSequenceName);
}

TPair<FString, FString> AssetInfo::GetBackLevelSequencePath() const
{
	FString levelSequenceDir = Common::DirGameEpisode + episodeDir + sceneDir + cutDir + Common::SuffixBack + TEXT_SLASH;
	FString levelSequenceName = Common::PrefixLS + TEXT_UNDERSCORE + episodeName + sceneName + cutName + Common::SuffixBack;

	return TPair<FString, FString>(levelSequenceDir, levelSequenceName);
}

TPair<FString, FString> AssetInfo::GetEffectLevelSequencePath() const
{
	FString levelSequenceDir = Common::DirGameEpisode + episodeDir + sceneDir + cutDir + Common::SuffixEffect + TEXT_SLASH;
	FString levelSequenceName = Common::PrefixLS + TEXT_UNDERSCORE + episodeName + sceneName + cutName + Common::SuffixEffect;

	return TPair<FString, FString>(levelSequenceDir, levelSequenceName);
}

TPair<FString, FString> AssetInfo::GetLightLevelSequencePath() const
{
	FString levelSequenceDir = Common::DirGameEpisode + episodeDir + sceneDir + cutDir + Common::SuffixLight + TEXT_SLASH;
	FString levelSequenceName = Common::PrefixLS + TEXT_UNDERSCORE + episodeName + sceneName + cutName + Common::SuffixLight;

	return TPair<FString, FString>(levelSequenceDir, levelSequenceName);
}

TPair<FString, FString> AssetInfo::GetCameraFBXPath() const
{
	// todo : Use origin path.
	FString outDir = FPaths::GetPath(originPath) + TEXT_SLASH;
	FString outName = Common::PrefixAS + TEXT_UNDERSCORE + episodeName + sceneName + cutName + Common::PrefixCAM + TEXT_UNDERSCORE + Common::SuffixCamera + Common::ExtensionFbx;

	return TPair<FString, FString>(outDir, outName);
}

TPair<FString, FString> AssetInfo::GetActorBlueprintPath() const
{
	FString actorDir = Common::DirGameAsset + type + TEXT_SLASH + categoryName + TEXT_SLASH;
	//FString actorName = Common::PrefixAC + TEXT_UNDERSCORE + type + TEXT_UNDERSCORE + name;
	FString actorName = Common::PrefixAC + TEXT_UNDERSCORE + name;

	return TPair<FString, FString>(actorDir, actorName);
}

TPair<FString, FString> AssetInfo::GetAnimationGamePath() const
{
	FString levelSequenceDir = Common::DirGameAnimation + episodeDir + sceneDir + cutDir;
	//FString levelSequenceName = Common::PrefixAS + TEXT_UNDERSCORE + episodeName + sceneName + cutName + type + TEXT_UNDERSCORE + name;
	//FString levelSequenceName = Common::PrefixAS + TEXT_UNDERSCORE + episodeName + sceneName + cutName + type + TEXT_UNDERSCORE + originName;
	FString levelSequenceName = prefixAsset + TEXT_UNDERSCORE + episodeName + sceneName + cutName + type + TEXT_UNDERSCORE + originName;

	return TPair<FString, FString>(levelSequenceDir, levelSequenceName);
}

TPair<FString, FString> AssetInfo::GetMasterSkeletonPath() const
{	
	//FString outDir = Common::DirGameAsset + type + TEXT_SLASH + categoryName + TEXT_SLASH + Common::PrefixRig + TEXT_SLASH + name + TEXT_SLASH;
	FString outDir = Common::DirGameAsset + type + TEXT_SLASH + categoryName + TEXT_SLASH + Common::PrefixRig + TEXT_SLASH;
	FString outName = Common::PrefixSKEL + TEXT_UNDERSCORE + name;

	return TPair<FString, FString>(outDir, outName);
}

TPair<FString, FString> AssetInfo::GetAnimationContentPath() const
{
	FString outDir = Common::DirContentAnimation + episodeDir + sceneDir + cutDir;
	//FString outName = Common::PrefixAS + TEXT_UNDERSCORE + episodeName + sceneName + cutName + type + TEXT_UNDERSCORE + name;
	//FString outName = Common::PrefixAS + TEXT_UNDERSCORE + episodeName + sceneName + cutName + type + TEXT_UNDERSCORE + originName;
	FString outName = prefixAsset + TEXT_UNDERSCORE + episodeName + sceneName + cutName + type + TEXT_UNDERSCORE + originName;

	return TPair<FString, FString>(outDir, outName);
}

TPair<FString, FString> AssetInfo::GetMasterDestinationPath() const
{
	//`Deprecated
	FString outDir = Common::DirGameAsset + type + TEXT_SLASH + prefixAsset + TEXT_UNDERSCORE + name + TEXT_SLASH;
	FString outName = prefixAsset + TEXT_UNDERSCORE + Common::SuffixEpisode + TEXT_UNDERSCORE + name;

	return TPair<FString, FString>(outDir, outName);
}

TTuple<int32, int32, int32> AssetInfo::GetNumInfo() const
{
	int32 episodeNum = GetEpisodeNumber();
	int32 sceneNum = GetSceneNumber();
	int32 cutNum = GetCutNumber();

	return TTuple<int32, int32, int32>(episodeNum, sceneNum, cutNum);
}

TTuple<FString, FString, FString> AssetInfo::GetNumInfoStr() const
{
	return TTuple<FString, FString, FString>(episodeNumber, sceneNumber, cutNumber);
}
