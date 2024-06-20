#include "r3dPCH.h"
#include "r3d.h"

#include "GameLevel.h"

namespace r3dGameLevel
{
	static char	HomeDirectory[256];
	static char	SaveDirectory[256];
	float		StartLevelTime = 0;

	r3dAtmosphere*	Environment = NULL;

	const char*	GetHomeDir() { return HomeDirectory; }
	const char* GetSaveDir() { return SaveDirectory; }
	void		SetHomeDir(const char* MapName, const char* Path);
	void		SetStartGameTime(float Time) { StartLevelTime = Time; }

	int IsSaving;
};

void r3dGameLevel::SetHomeDir(const char* MapName, const char* Path)
{
	sprintf(HomeDirectory, "%s\\%s", Path, MapName);
	strcpy(SaveDirectory, HomeDirectory);
}

void r3dGameLevel::SetSaveDir(const char* dir)
{
	strcpy(SaveDirectory, dir);
}
