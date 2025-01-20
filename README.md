# Animation Production Pipeline Using Unreal Engine

## Preface
- This is a tool for automating FBX import and Sequence creation for animation production in Unreal Engine.
- It is developed as an Editor Toolbar Extension Plugin.
- Files are imported and created according to our own naming conventions and production standards, which are managed in [AssetInfo](./Common/AssetInfo.h).
- Additionally, key strings based on these conventions, as well as important frame and resolution settings, are categorized and managed under [Common](./Common/Common.h).

<img src="./Images/image_preface1.png" width="40%"/> -> <img src="./Images/image_preface2.png" width="40%"/>

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

## Sequence Generator
- Create files by own naming convention

## Packet Manager for Web Server
- Check asset data

## Usage Examples
<img src="./Images/image_toolUI1.png" width="100%"/>
<img src="./Images/image_toolUI2.png" width="100%"/>
