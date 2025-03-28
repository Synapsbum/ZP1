#pragma once

#include "r3dAtmosphere.h"

extern_nspace(r3dGameLevel, float		StartLevelTime);

extern_nspace(r3dGameLevel, r3dAtmosphere*	Environment);

extern_nspace(r3dGameLevel, const char*	GetSaveDir());
extern_nspace(r3dGameLevel, const char*	GetHomeDir());
extern_nspace(r3dGameLevel, void		SetSaveDir(const char* dir));
extern_nspace(r3dGameLevel, void		SetHomeDir(const char* MapName, const char* Path = "Levels"));
extern_nspace(r3dGameLevel, void		SetStartGameTime(float Time));

extern_nspace(r3dGameLevel, int			IsSaving);

R3D_FORCEINLINE float EnvGetDayT()
{
	return r3dGameLevel::Environment->EnvGetDayT();
}

R3D_FORCEINLINE float EnvGetNightT()
{
	return r3dGameLevel::Environment->EnvGetNightT();
}

extern	int	LoadLevel_Objects(float BarRange, bool JustForBuilding_FastLoading = false);
extern  int LoadLevel_Groups();
extern	int	LoadLevel_MatLibs();
