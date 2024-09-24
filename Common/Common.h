#pragma once

#include "CoreMinimal.h"
#include "GenericPlatform/GenericPlatformFile.h"			//`Access dir visitor
#include "Log.h"
#include "PacketManager.h"

#define TEXT_UNDERSCORE TEXT("_")
#define TEXT_SLASH TEXT("/")
#define TEXT_MINUS_SIGN TEXT("-")

class Common
{
public:
	static const FString CategoryPipeline;
	static const FString CategoryTemp;
	static const FString CategoryChangeMesh;

	static const FString ExtensionAbc;
	static const FString ExtensionFbx;
	static const FString ExtensionUasset;

	static const FString PrefixHIGH;
	static const FString PrefixPROP;
	static const FString PrefixRig;
	static const FString PrefixCAM;
	static const FString PrefixCH;
	static const FString PrefixSKEL;
	static const FString PrefixSM;
	static const FString PrefixAS;
	static const FString PrefixAC;
	static const FString PrefixLS;
	static const FString PrefixEP;
	static const FString PrefixS;
	static const FString PrefixC;

	static const FString SuffixCamera;
	static const FString SuffixSkeleton;
	static const FString SuffixAni;
	static const FString SuffixBack;
	static const FString SuffixEffect;
	static const FString SuffixLight;
	static const FString SuffixEpisode;
	static const FString SuffixScene;
	static const FString SuffixCut;
	static const FString SuffixTest;

	static const FString DirContentAsset;
	static const FString DirContentAnimation;
	static const FString DirContentEpisode;

	static const FString DirGameAsset;
	static const FString DirGameAnimation;
	static const FString DirGameEpisode;

	static const FString DirServerAsset;
	static const FString DirPluginAsset;

	static const FFrameNumber AdjGeneralStart;
	static const FFrameNumber AdjPlaybackStart;
	static const FFrameNumber AdjMotionBlur;

public:
	static FString TitlePipeline();
	static FString TitleTemp();
	static FString TitleChangeMesh();

	static FString FrontAS();
	static FString EUWBPPath();

	static FFrameRate GetTickResolution();
	static FFrameRate GetDisplayRate();

	static FFrameNumber GetStartKeyTime();

	

private:
	static const FFrameRate tickResolution;
	static const FFrameRate displayRate;
};

class MenuCommand : public TCommands<MenuCommand>
{
public:
	MenuCommand();
	
public:
	TSharedPtr<FUICommandInfo> TestFunction;
	TSharedPtr<FUICommandInfo> CreateWidget;
	TSharedPtr<FUICommandInfo> ImportFBX;
	TSharedPtr<FUICommandInfo> SequenceGenerator;
	TSharedPtr<FUICommandInfo> AllTasks;

	// Use in ChangeMeshTool
	TSharedPtr<FUICommandInfo> ChangeKoreanSignMesh;
	TSharedPtr<FUICommandInfo> ChangeAlienSignMesh;

public:
	virtual void RegisterCommands() override;
};

struct DirVisitor : public IPlatformFile::FDirectoryVisitor
{
public:
	TArray<FString> OutDirArray;
	TArray<FString> OutPathArray;

public:
	virtual bool Visit(const TCHAR* pathOrDir, bool isDir) override;
};