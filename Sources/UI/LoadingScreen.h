#ifndef LOADINGSCREEN_H
#define LOADINGSCREEN_H

#include "r3d.h"
#include "APIScaleformGfx.h"
#include "multiplayer/ClientGameLogic.h"

class LoadingScreen
{
private:
	r3dScaleformMovie gfxLoadingScreen;

public:
	LoadingScreen();
	virtual ~LoadingScreen();

	void 	Load();
	void	Unload() { gfxLoadingScreen.Unload(); }

	int Update();

	void StartLoadingScreen() { Load(); }
	void StopLoadingScreen() { Unload(); }

	void SetLoading();
	void SetLoadingImage(const char* ImagePath);
	void SetData(const char* LevelName);
	void SetDataConnecting();
	void SetProgress(float progress);
	const char* GetRandomTips();
	const char* GetRandomLoadingImage();
}; extern LoadingScreen* gLoadingScreen;

void SetLoadingProgress(float progress);
void AdvanceLoadingProgress(float add);
float GetLoadingProgress();
void SetLoadingPhase(const char* Phase);
int DoLoadingScreen(volatile LONG* loading, const char* LevelFolder, const char* LevelName, int gameMode);

template <typename T>
int DoConnectScreen(T* Logic, bool (T::*CheckFunc)(), const char* Message, float TimeOut);
int DoConnectScreen(volatile LONG* loading, const char* Message, float TimeOut);

#define PROGRESS_LOAD_LEVEL_START 0.0f
#define PROGRESS_LOAD_LEVEL_END 1.0f
#define PLAYER_CACHE_INIT_END 1.0f
#endif  //LOADINGSCREEN_H
