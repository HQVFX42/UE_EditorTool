# Animation Production Pipeline Using Unreal Engine

## Preface
- This is a tool for automating FBX import and Sequence creation for animation production in Unreal Engine.
- It is developed as an Editor Toolbar Extension Plugin.
- Files are imported and created according to our own naming conventions and production standards, which are managed in [AssetInfo](./Common/AssetInfo.h).
- Additionally, key strings based on these conventions, as well as important frame and resolution settings, are categorized and managed under [Common](./Common/Common.h).

<img src="./Images/image_preface1.png" width="100%"/>
<img src="./Images/image_preface2.png" width="100%"/>

## FBX Importer
- Get files by own naming convention
	```cpp
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
	```
- Import automation to specific folder
	```cpp
	void AssetInfo::SetPaths()
	{
		episodeDir = episodeNumber + TEXT_SLASH;
		episodeName = episodeNumber + TEXT_UNDERSCORE;
		sceneDir = sceneNumber + TEXT_SLASH;
		sceneName = sceneNumber + TEXT_UNDERSCORE;
		cutDir = cutNumber + TEXT_SLASH;
		cutName = cutNumber + TEXT_UNDERSCORE;
	}
	```
 - Bind assets using Enums
  	```cpp
   	/**
	 * Returns name matching enum value, returns empty string if invalid
	 * 
	 * @param value
	 * @return 
	 */
	template<typename T>
	FString EnumToString(const T value)
	{
	    FString nameStr = StaticEnum<T>()->GetNameStringByValue(static_cast<__underlying_type(T)>(value));
	    FString resultStr = nameStr.Len() == 0 ? "" : nameStr;
	    return resultStr;
	}
   	```

## Sequence Generator
- Create files by own naming convention
	```cpp
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
 	```
## Packet Manager for Web Server
- Check asset data & Collect logs
  	```cpp
   	void APacketManager::SendUnrealLog(FString verbosity, FString log, FString computerName)
	{
		TSharedRef<FJsonObject> requestObj = MakeShared<FJsonObject>();
		requestObj->SetStringField(EnumToString(EJasonFieldType::Type), EnumToString(EPacketType::UnrealLog));
		requestObj->SetStringField(EnumToString(EJasonFieldType::Name), computerName);
		requestObj->SetStringField(EnumToString(EJasonFieldType::Verbosity), verbosity);
		requestObj->SetStringField(EnumToString(EJasonFieldType::Log), log);
	
		SendPacket(requestObj);
	}
   	```
  	```cpp
   	#pragma once
	#include "CoreMinimal.h"
	#include "PacketManager.h"
	
	DECLARE_LOG_CATEGORY_EXTERN(MYLog, Log, All);
	
	//`1. To string called fucntion & line number
	#define LOG_INFO (FString(__FUNCTION__) + TEXT("(") + FString::FromInt(__LINE__) + TEXT(")"))
	//`2. Print n1 macro
	#define LOG_S(Verbosity) UE_LOG(MYLog, Verbosity, TEXT("%s"), *LOG_INFO)
	//`3. Print info & text
	#define LOG(Verbosity, Format, ...) UE_LOG(MYLog, Verbosity, TEXT("%s %s"), *LOG_INFO, *FString::Printf(Format, ##__VA_ARGS__))
	//`4. Print assertion
	#define CHECK_EXIT(Expr, ...) {if(!(Expr)) {LOG_SERVER(Error, TEXT("Assertion : %s"), TEXT("<"#Expr">")); return __VA_ARGS__;}}
	//`5. Print assertion with condition
	#define ENSURE_EXIT(Expr, ...) {if(ensure(Expr) == false) {LOG_SERVER(Error, TEXT("Assertion : <"#Expr"> --> %s"), (##__VA_ARGS__)); return;}}
	//`6. Print log on screen
	#define	LOG_SCREEN(Format, ...) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(Format, ##__VA_ARGS__))
	//`7. Print log on server
	#define LOG_Address FPlatformProcess::ComputerName()
	#define LOG_SERVER_INFO(Format, ...) (LOG_INFO + " --> ") + (TEXT("%s"), *FString::Printf(Format, ##__VA_ARGS__))
	#define LOG_SERVER_S(Verbosity) APacketManager::Instance()->SendUnrealLog(""#Verbosity"", LOG_INFO, LOG_Address)
	#define LOG_SERVER(Verbosity, Format, ...) APacketManager::Instance()->SendUnrealLog(""#Verbosity"", FString(LOG_SERVER_INFO(Format, ##__VA_ARGS__)), LOG_Address)
   	```

## Usage Examples
<img src="./Images/image_toolUI0.jpg" width="100%"/>
<img src="./Images/image_toolUI1.png" width="40%"/> <img src="./Images/image_toolUI4.jpg" width="40%"/>
<img src="./Images/image_toolUI2.png" width="100%"/>
