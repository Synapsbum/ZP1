#include "r3dPCH.h"
#include "r3dProtect.h"

#include "m_AppSelect.h"

#include "CkHttpRequest.h"
#include "CkHttp.h"
#include "CkHttpResponse.h"

#include "../SF/Console/CmdProcessor.h"
#include "../SF/Console/Config.h"
#include "../SF/Console/Console.h"
#include "../SF/Version.h"

#include "r3dDebug.h"

#include "GameCommon.h"

#include "GameLevel.h"
#include "TrueNature2/Terrain3.h"

#include "Main_Network.h"

bool Destroy = false;
bool Play_Editor = false;
bool NeworkGame = false;
bool gNewLevelCreated = false;
bool DevTest = false;

const char* g_ServerKey;
extern int gDomainPort;
extern bool gDomainUseSSL;

#include "CkByteData.h"

void InputUpdate();

#ifndef FINAL_BUILD
Menu_AppSelect* mMenu_AppSelect;

void Create_AppSelect()
{
	mMenu_AppSelect = new Menu_AppSelect();
	mMenu_AppSelect->Init();
}

void Destroy_AppSelect()
{
	delete mMenu_AppSelect;
}

Menu_AppSelect::Menu_AppSelect()
{
	isInited_ = false;
}

Menu_AppSelect::~Menu_AppSelect()
{
}

extern bool g_bExit;
extern bool g_bEditMode;
extern bool g_bStartedAsParticleEditor;
extern bool g_bStartedAsPhysXEditor;
extern bool g_bStartedAsCharacterEditor;
extern void PlayEditor();

void SaveLevelData(char* Str);

extern bool IsNeedExit();
void Menu_AppSelect::Load()
{
	if (!gfxAppSelect.Load("Data\\Menu\\Studio\\Menu_AppSelect.swf", true))
		return;

#define MAKE_CALLBACK(FUNC) new r3dScaleformMovie::TGFxEICallback<Menu_AppSelect>(this, &Menu_AppSelect::FUNC)
	gfxAppSelect.RegisterEventHandler("eventExit", MAKE_CALLBACK(eventExit));
	gfxAppSelect.RegisterEventHandler("eventUpdateDB", MAKE_CALLBACK(eventUpdateDB));
	gfxAppSelect.RegisterEventHandler("eventParticleEditor", MAKE_CALLBACK(eventParticleEditor));
	gfxAppSelect.RegisterEventHandler("eventPhyscsEditor", MAKE_CALLBACK(eventPhyscsEditor));
	gfxAppSelect.RegisterEventHandler("eventCharacterEditor", MAKE_CALLBACK(eventCharacterEditor));
	gfxAppSelect.RegisterEventHandler("eventLoadMap", MAKE_CALLBACK(eventLoadMap));
	gfxAppSelect.RegisterEventHandler("eventLiveMap", MAKE_CALLBACK(eventLiveMap));
	gfxAppSelect.RegisterEventHandler("eventEditorMap", MAKE_CALLBACK(eventEditorMap));
	gfxAppSelect.RegisterEventHandler("eventCreateMap", MAKE_CALLBACK(eventCreateMap));
	gfxAppSelect.RegisterEventHandler("eventServer", MAKE_CALLBACK(eventServer));
	gfxAppSelect.RegisterEventHandler("eventDeleteMap", MAKE_CALLBACK(eventDeleteMap));
	gfxAppSelect.RegisterEventHandler("eventAssetsEditor", MAKE_CALLBACK(eventAssetsEditor));

	isInited_ = true;

	Destroy = false;
	Play_Editor = false;
	NeworkGame = false;
}

void Menu_AppSelect::Init()
{
	Load();

	// Frame Loop
	while (!Destroy) {
		if (IsNeedExit())
		{
			TerminateProcess(r3d_CurrentProcess, 0);
			break;
		}

		if (Keyboard->WasPressed(kbsF5)) // ServerTest
		{
			NeworkGame = true;
			DevTest = true;
			break;
		}

		Draw();

		Sleep(1);
	} // END

	ExecuteFunction(Play_Editor, NeworkGame);
}

void Menu_AppSelect::Draw()
{
	extern void tempDoMsgLoop();
	tempDoMsgLoop();

	if (r3dRenderer->DeviceAvailable)
	{
		gfxAppSelect.SetCurentRTViewport(Scaleform::GFx::Movie::SM_NoScale);
		gfxAppSelect.SetCurentRTViewport(Scaleform::GFx::Movie::SM_ExactFit);

		InputUpdate();

		gfxAppSelect.SetKeyboardCapture();

		r3dMouse::Show();
		r3dStartFrame();

		r3dRenderer->StartRender(1);
		r3dRenderer->StartFrame();

		r3dRenderer->SetRenderingMode(R3D_BLEND_ALPHA | R3D_BLEND_NZ);

		r3dRenderer->SetViewport(0.f, 0.f, (float)r3dRenderer->d3dpp.BackBufferWidth, (float)r3dRenderer->d3dpp.BackBufferHeight);
		D3D_V(r3dRenderer->pd3ddev->Clear(0, NULL, D3DCLEAR_TARGET, 0, 1.f, 0));

		gfxAppSelect.UpdateAndDraw();

		Console::RenderConsole();

		r3dRenderer->Flush();
		r3dRenderer->EndFrame();
		r3dRenderer->EndRender(true);

		r3dEndFrame();
	}
}

void DestroyMenuAppSelect()
{
	Destroy = true; // Break Loop Frame
}

/* Event's */
#pragma region Events
void Menu_AppSelect::ExecuteFunction(bool Play_Editor, bool NetworkGame)
{
	Destroy_AppSelect();

	if (Play_Editor)
		PlayEditor();
	else if (NetworkGame)
		ExecuteNetworkGame(DevTest);
}

void Menu_AppSelect::eventUpdateDB(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	if (!IsInited())
		return;

	UpdateDB(g_api_ip->GetString(), "Data/Weapons/itemsDB.xml");
}

void Menu_AppSelect::eventExit(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	if (!IsInited())
		return;

	r3dOutToLog("Exiting Application...\n");
	g_bExit = true;
}

void Menu_AppSelect::eventParticleEditor(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	if (!IsInited())
		return;

	g_bStartedAsParticleEditor = true;
	g_bStartedAsPhysXEditor = false;
	g_bStartedAsCharacterEditor = false;

	PrepareEditor("Editor_Particle", "Levels\\WorkInProgress\\");
	Play_Editor = true;
	DestroyMenuAppSelect();
}

void Menu_AppSelect::eventPhyscsEditor(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	if (!IsInited())
		return;

	g_bStartedAsParticleEditor = false;
	g_bStartedAsPhysXEditor = true;
	g_bStartedAsCharacterEditor = false;

	PrepareEditor("Editor_Physics", "Levels\\WorkInProgress\\");
	Play_Editor = true;
	DestroyMenuAppSelect();
}

void Menu_AppSelect::eventCharacterEditor(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	if (!IsInited())
		return;

	g_bStartedAsParticleEditor = false;
	g_bStartedAsPhysXEditor = false;
	g_bStartedAsCharacterEditor = true;

	PrepareEditor("Editor_Character", "Levels\\WorkInProgress\\");
	Play_Editor = true;
	DestroyMenuAppSelect();
}

void Menu_AppSelect::eventLoadMap(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	if (!IsInited())
		return;

	const char* LevelName = args[0].GetString();
	int tab = args[1].GetInt();

	g_bStartedAsParticleEditor = false;
	g_bStartedAsPhysXEditor = false;
	g_bStartedAsCharacterEditor = false;

	switch (tab)
	{
	case 1:
		PrepareEditor(LevelName);
		break;
	case 2:
		PrepareEditor(LevelName, "Levels\\WorkInProgress\\");
		break;

	default:
		PrepareEditor(LevelName);
		break;
	}

	Play_Editor = true;
	DestroyMenuAppSelect();
}

void Menu_AppSelect::eventLiveMap(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	if (!IsInited())
		return;

	AddLevelToList("Levels\\*.*");
}

void Menu_AppSelect::eventEditorMap(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	if (!IsInited())
		return;

	AddLevelToList("Levels\\WorkInProgress\\*.*");
}

void Menu_AppSelect::eventCreateMap(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	if (!IsInited())
		return;

	const char* Name = args[0].GetString();
	double TerreinStartHeight = args[1].GetNumber();
	double TerrainHeightRange = args[2].GetNumber();
	double CellSize = args[3].GetNumber();
	int GridSize = args[4].GetInt();

	if (CreateNewLevel(Name, (float)TerreinStartHeight, (float)TerrainHeightRange, (float)CellSize, GridSize))
	{
		PrepareEditor(Name, "Levels\\WorkInProgress");
		Play_Editor = true;
		DestroyMenuAppSelect();
	}
	else
	{
		Scaleform::GFx::Value var[3];
		var[0].SetString("Failed to create new map, reason: map name already exist!");
		var[1].SetBoolean(true);
		var[2].SetString("Error");
		pMovie->Invoke("_root.api.showInfoMsg", var, 3);
	}
}

void Menu_AppSelect::eventServer(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	if (!IsInited())
		return;

	const char* Mode = args[0].GetString();

	// Execute the NetworkGame
	Destroy = true;
	Play_Editor = false;
	NeworkGame = true;
}

void Menu_AppSelect::eventDeleteMap(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	if (!IsInited())
		return;

	std::string LevelName = args[0].GetString();
	int tab = args[1].GetInt();

	switch (tab)
	{
	case 1:
		DeleteDirectory("Levels\\" + LevelName);
		AddLevelToList("Levels\\*.*");
		break;

	case 2:
		DeleteDirectory("Levels\\WorkInProgress\\" + LevelName);
		AddLevelToList("Levels\\WorkInProgress\\*.*");
		break;
	}

}

void Menu_AppSelect::eventAssetsEditor(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	PrepareEditor("Editor_Base", "Levels\\WorkInProgress\\");
	Play_Editor = true;
	DestroyMenuAppSelect();
}
#pragma endregion

/* Function's */
#pragma region Functions
stringlist_t DirFiles;

std::set< std::string > ListExtensions;

static void* ____DummyObjectConstructor()
{
	return NULL;
}

bool Menu_AppSelect::CreateNewLevel(const char* Name, float TerrainStartHeight, float TerrainHeightRange, float CellSize, int GridSize)
{
	char Path[256];
	char Str[256];

	sprintf(Path, "Levels\\WorkInProgress\\%s", Name);
	if (mkdir(Path) == -1)
	{
		if (errno == EEXIST)
		{
			r3dOutToLog("Map with name \"%s\" already exist.\n", Name);
			return false;
		}
	}

	PrepareEditor(Name, "Levels\\WorkInProgress\\");

	r3d_assert(g_pPhysicsWorld == 0);
	g_pPhysicsWorld = game_new PhysXWorld();
	g_pPhysicsWorld->Init();

	r3dTerrain3::CreationParams params;

	// set settings
	params.StartHeight = TerrainStartHeight;
	params.HeightRange = TerrainHeightRange;

	params.CellCountX = GridSize;
	params.CellCountZ = GridSize;
	params.MaskSizeX = GridSize;
	params.MaskSizeZ = GridSize;

	params.CellSize = CellSize;
	params.LevelDir = r3dString("Levels\\WorkInProgress\\") + Name + "\\";;

	r3dTerrain3 createTerrain;

	// set first layer
	r3dTerrain3Layer lay = createTerrain.GetLayer(0);
	lay.DiffuseTex = r3dRenderer->LoadTexture("Data/TerrainData/Materials/RefTextures/1_white.dds");
	createTerrain.SetLayer(0, lay);

	// save settings
	createTerrain.SaveEmpty(params);

	// init gameworld and create objects
	GameWorld().Init(10, 10);
	obj_Terrain* pObjTerrain = game_new obj_Terrain();
	obj_Terrain& objTerrain = *pObjTerrain;
	objTerrain.DrawOrder = OBJ_DRAWORDER_FIRST;
	objTerrain.ObjFlags |= OBJFLAG_SkipCastRay;
	objTerrain.FileName = "terra1";
	AClass classData(NULL, "obj_Terrain", "Type", ____DummyObjectConstructor);
	classData.Name = "obj_Terrain";
	objTerrain.Class = &classData;
	GameWorld().AddObject(pObjTerrain);

	// save changed gameworld
	SaveLevelData(Str);
	GameWorld().Destroy();

	g_pPhysicsWorld->Destroy();
	SAFE_DELETE(g_pPhysicsWorld);

	delete g_pUndoHistory;
	g_pUndoHistory = NULL;

	gNewLevelCreated = true;

	return true;
}

bool Menu_AppSelect::FillFiles(const char *Dir, const char* szExtNames)
{
	WIN32_FIND_DATA fd;
	HANDLE hFind = ::FindFirstFile(Dir, &fd);
	if (hFind != INVALID_HANDLE_VALUE) {
		DirFiles.clear();
		do {
			if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
				DirFiles.push_back(fd.cFileName);
			}
		} while (::FindNextFile(hFind, &fd));
		::FindClose(hFind);
	}
	else
	{
		DirFiles.clear();
		return false;
	}

	return true;
}

void Menu_AppSelect::AddLevelToList(const char* Path)
{
	if (!IsInited())
		return;

	gfxAppSelect.Invoke("_root.api.clearList", "");

	if (FillFiles(Path, LevelName))
	{
		Scaleform::GFx::Value var[1];
		for (unsigned int i = 0; i < DirFiles.size(); i++)
		{
			var[0].SetString(DirFiles.at(i).c_str());
			gfxAppSelect.Invoke("_root.api.addToList", var, 1);
		}

		gfxAppSelect.Invoke("_root.api.RefreshList", "");
	}
}

void Menu_AppSelect::UpdateDB(const char* api_addr, const char* out_xml)
{
	CkHttp http;
	int success = http.UnlockComponent("wLiu9P.CB10599_JruQWlT4Da2k");
	if (success != 1)
	{
		Scaleform::GFx::Value var[3];
		var[0].SetString("Internal error!!!");
		var[1].SetBoolean(true);
		var[2].SetString("Error");
		gfxAppSelect.Invoke("_root.api.showInfoMsg", var, 3);
		return;
	}

	// get items DB
	{
		CkHttpRequest req;
		req.UsePost();
		req.put_Path("/ZP/api/php/api_getItemsDB.php");//ServerMode = 1
		req.AddParam("serverkey", "AD23D6F8-688B-4D79-B4D5-B3F661FD4558");//F1F5F7BA-0E83-47c1-B670-425B62D8F46E

		CkHttpResponse *resp = 0;
		resp = http.SynchronousRequest(api_addr, gDomainPort, gDomainUseSSL, req);
		if (!resp)
		{
			Scaleform::GFx::Value var[3];
			var[0].SetString("timeout getting items db");
			var[1].SetBoolean(true);
			var[2].SetString("Error");
			gfxAppSelect.Invoke("_root.api.showInfoMsg", var, 3);
			return;
		}

		// we can't use getBosyStr() because it'll fuckup characters inside UTF-8 xml
		CkByteData bodyData;
		resp->get_Body(bodyData);

		pugi::xml_document xmlFile;
		pugi::xml_parse_result parseResult = xmlFile.load_buffer_inplace(
			(void*)bodyData.getBytes(),
			bodyData.getSize(),
			pugi::parse_default,
			pugi::encoding_utf8);
		if (!parseResult)
			r3dError("Failed to parse server weapon XML, error: %s", parseResult.description());

		xmlFile.save_file(
			out_xml,
			PUGIXML_TEXT("\t"),
			pugi::format_default,
			pugi::encoding_utf8);
	}

	Scaleform::GFx::Value var[3];
	var[0].SetString("ItemsDB has been updated!");
	var[1].SetBoolean(true);
	var[2].SetString("Succesfully");
	gfxAppSelect.Invoke("_root.api.showInfoMsg", var, 3);
}

void Menu_AppSelect::PrepareEditor(const char* levelName, const char* Path)
{
	r3dGameLevel::SetHomeDir(levelName, Path);

	char Str[256];
	sprintf(Str, "%s\\Constants.var", r3dGameLevel::GetHomeDir());
	cvars_Read(Str);
}

int Menu_AppSelect::DeleteDirectory(const std::string &refcstrRootDirectory, bool bDeleteSubdirectories)
{
	bool            bSubdirectory = false;       // Flag, indicating whether
	// subdirectories have been found
	HANDLE          hFile;                       // Handle to directory
	std::string     strFilePath;                 // Filepath
	std::string     strPattern;                  // Pattern
	WIN32_FIND_DATA FileInformation;             // File information


	strPattern = refcstrRootDirectory + "\\*.*";
	hFile = ::FindFirstFile(strPattern.c_str(), &FileInformation);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (FileInformation.cFileName[0] != '.')
			{
				strFilePath.erase();
				strFilePath = refcstrRootDirectory + "\\" + FileInformation.cFileName;

				if (FileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					if (bDeleteSubdirectories)
					{
						// Delete subdirectory
						int iRC = DeleteDirectory(strFilePath, bDeleteSubdirectories);
						if (iRC)
							return iRC;
					}
					else
						bSubdirectory = true;
				}
				else
				{
					// Set file attributes
					if (::SetFileAttributes(strFilePath.c_str(),
						FILE_ATTRIBUTE_NORMAL) == FALSE)
						return ::GetLastError();

					// Delete file
					if (::DeleteFile(strFilePath.c_str()) == FALSE)
						return ::GetLastError();
				}
			}
		} while (::FindNextFile(hFile, &FileInformation) == TRUE);

		// Close handle
		::FindClose(hFile);

		DWORD dwError = ::GetLastError();
		if (dwError != ERROR_NO_MORE_FILES)
			return dwError;
		else
		{
			if (!bSubdirectory)
			{
				// Set directory attributes
				if (::SetFileAttributes(refcstrRootDirectory.c_str(),
					FILE_ATTRIBUTE_NORMAL) == FALSE)
					return ::GetLastError();

				// Delete directory
				if (::RemoveDirectory(refcstrRootDirectory.c_str()) == FALSE)
					return ::GetLastError();
			}
		}
	}

	return 0;
}
#pragma endregion
#endif