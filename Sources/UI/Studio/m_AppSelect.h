#pragma once

#include "r3d.h"
#include "APIScaleformGfx.h"

#ifndef FINAL_BUILD
class Menu_AppSelect
{
private:
	bool isInited_;
	r3dScaleformMovie gfxAppSelect;

public:
	Menu_AppSelect();
	~Menu_AppSelect();

	bool    IsInited() { return isInited_; }

	void 	Load();
	void	Unload() { gfxAppSelect.Unload(); }
	void 	Init();

	void	Draw();

private:
	/* Event's */
	void	ExecuteFunction(bool Play_Editor, bool NetworkGame);
	void	eventUpdateDB(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventExit(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventParticleEditor(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventPhyscsEditor(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventCharacterEditor(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventLoadMap(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventLiveMap(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventEditorMap(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventCreateMap(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventServer(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventDeleteMap(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventAssetsEditor(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);

	/* Function's */
#ifndef FINAL_BUILD
	bool CreateNewLevel(const char* Name, float TerrainStartHeight, float TerrainHeightRange, float CellSize, int GridSize);
	bool FillFiles(const char *Dir, const char* szExtNames);
	void AddLevelToList(const char* Path);
	void UpdateDB(const char* api_addr, const char* out_xml);
	void PrepareEditor(const char* levelName, const char* Path = "Levels\\");
	int	DeleteDirectory(const std::string &refcstrRootDirectory, bool bDeleteSubdirectories = true);
#endif

	const char* LevelName;

};
void Create_AppSelect();
void Destroy_AppSelect();
#endif