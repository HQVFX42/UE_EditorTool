#include "Common/Common.h"

#include "Misc/FrameRate.h"
#include "Misc/Paths.h"

const FString Common::CategoryPipeline = TEXT("Pipeline");
const FString Common::CategoryTemp = TEXT("_Temp");
const FString Common::CategoryChangeMesh = TEXT("Change Mesh");

const FString Common::ExtensionAbc = TEXT(".abc");
const FString Common::ExtensionFbx = TEXT(".fbx");
const FString Common::ExtensionUasset = TEXT(".uasset");

const FString Common::PrefixHIGH = TEXT("HIGH");
const FString Common::PrefixPROP = TEXT("PRP");
const FString Common::PrefixRig = TEXT("Rig");
const FString Common::PrefixCAM = TEXT("CAM");
const FString Common::PrefixCH = TEXT("CH");
const FString Common::PrefixSKEL = TEXT("SKEL");
const FString Common::PrefixSM = TEXT("SM");
const FString Common::PrefixAS = TEXT("LOW");
const FString Common::PrefixAC = TEXT("BP");
const FString Common::PrefixLS = TEXT("LS");
const FString Common::PrefixEP = TEXT("EP");
const FString Common::PrefixS = TEXT("S");
const FString Common::PrefixC = TEXT("C");

const FString Common::SuffixCamera = TEXT("Camera_001");
const FString Common::SuffixSkeleton = TEXT("Skeleton");
const FString Common::SuffixAni = TEXT("ANI");
const FString Common::SuffixBack = TEXT("BG");
const FString Common::SuffixEffect = TEXT("FX");
const FString Common::SuffixLight = TEXT("LGT");
const FString Common::SuffixEpisode = TEXT("MASTER");
const FString Common::SuffixScene = TEXT("SCENE");
const FString Common::SuffixCut = TEXT("CUT");
const FString Common::SuffixTest = TEXT("");

const FString Common::DirContentAsset = FPaths::ProjectContentDir() + TEXT("00_ASSET/");
const FString Common::DirContentAnimation = FPaths::ProjectContentDir() + TEXT("01_ANIMATION/");
const FString Common::DirContentEpisode = FPaths::ProjectContentDir() + TEXT("02_EPISODES/");

const FString Common::DirGameAsset = TEXT("/Game/00_ASSET/");
const FString Common::DirGameAnimation = TEXT("/Game/01_ANIMATION/");
const FString Common::DirGameEpisode = TEXT("/Game/02_EPISODES/");

const FString Common::DirServerAsset = TEXT("R:/");
const FString Common::DirPluginAsset = TEXT("/EditorMenuTool/");

const FFrameNumber Common::AdjGeneralStart = 0;
const FFrameNumber Common::AdjPlaybackStart = 0;
const FFrameNumber Common::AdjMotionBlur = -5;

const FFrameRate Common::tickResolution = FFrameRate(24000, 1);
const FFrameRate Common::displayRate = FFrameRate(30, 1);

FString Common::TitlePipeline()
{
	return Common::CategoryPipeline + TEXT(" Automation");
}

FString Common::TitleTemp()
{
	return Common::CategoryTemp + TEXT(" Menu");
}

FString Common::TitleChangeMesh()
{
	return Common::CategoryChangeMesh + " Automation";
}

FString Common::FrontAS()
{
	return Common::PrefixAS + TEXT_UNDERSCORE;
}

FString Common::EUWBPPath()
{
	return Common::DirPluginAsset + TEXT("WBP_SequenceEditor");
}

FFrameRate Common::GetTickResolution()
{
	return tickResolution;
}

FFrameRate Common::GetDisplayRate()
{
	return displayRate;
}

FFrameNumber Common::GetStartKeyTime()
{	
	//** Frame calculation */
	//FFrameRate TickResolution = ShotSequence->GetMovieScene()->GetTickResolution();
	//int32 ShotFrameUnit = TickResolution.AsDecimal() / 30;
	//int32 ShotDuration = (TickResolution).RoundToFrame().Value;
	return (Common::AdjMotionBlur * Common::GetTickResolution().AsDecimal() / Common::GetDisplayRate().AsDecimal());
}

MenuCommand::MenuCommand() : TCommands<MenuCommand>(TEXT("Pipeline"), FText::FromString("Pipeline Automation"), NAME_None, FAppStyle::GetAppStyleSetName())
{
}

void MenuCommand::RegisterCommands()
{
	#define LOCTEXT_NAMESPACE "Pipeline"

	UI_COMMAND(TestFunction, "Testing fucntion", "Testing...", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(CreateWidget, "Open Sequence Editor", "Open customized sequence editor", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(ImportFBX, "Import FBX with dialog", "FBX Import automation", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(SequenceGenerator, "Generate Sequence", "Update sequence if you need", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(AllTasks, "Initiate All tasks", "Automate import & generate sequence", EUserInterfaceActionType::Button, FInputGesture());

	UI_COMMAND(ChangeKoreanSignMesh, "Change to korean sign", "Automate change korean sign mesh", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(ChangeAlienSignMesh, "Change to alien sign", "Automate change alien sign mesh", EUserInterfaceActionType::Button, FInputGesture());

	#undef LOCTEXT_NAMESPACE
}


bool DirVisitor::Visit(const TCHAR* pathOrDir, bool isDir)
{
	if (isDir)
	{
		OutDirArray.Add(pathOrDir);
		LOG(Display, TEXT("dir found : %s"), pathOrDir);
	}
	else
	{
		OutPathArray.Add(pathOrDir);
		LOG(Display, TEXT("file found : %s"), pathOrDir);
	}

	return true;
}
