#include "r3dPCH.h"
#include "r3d.h"

#include "LoadingScreen.h"

#include "../SF/Console/CmdProcessor.h"
#include "../SF/Console/Config.h"
#include "../SF/Console/Console.h"
#include "../SF/Version.h"

#include "Multiplayer\MasterServerLogic.h"

#include "LangMngr.h"

#include "r3dDeviceQueue.h"

extern bool g_bEditMode;
bool IsNeedExit();
void InputUpdate();

#define IMAGE "LoadingScreen.dds"
#undef USE_LEVEL_LOADINGSCREEN

LoadingScreen* gLoadingScreen = new LoadingScreen();

LoadingScreen::LoadingScreen()
{
}

LoadingScreen::~LoadingScreen()
{
}

static volatile float	gProgress;
void LoadingScreen::Load()
{
	if (!gfxLoadingScreen.Load("Data\\Menu\\LoadingScreen\\LoadingScreen.swf", true))
		return;

#define MAKE_CALLBACK(FUNC) game_new r3dScaleformMovie::TGFxEICallback<LoadingScreen>(this, &LoadingScreen::FUNC)
}

int LoadingScreen::Update()
{
	extern void tempDoMsgLoop();
	tempDoMsgLoop();

	if (r3dRenderer->DeviceAvailable)
	{
		InputUpdate();

		gfxLoadingScreen.SetKeyboardCapture();
		gfxLoadingScreen.SetCurentRTViewport(Scaleform::GFx::Movie::SM_NoScale);
		gfxLoadingScreen.SetCurentRTViewport(Scaleform::GFx::Movie::SM_ExactFit);

		r3dMouse::Show();
		r3dStartFrame();

		r3dRenderer->StartRender(1);
		r3dRenderer->StartFrame();

		r3dRenderer->SetRenderingMode(R3D_BLEND_ALPHA | R3D_BLEND_NZ);

		r3dRenderer->SetViewport(0.f, 0.f, (float)r3dRenderer->d3dpp.BackBufferWidth, (float)r3dRenderer->d3dpp.BackBufferHeight);
		D3D_V(r3dRenderer->pd3ddev->Clear(0, NULL, D3DCLEAR_TARGET, 0, 1.f, 0));

		gfxLoadingScreen.UpdateAndDraw();

		Console::RenderConsole();

		r3dRenderer->Flush();
		r3dRenderer->EndFrame();
		r3dRenderer->EndRender(true);

		r3dEndFrame();
	}

	return 0;
}

void LoadingScreen::SetLoading()
{
	gfxLoadingScreen.Invoke("_root.api.SetLoading", "");
}

void LoadingScreen::SetLoadingImage(const char* ImagePath)
{
	Scaleform::GFx::Value var[1];
	var[0].SetString(ImagePath);
	gfxLoadingScreen.Invoke("_root.api.SetLoadingBackground", var, 1);
}

void LoadingScreen::SetData(const char* LevelName)
{
	Scaleform::GFx::Value var[2];
	var[0].SetString(LevelName);
	var[1].SetString(GetRandomTips());
	gfxLoadingScreen.Invoke("_root.api.SetData", var, 2);
}

void LoadingScreen::SetDataConnecting()
{
	Scaleform::GFx::Value var[1];
	var[0].SetString("Connecting...");
	gfxLoadingScreen.Invoke("_root.api.SetDataConnect", var, 1);
}

void LoadingScreen::SetProgress(float progress)
{
	Scaleform::GFx::Value var[1];
	var[0].SetNumber(progress*100.0f);
	gfxLoadingScreen.Invoke("_root.api.SetProgress", var, 1);
}

const char* LoadingScreen::GetRandomTips()
{
	struct Tip
	{
		const char* text;
	};
	typedef stdext::hash_map<DWORD, Tip> Tips;
	Tips Tips_;
	DWORD sumAdded = 0;

	const char* xmlPath = "Data\\Menu\\LoadingScreen\\Tips.xml";

	r3dFile* file = r3d_open(xmlPath, "rb");
	if (!file)
	{
		r3dOutToLog("Failed to open <LoadingScreen>Tips configuration file: %s\n", xmlPath);
		return "";
	}

	char* buffer = game_new char[file->size + 1];
	fread(buffer, file->size, 1, file);
	buffer[file->size] = 0;

	pugi::xml_document xmlDoc;
	pugi::xml_parse_result parseResult = xmlDoc.load_buffer_inplace(buffer, file->size);
	fclose(file);

	if (!parseResult)
		r3dError("Failed to parse <LoadingScreen>Tips XML file, error: %s", parseResult.description());

	pugi::xml_node Images = xmlDoc.child("Tips");
	pugi::xml_node Image = Images.child("Tip");

	while (Image)
	{
		Tip t;
		t.text = Image.attribute("text").value();
		Tips_.insert(Tips::value_type(sumAdded++, t));
		Image = Image.next_sibling();
	}

	delete[] buffer;

	Tips::iterator it = Tips_.find(u_random(sumAdded));
	return gLangMngr.getString(it->second.text);
}

const char* LoadingScreen::GetRandomLoadingImage()
{
	struct LoadingScreen
	{
		const char* Path;
	};
	typedef stdext::hash_map<DWORD, LoadingScreen> LoadingScreens;
	LoadingScreens LoadingScreens_;
	DWORD sumAdded = 0;

	const char* xmlPath = "Data\\Menu\\LoadingScreen\\Images.xml";

	r3dFile* file = r3d_open(xmlPath, "rb");
	if (!file)
	{
		r3dOutToLog("Failed to open <LoadingScreen>Images configuration file: %s\n", xmlPath);
		return "";
	}

	char* buffer = game_new char[file->size + 1];
	fread(buffer, file->size, 1, file);
	buffer[file->size] = 0;

	pugi::xml_document xmlDoc;
	pugi::xml_parse_result parseResult = xmlDoc.load_buffer_inplace(buffer, file->size);
	fclose(file);

	if (!parseResult)
		r3dError("Failed to parse <LoadingScreen>Images XML file, error: %s", parseResult.description());

	pugi::xml_node Images = xmlDoc.child("Images");
	pugi::xml_node Image = Images.child("Image");

	while (Image)
	{
		LoadingScreen l;
		l.Path = Image.attribute("Path").value();
		LoadingScreens_.insert(LoadingScreens::value_type(sumAdded++, l));
		Image = Image.next_sibling();
	}

	delete[] buffer;

	LoadingScreens::iterator it = LoadingScreens_.find((u_random(sumAdded)));
	return it->second.Path;
}

//------------------------------------------------------------------------

void SetLoadingProgress(float progress)
{
	progress = R3D_MAX(R3D_MIN(progress, 1.f), 0.f);
	InterlockedExchange((volatile long*)&gProgress, (LONG&)progress);
}

void AdvanceLoadingProgress(float add)
{
	float newVal = R3D_MAX(R3D_MIN(gProgress + add, 1.f), 0.f);
	InterlockedExchange((volatile long*)&gProgress, (LONG&)newVal);
}

float GetLoadingProgress()
{
	return gProgress;
}

void SetLoadingPhase(const char* Phase)
{
	Phase;
}

//------------------------------------------------------------------------
bool IsNeedExit();
int DoLoadingScreen(volatile LONG* loading, const char* LevelFolder, const char* LevelName, int gameMode)
{
	r3d_assert(gLoadingScreen);

	char sFullPath[512];
#ifdef USE_LEVEL_LOADINGSCREEN
	sprintf(sFullPath, "$%s\\%s", LevelFolder, IMAGE);
#else
	sprintf(sFullPath, "%s", gLoadingScreen->GetRandomLoadingImage());
#endif

	gLoadingScreen->SetLoading();
	gLoadingScreen->SetLoadingImage(sFullPath);
	gLoadingScreen->SetData(LevelName);

	while (*loading)
	{
		r3dProcessWindowMessages();

		if (IsNeedExit())
		{
			TerminateProcess(r3d_CurrentProcess, 0);
			return 0;
		}

		gLoadingScreen->Update();
		gLoadingScreen->SetProgress(gProgress);
	}

	return 1;
}

//------------------------------------------------------------------------

int DoConnectScreen(volatile LONG* Loading, float TimeOut)
{
	r3d_assert(gLoadingScreen);

	gLoadingScreen->SetLoading();
	gLoadingScreen->SetDataConnecting();

	bool checkTimeOut = TimeOut != 0.f;

	float endWait = r3dGetTime() + TimeOut;

	while (*Loading)
	{
		r3dProcessWindowMessages();

		if (IsNeedExit())
		{
			TerminateProcess(r3d_CurrentProcess, 0);
			return 0;
		}

		if (checkTimeOut && r3dGetTime() > endWait)
			return 0;

		gLoadingScreen->Update();
		gLoadingScreen->SetProgress(checkTimeOut ? 1.f - (endWait - r3dGetTime()) / TimeOut : gProgress);
	}

	return 1;
}

//------------------------------------------------------------------------

int DoConnectScreen(volatile LONG* Loading, const char* Message, float TimeOut)
{
	r3d_assert(gLoadingScreen);

	gLoadingScreen->SetLoading();
	gLoadingScreen->SetDataConnecting();

	bool checkTimeOut = TimeOut != 0.f;

	float endWait = r3dGetTime() + TimeOut;

	while (*Loading)
	{
		r3dProcessWindowMessages();

		if (IsNeedExit())
		{
			TerminateProcess(r3d_CurrentProcess, 0);
			return 0;
		}

		if (checkTimeOut && r3dGetTime() > endWait)
		{
			return 0;
		}

		gLoadingScreen->Update();
		gLoadingScreen->SetProgress(checkTimeOut ? 1.f - (endWait - r3dGetTime()) / TimeOut : gProgress);
		Sleep(33);
	}

	return 1;
}

//------------------------------------------------------------------------

template <typename T>
int DoConnectScreen(T* Logic, bool (T::*CheckFunc)(), const char* Message, float TimeOut)
{
	r3d_assert(gLoadingScreen);

	gLoadingScreen->SetLoading();
	gLoadingScreen->SetDataConnecting();

	bool checkTimeOut = TimeOut != 0.f;

	const float startWait = r3dGetTime();
	const float endWait = startWait + TimeOut;

	for (;;)
	{
		extern void tempDoMsgLoop();
		tempDoMsgLoop();

		if ((Logic->*CheckFunc)())
			break;

		if (IsNeedExit())
		{
			TerminateProcess(r3d_CurrentProcess, 0);
			return 0;
		}

		if (checkTimeOut && r3dGetTime() > endWait)
		{
			return 0;
		}

		if (r3dGetTime() > startWait + 1.0f)
		{
			gLoadingScreen->Update();
			gLoadingScreen->SetProgress(checkTimeOut ? 1.f - (endWait - r3dGetTime()) / TimeOut : gProgress);
		}

		Sleep(33);
	}

	return 1;
}

template int DoConnectScreen(ClientGameLogic* Logic, bool (ClientGameLogic::*CheckFunc)(), const char* Message, float TimeOut);
template int DoConnectScreen(MasterServerLogic* Logic, bool (MasterServerLogic::*CheckFunc)(), const char* Message, float TimeOut);

//------------------------------------------------------------------------