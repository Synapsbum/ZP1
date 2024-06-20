#include "r3dPCH.h"
#include "r3d.h"

#include "GameCommon.h"
#include "r3dDebug.h"

#include "FrontendWarZ.h"
#include "GameCode\UserFriends.h"
#include "GameCode\UserRewards.h"
#include "GameCode\UserSkills.h"
#include "GameCode\UserClans.h"
#include "GameCode\UserSettings.h"

#include "CkHttpRequest.h"
#include "CkHttpResponse.h"
#include "backend/HttpDownload.h"
#include "backend/WOBackendAPI.h"

#include "../rendering/Deffered/CommonPostFX.h"
#include "../rendering/Deffered/PostFXChief.h"

#include "multiplayer/MasterServerLogic.h"
#include "multiplayer/LoginSessionPoller.h"

#include "../ObjectsCode/weapons/WeaponArmory.h"
#include "../ObjectsCode/weapons/Weapon.h"
#include "../ObjectsCode/weapons/Ammo.h"
#include "../ObjectsCode/weapons/Gear.h"
#include "../ObjectsCode/ai/AI_Player.h"
#include "../ObjectsCode/ai/AI_PlayerAnim.h"
#include "../ObjectsCode/Gameplay/UIWeaponModel.h"
#include "GameLevel.h"
#include "Scaleform/Src/Render/D3D9/D3D9_Texture.h"
#include "../../Eternity/Source/r3dEternityWebBrowser.h"

#include "SectorMaster.h"
#include "TeamSpeakClient.h"
#include "LoadingScreen.h"

#include "HWInfo.h"

#include "shellapi.h"
#include "SteamHelper.h"
#include "../Editors/CameraSpotsManager.h"

#include "../inject detect/VirtualizerSDK.h"

#include "../SF/Console/CmdProcessor.h"
#include "../SF/Console/Config.h"
#include "../SF/Console/Console.h"
#include "../SF/Version.h"

#include "..\..\bink\CutScene.h"
#include <iostream>
#include <thread>

#include "..\Main_Network.h"

#if USE_BINK_MOVIE
CutScene* Intro;
static bool IsPlayingIntro = false;
static bool Call_Login_Screen = false;
#endif

// for IcmpSendEcho
#undef NTDDI_VERSION
#define NTDDI_VERSION NTDDI_WINXP
#include <iphlpapi.h>
#include <icmpapi.h>
#pragma comment(lib, "iphlpapi.lib")

char	Login_PassedLoginID[256] = "";
char	Login_PassedPwd[256] = "";
char	Login_PassedAuth[256] = "";
char	Login_HardwareID[256] = "";
char	Login_HDDID[256] = "";
char    hardwareid[50] = "";
static int LoginMenuExitFlag = 0;

void writeGameOptionsFile();
extern r3dScreenBuffer*	Scaleform_RenderToTextureRT;

extern void ReloadLocalization();

float getRatio(float num1, float num2)
{
	if (num1 == 0)
		return 0.0f;
	if (num2 == 0)
		return num1;

	return num1 / num2;
}

const char* getTimePlayedString(int timePlayed)
{
	int seconds = timePlayed % 60;
	int minutes = (timePlayed / 60) % 60;
	int hours = (timePlayed / 3600) % 24;
	int days = (timePlayed / 86400);

	static char tmpStr[64];
	sprintf(tmpStr, "%d:%02d:%02d", days, hours, minutes);
	return tmpStr;
}

const char* getReputationString(int reputation)
{
	const char* algnmt = "$rep_civilian";
	if (reputation >= 250000)
		algnmt = "$rep_god";
	else if (reputation >= 200000)
		algnmt = "$rep_hero";
	else if (reputation >= 150000)
		algnmt = "$rep_general";
	else if (reputation >= 100000)
		algnmt = "$rep_colonel";
	else if (reputation >= 50000)
		algnmt = "$rep_major";
	else if (reputation >= 25000)
		algnmt = "$rep_captain";
	else if (reputation >= 10000)
		algnmt = "$rep_lieutenant";
	else if (reputation >= 5000)
		algnmt = "$rep_sheriff";
	else if (reputation >= 1000)
		algnmt = "$rep_paragon";
	else if (reputation >= 500)
		algnmt = "$rep_vigilante";
	else if (reputation >= 250)
		algnmt = "$rep_guardian";
	else if (reputation >= 80)
		algnmt = "$rep_lawmen";
	else if (reputation >= 20)
		algnmt = "$rep_deputy";
	else if (reputation >= 10)
		algnmt = "$rep_constable";
	else if (reputation <= -250000)
		algnmt = "$rep_serial_killer";
	else if (reputation <= -200000)
		algnmt = "$rep_killer";
	else if (reputation <= -150000)
		algnmt = "$rep_cannibal";
	else if (reputation <= -100000)
		algnmt = "$rep_predator";
	else if (reputation <= -50000)
		algnmt = "$rep_exterminator";
	else if (reputation <= -25000)
		algnmt = "$rep_dark_warrior";
	else if (reputation <= -10000)
		algnmt = "$rep_warrior";
	else if (reputation <= -5000)
		algnmt = "$rep_maniac";
	else if (reputation <= -1000)
		algnmt = "$rep_assassin";
	else if (reputation <= -600)
		algnmt = "$rep_villain";
	else if (reputation <= -300)
		algnmt = "$rep_hitman";
	else if (reputation <= -100)
		algnmt = "$rep_bandit";
	else if (reputation <= -25)
		algnmt = "$rep_outlaw";
	else if (reputation <= -5)
		algnmt = "$rep_thug";

	return algnmt;
}

const char* getReputationIconName(int reputation)
{
	const char* algnmt = "civilian";
	if (reputation >= 250000)
		algnmt = "god";
	else if (reputation >= 200000)
		algnmt = "hero";
	else if (reputation >= 150000)
		algnmt = "general";
	else if (reputation >= 100000)
		algnmt = "colonel";
	else if (reputation >= 50000)
		algnmt = "major";
	else if (reputation >= 25000)
		algnmt = "captain";
	else if (reputation >= 10000)
		algnmt = "lieutenant";
	else if (reputation >= 5000)
		algnmt = "sheriff";
	else if (reputation >= 1000)
		algnmt = "paragon";
	else if (reputation >= 500)
		algnmt = "vigilante";
	else if (reputation >= 250)
		algnmt = "guardian";
	else if (reputation >= 80)
		algnmt = "lawman";
	else if (reputation >= 20)
		algnmt = "deputy";
	else if (reputation >= 10)
		algnmt = "constable";
	else if (reputation <= -250000)
		algnmt = "serial_killer";
	else if (reputation <= -200000)
		algnmt = "killer";
	else if (reputation <= -150000)
		algnmt = "cannibal";
	else if (reputation <= -100000)
		algnmt = "predator";
	else if (reputation <= -50000)
		algnmt = "exterminator";
	else if (reputation <= -25000)
		algnmt = "dark_warrior";
	else if (reputation <= -10000)
		algnmt = "warrior";
	else if (reputation <= -5000)
		algnmt = "maniac";
	else if (reputation <= -1000)
		algnmt = "assassin";
	else if (reputation <= -600)
		algnmt = "villain";
	else if (reputation <= -300)
		algnmt = "hitman";
	else if (reputation <= -100)
		algnmt = "bandit";
	else if (reputation <= -20)
		algnmt = "outlaw";
	else if (reputation <= -5)
		algnmt = "thug";

	return algnmt;
}

const char* getMapName(int mapID)
{
	if (mapID == GBGameInfo::MAPID_ZP_Test) return gLangMngr.getString("MapColorado");
	else if (mapID == GBGameInfo::MAPID_WZ_Cliffside) return gLangMngr.getString("MapCliffside");
	else if (mapID == GBGameInfo::MAPID_WZ_California) return gLangMngr.getString("MapCalifornia");
	else if (mapID == GBGameInfo::MAPID_WZ_Caliwood) return gLangMngr.getString("MapCaliwood");
	else if (mapID == GBGameInfo::MAPID_WZ_AircraftCarrier) return gLangMngr.getString("MapAircraft");
	else if (mapID == GBGameInfo::MAPID_ZH_Warehouse) return gLangMngr.getString("MapWarehouse");
	else if (mapID == GBGameInfo::MAPID_WZ_BRmap) return gLangMngr.getString("mapBR");
	else if (mapID == GBGameInfo::MAPID_WZ_Clearview_V2) return gLangMngr.getString("MapClearview");
	else if (mapID == GBGameInfo::MAPID_WZ_Nevada) return gLangMngr.getString("MapNevada");
	else if (mapID == GBGameInfo::MAPID_WZ_RockyFord) return gLangMngr.getString("MapRockyFord");
	else if (mapID == GBGameInfo::MAPID_WZ_Quarantine) return gLangMngr.getString("MapQuarantine");
	else if (mapID == GBGameInfo::MAPID_WZ_Trade_Map) return "TRADEMAP";
	else if (mapID == GBGameInfo::MAPID_ServerTest) return "DEVMAP";

	return "";
}

int		superPings_[2048]; // big enough to hold all possiblesupervisors id

int GetGamePing(DWORD superId)
{
	// high word of gameId is supervisor Id
	r3d_assert(superId < R3D_ARRAYSIZE(superPings_));
	return superPings_[superId];
}

FrontEnd::FrontEnd()
{
	extern bool g_bDisableP2PSendToHost;
	g_bDisableP2PSendToHost = true;

	RTScaleformTexture = NULL;
	needReInitScaleformTexture = false;

	prevGameResult = GRESULT_Unknown;

	CurrentBrowseChannel = 0;

	leaderboardSize = 1000;
	leaderboard_requestStartPos = 0;
	leaderboard_BoardSelected = 0;

	asyncThread_ = NULL;
	asyncErr_[0] = 0;

	CancelQuickJoinRequest = false;
	exitRequested_ = false;
	needExitByGameJoin_ = false;
	needReturnFromQuickJoin = false;

	lastServerReqTime_ = -1;
	masterConnectTime_ = -1;

	memset(&superPings_, 0, sizeof(superPings_));

	m_Player = 0;
	m_needPlayerRenderingRequest = 0;
	m_CreateHeroID = 0;
	m_CreateBodyIdx = 0;
	m_CreateHeadIdx = 0;
	m_CreateLegsIdx = 0;

	m_joinGameServerId = 0;
	m_joinGamePwd[0] = 0;

	m_buyGpPriceCents = -1;

	m_rentServerPrice = 0;

	loginThread = NULL;
	loginAnswerCode = ANS_Unactive;

	m_browseGamesMode = 0;

	needUpdateSettings_ = false;
	needUpdateMode_ = false;

	extern int g_CCBlackWhite;
	extern float g_fCCBlackWhitePwr;

	g_CCBlackWhite = false;
	g_fCCBlackWhitePwr = 0.0f;

	frontendStage = 0;
	loginMsgBoxOK_Exit = false;
	ReviveCharSuccessfull = 0;

	clanListRequest_SortID = 1;
	clanListRequest_StartPos = 0;
	clanListRequest_ClanListSize = 1000;

	m_browseGamesCurrentCur = 0;

	m_browseGamesNameFilter[0] = 0;

	m_closeRewardMessageActionID = 0;

	if (g_mouse_sensitivity->GetFloat() < g_mouse_sensitivity->GetMinVal() || g_mouse_sensitivity->GetFloat() > g_mouse_sensitivity->GetMaxVal())
		g_mouse_sensitivity->SetFloat(0.5f);
	if (s_sound_volume->GetFloat() < s_sound_volume->GetMinVal() || s_sound_volume->GetFloat() > s_sound_volume->GetMaxVal())
		s_sound_volume->SetFloat(1.0f);
	if (s_music_volume->GetFloat() < s_music_volume->GetMinVal() || s_music_volume->GetFloat() > s_music_volume->GetMaxVal())
		s_music_volume->SetFloat(1.0f);

	r_film_tone_a->SetFloat(0.15f);
	r_film_tone_b->SetFloat(0.50f);
	r_film_tone_c->SetFloat(0.10f);
	r_film_tone_d->SetFloat(0.20f);
	r_film_tone_e->SetFloat(0.02f);
	r_film_tone_f->SetFloat(0.30f);
	r_exposure_bias->SetFloat(0.5f);
	r_white_level->SetFloat(11.2f);

	gClientLogic().Reset(); // reset game finished, otherwise player will not update and will not update its skelet and will not render
}

FrontEnd::~FrontEnd()
{
#if USE_BINK_MOVIE
	SAFE_DELETE(Intro);
#endif
	asyncThread_ = NULL;
	loginThread = NULL;

	if (m_Player)
	{
		GameWorld().DeleteObject(m_Player);

		extern void DestroyGame(); // destroy game only if player was loaded. to prevent double call to destroy game
		DestroyGame();
	}

	extern bool g_bDisableP2PSendToHost;
	g_bDisableP2PSendToHost = false;

	WorldLightSystem.Destroy();
}

bool FrontEnd::LoadComputerToken() {
	HKEY hKey;
	int hr;
	hr = RegOpenKeyEx(HKEY_CURRENT_USER,
		"Software\\ZombieHunters\\WarZ",
		0,
		KEY_ALL_ACCESS,
		&hKey);
	if (hr != ERROR_SUCCESS)
	{
		CreateComputerToken();
		return true;
	}

	DWORD size = sizeof(hardwareid);
	hr = RegQueryValueEx(hKey, "hardwareid", NULL, NULL, (BYTE*)hardwareid, &size);
	RegCloseKey(hKey);

	return true;
}

void FrontEnd::CreateComputerToken()
{
	HKEY hKey;
	int hr;
	hr = RegCreateKeyEx(HKEY_CURRENT_USER,
		"Software\\ZombieHunters\\WarZ",
		0,
		NULL,
		REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS,
		NULL,
		&hKey,
		NULL);
	if (hr == ERROR_SUCCESS)
	{
		CHWInfo g_HardwareInfo;
		g_HardwareInfo.Grab();
		sprintf(hardwareid, "0x%I64x", g_HardwareInfo.uniqueId);

		DWORD size = strlen(hardwareid) + 1;

		hr = RegSetValueEx(hKey, "hardwareid", NULL, REG_SZ, (BYTE*)hardwareid, size);
		RegCloseKey(hKey);
	}
}

unsigned int WINAPI FrontEnd::LoginProcessThread(void* in_data)
{
#pragma omp parallel num_threads(omp_get_num_threads())

	r3dThreadAutoInstallCrashHelper crashHelper;
	FrontEnd* This = (FrontEnd*)in_data;

	r3d_assert(This->loginAnswerCode == ANS_Unactive);
	This->loginAnswerCode = ANS_Processing;
	gUserProfile.CustomerID = 0;
	gUserProfile.username[0] = 0;
	gUserProfile.email[0] = 0;

	CWOBackendReq req("api_Login.aspx");
	req.AddParam("loginID", Login_PassedLoginID);
	req.AddParam("password", Login_PassedPwd);
	req.AddParam("computerid", Login_HardwareID);
	req.AddParam("hddid", Login_HDDID);

	if (!req.Issue())
	{
		r3dOutToLog("Login FAILED, code: %d\n", req.resultCode_);
		This->loginAnswerCode = req.resultCode_ == 8 ? ANS_Timeout : ANS_Error;
		return 0;
	}

	int n = sscanf_s(req.bodyStr_, "%d %s %s %d",
		&gUserProfile.CustomerID,
		&gUserProfile.email,
		(unsigned)_countof(gUserProfile.email),
		&gUserProfile.username,
		(unsigned)_countof(gUserProfile.username),
		&gUserProfile.SessionID);

	if ((const char*)gUserProfile.email == "" ? n != 2 : n != 4 || (const char*)gUserProfile.username == "" ? n != 2 : n != 4)
	{
		r3dOutToLog("Login: bad answer\n");
		This->loginAnswerCode = ANS_Error;
		return 0;
	}

	if (gUserProfile.CustomerID == 0)
		This->loginAnswerCode = ANS_BadPassword;
	else if (gUserProfile.AccountStatus >= 200)
		This->loginAnswerCode = ANS_Frozen;
	else
		This->loginAnswerCode = ANS_Logged;

	return 0;
}

void FrontendWarZ_LoginProcessThread(void *in_data)
{
	FrontEnd::LoginProcessThread(in_data);
}

unsigned int WINAPI FrontEnd::LoginAuthThread(void* in_data)
{
	r3dThreadAutoInstallCrashHelper crashHelper;
	FrontEnd* This = (FrontEnd*)in_data;

	r3d_assert(This->loginAnswerCode == ANS_Unactive);
	This->loginAnswerCode = ANS_Processing;
	r3d_assert(gUserProfile.CustomerID);
	r3d_assert(gUserProfile.username);
	r3d_assert(gUserProfile.email);

	r3d_assert(gUserProfile.SessionID);

	CWOBackendReq req(&gUserProfile, "api_LoginSessionPoller.aspx");
	if (req.Issue() == true)
	{
		This->loginAnswerCode = ANS_Logged;
		return true;
	}

	gUserProfile.CustomerID = 0;
	gUserProfile.username[0] = 0;
	gUserProfile.email[0] = 0;

	gUserProfile.SessionID = 0;
	gUserProfile.AccountStatus = 0;

	r3dOutToLog("LoginAuth: %d\n", req.resultCode_);
	This->loginAnswerCode = ANS_BadPassword;
	return 0;
}

static bool getAuthFromRegistry(const char* token, int tokensize)
{
	// query for game registry node
	HKEY hKey;
	int hr;
	hr = RegOpenKeyEx(HKEY_CURRENT_USER,
		"Software\\Arktos Entertainment Group\\WarZ",
		0,
		KEY_ALL_ACCESS,
		&hKey);
	if (hr != ERROR_SUCCESS)
		return false;

	DWORD size = tokensize;
	hr = RegQueryValueEx(hKey, "LoginToken", NULL, NULL, (BYTE*)token, &size);
	if (hr != ERROR_SUCCESS)
		return false;

	// and clear it
	hr = RegDeleteValue(hKey, "LoginToken");
	RegCloseKey(hKey);
	return true;
}

bool FrontEnd::DecodeAuthParams()
{
	r3d_assert(Login_PassedAuth[0]);

	if (Login_PassedAuth[0] == '=' && Login_PassedAuth[1] == '=')
	{
		if (!getAuthFromRegistry(Login_PassedAuth, sizeof(Login_PassedAuth)))
		{
			r3dError("unable to get auth data");
			return false;
		}
	}

	CkString s1;
	s1 = Login_PassedAuth;
	s1.base64Decode("utf-8");

	char* authToken = (char*)s1.getAnsi();
	for (size_t i = 0; i < strlen(authToken); i++)
		authToken[i] = authToken[i] ^ 0x64;

	DWORD CustomerID = 0;
	DWORD SessionID = 0;
	DWORD AccountStatus = 0;
	int n = sscanf(authToken, "%d:%d:%d", &CustomerID, &SessionID, &AccountStatus);
	if (n != 3)
	{
		r3dError("unable to get session data");
		return false;
	}

	gUserProfile.CustomerID = CustomerID;
	gUserProfile.SessionID = SessionID;
	gUserProfile.AccountStatus = AccountStatus;
	return true;
}

void FrontEnd::LoginCheckAnswerCode()
{
	if (loginAnswerCode == ANS_Unactive)
		return;

	if (loginAnswerCode == ANS_Processing)
		return;

	// wait for thread to finish
	if (::WaitForSingleObject(loginThread, 1000) == WAIT_TIMEOUT)
		r3d_assert(0);

	CloseHandle(loginThread);
	loginThread = NULL;

	Scaleform::GFx::Value vars[3];
	switch (loginAnswerCode)
	{
	case ANS_Timeout:
		loginMsgBoxOK_Exit = true;
		vars[0].SetString(gLangMngr.getString("LoginMenu_CommError"));
		vars[1].SetBoolean(true);
		vars[2].SetString("$ERROR");
		gfxFrontEnd.Invoke("_root.api.showInfoMsg", vars, 3);
		break;
	case ANS_Error:
		loginMsgBoxOK_Exit = true;
		vars[0].SetString(gLangMngr.getString("LoginMenu_WrongLoginAnswer"));
		vars[1].SetBoolean(true);
		vars[2].SetString("$ERROR");
		gfxFrontEnd.Invoke("_root.api.showInfoMsg", vars, 3);
		break;
	case ANS_Logged:
		LoginMenuExitFlag = 1;
		break;

	case ANS_BadPassword:
		loginMsgBoxOK_Exit = true;
		vars[0].SetString(gLangMngr.getString("LoginMenu_LoginFailed"));
		vars[1].SetBoolean(true);
		vars[2].SetString("$ERROR");
		gfxFrontEnd.Invoke("_root.api.showInfoMsg", vars, 3);
		break;

	case ANS_Frozen:
		loginMsgBoxOK_Exit = true;
		vars[0].SetString(gLangMngr.getString("LoginMenu_AccountFrozen"));
		vars[1].SetBoolean(true);
		vars[2].SetString("$ERROR");
		gfxFrontEnd.Invoke("_root.api.showInfoMsg", vars, 2);
		break;
	}
}

static volatile LONG gProfileIsAquired = 0;
static volatile LONG gProfileOK = 0;
static volatile float gTimeWhenProfileLoaded = 0;
static volatile LONG gProfileLoadStage = 0;

extern CHWInfo g_HardwareInfo;

static void SetLoadStage(const char* stage)
{
	const static char* sname = NULL;
	static float stime = 0;
#ifndef FINAL_BUILD	
	if (sname)
	{
		r3dOutToLog("SetLoadStage: %4.2f sec in %s\n", r3dGetTime() - stime, sname);
	}
#endif

	sname = stage;
	stime = r3dGetTime();
	gProfileLoadStage++;
}

static int gl_Progress = 0;
static bool can_show_reward_message = false;
static void LoadFrontendGameData(FrontEnd* UI)
{
	//
	// load shooting gallery
	//
	SetLoadStage("FrontEnd Lighting Level");
	{
		extern void DoLoadGame(const char* LevelFolder, int MaxPlayers, bool unloadPrev, bool isMenuLevel);
		DoLoadGame(r3dGameLevel::GetHomeDir(), 4, true, true);
	}

	gl_Progress = 84;

	//
	// create player and FPS weapon
	//
	SetLoadStage("Player Model");
	{
		obj_Player* plr = (obj_Player *)srv_CreateGameObject("obj_Player", "Player", r3dPoint3D(0, 0, 0));
		plr->PlayerState = PLAYER_IDLE;
		plr->bDead = 0;
		plr->CurLoadout = gUserProfile.ProfileData.ArmorySlots[0];
		plr->m_disablePhysSkeleton = true;
		plr->m_fPlayerRotationTarget = plr->m_fPlayerRotation = 0;

		// we need it to be created as a networklocal character for physics.
		plr->NetworkLocal = true;
		plr->OnCreate();
		plr->NetworkLocal = false;
		// work around for loading fps model sometimes instead of proper tps model
		plr->UpdateLoadoutSlot(plr->CurLoadout, -1);
		// switch player to UI idle mode
		plr->uberAnim_->IsInUI = true;
		plr->uberAnim_->AnimPlayerState = -1;
		plr->uberAnim_->anim.StopAll();	// prevent animation blending on loadout switch
		plr->SyncAnimation(true);
		UI->SetLoadedThings(plr);
		PFX_3DLUTColorCorrection::s_damageHitColorCorrectionVal = 0.0f;
		can_show_reward_message = gUserProfile.ProfileData.RewardItemID > 0;
	}

	gl_Progress = 100;
}

static bool ActualGetProfileData(FrontEnd* UI)
{
	gl_Progress = 0;

	gProfileLoadStage = 0;

	SetLoadStage("ApiGetShopData");
	if (gUserProfile.ApiGetShopData() != 0)
		return false;

	gl_Progress = 14;

	// get game rewards from server.
	SetLoadStage("ApiGameRewards");
	if (g_GameRewards == NULL)
		g_GameRewards = game_new CGameRewards();
	if (!g_GameRewards->loaded_) {
		if (g_GameRewards->ApiGetDataGameRewards() != 0) {
			return false;
		}
	}

	gl_Progress = 28;

	// update items info only once and do not check for errors
	static bool gotCurItemsData = false;
	SetLoadStage("ApiGetItemsInfo");
	if (!gotCurItemsData) {
		gotCurItemsData = true;
		gUserProfile.ApiGetItemsInfo();
	}

	gl_Progress = 42;

	static bool gotGP2GDTable = false;
	SetLoadStage("ApiGetConvertGP2GD");
	if (!gotGP2GDTable)
	{
		gotGP2GDTable = true;
		gUserProfile.ApiGetGamePointsConvertsionRates();
	}

	gl_Progress = 56;

	// Blackmarket System Begin
	SetLoadStage("ApiFetchBlackmarketSetting");
	gUserProfile.ApiFetchBlackmarketSetting();
	// Blackmarket System End

	gl_Progress = 67;

	// get daily rewards only once, as we are showing them only when you login into game
	gUserProfile.ApiRetBonusGetInfo();

	SetLoadStage("GetProfile");
	if (gUserProfile.GetProfile() != 0)
		return false;

	gl_Progress = 70;

	// load player only after profile
	// need to load game data first, because of DestroyGame() in destructor
	LoadFrontendGameData(UI);

	if (gUserProfile.ProfileDataDirty > 0)
	{
		//@TODO: set dirty profile flag, repeat getting profile
		r3dOutToLog("@@@@@@@@@@ProfileDataDirty: %d\n", gUserProfile.ProfileDataDirty);
	}

	CWOBackendReq req(&gUserProfile, "api_ReportHWInfo.aspx");
	char buf[1024];

	sprintf(buf, "%I64d", g_HardwareInfo.uniqueId);
	req.AddParam("r00", buf);

	req.AddParam("r10", g_HardwareInfo.CPUString);
	req.AddParam("r11", g_HardwareInfo.CPUBrandString);

	sprintf(buf, "%d", g_HardwareInfo.CPUFreq);
	req.AddParam("r12", buf);

	sprintf(buf, "%d", g_HardwareInfo.TotalMemory);
	req.AddParam("r13", buf);

	sprintf(buf, "%d", g_HardwareInfo.DisplayW);
	req.AddParam("r20", buf);

	sprintf(buf, "%d", g_HardwareInfo.DisplayH);
	req.AddParam("r21", buf);

	sprintf(buf, "%d", g_HardwareInfo.gfxErrors);
	req.AddParam("r22", buf);

	sprintf(buf, "%d", g_HardwareInfo.gfxVendorId);
	req.AddParam("r23", buf);

	sprintf(buf, "%d", g_HardwareInfo.gfxDeviceId);
	req.AddParam("r24", buf);

	req.AddParam("r25", g_HardwareInfo.gfxDescription);

	sprintf(buf, "%d", g_HardwareInfo.gfxD3DVersion);
	req.AddParam("r26", buf);

	req.AddParam("r30", g_HardwareInfo.OSVersion);

	// aLca :: GetHDDInfo
	sprintf(buf, "%s", g_HardwareInfo.nDriveNumber);
	req.AddParam("r31", g_HardwareInfo.serialNum);

#ifndef FINAL_BUILD
	r3dOutToLog("just to be sure, HDDID: %s\n", g_HardwareInfo.serialNum);
#endif

	if (!req.Issue())
	{
		r3dOutToLog("ERROR: 0x0735613\n"); // r3dError("Failed to upload HW Info\n");
	}

	SetLoadStage(NULL);
	return true;
}

static unsigned int WINAPI GetProfileDataThread(void* UI)
{
#pragma omp parallel num_threads(omp_get_num_threads())
	r3dThreadAutoInstallCrashHelper crashHelper;
	r3dRandInitInTread rand_in_thread;

	FrontEnd* This = (FrontEnd*)UI;

	try
	{
		gProfileOK = 0;
		if (ActualGetProfileData(This))
		{
			gProfileOK = 1;
			gTimeWhenProfileLoaded = r3dGetTime();
		}
	}
	catch (const char* err)
	{
		// catch r3dError
		r3dOutToLog("GetProfileData error: %s\n", err);
	}

	InterlockedExchange(&gProfileIsAquired, 1);

	return 0;
}

static float aquireProfileStart = 0;
static HANDLE handleGetProfileData = 0;

void FrontEnd::Load()
{
	gfxFrontEnd.Load("Data\\Menu\\FrontEnd.swf", true);

#define MAKE_CALLBACK(FUNC) game_new r3dScaleformMovie::TGFxEICallback<FrontEnd>(this, &FrontEnd::FUNC)
	gfxFrontEnd.RegisterEventHandler("eventPlayGame", MAKE_CALLBACK(eventPlayGame));
	gfxFrontEnd.RegisterEventHandler("eventCancelQuickGameSearch", MAKE_CALLBACK(eventCancelQuickGameSearch));
	gfxFrontEnd.RegisterEventHandler("eventQuitGame", MAKE_CALLBACK(eventQuitGame));
	gfxFrontEnd.RegisterEventHandler("eventCreateCharacter", MAKE_CALLBACK(eventCreateCharacter));
	gfxFrontEnd.RegisterEventHandler("eventRenameCharacter", MAKE_CALLBACK(eventRenameCharacter));
	gfxFrontEnd.RegisterEventHandler("eventDeleteChar", MAKE_CALLBACK(eventDeleteChar));
	gfxFrontEnd.RegisterEventHandler("eventShowSurvivorsMap", MAKE_CALLBACK(eventShowSurvivorsMap));
	gfxFrontEnd.RegisterEventHandler("eventReviveChar", MAKE_CALLBACK(eventReviveChar));
	gfxFrontEnd.RegisterEventHandler("eventReviveCharMoney", MAKE_CALLBACK(eventReviveCharMoney));
	gfxFrontEnd.RegisterEventHandler("eventBuyItem", MAKE_CALLBACK(eventBuyItem));
	gfxFrontEnd.RegisterEventHandler("eventBackpackFromInventory", MAKE_CALLBACK(eventBackpackFromInventory));
	gfxFrontEnd.RegisterEventHandler("eventBackpackToInventory", MAKE_CALLBACK(eventBackpackToInventory));
	gfxFrontEnd.RegisterEventHandler("eventBackpackGridSwap", MAKE_CALLBACK(eventBackpackGridSwap));
	gfxFrontEnd.RegisterEventHandler("eventSetSelectedChar", MAKE_CALLBACK(eventSetSelectedChar));
	gfxFrontEnd.RegisterEventHandler("eventOpenBackpackSelector", MAKE_CALLBACK(eventOpenBackpackSelector));
	gfxFrontEnd.RegisterEventHandler("eventChangeBackpack", MAKE_CALLBACK(eventChangeBackpack));
	gfxFrontEnd.RegisterEventHandler("eventLearnSkill", MAKE_CALLBACK(eventLearnSkill));
	gfxFrontEnd.RegisterEventHandler("eventChangeOutfit", MAKE_CALLBACK(eventChangeOutfit));
	gfxFrontEnd.RegisterEventHandler("eventSetCurrentBrowseChannel", MAKE_CALLBACK(eventSetCurrentBrowseChannel));
	gfxFrontEnd.RegisterEventHandler("eventTrialRequestUpgrade", MAKE_CALLBACK(eventTrialRequestUpgrade));
	gfxFrontEnd.RegisterEventHandler("eventTrialUpgradeAccount", MAKE_CALLBACK(eventTrialUpgradeAccount));
	gfxFrontEnd.RegisterEventHandler("eventBuyPremiumAccount", MAKE_CALLBACK(eventBuyPremiumAccount));
	gfxFrontEnd.RegisterEventHandler("eventBuyAccount", MAKE_CALLBACK(eventBuyAccount));
	gfxFrontEnd.RegisterEventHandler("eventMarketplaceActive", MAKE_CALLBACK(eventMarketplaceActive));
	gfxFrontEnd.RegisterEventHandler("eventMoveAllItems", MAKE_CALLBACK(eventMoveAllItems));
	gfxFrontEnd.RegisterEventHandler("eventOpenURL", MAKE_CALLBACK(eventOpenURL));
	gfxFrontEnd.RegisterEventHandler("eventShowRewardPopPUp", MAKE_CALLBACK(eventShowRewardPopPUp));//AlexRedd:: daily rewards

	gfxFrontEnd.RegisterEventHandler("eventOptionsReset", MAKE_CALLBACK(eventOptionsReset));
	gfxFrontEnd.RegisterEventHandler("eventOptionsApply", MAKE_CALLBACK(eventOptionsApply));
	gfxFrontEnd.RegisterEventHandler("eventExtraOptionsApply", MAKE_CALLBACK(eventExtraOptionsApply));
	gfxFrontEnd.RegisterEventHandler("eventOptionsControlsReset", MAKE_CALLBACK(eventOptionsControlsReset));
	gfxFrontEnd.RegisterEventHandler("eventOptionsControlsApply", MAKE_CALLBACK(eventOptionsControlsApply));
	gfxFrontEnd.RegisterEventHandler("eventOptionsVoipApply", MAKE_CALLBACK(eventOptionsVoipApply));
	gfxFrontEnd.RegisterEventHandler("eventOptionsVoipReset", MAKE_CALLBACK(eventOptionsVoipReset));
	gfxFrontEnd.RegisterEventHandler("eventOptionsLanguageSelection", MAKE_CALLBACK(eventOptionsLanguageSelection));
	gfxFrontEnd.RegisterEventHandler("eventOptionsControlsRequestKeyRemap", MAKE_CALLBACK(eventOptionsControlsRequestKeyRemap));

	gfxFrontEnd.RegisterEventHandler("eventCreateChangeCharacter", MAKE_CALLBACK(eventCreateChangeCharacter));
	gfxFrontEnd.RegisterEventHandler("eventCreateCancel", MAKE_CALLBACK(eventCreateCancel));

	gfxFrontEnd.RegisterEventHandler("eventRequestPlayerRender", MAKE_CALLBACK(eventRequestPlayerRender));
	gfxFrontEnd.RegisterEventHandler("eventMsgBoxCallback", MAKE_CALLBACK(eventMsgBoxCallback));

	gfxFrontEnd.RegisterEventHandler("eventBrowseGamesRequestFilterStatus", MAKE_CALLBACK(eventBrowseGamesRequestFilterStatus));
	gfxFrontEnd.RegisterEventHandler("eventBrowseGamesSetFilter", MAKE_CALLBACK(eventBrowseGamesSetFilter));
	gfxFrontEnd.RegisterEventHandler("eventBrowseGamesJoin", MAKE_CALLBACK(eventBrowseGamesJoin));
	gfxFrontEnd.RegisterEventHandler("eventBrowseGamesOnAddToFavorites", MAKE_CALLBACK(eventBrowseGamesOnAddToFavorites));
	gfxFrontEnd.RegisterEventHandler("eventBrowseGamesRequestList", MAKE_CALLBACK(eventBrowseGamesRequestList));

	gfxFrontEnd.RegisterEventHandler("eventRequestMyClanInfo", MAKE_CALLBACK(eventRequestMyClanInfo));
	gfxFrontEnd.RegisterEventHandler("eventRequestClanList", MAKE_CALLBACK(eventRequestClanList));
	gfxFrontEnd.RegisterEventHandler("eventCreateClan", MAKE_CALLBACK(eventCreateClan));
	gfxFrontEnd.RegisterEventHandler("eventClanAdminDonateGC", MAKE_CALLBACK(eventClanAdminDonateGC));
	gfxFrontEnd.RegisterEventHandler("eventClanAdminAction", MAKE_CALLBACK(eventClanAdminAction));
	gfxFrontEnd.RegisterEventHandler("eventClanLeaveClan", MAKE_CALLBACK(eventClanLeaveClan));
	gfxFrontEnd.RegisterEventHandler("eventClanDonateGCToClan", MAKE_CALLBACK(eventClanDonateGCToClan));
	gfxFrontEnd.RegisterEventHandler("eventRequestClanApplications", MAKE_CALLBACK(eventRequestClanApplications));
	gfxFrontEnd.RegisterEventHandler("eventClanApplicationAction", MAKE_CALLBACK(eventClanApplicationAction));
	gfxFrontEnd.RegisterEventHandler("eventClanInviteToClan", MAKE_CALLBACK(eventClanInviteToClan));
	gfxFrontEnd.RegisterEventHandler("eventClanRespondToInvite", MAKE_CALLBACK(eventClanRespondToInvite));
	gfxFrontEnd.RegisterEventHandler("eventClanBuySlots", MAKE_CALLBACK(eventClanBuySlots));
	gfxFrontEnd.RegisterEventHandler("eventClanApplyToJoin", MAKE_CALLBACK(eventClanApplyToJoin));

	gfxFrontEnd.RegisterEventHandler("eventStorePurchaseGPCallback", MAKE_CALLBACK(eventStorePurchaseGPCallback));
	gfxFrontEnd.RegisterEventHandler("eventStorePurchaseGP", MAKE_CALLBACK(eventStorePurchaseGP));
	gfxFrontEnd.RegisterEventHandler("eventStorePurchaseGPRequest", MAKE_CALLBACK(eventStorePurchaseGPRequest));

	gfxFrontEnd.RegisterEventHandler("eventStorePurchaseGDCallback", MAKE_CALLBACK(eventStorePurchaseGDCallback));
	gfxFrontEnd.RegisterEventHandler("eventStorePurchaseGD", MAKE_CALLBACK(eventStorePurchaseGD));

	gfxFrontEnd.RegisterEventHandler("eventDonateGCtoServerCallback", MAKE_CALLBACK(eventDonateGCtoServerCallback));
	gfxFrontEnd.RegisterEventHandler("eventDonateGCtoServer", MAKE_CALLBACK(eventDonateGCtoServer));
	gfxFrontEnd.RegisterEventHandler("eventRequestShowDonateGCtoServer", MAKE_CALLBACK(eventRequestShowDonateGCtoServer));

	gfxFrontEnd.RegisterEventHandler("eventRequestLeaderboardData", MAKE_CALLBACK(eventRequestLeaderboardData));

	gfxFrontEnd.RegisterEventHandler("eventRequestGCTransactionData", MAKE_CALLBACK(eventRequestGCTransactionData));
	gfxFrontEnd.RegisterEventHandler("eventRequestLotteryData", MAKE_CALLBACK(eventRequestLotteryData));
	gfxFrontEnd.RegisterEventHandler("eventRequestUpgradeAccBonusLootData", MAKE_CALLBACK(eventRequestUpgradeAccBonusLootData));

	gfxFrontEnd.RegisterEventHandler("eventRequestMyServerList", MAKE_CALLBACK(eventRequestMyServerList));
	gfxFrontEnd.RegisterEventHandler("eventRequestMyServerInfo", MAKE_CALLBACK(eventRequestMyServerInfo));
	gfxFrontEnd.RegisterEventHandler("eventMyServerKickPlayer", MAKE_CALLBACK(eventMyServerKickPlayer));
	gfxFrontEnd.RegisterEventHandler("eventRentServerUpdatePrice", MAKE_CALLBACK(eventRentServerUpdatePrice));
	gfxFrontEnd.RegisterEventHandler("eventRentServer", MAKE_CALLBACK(eventRentServer));
	gfxFrontEnd.RegisterEventHandler("eventMyServerJoinServer", MAKE_CALLBACK(eventMyServerJoinServer));
	gfxFrontEnd.RegisterEventHandler("eventMyServerUpdateSettings", MAKE_CALLBACK(eventMyServerUpdateSettings));
	gfxFrontEnd.RegisterEventHandler("eventRenewServerUpdatePrice", MAKE_CALLBACK(eventRenewServerUpdatePrice));
	gfxFrontEnd.RegisterEventHandler("eventRenewServer", MAKE_CALLBACK(eventRenewServer));
	gfxFrontEnd.RegisterEventHandler("eventFriendlyFireSet", MAKE_CALLBACK(eventFriendlyFireSet));
	gfxFrontEnd.RegisterEventHandler("eventTeleportPlayers", MAKE_CALLBACK(eventTeleportPlayers));
	gfxFrontEnd.RegisterEventHandler("eventLoad_Loadout", MAKE_CALLBACK(eventLoad_Loadout));
	gfxFrontEnd.RegisterEventHandler("eventSave_Loadout", MAKE_CALLBACK(eventSave_Loadout));
	gfxFrontEnd.RegisterEventHandler("eventDelete_Loadout", MAKE_CALLBACK(eventDelete_Loadout));
	// Blackmarket System Begin
	gfxFrontEnd.RegisterEventHandler("eventMarketplaceShowBlackmarket", MAKE_CALLBACK(eventMarketplaceShowBlackmarket));
	gfxFrontEnd.RegisterEventHandler("eventBuyItemFromBlackmarket", MAKE_CALLBACK(eventBuyItemFromBlackmarket));
	gfxFrontEnd.RegisterEventHandler("eventRemoveItemFromBlackmarket", MAKE_CALLBACK(eventRemoveItemFromBlackmarket));
	gfxFrontEnd.RegisterEventHandler("eventSellItemOnBlackmarket", MAKE_CALLBACK(eventSellItemOnBlackmarket));
	gfxFrontEnd.RegisterEventHandler("eventBlackmarketRefresh", MAKE_CALLBACK(eventBlackmarketRefresh));
	gfxFrontEnd.RegisterEventHandler("eventDoLoadBlackmarketWithSelectedCategory", MAKE_CALLBACK(eventDoLoadBlackmarketWithSelectedCategory));
	// Blackmarket System End

	// reacquire the menu.
	gfxFrontEnd.SetKeyboardCapture();

	bindRTsToScaleForm();
}

void FrontEnd::postLoginStepInit(EGameResult gameResult)
{
	frontendStage = 1;
	prevGameResult = gameResult;

#if USE_BINK_MOVIE
	Call_Login_Screen = true;
#else
	PlayMusic();

	// show info message and render it one time
	gfxFrontEnd.Invoke("_root.api.showLoginMsg", "");
#endif

	gProfileIsAquired = 0;
	aquireProfileStart = r3dGetTime();
	handleGetProfileData = (HANDLE)_beginthreadex(NULL, 0, &GetProfileDataThread, this, 0, 0);
	if (handleGetProfileData == 0)
		r3dError("Failed to begin thread");

	// init things to load game level
	r3dGameLevel::SetHomeDir("WZ_FrontEndLighting");
	extern void InitGame_Start();
	InitGame_Start();
}

void FrontEnd::initLoginStep(const char * loginErrorMsg)
{
#if USE_BINK_MOVIE
	Intro = new CutScene;
	bool success = Intro->Init("Intro\\ZH-Intro.bik");
	if (success)
		IsPlayingIntro = true;
	else
		Call_Login_Screen = true;
#else
	PlayMusic();

	// show info message and render it one time
	gfxFrontEnd.Invoke("_root.api.showLoginMsg", "");
#endif

	LoginMenuExitFlag = 0;
	loginProcessStartTime = r3dGetTime();

	if (loginErrorMsg)
	{
		loginMsgBoxOK_Exit = true;
		Scaleform::GFx::Value vars[3];
		vars[0].SetString(loginErrorMsg);
		vars[1].SetBoolean(true);
		vars[2].SetString("$ERROR");
		gfxFrontEnd.Invoke("_root.api.showInfoMsg", vars, 3);
		return;
	}

	// if we have encoded login session information
	if (Login_PassedAuth[0])
	{
		if (DecodeAuthParams())
		{
			r3d_assert(loginThread == NULL);
			loginThread = (HANDLE)_beginthreadex(NULL, 0, &LoginAuthThread, this, 0, NULL);
			if (loginThread == NULL)
				r3dError("Failed to begin thread");
		}
		return;
	}

#pragma region LOCAL.INI
#ifndef FINAL_BUILD
	if (Login_PassedLoginID[0] == 0 || Login_PassedPwd[0] == 0 || Login_HardwareID[0] == 0 || Login_HDDID[0] == 0)
	{
		r3dscpy(Login_PassedLoginID, d_login->GetString());
		r3dscpy(Login_PassedPwd, d_password->GetString());
		r3dscpy(Login_HardwareID, d_hardware->GetString()); // aLca :: Not used anylonger i guess
#if _DEBUG
		r3dscpy(Login_HDDID, d_hddid->GetString());
#endif
		LoadComputerToken();
#if _DEBUG
		// if (strlen(Login_PassedUser) < 2 || strlen(Login_PassedPwd) < 2 || strlen(Login_HardwareID) < 2 || strlen(Login_HDDID) < 2)
		if (strlen(Login_PassedLoginID) < 2 || strlen(Login_PassedPwd) < 2 /*|| strlen(Login_HardwareID) < 2*/)
#else
		if (strlen(Login_PassedLoginID) < 2 || strlen(Login_PassedPwd) < 2 || strlen(Login_HardwareID) < 2)
#endif
		{
			r3dError("you should set login as d_login <user> d_password <pwd>  d_hardware <hwid> d_hddid <hddid> in local.ini");
		}
	}
#endif
#pragma endregion

	loginThread = (HANDLE)_beginthreadex(NULL, 0, &LoginProcessThread, this, 0, NULL);
}

extern void InputUpdate();
int FrontEnd::Draw()
{
	extern void tempDoMsgLoop();
	tempDoMsgLoop();

	if (gSteam.inited_)
		SteamAPI_RunCallbacks();

	{
		r3dPoint3D soundPos(0, 0, 0), soundDir(0, 0, 1), soundUp(0, 1, 0);
		SoundSys.Update(soundPos, soundDir, soundUp);
	}

#if USE_BINK_MOVIE
	if (Call_Login_Screen && !IsPlayingIntro)
	{
		PlayMusic();

		// show info message and render it one time
		gfxFrontEnd.Invoke("_root.api.showLoginMsg", "");
		Call_Login_Screen = false;
	}
#endif

	// update UI
	if (frontendStage == 0) // login stage
	{
		if (r3dRenderer->DeviceAvailable)
		{
#if USE_BINK_MOVIE
			if (!IsPlayingIntro)
			{
#endif
				gfxFrontEnd.SetCurentRTViewport(Scaleform::GFx::Movie::SM_NoScale);
				gfxFrontEnd.SetCurentRTViewport(Scaleform::GFx::Movie::SM_ExactFit);

				InputUpdate();

				gfxFrontEnd.SetKeyboardCapture();

				r3dMouse::Show();
				r3dStartFrame();

				r3dRenderer->StartRender(1);
				r3dRenderer->StartFrame();

				r3dRenderer->SetRenderingMode(R3D_BLEND_ALPHA | R3D_BLEND_NZ);

				r3dRenderer->SetViewport(0.f, 0.f, (float)r3dRenderer->d3dpp.BackBufferWidth, (float)r3dRenderer->d3dpp.BackBufferHeight);
				D3D_V(r3dRenderer->pd3ddev->Clear(0, NULL, D3DCLEAR_TARGET, 0, 1.f, 0));

				gfxFrontEnd.Invoke("_root.api.updateLoginMsg", gl_Progress);
				gfxFrontEnd.UpdateAndDraw();

				Console::RenderConsole();

				r3dRenderer->Flush();
				r3dRenderer->EndFrame();
				r3dRenderer->EndRender(true);

				r3dEndFrame();
#if USE_BINK_MOVIE
			}
			else
			{
				Call_Login_Screen = Intro->Play();
				if (Call_Login_Screen)
					IsPlayingIntro = false;
			}
#endif
		}

		LoginCheckAnswerCode();
		if (loginThread == NULL)
		{
			bool IsNeedExit();
			if (IsNeedExit())
				return FrontEndShared::RET_Exit;

			if (LoginMenuExitFlag == 1)
				return FrontEndShared::RET_LoggedIn;
			else if (LoginMenuExitFlag == -1) // error logging in
				return FrontEndShared::RET_Exit;
		}
	}
	else if (handleGetProfileData != 0 && gProfileIsAquired == 0)
	{
		if (r3dRenderer->DeviceAvailable)
		{
#if USE_BINK_MOVIE
			if (!IsPlayingIntro)
			{
#endif
				gfxFrontEnd.SetCurentRTViewport(Scaleform::GFx::Movie::SM_NoScale);
				gfxFrontEnd.SetCurentRTViewport(Scaleform::GFx::Movie::SM_ExactFit);

				InputUpdate();

				gfxFrontEnd.SetKeyboardCapture();

				r3dMouse::Show();
				r3dStartFrame();

				r3dRenderer->StartRender(1);
				r3dRenderer->StartFrame();

				r3dRenderer->SetRenderingMode(R3D_BLEND_ALPHA | R3D_BLEND_NZ);

				r3dRenderer->SetViewport(0.f, 0.f, (float)r3dRenderer->d3dpp.BackBufferWidth, (float)r3dRenderer->d3dpp.BackBufferHeight);
				D3D_V(r3dRenderer->pd3ddev->Clear(0, NULL, D3DCLEAR_TARGET, 0, 1.f, 0));

				gfxFrontEnd.Invoke("_root.api.updateLoginMsg", gl_Progress);
				gfxFrontEnd.UpdateAndDraw();

				Console::RenderConsole();

				r3dRenderer->Flush();
				r3dRenderer->EndFrame();
				r3dRenderer->EndRender(true);

				r3dEndFrame();
#if USE_BINK_MOVIE
			}
			else
			{
				Call_Login_Screen = Intro->Play();
				if (Call_Login_Screen)
					IsPlayingIntro = false;
			}
#endif
		}
	}
	else if (handleGetProfileData != 0) // still receiving informations from API
	{
		if (r3dRenderer->DeviceAvailable)
		{
#if USE_BINK_MOVIE
			if (!IsPlayingIntro)
			{
#endif
				gfxFrontEnd.SetCurentRTViewport(Scaleform::GFx::Movie::SM_NoScale);
				gfxFrontEnd.SetCurentRTViewport(Scaleform::GFx::Movie::SM_ExactFit);

				InputUpdate();

				gfxFrontEnd.SetKeyboardCapture();

				r3dMouse::Show();
				r3dStartFrame();

				r3dRenderer->StartRender(1);
				r3dRenderer->StartFrame();

				r3dRenderer->SetRenderingMode(R3D_BLEND_ALPHA | R3D_BLEND_NZ);

				r3dRenderer->SetViewport(0.f, 0.f, (float)r3dRenderer->d3dpp.BackBufferWidth, (float)r3dRenderer->d3dpp.BackBufferHeight);
				D3D_V(r3dRenderer->pd3ddev->Clear(0, NULL, D3DCLEAR_TARGET, 0, 1.f, 0));

				gfxFrontEnd.Invoke("_root.api.updateLoginMsg", gl_Progress);
				gfxFrontEnd.UpdateAndDraw();

				Console::RenderConsole();

				r3dRenderer->Flush();
				r3dRenderer->EndFrame();
				r3dRenderer->EndRender(true);

				r3dEndFrame();
#if USE_BINK_MOVIE
			}
			else
			{
				Call_Login_Screen = Intro->Play();
				if (Call_Login_Screen)
					IsPlayingIntro = false;
			}
#endif
		}

		// profile is acquired
		r3d_assert(gProfileIsAquired);

		if (!gProfileOK)
		{
			r3dOutToLog("Couldn't get profile data! stage: %d\n", gProfileLoadStage);
			return FrontEndShared::RET_Diconnected;
		}

		CloseHandle(handleGetProfileData);
		handleGetProfileData = 0;

		r3dOutToLog("Acquired base profile data for %f\n", r3dGetTime() - aquireProfileStart);
		if (gUserProfile.AccountStatus >= 200)
		{
			return FrontEndShared::RET_Banned;
		}

		extern void InitGame_Finish();
		InitGame_Finish();

		if (gUserProfile.ProfileDataDirty == 0)
			initFrontend();
	}
	else
	{
		// update UI & Rendering
		if (r3dRenderer->DeviceAvailable)
		{
#if USE_BINK_MOVIE
			if (!IsPlayingIntro)
			{
#endif
				if (RTScaleformTexture && Scaleform_RenderToTextureRT)
					RTScaleformTexture->Initialize(Scaleform_RenderToTextureRT->AsTex2D());

				GameWorld().StartFrame();
				r3dRenderer->SetCamera(gCam, true);

				GameWorld().Update();

				gfxFrontEnd.SetCurentRTViewport(Scaleform::GFx::Movie::SM_NoScale);
				gfxFrontEnd.SetCurentRTViewport(Scaleform::GFx::Movie::SM_ExactFit);

				InputUpdate();

				gfxFrontEnd.SetKeyboardCapture();

				r3dMouse::Show();
				r3dStartFrame();

				r3dRenderer->StartRender(1);
				r3dRenderer->StartFrame();

				r3dRenderer->SetRenderingMode(R3D_BLEND_ALPHA | R3D_BLEND_NZ);

				r3dRenderer->SetViewport(0.f, 0.f, (float)r3dRenderer->d3dpp.BackBufferWidth, (float)r3dRenderer->d3dpp.BackBufferHeight);
				D3D_V(r3dRenderer->pd3ddev->Clear(0, NULL, D3DCLEAR_TARGET, 0, 1.f, 0));

				if (m_needPlayerRenderingRequest)
					drawPlayer();

				gfxFrontEnd.UpdateAndDraw();

				Console::RenderConsole();

				r3dRenderer->Flush();
				r3dRenderer->EndFrame();
				r3dRenderer->EndRender(true);

				r3dEndFrame();
				GameWorld().EndFrame();
#if USE_BINK_MOVIE
			}
			else
			{
				Call_Login_Screen = Intro->Play();
				if (Call_Login_Screen)
					IsPlayingIntro = false;
			}
#endif
		}

		// check if profile is dirty
		if (gUserProfile.ProfileDataDirty != 0)
		{
			Scaleform::GFx::Value var[2];
			var[0].SetString("Waiting for profile to finish updating...");
			var[1].SetBoolean(false);
			gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);

			// retry
			gUserProfile.GetProfile();

			// success
			if (gUserProfile.ProfileDataDirty == 0)
			{
				gfxFrontEnd.Invoke("_root.api.hideInfoMsg", "");
				initFrontend();
			}
		}

		// at the moment we must have finished initializing things in background
		{
			//AlexRedd:: item reward message (used for battle royale)	
			if (gUserProfile.ProfileData.RewardItemID > 0 && gUserProfile.ProfileData.showRewardPopUp == 0 && can_show_reward_message && m_closeRewardMessageActionID == 0)
			{
				can_show_reward_message = false;
				m_closeRewardMessageActionID = 2;
				gfxFrontEnd.Invoke("_root.api.hideInfoMsg", "");
				const BaseItemConfig* ic = g_pWeaponArmory->getConfig(gUserProfile.ProfileData.RewardItemID);

				if (ic && ic->m_itemID != 0)
				{
					char tempMsg[128] = { 0 };
					sprintf(tempMsg, "%s %s", gLangMngr.getString("$FR_RewardMsg"), ic->m_StoreName);

					Scaleform::GFx::Value var[4];
					var[0].SetString(ic->m_StoreIcon);
					var[1].SetString(tempMsg);
					var[2].SetBoolean(true);
					var[3].SetString(gLangMngr.getString("$FR_RewardMsgTittle"));
					gfxFrontEnd.Invoke("_root.api.showInfoMsgCase", var, 4);

					SoundSys.PlayAndForget(SoundSys.GetEventIDByPath("Sounds/UI Events/Misc/UI_Menu_Mystery_Box_Unlocking"), r3dPoint3D(0, 0, 0));
					StartAsyncOperation(&FrontEnd::as_CloseRewardPopUpThread, &FrontEnd::OnCloseRewardPopUpSuccess);
				}
			}

			if (m_waitingForKeyRemap != -1)
			{
				// query input manager for any input
				bool conflictRemapping = false;
				if (InputMappingMngr->attemptRemapKey((r3dInputMappingMngr::KeybordShortcuts)m_waitingForKeyRemap, conflictRemapping))
				{
					Scaleform::GFx::Value var[2];
					var[0].SetNumber(m_waitingForKeyRemap);
					var[1].SetString(InputMappingMngr->getKeyName((r3dInputMappingMngr::KeybordShortcuts)m_waitingForKeyRemap));
					gfxFrontEnd.Invoke("_root.api.updateKeyboardMapping", var, 2);
					m_waitingForKeyRemap = -1;

					void writeInputMap();
					writeInputMap();

					if (conflictRemapping)
					{
						Scaleform::GFx::Value var[2];
						var[0].SetString(gLangMngr.getString("ConflictRemappingKeys"));
						var[1].SetBoolean(true);
						gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);
					}
				}
			}

			if (gUserProfile.ProfileData.ArmorySlots[gUserProfile.SelectedCharID].Alive == 0 && gUserProfile.ProfileData.ArmorySlots[gUserProfile.SelectedCharID].Hardcore == 0) // dead
			{
				// for now, use hard coded revive time
				const int timeToReviveInSec = gUserProfile.ProfileData.PremiumAcc > 0 ? 5 * 60 : 10 * 60; // 5 and 10 min

				Scaleform::GFx::Value var[3];

				int timeLeftToRevive = R3D_MAX(gUserProfile.ProfileData.ArmorySlots[gUserProfile.SelectedCharID].SecToRevive - int(r3dGetTime() - gTimeWhenProfileLoaded), 0);
				var[0].SetInt(timeLeftToRevive);
				int perc = 100 - int((float(timeLeftToRevive) / float(timeToReviveInSec))*100.0f);
				var[1].SetInt(perc);
#ifdef FINAL_BUILD
				var[2].SetBoolean(timeLeftToRevive <= (timeToReviveInSec - 0)); // show revive button after 0 minutes after death to prevent abuse
#else
				var[2].SetBoolean(gUserProfile.ProfileData.isDevAccount ? true : (timeLeftToRevive <= (timeToReviveInSec - 0)));
#endif
				gfxFrontEnd.Invoke("_root.api.updateDeadTimer", var, 3);
			}

			settingsChangeFlags_ = 0;

			ProcessAsyncOperation();

			if (needUpdateSettings_)
			{
				UpdateSettings();
				needUpdateSettings_ = false;
			}

			if (needUpdateMode_)
			{
				r3dRenderer->UpdateMode();
				needUpdateMode_ = false;
			}

			if (asyncThread_ == NULL)
			{
				// Blackmarket System Begin
				if (m_BlackmarketRefreshDirty)
				{
					m_BlackmarketListWasOutdated = true;

					// blackmarket diry - refresh
					// use frontend to request currently selected categories
					Scaleform::GFx::Value vars[1];
					vars[0].SetInt((int)(eMarketplaceScreen::Blackmarket));
					gfxFrontEnd.Invoke("_root.api.Main.Marketplace.DoLoadBlackmarketWithSelectedTab", vars, 1);

					m_BlackmarketRefreshDirty = false;
				}
				// Blackmarket System End

				if (!gLoginSessionPoller.IsConnected()) {
					Scaleform::GFx::Value var[2];
					var[0].SetString("Session is not valid!");
					var[1].SetBoolean(true);
					gfxFrontEnd.Invoke("_root.api.showInfoMsgCloseGame", var, 2);
				}

				if (!gMasterServerLogic.IsConnected())
				{
					if (gMasterServerLogic.shuttingDown_)
					{
						Scaleform::GFx::Value var[2];
						var[0].SetString("Server shutted down!");
						var[1].SetBoolean(true);
						gfxFrontEnd.Invoke("_root.api.showInfoMsgCloseGame", var, 2);
					}
					else
					{
						Scaleform::GFx::Value var[2];
						var[0].SetString("Server immediately shutted down!");
						var[1].SetBoolean(true);
						gfxFrontEnd.Invoke("_root.api.showInfoMsgCloseGame", var, 2);
					}
				}

				if (needExitByGameJoin_)
				{
					if (!gUserProfile.ProfileData.ArmorySlots[gUserProfile.SelectedCharID].Alive)
					{
						needExitByGameJoin_ = false;

						Scaleform::GFx::Value var[2];
						var[0].SetString(gLangMngr.getString("$FR_PLAY_GAME_SURVIVOR_DEAD"));
						var[1].SetBoolean(true);
						gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);
						return 0;
					}
					return FrontEndShared::RET_JoinGame;
				}
			}
		}
	}

	bool IsNeedExit();
	if (IsNeedExit())
		return FrontEndShared::RET_Exit;

	if (exitRequested_)
		return FrontEndShared::RET_Exit;

	return 0;
}

void FrontEnd::Process_Master_Packages(DWORD peerId, const r3dNetPacketHeader* packetData, int packetSize)
{
	/*switch (packetData->EventID)
	{
	default:
		break;
	}*/
}

void FrontEnd::bindRTsToScaleForm()
{
	RTScaleformTexture = gfxFrontEnd.BoundRTToImage("merc_rendertarget", Scaleform_RenderToTextureRT->AsTex2D(), (int)Scaleform_RenderToTextureRT->Width, (int)Scaleform_RenderToTextureRT->Height);

	if (RTScaleformTexture && Scaleform_RenderToTextureRT)
		RTScaleformTexture->Initialize(Scaleform_RenderToTextureRT->AsTex2D());
}

void FrontEnd::drawPlayer()
{
	m_Player->Darkness = 0;
	if (gUserProfile.ProfileData.ArmorySlots[gUserProfile.SelectedCharID].Alive == 0 && gUserProfile.ProfileData.ArmorySlots[gUserProfile.SelectedCharID].Hardcore == 1)
		m_Player->Darkness = 1;

	m_Player->UpdateTransform();
	r3dPoint3D size = m_Player->GetBBoxLocal().Size;

	float distanceZ = GetOptimalDist(size, 30.0f);

	r3dPoint3D camPos(0, 0, 0);
	r3dPoint3D playerPosHome(0, 0, 0);
	r3dPoint3D playerPosCreate(0, 0, 0);

	// CHARACTER & CAMERA POSITIONS BEGIN
	if (m_needPlayerRenderingRequest == 1) // Centered
	{
		m_Player->SetPosition(playerPosHome);
		camPos = r3dPoint3D(m_Player->GetPosition().x, m_Player->GetPosition().y + (m_Player->Height / 2), m_Player->GetPosition().z + distanceZ);

		gCam = camPos;
		gCam.vPointTo = (r3dPoint3D(m_Player->GetPosition().x, m_Player->GetPosition().y + (m_Player->Height / 2), m_Player->GetPosition().z) - gCam).NormalizeTo();
		gCam.FOV = 45;
	}
	else if (m_needPlayerRenderingRequest == 2) // right sided
	{
		m_Player->SetPosition(playerPosHome);
		camPos = r3dPoint3D(m_Player->GetPosition().x + 0.2f, m_Player->GetPosition().y + (m_Player->Height / 2), m_Player->GetPosition().z + distanceZ);

		gCam = camPos;
		gCam.vPointTo = (r3dPoint3D(m_Player->GetPosition().x, m_Player->GetPosition().y + (m_Player->Height / 2), m_Player->GetPosition().z) - gCam).NormalizeTo();
		gCam.FOV = 45;
	}
	// CHARACTER & CAMERA POSITIONS END

	// ROTATE CHARACTER BEGIN
	if (Mouse->IsPressed(r3dMouse::mRightButton) && m_needPlayerRenderingRequest && m_Player->m_enableRendering)
	{
		r3dVector CamPos(0, 0, 0);
		int mMX = Mouse->m_MouseMoveX;
		float  glb_MouseSensAdj = g_mouse_sensitivity->GetFloat();

		static float camangle = 0;
		camangle += float(-mMX) * glb_MouseSensAdj;
		m_Player->m_fPlayerRotationTarget = m_Player->m_fPlayerRotation = -camangle;
	}
	// ROTATE CHARACTER END

	// ACTUALLY DRAWING PROCESS START
	struct BeginEndEvent
	{
		BeginEndEvent()
		{
			D3DPERF_BeginEvent(0, L"FrontendUI::drawPlayer");
		}

		~BeginEndEvent()
		{
			D3DPERF_EndEvent();
		}
	} beginEndEvent;

	CurRenderPipeline->PreRender();
	CurRenderPipeline->Render();

	CurRenderPipeline->AppendPostFXes();

	{
		PFX_Fill::Settings fsts;

		fsts.ColorWriteMask = D3DCOLORWRITEENABLE_ALPHA;

		gPFX_Fill.PushSettings(fsts);

		g_pPostFXChief->AddFX(gPFX_Fill, PostFXChief::RTT_PINGPONG_LAST, PostFXChief::RTT_DIFFUSE_32BIT);

		PFX_StencilToMask::Settings ssts;

		ssts.Value = float4(0, 0, 0, 1);

		gPFX_StencilToMask.PushSettings(ssts);

		g_pPostFXChief->AddFX(gPFX_StencilToMask, PostFXChief::RTT_PINGPONG_LAST);

		{
			r3dScreenBuffer* buf = g_pPostFXChief->GetBuffer(PostFXChief::RTT_PINGPONG_LAST);
			r3dScreenBuffer* buf_scaleform = g_pPostFXChief->GetBuffer(PostFXChief::RTT_UI_CHARACTER_32BIT);

			PFX_Copy::Settings sts;

			sts.TexScaleX = 1.0f;
			sts.TexScaleY = 1.0f;
			sts.TexOffsetX = 0.0f;
			sts.TexOffsetY = 0.0f;

			gPFX_Copy.PushSettings(sts);

			g_pPostFXChief->AddFX(gPFX_Copy, PostFXChief::RTT_UI_CHARACTER_32BIT);
		}

		g_pPostFXChief->Execute(false, true);
	}

	r3dRenderer->SetVertexShader();
	r3dRenderer->SetPixelShader();
	// ACTUALLY DRAWING END
}

void FrontEnd::StartAsyncOperation(fn_thread threadFn, fn_finish finishFn)
{
	r3d_assert(asyncThread_ == NULL);

	asyncFinish_ = finishFn;
	asyncErr_[0] = 0;
	asyncThread_ = (HANDLE)_beginthreadex(NULL, 0, threadFn, this, 0, NULL);
	if (asyncThread_ == NULL)
		r3dError("Failed to begin thread");
}

void FrontEnd::SetAsyncError(int apiCode, const char* msg)
{
	if (gMasterServerLogic.shuttingDown_)
	{
		sprintf(asyncErr_, "%s", gLangMngr.getString("MSShutdown1"));
		return;
	}

	if (apiCode == 0) {
		sprintf(asyncErr_, "%s", msg);
	}
	else {
		sprintf(asyncErr_, "%s, code:%d", msg, apiCode);
	}
}

void FrontEnd::ProcessAsyncOperation()
{
	if (asyncThread_ == NULL)
		return;

	DWORD w0 = WaitForSingleObject(asyncThread_, 0);
	if (w0 == WAIT_TIMEOUT)
		return;

	CloseHandle(asyncThread_);
	asyncThread_ = NULL;

	if (gMasterServerLogic.badClientVersion_)
	{
		Scaleform::GFx::Value args[2];
		args[0].SetString(gLangMngr.getString("ClientMustBeUpdated"));
		args[1].SetBoolean(true);
		gfxFrontEnd.Invoke("_root.api.showInfoMsg", args, 2);
		//@TODO: on infoMsg closing, exit app.
		return;
	}

	if (asyncErr_[0])
	{
		Scaleform::GFx::Value args[2];
		args[0].SetString(asyncErr_);
		args[1].SetBoolean(true);
		gfxFrontEnd.Invoke("_root.api.showInfoMsg", args, 2);
		return;
	}

	if (asyncFinish_)
		(this->*asyncFinish_)();
}

void FrontEnd::addClientSurvivor(const wiCharDataFull& slot, int slotIndex)
{
	Scaleform::GFx::Value var[27];
	char tmpGamertag[128];
	if (slot.ClanID != 0)
		sprintf(tmpGamertag, "[%s] %s", slot.ClanTag, slot.Gamertag);
	else
		r3dscpy(tmpGamertag, slot.Gamertag);
	var[0].SetString(tmpGamertag);
	var[1].SetNumber(slot.Health);
	var[2].SetNumber(slot.Stats.XP);
	var[3].SetNumber(slot.Stats.TimePlayed);
	var[4].SetNumber(slot.Hardcore);
	var[5].SetNumber(slot.HeroItemID);
	var[6].SetNumber(slot.HeadIdx);
	var[7].SetNumber(slot.BodyIdx);
	var[8].SetNumber(slot.LegsIdx);
	var[9].SetNumber(slot.Alive);
	var[10].SetNumber(slot.Hunger);
	var[11].SetNumber(slot.Thirst);
	var[12].SetNumber(slot.Toxic);
	var[13].SetNumber(slot.BackpackID);
	var[14].SetNumber(slot.BackpackSize);

	float totalWeight = slot.getTotalWeight();
	if (slot.Skills[CUserSkills::SKILL_Physical3])
		totalWeight *= 0.95f;
	if (slot.Skills[CUserSkills::SKILL_Physical7])
		totalWeight *= 0.9f;
	var[15].SetNumber(totalWeight);		// weight
	var[16].SetNumber(slot.Stats.KilledZombies);		// zombies Killed
	var[17].SetNumber(slot.Stats.KilledBandits);		// bandits killed
	var[18].SetNumber(slot.Stats.KilledSurvivors);		// civilians killed
	char repS[64];
	sprintf(repS, "%s : %d", getReputationString(slot.Stats.Reputation), slot.Stats.Reputation);
	var[19].SetString(repS);	// alignment

	const char* lastMap = getMapName(slot.GameMapId);
	var[20].SetString(lastMap);

	//if (gUserProfile.ProfileData.PremiumAcc > 0 || gUserProfile.ProfileData.isDevAccount > 0) // AlexRedd:: GI access for everyone
		var[21].SetBoolean(true);
	//else
		//var[21].SetBoolean(slot.GameFlags & wiCharDataFull::GAMEFLAG_NearPostBox);
	var[22].SetInt(slot.Stats.XP - slot.Stats.SpendXP);
	/*char kdrS[32];
	sprintf(kdrS, "%.2f", getRatio(float(slot.Stats.KilledSurvivors + slot.Stats.KilledBandits), float(slot.Stats.Deaths)));
	var[23].SetString(kdrS); // kdr
	char hsrS[32];
	sprintf(hsrS, "%.2f", getRatio(float(slot.Stats.ShotsHeadshots), float(slot.Stats.ShotsFired)));
	var[24].SetString(hsrS); // hsr
	var[25].SetInt(int(getRatio((float)slot.Stats.ShotsHits, (float)slot.Stats.ShotsFired)*100.0f)); // accuracy
	var[26].SetInt(slot.Stats.Deaths); // died*/

	gfxFrontEnd.Invoke("_root.api.addClientSurvivor", var, 23);

	addBackpackItems(slot, slotIndex);

	for (int i = 0; i < 34; ++i)
	{
		if (slot.Skills[i] > 0)
		{
			var[1].SetInt(i);
			gfxFrontEnd.Invoke("_root.api.setSkillLearnedSurvivor", var, 2);
		}
	}
}

void FrontEnd::addBackpackItems(const wiCharDataFull& slot, int slotIndex)
{
	Scaleform::GFx::Value var[8];
	for (int a = 0; a < slot.BackpackSize; a++)
	{
		if (slot.Items[a].InventoryID != 0)
		{
			var[0].SetInt(slotIndex);
			var[1].SetInt(a);
			var[2].SetUInt(uint32_t(slot.Items[a].InventoryID));
			var[3].SetUInt(slot.Items[a].itemID);
			var[4].SetInt(slot.Items[a].quantity);
			var[5].SetInt(slot.Items[a].Var1);
			var[6].SetInt(slot.Items[a].Var2);
			var[7].SetString(getAdditionalDescForItem(slot.Items[a].itemID, slot.Items[a].Var1, slot.Items[a].Var2, slot.Items[a].Var3));
			gfxFrontEnd.Invoke("_root.api.addBackpackItem", var, 8);
		}
	}
}

void FrontEnd::initVoipOptions()
{
	gfxFrontEnd.Invoke("_root.api.Main.OptionsVoip.clearDevicesArray", NULL, 0);

	if (gTeamSpeakClient.m_playbackDevices.size() == 0 && gTeamSpeakClient.m_captureDevices.size() == 0)
	{
		g_voip_enable->SetBool(false);
		writeGameOptionsFile();
		return;
	}

	// reset index in case when we're out of current devices count
	if (g_voip_InputDeviceInd->GetInt() >= (int)gTeamSpeakClient.m_captureDevices.size())
		g_voip_InputDeviceInd->SetInt(-1);
	if (g_voip_OutputDeviceInd->GetInt() >= (int)gTeamSpeakClient.m_playbackDevices.size())
		g_voip_OutputDeviceInd->SetInt(-1);

	for (size_t i = 0; i < gTeamSpeakClient.m_playbackDevices.size(); i++)
	{
		const CTeamSpeakClient::dev_s& dev = gTeamSpeakClient.m_playbackDevices[i];
		Scaleform::GFx::Value var[1];
		var[0].SetString(dev.name.c_str());
		gfxFrontEnd.Invoke("_root.api.Main.OptionsVoip.addOutputDevice", var, 1);

		if (g_voip_OutputDeviceInd->GetInt() == -1 && dev.isDefault)
			g_voip_OutputDeviceInd->SetInt(i);
	}

	for (size_t i = 0; i < gTeamSpeakClient.m_captureDevices.size(); i++)
	{
		const CTeamSpeakClient::dev_s& dev = gTeamSpeakClient.m_captureDevices[i];
		Scaleform::GFx::Value var[1];
		var[0].SetString(dev.name.c_str());
		gfxFrontEnd.Invoke("_root.api.Main.OptionsVoip.addInputDevice", var, 1);

		if (g_voip_InputDeviceInd->GetInt() == -1 && dev.isDefault)
			g_voip_InputDeviceInd->SetInt(i);
	}

	writeGameOptionsFile();
}

void FrontEnd::initFrontend()
{
	if (dictionaryHash_.Size() == 0)
	{
		r3dFile* f = r3d_open("Data/LangPack/dictionary.dat", "rb");
		char tmpStr[256];
		while (fgets(tmpStr, 256, f) != NULL)
		{
			size_t len = strlen(tmpStr);
			for (size_t i = 0; i < len; ++i)
			{
				if (tmpStr[i] == 13 || tmpStr[i] == 10)
					tmpStr[i] = 0;
			}
			dictionaryHash_.Add(tmpStr, tmpStr);
		}
		fclose(f);
	}

	initItems();

	// tutorial screens
	/*{
	if(g_shown_marketplace_tutorial->GetBool() == false)
	{
	gfxFrontEnd.SetVariable("_root.api.Main.Marketplace.Marketplace.Tutorial.visible", true);
	}
	}*/

	// add skill info
	{
		// addSkillInfo(id:uint, name:String, desc:String, icon:String, iconBW:String, cost:uint)
		Scaleform::GFx::Value var[6];
		char tmpStr[64];
		for (int i = 0; i < 34; ++i)
		{
			var[0].SetUInt(i);
			sprintf(tmpStr, "SkillName%d", i);
			var[1].SetString(gLangMngr.getString(tmpStr));
			sprintf(tmpStr, "SkillDesc%d", i);
			var[2].SetString(gLangMngr.getString(tmpStr));
			char iconPath[512];
			sprintf(iconPath, "$Data/Menu/skillIcons/Skill%d.dds", i);
			var[3].SetString(iconPath);
			char iconPathBW[512];
			sprintf(iconPathBW, "$Data/Menu/skillIcons/Skill%dBW.dds", i);
			var[4].SetString(iconPathBW);
			var[5].SetUInt(gUserProfile.ShopSkillCosts[i][0]);
			gfxFrontEnd.Invoke("_root.api.addSkillInfo", var, 6);
		}
	}

	initVoipOptions();

	Scaleform::GFx::Value var[20];
	// init browse channel buttons
	{
#ifdef ENABLE_BATTLE_ROYALE
		//AlexRedd:: BR mode
		var[0].SetBoolean(true/*gUserProfile.IsTrialAccount() || gUserProfile.ProfileData.isDevAccount>0*/); // BR channel
#else
		var[0].SetBoolean(false); // BR channel
#endif //ENABLE_BATTLE_ROYALE
		var[1].SetBoolean(true/*!gUserProfile.IsTrialAccount()*/); // official channel
		var[2].SetBoolean(true/*!gUserProfile.IsTrialAccount()*/); // private servers
		var[3].SetBoolean(true/*gUserProfile.ShopPremiumAcc>0 || gUserProfile.ProfileData.PremiumAcc>0*/); // premium servers (enable only if premium is available for sale or player alreayd has premium account
		var[4].SetBoolean(false/*!gUserProfile.IsTrialAccount()*/); // strongholds		
		var[5].SetBoolean(true/*gUserProfile.ShopPremiumAcc>0 || gUserProfile.ProfileData.PremiumAcc>0*/); //DEV EVENT SERVERS 
		var[6].SetBoolean(true/*!gUserProfile.IsTrialAccount()*/); // pvp servers
		gfxFrontEnd.Invoke("_root.api.Main.BrowseGamesChannelsAnim.initButtons", var, 7);
	}

	gfxFrontEnd.SetVariable("_root.api.isTrialAccount", gUserProfile.IsTrialAccount());
	//gfxFrontEnd.SetVariable("_root.api.Main.Marketplace.Marketplace.Tab10.visible", false);
	//gfxFrontEnd.SetVariable("_root.api.Main.Marketplace.Marketplace.Tab11.visible", false);
	//gfxFrontEnd.SetVariable("_root.api.Main.Marketplace.Marketplace.Tab12.visible", false);	
	//gfxFrontEnd.SetVariable("_root.api.Main.SurvivorsAnim.Survivors.BtnUpgtrial.visible", gUserProfile.ProfileData.AccountType == 2 ? true : false);
	gfxFrontEnd.SetVariable("_root.api.Main.SurvivorsAnim.Survivors.BtnUpgtrial.Text.Text.text", gLangMngr.getString("$FR_UpgradeTrialAccount"));
	gfxFrontEnd.SetVariable("_root.api.Main.SurvivorsAnim.Survivors.PremiumAcc.visible", gUserProfile.ProfileData.PremiumAcc > 0 ? true : false);

	//gfxFrontEnd.SetVariable("_root.api.Main.OptionsExtra.OptionsExtraClip.OptStat12.visible", false);// hide the fastload option
	gfxFrontEnd.SetVariable("_root.api.Main.OptionsExtra.OptionsExtraClip.OptStat4.visible", false);// hide the increase fps option

	//AlexRedd:: daily rewards screen
	if (gUserProfile.ProfileData.showRewardPopUp > 0 && m_closeRewardMessageActionID == 0)
	{
		m_closeRewardMessageActionID = 1;
		gfxFrontEnd.Invoke("_root.api.hideInfoMsg", "");
		StartAsyncOperation(&FrontEnd::as_ShowRewardPopUpThread, &FrontEnd::OnShowRewardPopUpSuccess);
	}

	//AlexRedd:: welcome text
	char welcomeText[128] = { 0 };
	sprintf(welcomeText, gUserProfile.ProfileData.isDevAccount > 0 ? gLangMngr.getString("$FR_WelcomeDEV") : gLangMngr.getString("$FR_Welcome"), gUserProfile.username);
	gfxFrontEnd.SetVariable("_root.api.Main.SurvivorsAnim.Survivors.WelcomeText.Text.text", welcomeText);

	//AlexRedd:: Premium text
	if (gUserProfile.ProfileData.PremiumAcc > 0)
	{
		int minutesLeft = gUserProfile.ProfileData.PremiumAcc;
		char buf[1024];
		// temp sring. maybe create a valid date and show it
		int daysLeft = minutesLeft / 1440;
		int hoursLeft = minutesLeft / 60;
		sprintf(buf, gLangMngr.getString("PremiumAccountLeft"), (daysLeft > 0) ? (daysLeft > 0) ? (daysLeft) : (hoursLeft) : (hoursLeft > 0) ? (hoursLeft) : (minutesLeft),
			(daysLeft > 0) ? (daysLeft > 0) ? gLangMngr.getString("$FR_PAUSE_INVENTORY_DAYS") : gLangMngr.getString("$FR_PAUSE_INVENTORY_HR") :
			(hoursLeft > 0) ? gLangMngr.getString("$FR_PAUSE_INVENTORY_HR") : gLangMngr.getString("$FR_PAUSE_INVENTORY_MIN"));
		gfxFrontEnd.SetVariable("_root.api.Main.SurvivorsAnim.Survivors.PremiumAcc.PremiumText.text", buf);
	}
	/////

	/*if(gUserProfile.IsTrialAccount())
	{
	gfxFrontEnd.SetVariable("_root.api.Main.SurvivorsAnim.Survivors.BtnUpgtrial.visible", true);
	}
	if(gUserProfile.ProfileData.PremiumAcc>0)
	{
	gfxFrontEnd.SetVariable("_root.api.Main.SurvivorsAnim.Survivors.PremiumAcc.visible", true);
	}*/

	// send survivor info
	for (int i = 0; i < gUserProfile.ProfileData.NumSlots; ++i)
	{
		addClientSurvivor(gUserProfile.ProfileData.ArmorySlots[i], i);
	}

	updateSurvivorTotalWeight(gUserProfile.SelectedCharID);

	var[0].SetInt(gUserProfile.ProfileData.GamePoints);
	gfxFrontEnd.Invoke("_root.api.setGC", var, 1);

	var[0].SetInt(gUserProfile.ProfileData.GameDollars);
	gfxFrontEnd.Invoke("_root.api.setDollars", var, 1);

	var[0].SetInt(gUserProfile.ProfileData.XP50Boosts);
	gfxFrontEnd.Invoke("_root.api.setXPBoosts", var, 1);

	var[0].SetInt(0);
	gfxFrontEnd.Invoke("_root.api.setCells", var, 1);

	{
		char tempText[512] = "0";
		if (gUserProfile.ShopCharReviveGC > 0)
		{
			sprintf(tempText, "<font color=\"#ffc300\">%d</font>", gUserProfile.ShopCharReviveGC);
			gfxFrontEnd.Invoke("_root.api.Main.PopUpEarlyRevival.Value.IconGC.gotoAndStop", 1);
		}
		else if (gUserProfile.ShopCharReviveGD > 0)
		{
			sprintf(tempText, "<font color=\"#00b2ff\">%d</font>", gUserProfile.ShopCharReviveGD);
			gfxFrontEnd.Invoke("_root.api.Main.PopUpEarlyRevival.Value.IconGC.gotoAndStop", 2);
		}
		gfxFrontEnd.SetVariable("_root.api.EarlyRevival_Price", tempText);
	}
	gfxFrontEnd.SetVariable("_root.api.PremiumAccount_Price", gUserProfile.ShopPremiumAcc);
	gfxFrontEnd.SetVariable("_root.api.ChangeName_Price", gUserProfile.ShopCharRename);
	gfxFrontEnd.SetVariable("_root.api.LegendAccount_Price", gUserProfile.ShopLegendAcc);
	gfxFrontEnd.SetVariable("_root.api.PioneerAccount_Price", gUserProfile.ShopPioneerAcc);
	{
		char tempText[512];
		if (gUserProfile.ShopCharTeleportGD > 0)
			sprintf(tempText, "%s \n %s <font color=\"#00b2ff\">%d</font>", gLangMngr.getString("$FR_TeleportConf"), gLangMngr.getString("$FR_TeleportGDCost"), gUserProfile.ShopCharTeleportGD);
		else if (gUserProfile.ShopCharTeleportGC > 0)
			sprintf(tempText, "%s \n %s <font color=\"#ffc300\">%d</font>", gLangMngr.getString("$FR_TeleportConf"), gLangMngr.getString("$FR_TeleportGCCost"), gUserProfile.ShopCharTeleportGC);
		else
			sprintf(tempText, gLangMngr.getString("$FR_TeleportConf"));
		gfxFrontEnd.SetVariable("_root.api.TeleportChar_Price", tempText);
	}

	for (int i = 0; i < r3dInputMappingMngr::KS_NUM; ++i)
	{
		Scaleform::GFx::Value args[2];
		args[0].SetString(gLangMngr.getString(InputMappingMngr->getMapName((r3dInputMappingMngr::KeybordShortcuts)i)));
		args[1].SetString(InputMappingMngr->getKeyName((r3dInputMappingMngr::KeybordShortcuts)i));
		gfxFrontEnd.Invoke("_root.api.addKeyboardMapping", args, 2);
	}

	SyncGraphicsUI();
	r_fxaa->SetBool(true);

	gfxFrontEnd.SetVariable("_root.api.SelectedChar", gUserProfile.SelectedCharID);
	m_Player->uberAnim_->anim.StopAll();
	m_Player->UpdateLoadoutSlot(gUserProfile.ProfileData.ArmorySlots[gUserProfile.SelectedCharID], -1);

	gfxFrontEnd.Invoke("_root.api.showSurvivorsScreen", "");

	// AlexRedd:: check if user has other language set english client
	if (strcmp(g_user_language->GetString(), "french") == 0 || strcmp(g_user_language->GetString(), "german") == 0 ||
		strcmp(g_user_language->GetString(), "italian") == 0 || /*strcmp(g_user_language->GetString(), "spanish") == 0 ||*/
		strcmp(g_user_language->GetString(), "chinese") == 0)
	{
		//g_user_language->SetString("chinese"); 
		//g_user_language->SetString("english"); // ½ûÓÃÆäËûÓïÑÔ
		writeGameOptionsFile();
		ReloadLocalization();
	}
	/*if(!gUserProfile.IsRussianAccount())
	gfxFrontEnd.SetVariable("_root.api.Main.OptionsLang.OptionsLangClip.Lang6.visible", false);*/

	gfxFrontEnd.Invoke("_root.api.setLanguage", g_user_language->GetString());

	// init clan icons
	// important: DO NOT CHANGE THE ORDER OF ICONS!!! EVER!
	{
		gfxFrontEnd.Invoke("_root.api.addClanIcon", "$Data/Menu/clanIcons/clan_survivor.dds");
		gfxFrontEnd.Invoke("_root.api.addClanIcon", "$Data/Menu/clanIcons/clan_bandit.dds");
		gfxFrontEnd.Invoke("_root.api.addClanIcon", "$Data/Menu/clanIcons/clan_lawman.dds");
		gfxFrontEnd.Invoke("_root.api.addClanIcon", "$Data/Menu/clanIcons/clan_skull.dds");
		gfxFrontEnd.Invoke("_root.api.addClanIcon", "$Data/Menu/clanIcons/clan_tiger.dds");
		gfxFrontEnd.Invoke("_root.api.addClanIcon", "$Data/Menu/clanIcons/clan_strong.dds");
		// add new icons at the end!
	}
	{
		//public function addClanSlotBuyInfo(buyIdx:uint, price:uint, numSlots:uint)
		Scaleform::GFx::Value var[3];
		for (int i = 0; i < 6; ++i)
		{
			var[0].SetUInt(i);
			var[1].SetUInt(gUserProfile.ShopClanAddMembers_GP[i]);
			var[2].SetUInt(gUserProfile.ShopClanAddMembers_Num[i]);
			gfxFrontEnd.Invoke("_root.api.addClanSlotBuyInfo", var, 3);
		}
	}

	// add server rental info
	{
		Scaleform::GFx::Value var[3];
		var[0].SetInt(GBGameInfo::MAPID_ZP_Test);
		var[1].SetString(gLangMngr.getString("MapColorado"));
		gfxFrontEnd.Invoke("_root.api.addRentServer_MapInfo", var, 2);

		/*var[0].SetInt(GBGameInfo::MAPID_WZ_Caliwood);
		var[1].SetString(gLangMngr.getString("MapCaliwood"));
		gfxFrontEnd.Invoke("_root.api.addRentServer_MapInfo", var, 2);	*/

		var[0].SetInt(GBGameInfo::MAPID_WZ_Cliffside); // need id
		var[1].SetString(gLangMngr.getString("MapCliffside"));
		gfxFrontEnd.Invoke("_root.api.addRentServer_StrongholdInfo", var, 2);

		var[0].SetInt(GBNET_REGION_US_West);
		var[1].SetString(gLangMngr.getString("$FR_REGION_NA"));
		gfxFrontEnd.Invoke("_root.api.addRentServer_RegionInfo", var, 2);

		// MAKE SURE TO SYNC slot numbers with backend API api_ServersMgr::GameworldSlots
		// MAKE SURE TO SYNC slot numbers with eventRenewServerUpdatePrice
		var[0].SetInt(0);
		var[1].SetString("10");
		var[2].SetBoolean(true);
		gfxFrontEnd.Invoke("_root.api.addRentServer_SlotsInfo", var, 3);
		var[0].SetInt(1);
		var[1].SetString("30");
		var[2].SetBoolean(true);
		gfxFrontEnd.Invoke("_root.api.addRentServer_SlotsInfo", var, 3);
		var[0].SetInt(2);
		var[1].SetString("50");
		var[2].SetBoolean(true);
		gfxFrontEnd.Invoke("_root.api.addRentServer_SlotsInfo", var, 3);
		var[0].SetInt(3);
		var[1].SetString("70");
		var[2].SetBoolean(true);
		gfxFrontEnd.Invoke("_root.api.addRentServer_SlotsInfo", var, 3);
		var[0].SetInt(4);
		var[1].SetString("100");
		var[2].SetBoolean(true);
		gfxFrontEnd.Invoke("_root.api.addRentServer_SlotsInfo", var, 3);

		// MAKE SURE TO SYNC actual slot numbers with backend API api_ServersMgr::StrongholdSlots
		// MAKE SURE TO SYNC slot numbers with eventRenewServerUpdatePrice
		var[0].SetInt(0);
		var[1].SetString("10");
		var[2].SetBoolean(true);
		gfxFrontEnd.Invoke("_root.api.addRentServer_SlotsInfoStronghold", var, 3);
		var[0].SetInt(1);
		var[1].SetString("20");
		var[2].SetBoolean(true);
		gfxFrontEnd.Invoke("_root.api.addRentServer_SlotsInfoStronghold", var, 3);
		var[0].SetInt(2);
		var[1].SetString("30");
		var[2].SetBoolean(true);
		gfxFrontEnd.Invoke("_root.api.addRentServer_SlotsInfoStronghold", var, 3);
		var[0].SetInt(3);
		var[1].SetString("40");
		var[2].SetBoolean(true);
		gfxFrontEnd.Invoke("_root.api.addRentServer_SlotsInfoStronghold", var, 3);
		var[0].SetInt(4);
		var[1].SetString("50");
		var[2].SetBoolean(true);
		gfxFrontEnd.Invoke("_root.api.addRentServer_SlotsInfoStronghold", var, 3);

		var[0].SetInt(0); // rent ID
		var[1].SetUInt(3); // number of months\weeks
		var[2].SetString(gLangMngr.getString("Days")); // string
		gfxFrontEnd.Invoke("_root.api.addRentServer_RentInfo", var, 3);
		var[0].SetInt(1); // rent ID
		var[1].SetUInt(7); // number of months\weeks
		var[2].SetString(gLangMngr.getString("Days")); // string
		gfxFrontEnd.Invoke("_root.api.addRentServer_RentInfo", var, 3);
		var[0].SetInt(2);
		var[1].SetUInt(15);
		var[2].SetString(gLangMngr.getString("Days"));
		gfxFrontEnd.Invoke("_root.api.addRentServer_RentInfo", var, 3);
		var[0].SetInt(3);
		var[1].SetUInt(30);
		var[2].SetString(gLangMngr.getString("Days"));
		gfxFrontEnd.Invoke("_root.api.addRentServer_RentInfo", var, 3);
		var[0].SetInt(4);
		var[1].SetUInt(60);
		var[2].SetString(gLangMngr.getString("Days"));
		gfxFrontEnd.Invoke("_root.api.addRentServer_RentInfo", var, 3);

		var[0].SetInt(0);
		var[1].SetString(gLangMngr.getString("$FR_optNo"));
		gfxFrontEnd.Invoke("_root.api.addRentServer_PVEInfo", var, 2);
		// disabled until proper design
		//var[0].SetInt(1);
		//var[1].SetString(gLangMngr.getString("$FR_optYes"));
		//gfxFrontEnd.Invoke("_root.api.addRentServer_PVEInfo", var, 2);
	}

	gfxFrontEnd.Invoke("_root.api.hideLoginMsg", "");

	m_waitingForKeyRemap = -1;
	m_needPlayerRenderingRequest = 1; // by default when FrontendInit we are in home screen, so show player

	{
		extern char g_Custom_KickMsg[256];
		Scaleform::GFx::Value vars[3];
		vars[1].SetBoolean(true);
		vars[2].SetString("$ERROR");
		switch (prevGameResult)
		{
		case GRESULT_Failed_To_Join_Game:
			vars[0].SetString(gLangMngr.getString("FailedToJoinGame"));
			gfxFrontEnd.Invoke("_root.api.showInfoMsg", vars, 3);
			break;
		case GRESULT_Timeout:
			vars[0].SetString(gLangMngr.getString("TimeoutJoiningGame"));
			gfxFrontEnd.Invoke("_root.api.showInfoMsg", vars, 3);
			break;
		case GRESULT_StillInGame:
			vars[0].SetString(gLangMngr.getString("FailedToJoinGameStillInGame"));
			gfxFrontEnd.Invoke("_root.api.showInfoMsg", vars, 3);
			break;
		case GRESULT_Disconnect:
			if (g_Custom_KickMsg[0])
			{
				vars[0].SetString(g_Custom_KickMsg);
				gfxFrontEnd.Invoke("_root.api.showInfoMsg", vars, 3);
				memset(g_Custom_KickMsg, 0, sizeof(g_Custom_KickMsg));
			}
			else
			{
				vars[0].SetString(gLangMngr.getString("DisconnectMsg"));
				gfxFrontEnd.Invoke("_root.api.showInfoMsg", vars, 3);
			}
			break;
		}
	}
	prevGameResult = GRESULT_Unknown;

	scan_loadout();

	// Blackmarket System Begin
	clearBlackmarketSettingUI(gfxFrontEnd);
	addBlackmarketSettingToUI(gfxFrontEnd);
	// Blackmarket System End
}

void FrontEnd::eventPlayGame(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	if (gUserProfile.ProfileData.NumSlots == 0)
		return;

	StartAsyncOperation(&FrontEnd::as_PlayGameThread);
}

void FrontEnd::eventCancelQuickGameSearch(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	CancelQuickJoinRequest = true;
}

void FrontEnd::eventQuitGame(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	exitRequested_ = true;
}

void FrontEnd::eventCreateCharacter(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3dscpy(m_CreateGamerTag, args[0].GetString()); // gamertag
	m_CreateHeroID = args[1].GetInt(); // hero
	m_CreateGameMode = args[2].GetInt(); // mode
	m_CreateHeadIdx = args[3].GetInt(); // bodyID
	m_CreateBodyIdx = args[4].GetInt(); // headID
	m_CreateLegsIdx = args[5].GetInt(); // legsID

	if (gUserProfile.IsTrialAccount())
	{
		// only one survivor allowed for trial accounts
		if (gUserProfile.ProfileData.NumSlots > 0)
		{
			Scaleform::GFx::Value var[2];
			var[0].SetString(gLangMngr.getString("TrialAccountLimit_SurvivorSlots"));
			var[1].SetString(gLangMngr.getString("$TrialAccountLimit"));
			gfxFrontEnd.Invoke("_root.api.showTrialAccountLimit", var, 2);
			return;
		}
	}

	if (strpbrk(m_CreateGamerTag, "!@#$%^&*()-=+_<>,./?'\":;|{}[]") != NULL) // do not allow this symbols
	{
		Scaleform::GFx::Value var[2];
		var[0].SetString(gLangMngr.getString("CharacterNameCannotContaintSpecialSymbols"));
		var[1].SetBoolean(true);
		gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);
		return;
	}
	// check profanity filter
	{
		bool hasProfanity = false;
		dictionaryHash_.IterateStart();
		while (dictionaryHash_.IterateNext())
		{
			const char* profanityEntry = dictionaryHash_.IterateGetKey().c_str();
			if (stristr(m_CreateGamerTag, profanityEntry))
			{
				hasProfanity = true;
				break;
			}
		}
		if (hasProfanity)
		{
			Scaleform::GFx::Value var[2];
			var[0].SetString(gLangMngr.getString("CharacterNameCannotContainProfanity"));
			var[1].SetBoolean(true);
			gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);
			return;
		}
	}

	if (gUserProfile.ProfileData.NumSlots >= 5)
	{
		Scaleform::GFx::Value var[2];
		var[0].SetString(gLangMngr.getString("CannotCreateMoreThan5Char"));
		var[1].SetBoolean(true);
		gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);
		return;
	}

	Scaleform::GFx::Value var[2];
	var[0].SetString(gLangMngr.getString("OneMomentPlease"));
	var[1].SetBoolean(false);
	gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);

	StartAsyncOperation(&FrontEnd::as_CreateCharThread, &FrontEnd::OnCreateCharSuccess);
}

void FrontEnd::eventRenameCharacter(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3dscpy(m_CreateGamerTag, args[0].GetString()); // gamertag

	if (strpbrk(m_CreateGamerTag, "!@#$%^&*()-=+_<>,./?'\":;|{}[]") != NULL) // do not allow this symbols
	{
		Scaleform::GFx::Value var[2];
		var[0].SetString(gLangMngr.getString("CharacterNameCannotContaintSpecialSymbols"));
		var[1].SetBoolean(true);
		gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);
		return;
	}
	// check profanity filter
	{
		bool hasProfanity = false;
		dictionaryHash_.IterateStart();
		while (dictionaryHash_.IterateNext())
		{
			const char* profanityEntry = dictionaryHash_.IterateGetKey().c_str();
			if (stristr(m_CreateGamerTag, profanityEntry))
			{
				hasProfanity = true;
				break;
			}
		}
		if (hasProfanity)
		{
			Scaleform::GFx::Value var[2];
			var[0].SetString(gLangMngr.getString("CharacterNameCannotContainProfanity"));
			var[1].SetBoolean(true);
			gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);
			return;
		}
	}

	Scaleform::GFx::Value var[2];
	var[0].SetString(gLangMngr.getString("OneMomentPlease"));
	var[1].SetBoolean(false);
	gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);

	r3dscpy(CharRename_PreviousName, gUserProfile.ProfileData.ArmorySlots[gUserProfile.SelectedCharID].Gamertag);

	StartAsyncOperation(&FrontEnd::as_CharRenameThread, &FrontEnd::OnCharRenameSuccess);
}

void FrontEnd::eventDeleteChar(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	Scaleform::GFx::Value var[2];
	var[0].SetString(gLangMngr.getString("OneMomentPlease"));
	var[1].SetBoolean(false);
	gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);

	StartAsyncOperation(&FrontEnd::as_DeleteCharThread, &FrontEnd::OnDeleteCharSuccess);
}

extern const char* getLevelDirBasedOnLevelID(int mapID);
extern bool ParseXMLFile(const char *xmlFileName, pugi::xml_document &outXml, char *& outBuf);
extern r3dPoint2D getMinimapPosWithExternalSize(const r3dPoint3D& pos, const r3dPoint3D& worldOrigin, const r3dPoint3D& worldSize);
void FrontEnd::eventShowSurvivorsMap(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount == 0);

	wiCharDataFull& w = gUserProfile.ProfileData.ArmorySlots[gUserProfile.SelectedCharID];
	if (w.Alive != 1)
	{
		Scaleform::GFx::Value var[2];
		var[0].SetString(gLangMngr.getString("SurvivorIsNotSpawnedYet"));
		var[1].SetBoolean(true);
		gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);
		return;
	}

	char HomeDir[512];
	sprintf(HomeDir, "Levels\\%s\\minimap.dds", getLevelDirBasedOnLevelID(w.GameMapId));

	if (r3dFileExists(HomeDir))
	{
		char HomeDir2[512];
		sprintf(HomeDir2, "$%s", HomeDir);
		gfxFrontEnd.Invoke("_root.api.Main.SurvivorsAnim.loadSurvivorMap", HomeDir2);
	}
	else
	{
		Scaleform::GFx::Value var[2];
		var[0].SetString(gLangMngr.getString("MinimapNotAvailableForThisLevel"));
		var[1].SetBoolean(true);
		gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);
		return;
	}

	// we need to load level xml to read minimap size...
	r3dPoint3D minimapOrigin(0, 0, 0), minimapSize(0, 0, 0);
	{
		char fname[MAX_PATH];
		sprintf(fname, "Levels\\%s\\LevelData.xml", getLevelDirBasedOnLevelID(w.GameMapId));
		r3d_assert(r3dFileExists(fname));

		pugi::xml_document xmlLevelFile;
		char *levelData = 0;
		ParseXMLFile(fname, xmlLevelFile, levelData);

		pugi::xml_node xmlLevel = xmlLevelFile.child("level");

		minimapOrigin.x = xmlLevel.attribute("minimapOrigin.x").as_float();
		minimapOrigin.z = xmlLevel.attribute("minimapOrigin.z").as_float();
		minimapSize.x = xmlLevel.attribute("minimapSize.x").as_float();
		minimapSize.z = xmlLevel.attribute("minimapSize.z").as_float();

		delete[] levelData;
	}

	//AlexRedd:: teleport system
	if (w.GameMapId == GBGameInfo::MAPID_ZP_Test)
	{
		Scaleform::GFx::Value var[2];
		var[0].SetString(gLangMngr.getString("OneMomentPlease"));
		var[1].SetBoolean(false);
		gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);

		StartAsyncOperation(&FrontEnd::as_RequestTeleportThread, &FrontEnd::OnRequestTeleportSuccess);
	}
	else {
		for (int i = 0; i < wiUserProfile::MAX_LOADOUT_SLOTS; ++i)
		{
			wiCharDataFull& w2 = gUserProfile.ProfileData.ArmorySlots[i];
			if (w2.Alive == 1 && w2.GameMapId == w.GameMapId)
			{
				r3dPoint2D mapPos = getMinimapPosWithExternalSize(w2.GamePos, minimapOrigin, minimapSize);
				Scaleform::GFx::Value var[3];
				var[0].SetNumber(mapPos.x);
				var[1].SetNumber(mapPos.y);
				var[2].SetString(w2.Gamertag);
				gfxFrontEnd.Invoke("_root.api.Main.SurvivorsAnim.addSurvivorPinToMap", var, 3);
			}
		}
	}

	gfxFrontEnd.SetVariable("_root.api.Main.SurvivorsAnim.Survivors.PopupLastMap.Title.text", getMapName(w.GameMapId));
	gfxFrontEnd.SetVariable("_root.api.Main.SurvivorsAnim.Survivors.PopupLastMap.visible", true);
}

void FrontEnd::eventShowSurvivorsMapRefresh()
{
	wiCharDataFull& w = gUserProfile.ProfileData.ArmorySlots[gUserProfile.SelectedCharID];

	char HomeDir[512];
	sprintf(HomeDir, "Levels\\%s\\minimap.dds", getLevelDirBasedOnLevelID(w.GameMapId));

	if (r3dFileExists(HomeDir))
	{
		char HomeDir2[512];
		sprintf(HomeDir2, "$%s", HomeDir);
		gfxFrontEnd.Invoke("_root.api.Main.SurvivorsAnim.loadSurvivorMap", HomeDir2);
	}
	else
	{
		Scaleform::GFx::Value var[2];
		var[0].SetString(gLangMngr.getString("MinimapNotAvailableForThisLevel"));
		var[1].SetBoolean(true);
		gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);
		return;
	}

	// we need to load level xml to read minimap size...
	r3dPoint3D minimapOrigin(0, 0, 0), minimapSize(0, 0, 0);
	{
		char fname[MAX_PATH];
		sprintf(fname, "Levels\\%s\\LevelData.xml", getLevelDirBasedOnLevelID(w.GameMapId));
		r3d_assert(r3dFileExists(fname));

		pugi::xml_document xmlLevelFile;
		char *levelData = 0;
		ParseXMLFile(fname, xmlLevelFile, levelData);

		pugi::xml_node xmlLevel = xmlLevelFile.child("level");

		minimapOrigin.x = xmlLevel.attribute("minimapOrigin.x").as_float();
		minimapOrigin.z = xmlLevel.attribute("minimapOrigin.z").as_float();
		minimapSize.x = xmlLevel.attribute("minimapSize.x").as_float();
		minimapSize.z = xmlLevel.attribute("minimapSize.z").as_float();

		delete[] levelData;
	}

	if (w.GameMapId == GBGameInfo::MAPID_ZP_Test) // colorado 
	{
		gfxFrontEnd.Invoke("_root.api.Main.SurvivorsAnim.refreshSurvivorsMap", "");
		StartAsyncOperation(&FrontEnd::as_RequestTeleportThread, &FrontEnd::OnRequestTeleportSuccess);
	}
}

void FrontEnd::eventReviveChar(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	Scaleform::GFx::Value var[2];
	var[0].SetString(gLangMngr.getString("OneMomentPlease"));
	var[1].SetBoolean(false);
	gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);

	ReviveCharSuccessfull = 0;

	StartAsyncOperation(&FrontEnd::as_ReviveCharThread, &FrontEnd::OnReviveCharSuccess);
}

void FrontEnd::eventReviveCharMoney(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	Scaleform::GFx::Value var[2];
	var[0].SetString(gLangMngr.getString("OneMomentPlease"));
	var[1].SetBoolean(false);
	gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);

	ReviveCharSuccessfull = 0;

	StartAsyncOperation(&FrontEnd::as_ReviveCharThread2, &FrontEnd::OnReviveCharSuccess);
}

void FrontEnd::eventBuyItem(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	mStore_BuyItemID = args[0].GetUInt(); // legsID
	mStore_BuyPrice = args[1].GetInt();
	mStore_BuyPriceGD = args[2].GetInt();
	mStore_BuyQuantity = args[3].GetInt(); // new quantity code
	mStore_BuyPrice *= mStore_BuyQuantity; // make price take into account the new quantity!
	mStore_BuyPriceGD *= mStore_BuyQuantity; // make price take into account the new quantity!

	if (mStore_BuyQuantity > 50)
	{
		Scaleform::GFx::Value var[2];
		var[0].SetString(gLangMngr.getString("MaxQuantityLimitToBuyItems"));
		var[1].SetBoolean(true);
		gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);
		return;
	}
	else if (mStore_BuyPriceGD)
	{
		if (gUserProfile.ProfileData.GameDollars < mStore_BuyPriceGD)
		{
			Scaleform::GFx::Value var[2];
			var[0].SetString(gLangMngr.getString("NotEnougMoneyToBuyItem"));
			var[1].SetBoolean(true);
			gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);
			return;
		}
	}
	else if (gUserProfile.ProfileData.GamePoints < mStore_BuyPrice)
	{
		Scaleform::GFx::Value var[2];
		var[0].SetString(gLangMngr.getString("NotEnougMoneyToBuyItem"));
		var[1].SetBoolean(true);
		gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);
		return;
	}

	const BaseItemConfig* itemConfig = g_pWeaponArmory->getAccountItemConfig(mStore_BuyItemID);
	if (itemConfig && itemConfig->category == storecat_LootBox)
	{
		if (gUserProfile.haveFreeInventorySlot(false) == false)
		{
			Scaleform::GFx::Value var[2];
			var[0].SetString(gLangMngr.getString("InGameUI_ErrorMsg_NoInventorySpace"));
			var[1].SetBoolean(true);
			gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);
			return;
		}


		Scaleform::GFx::Value var[2];
		var[0].SetString(gLangMngr.getString("OneMomentPlease"));
		var[1].SetBoolean(false);
		gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);


		StartAsyncOperation(&FrontEnd::as_BuyLootBoxThread, &FrontEnd::OnBuyLootBoxSuccess);
	}
	else
	{
		if (gUserProfile.haveFreeInventorySlot(false) == false)
		{
			Scaleform::GFx::Value var[2];
			var[0].SetString(gLangMngr.getString("InGameUI_ErrorMsg_NoInventorySpace"));
			var[1].SetBoolean(true);
			gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);
			return;
		}

		Scaleform::GFx::Value var[2];
		var[0].SetString(gLangMngr.getString("OneMomentPlease"));
		var[1].SetBoolean(false);
		gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);

		StartAsyncOperation(&FrontEnd::as_BuyItemThread, &FrontEnd::OnBuyItemSuccess);
	}
}

void FrontEnd::DelayServerRequest()
{
	// allow only one server request per second
	if (r3dGetTime() < lastServerReqTime_ + 1.0f) {
		::Sleep(1000);
	}
	lastServerReqTime_ = r3dGetTime();
}

bool FrontEnd::ConnectToMasterServer()
{
	masterConnectTime_ = r3dGetTime();
	if (gMasterServerLogic.badClientVersion_)
		return false;
	if (gMasterServerLogic.IsConnected())
		return true;

	const float endTime = r3dGetTime() + 30.0f;
	while (r3dGetTime() < endTime)
	{
		::Sleep(10);
		//if(gMasterServerLogic.IsConnected())
		//	return true;

		if (gMasterServerLogic.versionChecked_ && gMasterServerLogic.badClientVersion_)
			return false;

		// if we received server id, connection is ok.
		if (gMasterServerLogic.masterServerId_)
		{
			r3d_assert(gMasterServerLogic.versionChecked_);
			return true;
		}

		// early timeout by enet connect fail
		if (!gMasterServerLogic.net_->IsStillConnecting())
			break;
	}

	needReturnFromQuickJoin = true;
	SetAsyncError(0, gLangMngr.getString("TimeoutToMasterServer"));
	return false;
}

bool FrontEnd::WaitForGameJoinAnswer()
{
	const float endTime = r3dGetTime() + 60.0f;
	while (r3dGetTime() < endTime)
	{
		::Sleep(10);

		if (CancelQuickJoinRequest)
		{
			CancelQuickJoinRequest = false;
			break;
		}

		if (!gMasterServerLogic.IsConnected())
		{
			break;
		}

		if (gMasterServerLogic.gameJoinAnswered_)
			return true;
	}

	return false;
}

bool FrontEnd::ParseGameJoinAnswer()
{
	r3d_assert(gMasterServerLogic.gameJoinAnswered_);

	switch (gMasterServerLogic.gameJoinAnswer_.result)
	{
	case GBPKT_M2C_JoinGameAns_s::rOk:
		needExitByGameJoin_ = true;
		return true;
	case GBPKT_M2C_JoinGameAns_s::rMasterStarting:
		SetAsyncError(0, gLangMngr.getString("JoinMasterStarting"));
		return false;
	case GBPKT_M2C_JoinGameAns_s::rNoGames:
		SetAsyncError(0, gLangMngr.getString("JoinGameNoGames"));
		return false;
	case GBPKT_M2C_JoinGameAns_s::rGameFull:
		SetAsyncError(0, gLangMngr.getString("GameIsFull"));
		return false;
	case GBPKT_M2C_JoinGameAns_s::rGameFinished:
		SetAsyncError(0, gLangMngr.getString("GameIsAlmostFinished"));
		return false;
	case GBPKT_M2C_JoinGameAns_s::rGameNotFound:
		SetAsyncError(0, gLangMngr.getString("GameNotFound"));
		return false;
	case GBPKT_M2C_JoinGameAns_s::rWrongPassword:
		SetAsyncError(0, gLangMngr.getString("WrongPassword"));
		return false;
	case GBPKT_M2C_JoinGameAns_s::rLevelTooLow:
		SetAsyncError(0, gLangMngr.getString("GameTooLow"));
		return false;
	case GBPKT_M2C_JoinGameAns_s::rLevelTooHigh:
		SetAsyncError(0, gLangMngr.getString("GameTooHigh"));
		return false;
	case GBPKT_M2C_JoinGameAns_s::rJoinDelayActive:
		SetAsyncError(0, gLangMngr.getString("JoinDelayActive"));
		return false;
		//AlexRedd:: BR mode
	case GBPKT_M2C_JoinGameAns_s::rGameIsStarted:
		SetAsyncError(0, gLangMngr.getString("GameIsStarted"));
		return false;
	}

	char buf[128];
	sprintf(buf, gLangMngr.getString("UnableToJoinGameCode"), gMasterServerLogic.gameJoinAnswer_.result);
	SetAsyncError(0, buf);
	return  false;
}

unsigned int WINAPI FrontEnd::as_PlayGameThread(void* in_data)
{
	r3dThreadAutoInstallCrashHelper crashHelper;
	FrontEnd* This_ = (FrontEnd*)in_data;

	This_->DelayServerRequest();
	if (!This_->ConnectToMasterServer())
		return 0;

	NetPacketsMaster::GBPKT_C2M_QuickGameReq_s n;
	n.CustomerID = gUserProfile.CustomerID;
#ifndef FINAL_BUILD
	n.gameMap = d_use_test_map->GetInt();
#else
	n.gameMap = 0xFF;
#endif
	n.region = GBNET_REGION_US_West;
	n.browseChannel = (BYTE)This_->CurrentBrowseChannel;
	n.playerGameTime = gUserProfile.ProfileData.TimePlayed / 60 / 60; // convert to hours

	gMasterServerLogic.SendJoinQuickGame(n);

	if (This_->WaitForGameJoinAnswer() == false)
	{
		This_->SetAsyncError(0, gLangMngr.getString("TimeoutJoiningGame"));
		This_->needReturnFromQuickJoin = true;
		return 0;
	}

	if (This_->ParseGameJoinAnswer())
		return 1;

	This_->needReturnFromQuickJoin = true;
	return 0;
}

unsigned int WINAPI FrontEnd::as_JoinGameThread(void* in_data)
{
	r3dThreadAutoInstallCrashHelper crashHelper;
	FrontEnd* This_ = (FrontEnd*)in_data;

	This_->DelayServerRequest();
	if (!This_->ConnectToMasterServer())
		return 0;

	const float gameStartWaitTime = r3dGetTime() + 120.0f;	// allow 2 mins to start the game
	while (r3dGetTime() < gameStartWaitTime)
	{
		gMasterServerLogic.SendJoinGame(This_->m_joinGameServerId, This_->m_joinGamePwd);
		if (This_->WaitForGameJoinAnswer() == false)
			break;

		if (gMasterServerLogic.gameJoinAnswer_.result == GBPKT_M2C_JoinGameAns_s::rGameStarting)
		{
			// game is starting, send new request after some delay
			::Sleep(15000);
			continue;
		}

		return This_->ParseGameJoinAnswer() == true ? 1 : 0;
	}

	This_->SetAsyncError(0, gLangMngr.getString("TimeoutJoiningGame"));
	return 0;
}

unsigned int WINAPI FrontEnd::as_CreateCharThread(void* in_data)
{
	r3dThreadAutoInstallCrashHelper crashHelper;
	FrontEnd* This = (FrontEnd*)in_data;

	This->DelayServerRequest();

	int apiCode = gUserProfile.ApiCharCreate(This->m_CreateGamerTag, This->m_CreateGameMode, This->m_CreateHeroID, This->m_CreateHeadIdx, This->m_CreateBodyIdx, This->m_CreateLegsIdx);
	if (apiCode != 0)
	{
		if (apiCode == 9)
		{
			This->SetAsyncError(0, gLangMngr.getString("ThisNameIsAlreadyInUse"));
		}
		else if (apiCode == 7)
		{
			This->SetAsyncError(0, gLangMngr.getString("NameContainInvalidSymbols"));
		}
		else
		{
			This->SetAsyncError(apiCode, gLangMngr.getString("CreateCharacterFail"));
		}
		return 0;
	}

	return 1;
}

void FrontEnd::OnCreateCharSuccess()
{
	gfxFrontEnd.Invoke("_root.api.hideInfoMsg", "");

	Scaleform::GFx::Value var[20];

	int	i = gUserProfile.ProfileData.NumSlots - 1;
	{
		addClientSurvivor(gUserProfile.ProfileData.ArmorySlots[i], i);
	}

	var[0].SetInt(i);
	gfxFrontEnd.Invoke("_root.api.createCharSuccessful", var, 1);

	gUserProfile.SelectedCharID = i;
	m_Player->uberAnim_->anim.StopAll();	// prevent animation blending on loadout switch
	m_Player->UpdateLoadoutSlot(gUserProfile.ProfileData.ArmorySlots[gUserProfile.SelectedCharID], -1);
	return;
}

unsigned int WINAPI FrontEnd::as_DeleteCharThread(void* in_data)
{
	r3dThreadAutoInstallCrashHelper crashHelper;
	FrontEnd* This = (FrontEnd*)in_data;

	This->DelayServerRequest();

	int apiCode = gUserProfile.ApiCharDelete();
	if (apiCode != 0)
	{
		if (apiCode == 7)
			This->SetAsyncError(0, gLangMngr.getString("CannotDeleteCharThatIsClanLeader"));
		else
			This->SetAsyncError(apiCode, gLangMngr.getString("FailedToDeleteChar"));
		return 0;
	}

	return 1;
}

void FrontEnd::OnDeleteCharSuccess()
{
	gfxFrontEnd.Invoke("_root.api.hideInfoMsg", "");

	gfxFrontEnd.Invoke("_root.api.deleteCharSuccessful", "");

	gUserProfile.SelectedCharID = 0;
	m_Player->uberAnim_->anim.StopAll();	// prevent animation blending on loadout switch
	m_Player->UpdateLoadoutSlot(gUserProfile.ProfileData.ArmorySlots[gUserProfile.SelectedCharID], -1);
}

unsigned int WINAPI FrontEnd::as_ReviveCharThread(void* in_data)
{
	r3dThreadAutoInstallCrashHelper crashHelper;
	FrontEnd* This = (FrontEnd*)in_data;

	This->DelayServerRequest();

	int needMoney = 0;
	int secToRevive = 0;
	int apiCode = gUserProfile.ApiCharReviveCheck(&needMoney, &secToRevive);
	if (apiCode != 0)
	{
		This->SetAsyncError(apiCode, gLangMngr.getString("FailedToReviveChar"));
		return 0;
	}

	if (needMoney > 0)
	{
		This->ReviveCharSuccessfull = 2;
	}
	else
	{
		apiCode = gUserProfile.ApiCharRevive(needMoney);
		if (apiCode != 0)
		{
			This->SetAsyncError(apiCode, gLangMngr.getString("FailedToReviveChar"));
			return 0;
		}
		else
			This->ReviveCharSuccessfull = 1;
	}

	return 1;
}

unsigned int WINAPI FrontEnd::as_ReviveCharThread2(void* in_data)
{
	r3dThreadAutoInstallCrashHelper crashHelper;
	FrontEnd* This = (FrontEnd*)in_data;

	This->DelayServerRequest();

	int needMoney = 0;
	int secToRevive = 0;
	int apiCode = gUserProfile.ApiCharReviveCheck(&needMoney, &secToRevive);
	if (apiCode != 0)
	{
		This->SetAsyncError(apiCode, gLangMngr.getString("FailedToReviveChar"));
		return 0;
	}

	if (gUserProfile.ShopCharReviveGC > 0)
	{
		if (gUserProfile.ProfileData.GamePoints < gUserProfile.ShopCharReviveGC)
		{
			This->SetAsyncError(0, gLangMngr.getString("NotEnoughGC"));
			return 0;
		}
	}

	if (gUserProfile.ShopCharReviveGD > 0)
	{
		if (gUserProfile.ProfileData.GameDollars < gUserProfile.ShopCharReviveGD)
		{
			This->SetAsyncError(0, gLangMngr.getString("NotEnoughGD"));
			return 0;
		}
	}

	// this function called when user aknowledged that he is okay with paying money for revive
	apiCode = gUserProfile.ApiCharRevive(needMoney);
	if (apiCode != 0)
	{
		This->SetAsyncError(apiCode, gLangMngr.getString("FailedToReviveChar"));
		return 0;
	}
	else
		This->ReviveCharSuccessfull = 1;

	return 1;
}

void FrontEnd::OnReviveCharSuccess()
{
	if (ReviveCharSuccessfull == 1)
	{
		gfxFrontEnd.Invoke("_root.api.hideInfoMsg", "");

		// sync what server does. after revive you are allowed to access global inventory
		gUserProfile.ProfileData.ArmorySlots[gUserProfile.SelectedCharID].GameFlags |= wiCharDataFull::GAMEFLAG_NearPostBox;

		gfxFrontEnd.Invoke("_root.api.reviveCharSuccessful", "");
		Scaleform::GFx::Value var[1];
		var[0].SetInt(gUserProfile.ProfileData.GamePoints);
		gfxFrontEnd.Invoke("_root.api.setGC", var, 1);
		var[0].SetInt(gUserProfile.ProfileData.GameDollars);
		gfxFrontEnd.Invoke("_root.api.setDollars", var, 1);
	}
	else if (ReviveCharSuccessfull == 2)
	{
		gfxFrontEnd.Invoke("_root.api.hideInfoMsg", "");
		gfxFrontEnd.Invoke("_root.api.Main.PopUpEarlyRevival.showPopUp", "");
	}
	ReviveCharSuccessfull = 0;
	return;
}

void FrontEnd::initItems()
{
	addItemCategoriesToUI(gfxFrontEnd);

	// add items
	{
		addItemsAndCategoryToUI(gfxFrontEnd);

	}

	updateInventoryAndSkillItems();
	addStore();
}

void FrontEnd::updateInventoryAndSkillItems()
{
	reloadInventoryInfo(gfxFrontEnd);
}

static void addAllItemsToStore()
{
	// reset store and add all items from DB
	g_NumStoreItems = 0;

#define SET_STOREITEM \
		memset(&st, 0, sizeof(st)); \
		st.itemID = item->m_itemID;\
		st.pricePerm = 60000;\
		st.gd_pricePerm = 0;

	std::vector<const ModelItemConfig*> allItems;
	std::vector<const WeaponConfig*> allWpns;
	std::vector<const GearConfig*> allGear;

	g_pWeaponArmory->startItemSearch();
	while (g_pWeaponArmory->searchNextItem())
	{
		uint32_t itemID = g_pWeaponArmory->getCurrentSearchItemID();
		const WeaponConfig* weaponConfig = g_pWeaponArmory->getWeaponConfig(itemID);
		if (weaponConfig)
		{
			allWpns.push_back(weaponConfig);
		}

		const ModelItemConfig* itemConfig = g_pWeaponArmory->getAccountItemConfig(itemID);
		if (itemConfig)
		{
			allItems.push_back(itemConfig);
		}

		const GearConfig* gearConfig = g_pWeaponArmory->getGearConfig(itemID);
		if (gearConfig)
		{
			allGear.push_back(gearConfig);
		}
	}

	const size_t itemSize = allItems.size();
	const size_t weaponSize = allWpns.size();
	const size_t gearSize = allGear.size();

	for (size_t i = 0; i < itemSize; ++i)
	{
		const ModelItemConfig* item = allItems[i];
		wiStoreItem& st = g_StoreItems[g_NumStoreItems++];
		SET_STOREITEM;
	}

	for (size_t i = 0; i < weaponSize; ++i)
	{
		const WeaponConfig* item = allWpns[i];
		wiStoreItem& st = g_StoreItems[g_NumStoreItems++];
		SET_STOREITEM;
	}

	for (size_t i = 0; i < gearSize; ++i)
	{
		const GearConfig* item = allGear[i];
		wiStoreItem& st = g_StoreItems[g_NumStoreItems++];
		SET_STOREITEM;
	}

#undef SET_STOREITEM
}

void FrontEnd::addStore()
{
#if 0
	// add all items to store for test purpose
	addAllItemsToStore();
#endif	

	addStoreToUI(gfxFrontEnd);
	cMarketplaceScreen = eMarketplaceScreen::Market;
}

unsigned int WINAPI  FrontEnd::as_BuyLootBoxThread(void* in_data)
{
	r3dThreadAutoInstallCrashHelper crashHelper;
	FrontEnd* This = (FrontEnd*)in_data;

	This->DelayServerRequest();

	int buyIdx = This->StoreDetectBuyIdx();
	if (buyIdx == 0)
	{
		This->SetAsyncError(-1, gLangMngr.getString("BuyItemFailNoIndex"));
		return 0;
	}

	int apiCode = gUserProfile.ApiLootBoxBuy(This->mStore_BuyItemID, buyIdx, This->mStore_BuyQuantity);
	if (apiCode != 0)
	{
		This->SetAsyncError(apiCode, gLangMngr.getString("BuyItemFail"));
		return 0;
	}
	return 1;
}

void  FrontEnd::OnBuyLootBoxSuccess()
{
	if (gUserProfile.ProfileData.lastWinID > 0)
	{
		char Pic[256] = { 0 };
		char itemname[128] = { 0 };
		char title[64] = { 0 };

		const GearConfig* gc = g_pWeaponArmory->getGearConfig(gUserProfile.ProfileData.lastWinID);
		const BaseItemConfig* ic = g_pWeaponArmory->getWeaponConfig(gUserProfile.ProfileData.lastWinID);
		const WeaponAttachmentConfig* atmCfg = g_pWeaponArmory->getAttachmentConfig(gUserProfile.ProfileData.lastWinID);

		if (gc || ic || atmCfg)
		{
			sprintf(title, "$FR_Congratulations");
			SoundSys.PlayAndForget(SoundSys.GetEventIDByPath("Sounds/UI Events/Misc/UI_Menu_Mystery_Box_Unlocking"), r3dPoint3D(0, 0, 0));
		}
		else
		{
			sprintf(title, "$FR_Oops");
			SoundSys.PlayAndForget(SoundSys.GetEventIDByPath("Sounds/UI Events/Misc/UI_Menu_Mystery_Box_Reveal"), r3dPoint3D(0, 0, 0));
		}

		if (gc)
		{
			sprintf(Pic, gc->m_StoreIcon);
			sprintf(itemname, gc->m_StoreName);
		}
		else if (ic)
		{
			sprintf(Pic, ic->m_StoreIcon);
			sprintf(itemname, ic->m_StoreName);
		}
		else if (atmCfg)
		{
			sprintf(Pic, atmCfg->m_StoreIcon);
			sprintf(itemname, atmCfg->m_StoreName);
		}
		else
		{
			sprintf(Pic, "$Data/Weapons/StoreIcons/Oops.dds");
			switch (u_random(3))
			{
			case 0:
				sprintf(itemname, "$FR_YouFoundNothing");
				break;
			case 1:
				sprintf(itemname, "$FR_YouFoundNothing1");
				break;
			case 2:
				sprintf(itemname, "$FR_YouFoundNothing2");
				break;
			}
		}

		//setCurrency();
		gUserProfile.GetProfile();
		Scaleform::GFx::Value var3[1];

		var3[0].SetInt(gUserProfile.ProfileData.GamePoints);
		gfxFrontEnd.Invoke("_root.api.setGC", var3, 1);

		var3[0].SetInt(gUserProfile.ProfileData.GameDollars);
		gfxFrontEnd.Invoke("_root.api.setDollars", var3, 1);

		var3[0].SetInt(gUserProfile.ProfileData.XP50Boosts);
		gfxFrontEnd.Invoke("_root.api.setXPBoosts", var3, 1);

		Scaleform::GFx::Value var2[7];
		// clear inventory DB
		gfxFrontEnd.Invoke("_root.api.clearInventory", NULL, 0);

		// add all items
		for (uint32_t i = 0; i < gUserProfile.ProfileData.NumItems; ++i)
		{
			var2[0].SetUInt(uint32_t(gUserProfile.ProfileData.Inventory[i].InventoryID));
			var2[1].SetUInt(gUserProfile.ProfileData.Inventory[i].itemID);
			var2[2].SetNumber(gUserProfile.ProfileData.Inventory[i].quantity);
			var2[3].SetNumber(gUserProfile.ProfileData.Inventory[i].Var1);
			var2[4].SetNumber(gUserProfile.ProfileData.Inventory[i].Var2);
			bool isConsumable = false;
			{
				const WeaponConfig* wc = g_pWeaponArmory->getWeaponConfig(gUserProfile.ProfileData.Inventory[i].itemID);
				if (wc && wc->category == storecat_UsableItem && wc->m_isConsumable)
					isConsumable = true;
			}
			var2[5].SetBoolean(isConsumable);
			var2[6].SetString(getAdditionalDescForItem(gUserProfile.ProfileData.Inventory[i].itemID, gUserProfile.ProfileData.Inventory[i].Var1, gUserProfile.ProfileData.Inventory[i].Var2, gUserProfile.ProfileData.Inventory[i].Var3));
			gfxFrontEnd.Invoke("_root.api.addInventoryItem", var2, 7);
		}

		gfxFrontEnd.Invoke("_root.api.buyItemSuccessful", "");
		gfxFrontEnd.Invoke("_root.api.hideInfoMsg", "");

		Scaleform::GFx::Value var[4];
		var[0].SetString(Pic);
		var[1].SetString(itemname);
		var[2].SetBoolean(true);
		var[3].SetString(title);
		gfxFrontEnd.Invoke("_root.api.showInfoMsgCase", var, 4);
	}
	else
	{
		gfxFrontEnd.Invoke("_root.api.buyItemSuccessful", "");
		gfxFrontEnd.Invoke("_root.api.hideInfoMsg", "");


		Scaleform::GFx::Value var[2];
		var[0].SetString("Failed to open case");
		var[1].SetBoolean(true);
		gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);
	}

	mStore_BuyItemID = 0;
	return;
}

unsigned int WINAPI FrontEnd::as_BuyItemThread(void* in_data)
{
	r3dThreadAutoInstallCrashHelper crashHelper;
	FrontEnd* This = (FrontEnd*)in_data;

	This->DelayServerRequest();

	int buyIdx = This->StoreDetectBuyIdx();
	if (buyIdx == 0)
	{
		This->SetAsyncError(-1, gLangMngr.getString("BuyItemFailNoIndex"));
		return 0;
	}

	int apiCode = gUserProfile.ApiBuyItem(This->mStore_BuyItemID, buyIdx, &This->m_inventoryID, This->mStore_BuyQuantity);

	if (apiCode == 5)
	{		
		This->SetAsyncError(0, gLangMngr.getString("InGameUI_ErrorMsg_LotteryLimit"));
		return 0;
	}

	if (apiCode == 8)
	{
		gUserProfile.GetProfile();
		int minutesLeft = gUserProfile.ProfileData.PremiumAcc;
		char buf[1024];
		// temp sring. maybe create a valid date and show it
		int daysLeft = minutesLeft / 1440;
		int hoursLeft = minutesLeft / 60;
		sprintf(buf, gLangMngr.getString("PremiumTimeError"), (daysLeft > 0) ? (daysLeft > 0) ? (daysLeft) : (hoursLeft) : (hoursLeft > 0) ? (hoursLeft) : (minutesLeft),
			(daysLeft > 0) ? (daysLeft > 0) ? gLangMngr.getString("$FR_PAUSE_INVENTORY_DAYS") : gLangMngr.getString("$FR_PAUSE_INVENTORY_HR") :
			(hoursLeft > 0) ? gLangMngr.getString("$FR_PAUSE_INVENTORY_HR") : gLangMngr.getString("$FR_PAUSE_INVENTORY_MIN"));
		This->SetAsyncError(0, buf);
		return 0;
	}
	if (apiCode == 9)
	{
		gUserProfile.GetProfile();
		int minutesLeft = gUserProfile.ProfileData.XP50Boosts;
		char buf[1024];
		// temp sring. maybe create a valid date and show it
		int daysLeft = minutesLeft / 1440;
		int hoursLeft = minutesLeft / 60;
		sprintf(buf, gLangMngr.getString("XPBoostsTimeError"), (daysLeft > 0) ? (daysLeft > 0) ? (daysLeft) : (hoursLeft) : (hoursLeft > 0) ? (hoursLeft) : (minutesLeft),
			(daysLeft > 0) ? (daysLeft > 0) ? gLangMngr.getString("$FR_PAUSE_INVENTORY_DAYS") : gLangMngr.getString("$FR_PAUSE_INVENTORY_HR") :
			(hoursLeft > 0) ? gLangMngr.getString("$FR_PAUSE_INVENTORY_HR") : gLangMngr.getString("$FR_PAUSE_INVENTORY_MIN"));
		This->SetAsyncError(0, buf);
		return 0;
	}

	if (apiCode != 0)
	{
		This->SetAsyncError(apiCode, gLangMngr.getString("BuyItemFail"));
		return 0;
	}

	return 1;
}

void FrontEnd::OnBuyItemSuccess()
{
	gUserProfile.GetProfile();

	updateInventoryAndSkillItems();

	Scaleform::GFx::Value var[1];
	var[0].SetInt(gUserProfile.ProfileData.GamePoints);
	gfxFrontEnd.Invoke("_root.api.setGC", var, 1);

	var[0].SetInt(gUserProfile.ProfileData.GameDollars);
	gfxFrontEnd.Invoke("_root.api.setDollars", var, 1);

	var[0].SetInt(gUserProfile.ProfileData.XP50Boosts);
	gfxFrontEnd.Invoke("_root.api.setXPBoosts", var, 1);

	gfxFrontEnd.Invoke("_root.api.buyItemSuccessful", "");

	gfxFrontEnd.Invoke("_root.api.hideInfoMsg", "");
}

int FrontEnd::StoreDetectBuyIdx()
{
	return getStoreBuyIdxUI(mStore_BuyPriceGD, mStore_BuyPrice);
}

void FrontEnd::eventBackpackFromInventory(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	const wiCharDataFull& slot = gUserProfile.ProfileData.ArmorySlots[gUserProfile.SelectedCharID];
	//if(!(slot.GameFlags & wiCharDataFull::GAMEFLAG_NearPostBox))
	//return;

	m_inventoryID = args[0].GetUInt();
	m_gridTo = args[1].GetInt();
	m_Amount = args[2].GetInt();

	if (m_gridTo == 0)
	{
		wiCharDataFull& slot = gUserProfile.ProfileData.ArmorySlots[gUserProfile.SelectedCharID];

		slot.Attachment[0].attachments[WPN_ATTM_MUZZLE] = 0;
		slot.Attachment[0].attachments[WPN_ATTM_UPPER_RAIL] = 0;
		slot.Attachment[0].attachments[WPN_ATTM_LEFT_RAIL] = 0;
		slot.Attachment[0].attachments[WPN_ATTM_BOTTOM_RAIL] = 0;
		slot.Attachment[0].attachments[WPN_ATTM_CLIP] = 0;
		m_Player->UpdateLoadoutSlot(gUserProfile.ProfileData.ArmorySlots[gUserProfile.SelectedCharID], -1);
	}

	const WeaponAttachmentConfig* wac = g_pWeaponArmory->getAttachmentConfig(slot.Items[m_gridTo].itemID);

	if (wac)
	{
		wiCharDataFull& slot1 = gUserProfile.ProfileData.ArmorySlots[gUserProfile.SelectedCharID];

		int SlotWeapon1ID = slot1.Items[wiCharDataFull::CHAR_LOADOUT_WEAPON1].itemID;
		int SlotWeapon2ID = slot1.Items[wiCharDataFull::CHAR_LOADOUT_WEAPON2].itemID;

		int Quantity = 0;

		for (int a = 0; a < slot.BackpackSize; a++)
		{
			if (slot1.Items[a].itemID == slot.Items[m_gridFrom].itemID)
			{
				Quantity += slot1.Items[a].quantity;
			}
		}

		Quantity--;


		if (slot1.Attachment[wiCharDataFull::CHAR_LOADOUT_WEAPON1].attachments[wac->m_type] == slot.Items[m_gridFrom].itemID && Quantity <= 0)
			slot1.Attachment[wiCharDataFull::CHAR_LOADOUT_WEAPON1].attachments[wac->m_type] = 0;

		if (slot1.Attachment[wiCharDataFull::CHAR_LOADOUT_WEAPON2].attachments[wac->m_type] == slot.Items[m_gridFrom].itemID && Quantity <= 0)
			slot1.Attachment[wiCharDataFull::CHAR_LOADOUT_WEAPON2].attachments[wac->m_type] = 0;

		int apiCode = gUserProfile.ApiUpdateAttachment();

		if (apiCode != 0)
		{
			if (apiCode == 7)
				SetAsyncError(0, gLangMngr.getString("GameSessionHasNotClosedYet"));
			else if (apiCode == 9)
				SetAsyncError(0, gLangMngr.getString("InGameUI_ErrorMsg_NoInventorySpace"));
			else
				SetAsyncError(apiCode, gLangMngr.getString("BackpackToInventoryFail"));
			return;
		}
	}

	Scaleform::GFx::Value var[2];
	var[0].SetString(gLangMngr.getString("OneMomentPlease"));
	var[1].SetBoolean(false);
	gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);

	uint32_t itemID = 0;
	for (uint32_t i = 0; i < gUserProfile.ProfileData.NumItems; ++i)
	{
		if (gUserProfile.ProfileData.Inventory[i].InventoryID == m_inventoryID)
		{
			itemID = gUserProfile.ProfileData.Inventory[i].itemID;
			break;
		}
	}

	// check to see if there is anything in backpack, and if there is, then we need to firstly move that item to inventory
	if (m_gridTo != -1 && slot.Items[m_gridTo].itemID != 0 && slot.Items[m_gridTo].itemID != itemID)
	{
		m_gridFrom = m_gridTo;
		m_Amount2 = slot.Items[m_gridTo].quantity;

		// check weight
		float totalWeight = slot.getTotalWeight();
		const BaseItemConfig* bic = g_pWeaponArmory->getConfig(slot.Items[m_gridTo].itemID);
		if (bic)
			totalWeight -= bic->m_Weight*slot.Items[m_gridTo].quantity;

		bic = g_pWeaponArmory->getConfig(itemID);
		if (bic)
		{
			if (m_gridTo != 0 && m_gridTo != 1 && m_gridTo != 6 && m_gridTo != 7)
				totalWeight += bic->m_Weight*m_Amount;

			if (slot.Skills[CUserSkills::SKILL_Physical3])
				totalWeight *= 0.95f;
			if (slot.Skills[CUserSkills::SKILL_Physical7])
				totalWeight *= 0.9f;
		}

		const BackpackConfig* bc = g_pWeaponArmory->getBackpackConfig(slot.BackpackID);
		r3d_assert(bc);
		if (totalWeight > bc->m_maxWeight)
		{
			Scaleform::GFx::Value var[3];
			var[0].SetString(gLangMngr.getString("FR_PAUSE_TOO_MUCH_WEIGHT"));
			var[1].SetBoolean(true);
			var[2].SetString("");
			gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 3);
			return;
		}

		StartAsyncOperation(&FrontEnd::as_BackpackFromInventorySwapThread, &FrontEnd::OnBackpackFromInventorySuccess);
	}
	else
	{
		// check weight
		float totalWeight = slot.getTotalWeight();

		const BaseItemConfig* bic = g_pWeaponArmory->getConfig(itemID);
		if (bic)
		{
			if (m_gridTo != 0 && m_gridTo != 1 && m_gridTo != 6 && m_gridTo != 7)
				totalWeight += bic->m_Weight*m_Amount;

			if (slot.Skills[CUserSkills::SKILL_Physical3])
				totalWeight *= 0.95f;
			if (slot.Skills[CUserSkills::SKILL_Physical7])
				totalWeight *= 0.9f;
		}

		const BackpackConfig* bc = g_pWeaponArmory->getBackpackConfig(slot.BackpackID);
		r3d_assert(bc);
		if (totalWeight > bc->m_maxWeight)
		{
			Scaleform::GFx::Value var[2];
			var[0].SetString(gLangMngr.getString("FR_PAUSE_TOO_MUCH_WEIGHT"));
			var[1].SetBoolean(true);
			gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);
			return;
		}

		StartAsyncOperation(&FrontEnd::as_BackpackFromInventoryThread, &FrontEnd::OnBackpackFromInventorySuccess);
	}
}

void FrontEnd::as_LOADOUT_BackpackFromInventoryThread(int currentThread, int ItemID, int slotID, int Quantity)
{
	as_LOADOUT_current_running_thread++;

	int apiCode = -1;
	__int64	m_inventoryID = 0;
	switch (currentThread)
	{
	case 1:
	{
		// first move all items to GI
		int count_of_tries = 10;
		do
		{
			apiCode = gUserProfile.ApiBackpackToInventory(slotID, Quantity);

			if (apiCode == 6) // retry if something failed - fixed bad slot bug
			{
				r3dOutToLog("!!!! Loadout Change Thread failed, retrying... %i\n", count_of_tries);
				count_of_tries--;
				continue;
			}

			break;

		} while (count_of_tries > 0);
	}
	break;

	case 2:
	{
		for (uint32_t j = 0; j < gUserProfile.ProfileData.NumItems; ++j)
		{
			if (gUserProfile.ProfileData.Inventory[j].InventoryID != -1 && gUserProfile.ProfileData.Inventory[j].itemID == ItemID)
			{
				m_inventoryID = gUserProfile.ProfileData.Inventory[j].InventoryID;
				break;
			}
		}

		if (m_inventoryID > 0)
		{
			int apiCode = gUserProfile.ApiChangeBackpack(m_inventoryID);
		}
	}
	break;

	case 3:
	{
		for (uint32_t j = 0; j < gUserProfile.ProfileData.NumItems; ++j)
		{
			if (gUserProfile.ProfileData.Inventory[j].InventoryID != -1 && gUserProfile.ProfileData.Inventory[j].itemID == ItemID)
			{
				m_inventoryID = gUserProfile.ProfileData.Inventory[j].InventoryID;
				break;
			}
		}

		if (m_inventoryID > 0)
		{
			apiCode = gUserProfile.ApiBackpackFromInventory(m_inventoryID, slotID, Quantity);
		}
	}
	break;
	}

	as_LOADOUT_current_running_thread--;
}

unsigned int WINAPI FrontEnd::as_LOADOUT_BackpackFromInventoryThread_MAIN(void* in_data)
{
	r3dThreadAutoInstallCrashHelper crashHelper;
	FrontEnd* This = (FrontEnd*)in_data;

	This->as_LOADOUT_current_running_thread = 0;

	// first move all items to GI
	const wiCharDataFull& slot = gUserProfile.ProfileData.ArmorySlots[gUserProfile.SelectedCharID];
	for (int i = 0; i < slot.BackpackSize; ++i)
	{
		const wiInventoryItem& wi = slot.Items[i];
		if (wi.itemID <= 0)
			continue;

		bool move_set = false;
		int to_move_quantity = 1;
		if (slot.Items[i].itemID == This->loadout_.item[i].ItemID)
		{
			to_move_quantity = slot.Items[i].quantity;
			if (to_move_quantity <= 0)
				to_move_quantity = 1;

			to_move_quantity -= This->loadout_.item[i].Quantity;
			if (to_move_quantity <= 0)
				continue;

			move_set = true;
		}

		std::thread loadout_change(&FrontEnd::as_LOADOUT_BackpackFromInventoryThread, This, 1, This->loadout_.item[i].ItemID, i, (move_set ? to_move_quantity : wi.quantity));
		loadout_change.join();
	}

	while (This->as_LOADOUT_current_running_thread > 0)
		continue;

	// check equiped backpack
	if (slot.BackpackID != This->loadout_.BackpackID)
	{
		std::thread loadout_change(&FrontEnd::as_LOADOUT_BackpackFromInventoryThread, This, 2, This->loadout_.BackpackID, -1, -1);
		loadout_change.join();
	}

	while (This->as_LOADOUT_current_running_thread > 0)
		continue;

	// Inventory items to backpack
	for (int i = 0; i < This->loadout_.Backpacksize; i++)
	{
		if (This->loadout_.item[i].ItemID <= 0)
			continue;

		if (i >= slot.BackpackSize)
			break;

		int	to_move_quantity = 1;
		if (slot.Items[i].itemID == This->loadout_.item[i].ItemID)
		{
			to_move_quantity = This->loadout_.item[i].Quantity;
			if (to_move_quantity <= 0)
				to_move_quantity = 1;

			to_move_quantity -= slot.Items[i].quantity;
			if (to_move_quantity <= 0)
				continue;
		}
		else
		{
			for (uint32_t j = 0; j < gUserProfile.ProfileData.NumItems; ++j)
			{
				if (gUserProfile.ProfileData.Inventory[j].InventoryID != -1 && gUserProfile.ProfileData.Inventory[j].itemID == This->loadout_.item[i].ItemID)
				{
					to_move_quantity = This->loadout_.item[i].Quantity;
					break;
				}
			}
		}

		std::thread loadout_change(&FrontEnd::as_LOADOUT_BackpackFromInventoryThread, This, 3, This->loadout_.item[i].ItemID, This->loadout_.item[i].SlotID, to_move_quantity);
		loadout_change.join();
	}

	while (This->as_LOADOUT_current_running_thread > 0)
		continue;

	return 1;
}

void FrontEnd::OnLOADOUT_BackpackFromInventorySuccess()
{
	Scaleform::GFx::Value var[8];
	gfxFrontEnd.Invoke("_root.api.clearBackpack", "");
	int	slot = gUserProfile.SelectedCharID;

	updateClientSurvivor(gUserProfile.ProfileData.ArmorySlots[slot]);

	addBackpackItems(gUserProfile.ProfileData.ArmorySlots[slot], slot);

	updateInventoryAndSkillItems();

	updateSurvivorTotalWeight(slot);

	gfxFrontEnd.Invoke("_root.api.hideInfoMsg", "");
	gfxFrontEnd.Invoke("_root.api.changeBackpackSuccess", "");
	gfxFrontEnd.Invoke("_root.api.backpackFromInventorySuccess", "");
}


unsigned int WINAPI FrontEnd::as_BackpackFromInventorySwapThread(void* in_data)
{
	r3dThreadAutoInstallCrashHelper crashHelper;
	FrontEnd* This = (FrontEnd*)in_data;

	This->DelayServerRequest();

	// move item in backpack to inventory
	int apiCode = gUserProfile.ApiBackpackToInventory(This->m_gridFrom, This->m_Amount2);
	if (apiCode != 0)
	{
		if (apiCode == 7)
			This->SetAsyncError(0, gLangMngr.getString("GameSessionHasNotClosedYet"));
		else if (apiCode == 8)
			This->SetAsyncError(0, gLangMngr.getString("InGameUI_ErrorMsg_GlobalInventoryBanned"));
		else if (apiCode == 9)
			This->SetAsyncError(0, gLangMngr.getString("InGameUI_ErrorMsg_NoInventorySpace"));
		else
			This->SetAsyncError(apiCode, gLangMngr.getString("BackpackFromInventoryFail"));
		return 0;
	}

	apiCode = gUserProfile.ApiBackpackFromInventory(This->m_inventoryID, This->m_gridTo, This->m_Amount);
	if (apiCode != 0)
	{
		if (apiCode == 7)
			This->SetAsyncError(0, gLangMngr.getString("GameSessionHasNotClosedYet"));
		else
			This->SetAsyncError(apiCode, gLangMngr.getString("BackpackFromInventoryFail"));
		return 0;
	}

	return 1;
}


unsigned int WINAPI FrontEnd::as_BackpackFromInventoryThread(void* in_data)
{
	r3dThreadAutoInstallCrashHelper crashHelper;
	FrontEnd* This = (FrontEnd*)in_data;

	This->DelayServerRequest();

	int apiCode = gUserProfile.ApiBackpackFromInventory(This->m_inventoryID, This->m_gridTo, This->m_Amount);
	if (apiCode != 0)
	{
		if (apiCode == 7)
			This->SetAsyncError(0, gLangMngr.getString("GameSessionHasNotClosedYet"));
		else if (apiCode == 8)
			This->SetAsyncError(0, gLangMngr.getString("InGameUI_ErrorMsg_GlobalInventoryBanned"));
		else if (apiCode == 9)//AlexRedd:: no drop backpack
			This->SetAsyncError(0, gLangMngr.getString("InGameUI_ErrorMsg_Forbidden"));
		else
			This->SetAsyncError(apiCode, gLangMngr.getString("BackpackFromInventoryFail"));
		return 0;
	}

	return 1;
}

void FrontEnd::OnBackpackFromInventorySuccess()
{
	Scaleform::GFx::Value var[8];
	gfxFrontEnd.Invoke("_root.api.clearBackpack", "");
	int	slot = gUserProfile.SelectedCharID;

	addBackpackItems(gUserProfile.ProfileData.ArmorySlots[slot], slot);

	updateInventoryAndSkillItems();

	updateSurvivorTotalWeight(slot);

	gfxFrontEnd.Invoke("_root.api.hideInfoMsg", "");
	gfxFrontEnd.Invoke("_root.api.backpackFromInventorySuccess", "");
}

void FrontEnd::updateSurvivorTotalWeight(int survivor)
{
	wiCharDataFull& slot = gUserProfile.ProfileData.ArmorySlots[survivor];
	float totalWeight = slot.getTotalWeight();

	if (slot.Skills[CUserSkills::SKILL_Physical3])
		totalWeight *= 0.95f;
	if (slot.Skills[CUserSkills::SKILL_Physical7])
		totalWeight *= 0.9f;

	Scaleform::GFx::Value var[2];
	char tmpGamertag[128];
	if (slot.ClanID != 0)
		sprintf(tmpGamertag, "[%s] %s", slot.ClanTag, slot.Gamertag);
	else
		r3dscpy(tmpGamertag, slot.Gamertag);

	var[0].SetString(tmpGamertag);
	var[1].SetNumber(totalWeight);
	gfxFrontEnd.Invoke("_root.api.updateClientSurvivorWeight", var, 2);
}

void FrontEnd::eventBackpackToInventory(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	const wiCharDataFull& slot = gUserProfile.ProfileData.ArmorySlots[gUserProfile.SelectedCharID];
	//if(!(slot.GameFlags & wiCharDataFull::GAMEFLAG_NearPostBox))
	//return;

	m_gridFrom = args[0].GetInt();
	m_Amount = args[1].GetInt();

	const WeaponAttachmentConfig* wac = g_pWeaponArmory->getAttachmentConfig(slot.Items[m_gridFrom].itemID);

	if (wac)
	{
		wiCharDataFull& slot1 = gUserProfile.ProfileData.ArmorySlots[gUserProfile.SelectedCharID];

		int SlotWeapon1ID = slot1.Items[wiCharDataFull::CHAR_LOADOUT_WEAPON1].itemID;
		int SlotWeapon2ID = slot1.Items[wiCharDataFull::CHAR_LOADOUT_WEAPON2].itemID;

		int Quantity = 0;

		for (int a = 0; a < slot.BackpackSize; a++)
		{
			if (slot1.Items[a].itemID == slot.Items[m_gridFrom].itemID)
			{
				Quantity += slot1.Items[a].quantity;
			}
		}

		Quantity--;

		//	r3dOutToLog("############ Quantity %i %i \n",Quantity,slot.Items[m_gridFrom].itemID);

		if (slot1.Attachment[wiCharDataFull::CHAR_LOADOUT_WEAPON1].attachments[wac->m_type] == slot.Items[m_gridFrom].itemID && Quantity <= 0)
			slot1.Attachment[wiCharDataFull::CHAR_LOADOUT_WEAPON1].attachments[wac->m_type] = 0;

		if (slot1.Attachment[wiCharDataFull::CHAR_LOADOUT_WEAPON2].attachments[wac->m_type] == slot.Items[m_gridFrom].itemID && Quantity <= 0)
			slot1.Attachment[wiCharDataFull::CHAR_LOADOUT_WEAPON2].attachments[wac->m_type] = 0;

		int apiCode = gUserProfile.ApiUpdateAttachment();

		if (apiCode != 0)
		{
			if (apiCode == 7)
				SetAsyncError(0, gLangMngr.getString("GameSessionHasNotClosedYet"));
			else if (apiCode == 9)
				SetAsyncError(0, gLangMngr.getString("InGameUI_ErrorMsg_NoInventorySpace"));
			else
				SetAsyncError(apiCode, gLangMngr.getString("BackpackToInventoryFail"));
			return;
		}
	}

	Scaleform::GFx::Value var[2];
	var[0].SetString(gLangMngr.getString("OneMomentPlease"));
	var[1].SetBoolean(false);
	gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);

	StartAsyncOperation(&FrontEnd::as_BackpackToInventoryThread, &FrontEnd::OnBackpackToInventorySuccess);
}

unsigned int WINAPI FrontEnd::as_BackpackToInventoryThread(void* in_data)
{
	r3dThreadAutoInstallCrashHelper crashHelper;
	FrontEnd* This = (FrontEnd*)in_data;

	This->DelayServerRequest();

	int apiCode = gUserProfile.ApiBackpackToInventory(This->m_gridFrom, This->m_Amount);
	if (apiCode != 0)
	{
		if (apiCode == 7)
			This->SetAsyncError(0, gLangMngr.getString("GameSessionHasNotClosedYet"));
		else if (apiCode == 9)
			This->SetAsyncError(0, gLangMngr.getString("InGameUI_ErrorMsg_NoInventorySpace"));
		else
			This->SetAsyncError(apiCode, gLangMngr.getString("BackpackToInventoryFail"));
		return 0;
	}

	return 1;
}

void FrontEnd::OnBackpackToInventorySuccess()
{
	Scaleform::GFx::Value var[8];
	gfxFrontEnd.Invoke("_root.api.clearBackpack", "");
	int	slot = gUserProfile.SelectedCharID;

	addBackpackItems(gUserProfile.ProfileData.ArmorySlots[slot], slot);

	updateInventoryAndSkillItems();

	updateSurvivorTotalWeight(slot);

	gfxFrontEnd.Invoke("_root.api.hideInfoMsg", "");
	gfxFrontEnd.Invoke("_root.api.backpackToInventorySuccess", "");


	return;
}

void FrontEnd::eventBackpackGridSwap(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	m_gridFrom = args[0].GetInt();
	m_gridTo = args[1].GetInt();

	Scaleform::GFx::Value var[2];
	var[0].SetString(gLangMngr.getString("OneMomentPlease"));
	var[1].SetBoolean(false);
	gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);

	if (m_gridFrom == 0 || m_gridTo == 0)
	{
		wiCharDataFull& slot = gUserProfile.ProfileData.ArmorySlots[gUserProfile.SelectedCharID];

		slot.Attachment[0].attachments[WPN_ATTM_MUZZLE] = 0;
		slot.Attachment[0].attachments[WPN_ATTM_UPPER_RAIL] = 0;
		slot.Attachment[0].attachments[WPN_ATTM_LEFT_RAIL] = 0;
		slot.Attachment[0].attachments[WPN_ATTM_BOTTOM_RAIL] = 0;
		slot.Attachment[0].attachments[WPN_ATTM_CLIP] = 0;
		m_Player->UpdateLoadoutSlot(gUserProfile.ProfileData.ArmorySlots[gUserProfile.SelectedCharID], -1);
	}

	StartAsyncOperation(&FrontEnd::as_BackpackGridSwapThread, &FrontEnd::OnBackpackGridSwapSuccess);
}

void FrontEnd::eventSetSelectedChar(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	gUserProfile.SelectedCharID = args[0].GetInt();
	m_Player->uberAnim_->anim.StopAll();	// prevent animation blending on loadout switch
	m_Player->UpdateLoadoutSlot(gUserProfile.ProfileData.ArmorySlots[gUserProfile.SelectedCharID], -1);
}

void FrontEnd::eventOpenBackpackSelector(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount == 0);
	Scaleform::GFx::Value var[2];

	// clear
	gfxFrontEnd.Invoke("_root.api.clearBackpacks", "");

	std::vector<uint32_t> uniqueBackpacks; // to filter identical backpack

	int backpackSlotIDInc = 0;
	// add backpack content info
	{
		wiCharDataFull& slot = gUserProfile.ProfileData.ArmorySlots[gUserProfile.SelectedCharID];

		for (int a = 0; a < slot.BackpackSize; a++)
		{
			if (slot.Items[a].itemID != 0)
			{
				if (std::find<std::vector<uint32_t>::iterator, uint32_t>(uniqueBackpacks.begin(), uniqueBackpacks.end(), slot.Items[a].itemID) != uniqueBackpacks.end())
					continue;

				const BackpackConfig* bpc = g_pWeaponArmory->getBackpackConfig(slot.Items[a].itemID);
				if (bpc)
				{
					// add backpack info
					var[0].SetInt(backpackSlotIDInc++);
					var[1].SetUInt(slot.Items[a].itemID);
					gfxFrontEnd.Invoke("_root.api.addBackpack", var, 2);

					uniqueBackpacks.push_back(slot.Items[a].itemID);
				}
			}
		}
	}
	// add inventory info
	for (uint32_t i = 0; i < gUserProfile.ProfileData.NumItems; ++i)
	{
		if (std::find<std::vector<uint32_t>::iterator, uint32_t>(uniqueBackpacks.begin(), uniqueBackpacks.end(), gUserProfile.ProfileData.Inventory[i].itemID) != uniqueBackpacks.end())
			continue;

		const BackpackConfig* bpc = g_pWeaponArmory->getBackpackConfig(gUserProfile.ProfileData.Inventory[i].itemID);
		if (bpc)
		{
			// add backpack info
			var[0].SetInt(backpackSlotIDInc++);
			var[1].SetUInt(gUserProfile.ProfileData.Inventory[i].itemID);
			gfxFrontEnd.Invoke("_root.api.addBackpack", var, 2);

			uniqueBackpacks.push_back(gUserProfile.ProfileData.Inventory[i].itemID);
		}
	}

	gfxFrontEnd.Invoke("_root.api.showChangeBackpack", "");
}

void FrontEnd::eventChangeBackpack(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount == 2);
	uint32_t itemID = args[1].GetUInt();

	__int64 inventoryID = 0;
	wiCharDataFull& slot = gUserProfile.ProfileData.ArmorySlots[gUserProfile.SelectedCharID];
	if (slot.BackpackID == itemID) // same backpack, skip
	{
		Scaleform::GFx::Value vars[2];
		vars[0].SetString(gLangMngr.getString("SwitchBackpackSameBackpacks"));
		vars[1].SetBoolean(true);
		gfxFrontEnd.Invoke("_root.api.showInfoMsg", vars, 2);
		return;
	}

	// find inventory id with that itemID
	for (int a = 0; a < slot.BackpackSize; a++)
	{
		if (slot.Items[a].itemID == itemID)
		{
			inventoryID = slot.Items[a].InventoryID;
			break;
		}
	}
	if (inventoryID == 0)
	{
		for (uint32_t i = 0; i < gUserProfile.ProfileData.NumItems; ++i)
		{
			if (gUserProfile.ProfileData.Inventory[i].itemID == itemID)
			{
				inventoryID = gUserProfile.ProfileData.Inventory[i].InventoryID;
				break;
			}
		}
	}

	if (inventoryID == 0)
	{
		Scaleform::GFx::Value vars[3];
		vars[0].SetString(gLangMngr.getString("FailedToFindBackpack"));
		vars[1].SetBoolean(true);
		vars[2].SetString("$ERROR");
		gfxFrontEnd.Invoke("_root.api.showInfoMsg", vars, 3);
		return;
	}

	// check if user has enough space in GI to put extra items if any
	{
		const BackpackConfig* bpc_current = g_pWeaponArmory->getBackpackConfig(slot.BackpackID);
		const BackpackConfig* bpc_new = g_pWeaponArmory->getBackpackConfig(itemID);
		r3d_assert(bpc_current);
		r3d_assert(bpc_new);
		if (bpc_new->m_maxSlots < bpc_current->m_maxSlots)
		{
			int extraItems = 0;
			for (int i = bpc_new->m_maxSlots; i < slot.BackpackSize; ++i)
			{
				if (slot.Items[i].itemID > 0)
					extraItems++;
			}

			if (extraItems)
			{
				if ((gUserProfile.ProfileData.NumItems + extraItems) > wiUserProfile::INVENTORY_SIZE_LIMIT)
				{
					Scaleform::GFx::Value vars[3];
					vars[0].SetString(gLangMngr.getString("InGameUI_ErrorMsg_NoInventorySpace"));
					vars[1].SetBoolean(true);
					vars[2].SetString("$ERROR");
					gfxFrontEnd.Invoke("_root.api.showInfoMsg", vars, 3);
					return;
				}
			}
		}
	}

	Scaleform::GFx::Value var[2];
	var[0].SetString(gLangMngr.getString("OneMomentPlease"));
	var[1].SetBoolean(false);
	gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);

	mChangeBackpack_inventoryID = inventoryID;
	StartAsyncOperation(&FrontEnd::as_BackpackChangeThread, &FrontEnd::OnBackpackChangeSuccess);
}

unsigned int WINAPI FrontEnd::as_BackpackChangeThread(void* in_data)
{
	r3dThreadAutoInstallCrashHelper crashHelper;
	FrontEnd* This = (FrontEnd*)in_data;

	This->DelayServerRequest();

	int apiCode = gUserProfile.ApiChangeBackpack(This->mChangeBackpack_inventoryID);
	if (apiCode != 0)
	{
		if (apiCode == 8)
			This->SetAsyncError(0, gLangMngr.getString("InGameUI_ErrorMsg_GlobalInventoryBanned"));
		else
			This->SetAsyncError(apiCode, "Backpack change failed");
		return 0;
	}

	return 1;
}

void FrontEnd::updateClientSurvivor(const wiCharDataFull& slot)
{
	Scaleform::GFx::Value var[11];
	char tmpGamertag[128];
	if (slot.ClanID != 0)
		sprintf(tmpGamertag, "[%s] %s", slot.ClanTag, slot.Gamertag);
	else
		r3dscpy(tmpGamertag, slot.Gamertag);
	var[0].SetString(tmpGamertag);
	var[1].SetNumber(slot.Health);
	var[2].SetNumber(slot.Stats.XP);
	var[3].SetNumber(slot.Stats.TimePlayed);
	var[4].SetNumber(slot.Alive);
	var[5].SetNumber(slot.Hunger);
	var[6].SetNumber(slot.Thirst);
	var[7].SetNumber(slot.Toxic);
	var[8].SetNumber(slot.BackpackID);
	var[9].SetNumber(slot.BackpackSize);
	var[10].SetInt(slot.Stats.XP - slot.Stats.SpendXP);
	gfxFrontEnd.Invoke("_root.api.updateClientSurvivor", var, 11);
}

void FrontEnd::OnBackpackChangeSuccess()
{
	gfxFrontEnd.Invoke("_root.api.hideInfoMsg", "");

	wiCharDataFull& slot = gUserProfile.ProfileData.ArmorySlots[gUserProfile.SelectedCharID];

	updateClientSurvivor(slot);

	addBackpackItems(slot, gUserProfile.SelectedCharID);
	updateInventoryAndSkillItems();

	updateSurvivorTotalWeight(gUserProfile.SelectedCharID);

	gfxFrontEnd.Invoke("_root.api.changeBackpackSuccess", "");
}

void FrontEnd::eventCreateChangeCharacter(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	PlayerCreateLoadout.HeroItemID = args[0].GetInt();
	PlayerCreateLoadout.HeadIdx = args[1].GetInt();
	PlayerCreateLoadout.BodyIdx = args[2].GetInt();
	PlayerCreateLoadout.LegsIdx = args[3].GetInt();

	m_Player->uberAnim_->anim.StopAll();	// prevent animation blending on loadout switch
	m_Player->UpdateLoadoutSlot(PlayerCreateLoadout, -1);
}

void FrontEnd::eventCreateCancel(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	m_Player->uberAnim_->anim.StopAll();	// prevent animation blending on loadout switch
	m_Player->UpdateLoadoutSlot(gUserProfile.ProfileData.ArmorySlots[gUserProfile.SelectedCharID], -1);
}

void FrontEnd::eventRequestPlayerRender(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount == 1);
	m_needPlayerRenderingRequest = args[0].GetInt();
}

unsigned int WINAPI FrontEnd::as_BackpackGridSwapThread(void* in_data)
{
	r3dThreadAutoInstallCrashHelper crashHelper;
	FrontEnd* This = (FrontEnd*)in_data;

	This->DelayServerRequest();

	int apiCode = gUserProfile.ApiBackpackGridSwap(This->m_gridFrom, This->m_gridTo);
	if (apiCode != 0)
	{
		if (apiCode == 7)
			This->SetAsyncError(0, gLangMngr.getString("GameSessionHasNotClosedYet"));
		else
			This->SetAsyncError(apiCode, gLangMngr.getString("BackpackToBackpackFail"));
		return 0;
	}

	return 1;
}

void FrontEnd::OnBackpackGridSwapSuccess()
{
	Scaleform::GFx::Value var[8];

	gfxFrontEnd.Invoke("_root.api.clearBackpack", "");
	int	slot = gUserProfile.SelectedCharID;

	addBackpackItems(gUserProfile.ProfileData.ArmorySlots[slot], slot);

	updateSurvivorTotalWeight(slot);

	gfxFrontEnd.Invoke("_root.api.hideInfoMsg", "");
	gfxFrontEnd.Invoke("_root.api.backpackGridSwapSuccess", "");
	return;
}

void FrontEnd::eventOptionsLanguageSelection(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(args);
	r3d_assert(argCount == 1);

	const char* newLang = args[0].GetString();

	if (strcmp(newLang, g_user_language->GetString()) == 0)
		return; // same language

	/*
	#ifdef FINAL_BUILD
	{
	Scaleform::GFx::Value var[2];
	var[0].SetString(L"LOCALIZATIONS ARE COMING SOON");
	var[1].SetBoolean(true);
	pMovie->Invoke("_root.api.showInfoMsg", var, 2);
	return;
	}
	#endif
	*/

	g_user_language->SetString(newLang);

	ReloadLocalization();

	gfxFrontEnd.Invoke("_root.api.setLanguage", g_user_language->GetString());

	// write to ini file
	writeGameOptionsFile();
}

void FrontEnd::AddSettingsChangeFlag(DWORD flag)
{
	settingsChangeFlags_ |= flag;
}

static int compRes(const void* r1, const void* r2)
{
	const r3dDisplayResolution* rr1 = (const r3dDisplayResolution*)r1;
	const r3dDisplayResolution* rr2 = (const r3dDisplayResolution*)r2;

	return rr1->Width - rr2->Width; // sort resolutions by width
}

void FrontEnd::SyncGraphicsUI()
{
	const DisplayResolutions& reses = r3dRenderer->GetDisplayResolutions();

	DisplayResolutions supportedReses;

	for (uint32_t i = 0, e = reses.Count(); i < e; i++)
	{
		const r3dDisplayResolution& r = reses[i];
		float aspect = (float)r.Width / r.Height;
		supportedReses.PushBack(r);
	}

	if (supportedReses.Count() == 0)
		r3dError("Couldn't find any supported video resolutions. Bad video driver?!\n");

	qsort(&supportedReses[0], supportedReses.Count(), sizeof supportedReses[0], compRes);

	gfxFrontEnd.Invoke("_root.api.clearScreenResolutions", "");
	for (uint32_t i = 0, e = supportedReses.Count(); i < e; i++)
	{
		char resString[128] = { 0 };
		const r3dDisplayResolution& r = supportedReses[i];
		_snprintf(resString, sizeof resString - 1, "%d x %d", r.Width, r.Height);
		gfxFrontEnd.Invoke("_root.api.addScreenResolution", resString);
	}

	int width = r_width->GetInt();
	int height = r_height->GetInt();

	int desktopWidth, desktopHeight;
	r3dGetDesktopDimmensions(&desktopWidth, &desktopHeight);

	if (!r_ini_read->GetBool())
	{
		if (desktopWidth < width || desktopHeight < height)
		{
			width = desktopWidth;
			height = desktopHeight;
		}
	}

	bool finalResSet = false;
	int selectedRes = 0;
	for (uint32_t i = 0, e = supportedReses.Count(); i < e; i++)
	{
		const r3dDisplayResolution& r = supportedReses[i];
		if (width == r.Width && height == r.Height)
		{
			selectedRes = i;
			finalResSet = true;
			break;
		}
	}

	if (!finalResSet)
	{
		int bestSum = 0;

		for (uint32_t i = 0, e = supportedReses.Count(); i < e; i++)
		{
			const r3dDisplayResolution& r = supportedReses[i];

			if (width >= r.Width &&
				height >= r.Height)
			{
				if (r.Width + r.Height > bestSum)
				{
					selectedRes = i;
					bestSum = r.Width + r.Height;
					finalResSet = true;
				}
			}
		}
	}

	if (!finalResSet)
	{
		int bestSum = 0x7fffffff;

		// required mode too small, find smallest mode..
		for (uint32_t i = 0, e = supportedReses.Count(); i < e; i++)
		{
			const r3dDisplayResolution& r = supportedReses[i];

			if (r.Width + r.Height < bestSum)
			{
				finalResSet = true;

				selectedRes = i;
				bestSum = r.Width + r.Height;
			}
		}
	}

	Scaleform::GFx::Value var[30];
	var[0].SetNumber(selectedRes);
	var[1].SetNumber(r_overall_quality->GetInt());
	var[2].SetNumber(((r_gamma_pow->GetFloat() - 2.2f) + 1.0f) / 2.0f);
	var[3].SetNumber(0.0f);
	var[4].SetNumber(s_sound_volume->GetFloat());
	var[5].SetNumber(s_music_volume->GetFloat());
	var[6].SetNumber(0.0f);
	var[7].SetNumber(g_tps_camera_mode->GetInt());
	var[8].SetNumber(g_enable_voice_commands->GetBool());
	var[9].SetNumber(r_antialiasing_quality->GetInt());
	var[10].SetNumber(r_ssao_quality->GetInt());
	var[11].SetNumber(r_terrain_quality->GetInt());
	var[12].SetNumber(r_environment_quality->GetInt());
	var[13].SetNumber(0);
	var[14].SetNumber(r_shadows_quality->GetInt() + 1);
	var[15].SetNumber(r_lighting_quality->GetInt());
	var[16].SetNumber(0);
	var[17].SetNumber(r_mesh_quality->GetInt());
	var[18].SetNumber(r_anisotropy_quality->GetInt());
	var[19].SetNumber(r_postprocess_quality->GetInt());
	var[20].SetNumber(r_texture_quality->GetInt());
	var[21].SetNumber(g_vertical_look->GetBool());
	var[22].SetNumber(0); // not used
	var[23].SetNumber(g_mouse_wheel->GetBool());
	var[24].SetNumber(g_mouse_sensitivity->GetFloat());
	var[25].SetNumber(g_mouse_acceleration->GetBool());
	var[26].SetNumber(g_toggle_aim->GetBool());
	var[27].SetNumber(g_toggle_crouch->GetBool());
	var[28].SetNumber(r_ScreenMode->GetInt());
	var[29].SetNumber(r_vsync_enabled->GetInt() + 1);

	gfxFrontEnd.Invoke("_root.api.setOptions", var, 30);

	{
		var[0].SetBoolean(g_voip_enable->GetBool());
		var[1].SetBoolean(g_voip_showChatBubble->GetBool());
		var[2].SetNumber(g_voip_InputDeviceInd->GetInt());
		var[3].SetNumber(g_voip_OutputDeviceInd->GetInt());
		var[4].SetNumber(g_voip_volume->GetFloat());
		gfxFrontEnd.Invoke("_root.api.setVoipOptions", var, 5);
	}

	{	//AlexRedd:: Extra options
		var[0].SetNumber(g_chat_message_sound->GetInt() + 1);
		var[1].SetNumber(g_crosshaire_mode->GetInt() + 1);
		var[2].SetNumber(g_highlight_mode->GetInt() + 1);
		var[3].SetNumber(g_increase_fps->GetInt() + 1);
		var[4].SetNumber(g_disable_grass->GetInt() + 1);
		var[5].SetNumber(g_disable_jump_sound->GetInt() + 1);
		var[6].SetNumber(g_enable_alpha_sounds->GetInt() + 1);
		var[7].SetNumber(g_enable_killfeed_messages->GetInt() + 1);
		var[8].SetNumber(g_enable_killstreak_sounds->GetInt() + 1);
		var[9].SetNumber(g_killstreak_sounds->GetInt() + 1);
		var[10].SetNumber(g_crosshaire_color->GetInt() + 1);
		var[11].SetNumber(g_FastLoad->GetInt() + 1);
		var[12].SetNumber(r_highlight_att_clr->GetInt() + 1);
		var[13].SetNumber(r_highlight_grenade_clr->GetInt() + 1);
		var[14].SetNumber(r_highlight_wpn_clr->GetInt() + 1);
		var[15].SetNumber(r_highlight_melee_clr->GetInt() + 1);
		var[16].SetNumber(r_highlight_food_clr->GetInt() + 1);
		var[17].SetNumber(r_highlight_gear_clr->GetInt() + 1);
		var[18].SetNumber(r_highlight_bp_clr->GetInt() + 1);
		var[19].SetNumber(r_highlight_ui_clr->GetInt() + 1);
		var[20].SetNumber(r_highlight_other_clr->GetInt() + 1);
		gfxFrontEnd.Invoke("_root.api.setExtraOptions", var, 21);
	}

	gfxFrontEnd.Invoke("_root.api.reloadOptions", "");
}

void FrontEnd::eventOptionsReset(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(args);
	r3d_assert(argCount == 0);

	// get options
	g_tps_camera_mode->SetInt(0);
	g_enable_voice_commands->SetBool(true);

	int old_screenmode = r_ScreenMode->GetInt();
	r_ScreenMode->SetInt(0);

	int old_vsync = r_vsync_enabled->GetInt();
	r_vsync_enabled->SetInt(0);

	//AlexRedd:: Extra options
	g_chat_message_sound->SetInt(0);
	g_crosshaire_mode->SetInt(0);
	g_highlight_mode->SetInt(0);
	g_increase_fps->SetBool(false);
	g_disable_grass->SetBool(true);
	g_disable_jump_sound->SetBool(true);
	g_enable_alpha_sounds->SetBool(false);
	g_enable_killfeed_messages->SetBool(true);
	g_enable_killstreak_sounds->SetBool(true);
	g_killstreak_sounds->SetBool(false);
	g_crosshaire_color->SetInt(0);
	g_FastLoad->SetBool(true);
	r_highlight_att_clr->SetInt(0);
	r_highlight_grenade_clr->SetInt(0);
	r_highlight_wpn_clr->SetInt(0);
	r_highlight_melee_clr->SetInt(0);
	r_highlight_food_clr->SetInt(0);
	r_highlight_gear_clr->SetInt(0);
	r_highlight_bp_clr->SetInt(0);
	r_highlight_ui_clr->SetInt(0);
	r_highlight_other_clr->SetInt(0);

	switch (r3dGetDeviceStrength())
	{
	case S_WEAK:
		r_overall_quality->SetInt(1);
		break;
	case S_MEDIUM:
		r_overall_quality->SetInt(2);
		break;
	case S_STRONG:
		r_overall_quality->SetInt(3);
		break;
	case S_ULTRA:
		r_overall_quality->SetInt(4);
		break;
	default:
		r_overall_quality->SetInt(1);
		break;
	}

	DWORD settingsChangedFlags = 0;
	GraphicSettings settings;

	switch (r_overall_quality->GetInt())
	{
	case 1:
		FillDefaultSettings(settings, S_WEAK);
		settingsChangedFlags = SetDefaultSettings(S_WEAK);
		break;
	case 2:
		FillDefaultSettings(settings, S_MEDIUM);
		settingsChangedFlags = SetDefaultSettings(S_MEDIUM);
		break;
	case 3:
		FillDefaultSettings(settings, S_STRONG);
		settingsChangedFlags = SetDefaultSettings(S_STRONG);
		break;
	case 4:
		FillDefaultSettings(settings, S_ULTRA);
		settingsChangedFlags = SetDefaultSettings(S_ULTRA);
		break;
	case 5:
	{
		settings.mesh_quality = (int)args[17].GetNumber();
		settings.texture_quality = (int)args[20].GetNumber();
		settings.terrain_quality = (int)args[11].GetNumber();
		settings.shadows_quality = (int)args[14].GetNumber() - 1;
		settings.lighting_quality = (int)args[15].GetNumber();
		settings.environment_quality = (int)args[12].GetNumber();
		settings.anisotropy_quality = (int)args[18].GetNumber();
		settings.postprocess_quality = (int)args[19].GetNumber();
		SaveCustomSettings(settings);
	}
	break;
	default:
		r3d_assert(false);
	}

	// AA\SSAO is separate and can be changed at any overall quality level
	settings.antialiasing_quality = 0;
	settings.ssao_quality = 0;
	settingsChangedFlags |= GraphSettingsToVars(settings);
	AddSettingsChangeFlag(settingsChangedFlags);

	// clamp brightness and contrast, otherwise if user set it to 0 the screen will be white
	//r_brightness			->SetFloat(0.5f);
	//r_contrast				->SetFloat(0.5f);
	r_gamma_pow->SetFloat(2.2f);

	s_sound_volume->SetFloat(1.0f);
	s_music_volume->SetFloat(1.0f);
	//s_comm_volume			->SetFloat(1.0f);

	SetNeedUpdateSettings();

	// write to ini file
	writeGameOptionsFile();
	SyncGraphicsUI();

	if (old_screenmode != r_ScreenMode->GetInt() || old_vsync != r_vsync_enabled->GetInt())
		SetNeedUpdateMode();
}

void FrontEnd::eventOptionsApply(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(args);
	r3d_assert(argCount == 23);

	// get options
	g_tps_camera_mode->SetInt((int)args[7].GetNumber());
	g_enable_voice_commands->SetBool(!!(int)args[8].GetNumber());

	const char* res = args[0].GetString();
	int width = 1280, height = 720;

	for (; *res < '0' || *res > '9';) res++;
	int n = sscanf(res, "%d", &width);
	r3d_assert(n == 1);
	for (; *res >= '0' && * res <= '9';) res++;
	for (; *res < '0' || *res > '9';) res++;
	n = sscanf(res, "%d", &height);
	r3d_assert(n == 1);

	int old_width = r_width->GetInt();
	r_width->SetInt(width);
	int old_height = r_height->GetInt();
	r_height->SetInt(height);

	int old_screenmode = r_ScreenMode->GetInt();
	r_ScreenMode->SetInt((int)args[21].GetNumber());

	int old_vsync = r_vsync_enabled->GetInt();
	r_vsync_enabled->SetInt((int)args[22].GetNumber() - 1);

	r_overall_quality->SetInt((int)args[1].GetNumber());

	DWORD settingsChangedFlags = 0;
	GraphicSettings settings;

	switch (r_overall_quality->GetInt())
	{
	case 1:
		FillDefaultSettings(settings, S_WEAK);
		settingsChangedFlags = SetDefaultSettings(S_WEAK);
		break;
	case 2:
		FillDefaultSettings(settings, S_MEDIUM);
		settingsChangedFlags = SetDefaultSettings(S_MEDIUM);
		break;
	case 3:
		FillDefaultSettings(settings, S_STRONG);
		settingsChangedFlags = SetDefaultSettings(S_STRONG);
		break;
	case 4:
		FillDefaultSettings(settings, S_ULTRA);
		settingsChangedFlags = SetDefaultSettings(S_ULTRA);
		break;
	case 5:
	{
		settings.mesh_quality = (int)args[17].GetNumber();
		settings.texture_quality = (int)args[20].GetNumber();
		settings.terrain_quality = (int)args[11].GetNumber();
		settings.shadows_quality = (int)args[14].GetNumber() - 1;
		settings.lighting_quality = (int)args[15].GetNumber();
		settings.environment_quality = (int)args[12].GetNumber();
		settings.anisotropy_quality = (int)args[18].GetNumber();
		settings.postprocess_quality = (int)args[19].GetNumber();
		SaveCustomSettings(settings);
	}
	break;
	default:
		r3d_assert(false);
	}

	// AA\SSAO is separate and can be changed at any overall quality level
	settings.antialiasing_quality = (int)args[9].GetNumber();
	settings.ssao_quality = (int)args[10].GetNumber();

	settingsChangedFlags |= GraphSettingsToVars(settings);

	AddSettingsChangeFlag(settingsChangedFlags);

	r_gamma_pow->SetFloat(2.2f + (float(args[2].GetNumber())*2.0f - 1.0f));

	s_sound_volume->SetFloat(R3D_CLAMP((float)args[4].GetNumber(), 0.0f, 1.0f));
	s_music_volume->SetFloat(R3D_CLAMP((float)args[5].GetNumber(), 0.0f, 1.0f));

	SetNeedUpdateSettings();
	SyncGraphicsUI();

	// write to ini file
	writeGameOptionsFile();

	if (old_screenmode != r_ScreenMode->GetInt() || old_width != r_width->GetInt() || old_height != r_height->GetInt() || old_vsync != r_vsync_enabled->GetInt())
		SetNeedUpdateMode();
}

void boostfps()
{
	r_grass_anim_speed->SetBool(false);
	r_grass_ssao->SetBool(false);
	r_multithreading->SetBool(false); // or we fuck our multithreading
	r_bloom->SetBool(false);
	r_shadows->SetBool(false);
	r_shadow_blur->SetBool(false);
	r_allow_hw_shadows->SetBool(true);
	r_optimize_shadow_map->SetBool(true);
	r_half_res_particles->SetBool(true);
	r_split_grass_render->SetBool(true);
	r_ssao->SetBool(false);
	r_vsync_enabled->SetBool(false);
	r_particle_shadows->SetBool(false);
	r_double_depth_ssao->SetBool(true);
	r_optimize_meshes->SetBool(true);
	r_hardware_shadow_method->SetBool(true);
	r_cw_shadows->SetBool(true);
	r_optimized_ssao->SetBool(false);

	//g_disable_grass->SetInt(false);//disable grass	
}

void FrontEnd::eventExtraOptionsApply(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(args);
	r3d_assert(argCount == 21);

	bool old_increase = g_increase_fps->GetBool();
	bool old_disablegrass = g_disable_grass->GetBool();

	//AlexRedd:: Extra options
	g_chat_message_sound->SetInt((int)args[0].GetNumber() - 1);
	g_crosshaire_mode->SetInt((int)args[1].GetNumber() - 1);
	g_highlight_mode->SetInt((int)args[2].GetNumber() - 1);
	g_increase_fps->SetInt((int)args[3].GetNumber() - 1);
	g_disable_grass->SetInt((int)args[4].GetNumber() - 1);
	g_disable_jump_sound->SetInt((int)args[5].GetNumber() - 1);
	g_enable_alpha_sounds->SetInt((int)args[6].GetNumber() - 1);
	g_enable_killfeed_messages->SetInt((int)args[7].GetNumber() - 1);
	g_enable_killstreak_sounds->SetInt((int)args[8].GetNumber() - 1);
	g_killstreak_sounds->SetInt((int)args[9].GetNumber() - 1);
	g_crosshaire_color->SetInt((int)args[10].GetNumber() - 1);
	g_FastLoad->SetInt((int)args[11].GetNumber() - 1);
	r_highlight_att_clr->SetInt((int)args[12].GetNumber() - 1);
	r_highlight_grenade_clr->SetInt((int)args[13].GetNumber() - 1);
	r_highlight_wpn_clr->SetInt((int)args[14].GetNumber() - 1);
	r_highlight_melee_clr->SetInt((int)args[15].GetNumber() - 1);
	r_highlight_food_clr->SetInt((int)args[16].GetNumber() - 1);
	r_highlight_gear_clr->SetInt((int)args[17].GetNumber() - 1);
	r_highlight_bp_clr->SetInt((int)args[18].GetNumber() - 1);
	r_highlight_ui_clr->SetInt((int)args[19].GetNumber() - 1);
	r_highlight_other_clr->SetInt((int)args[20].GetNumber() - 1);

	if (old_increase != g_increase_fps->GetBool() || old_disablegrass != g_disable_grass->GetBool())
	{
		// show message telling player that to change windows\fullscreen he have to restart game		
		Scaleform::GFx::Value var[2];
		var[0].SetString(gLangMngr.getString("RestartGameForChangesToTakeEffect"));
		var[1].SetBoolean(true);
		gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);
	}

	if (g_increase_fps->GetBool())
		boostfps();

	SetNeedUpdateSettings();
	SyncGraphicsUI();

	// write to ini file
	writeGameOptionsFile();
}

void FrontEnd::SetNeedUpdateSettings()
{
	needUpdateSettings_ = true;
}

void FrontEnd::SetNeedUpdateMode()
{
	needUpdateMode_ = true;
}

void FrontEnd::UpdateSettings()
{
	Mouse->SetRange(r3dRenderer->HLibWin);

	void applyGraphicsOptions(uint32_t settingsFlags);

	applyGraphicsOptions(settingsChangeFlags_);

	gfxFrontEnd.UpdateTextureMatrices("merc_rendertarget", (int)Scaleform_RenderToTextureRT->Width, (int)Scaleform_RenderToTextureRT->Height);
}

void FrontEnd::eventOptionsControlsRequestKeyRemap(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(args);
	r3d_assert(argCount == 1);

	int remapIndex = (int)args[0].GetNumber();
	r3d_assert(m_waitingForKeyRemap == -1);

	r3d_assert(remapIndex >= 0 && remapIndex < r3dInputMappingMngr::KS_NUM);
	m_waitingForKeyRemap = remapIndex;
}

void FrontEnd::eventOptionsControlsReset(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(args);
	r3d_assert(argCount == 0);

	//	InputMappingMngr->resetKeyMappingsToDefault();
	for (int i = 0; i < r3dInputMappingMngr::KS_NUM; ++i)
	{
		Scaleform::GFx::Value args[2];
		args[0].SetString(gLangMngr.getString(InputMappingMngr->getMapName((r3dInputMappingMngr::KeybordShortcuts)i)));
		args[1].SetString(InputMappingMngr->getKeyName((r3dInputMappingMngr::KeybordShortcuts)i));
		gfxFrontEnd.Invoke("_root.api.setKeyboardMapping", args, 2);
	}
	void writeInputMap();
	writeInputMap();

	// update those to match defaults in Vars.h
	g_vertical_look->SetBool(false);
	g_mouse_wheel->SetBool(true);
	g_mouse_sensitivity->SetFloat(1.0f);
	g_mouse_acceleration->SetBool(false);
	g_toggle_aim->SetBool(false);
	g_toggle_crouch->SetBool(false);

	// write to ini file
	writeGameOptionsFile();
	SyncGraphicsUI();
}

void FrontEnd::eventOptionsControlsApply(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(args);
	r3d_assert(argCount == 7);

	g_vertical_look->SetBool(!!(int)args[0].GetNumber());
	g_mouse_wheel->SetBool(!!(int)args[2].GetNumber());
	g_mouse_sensitivity->SetFloat((float)args[3].GetNumber());
	g_mouse_acceleration->SetBool(!!(int)args[4].GetNumber());
	g_toggle_aim->SetBool(!!(int)args[5].GetNumber());
	g_toggle_crouch->SetBool(!!(int)args[6].GetNumber());

	// write to ini file
	writeGameOptionsFile();

	SyncGraphicsUI();
}

void FrontEnd::eventOptionsVoipApply(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(args);
	r3d_assert(argCount == 5);

	g_voip_enable->SetBool(!!(int)args[0].GetNumber());
	g_voip_showChatBubble->SetBool(!!(int)args[1].GetNumber());
	g_voip_InputDeviceInd->SetInt((int)args[2].GetNumber());
	g_voip_OutputDeviceInd->SetInt((int)args[3].GetNumber());
	g_voip_volume->SetFloat((float)args[4].GetNumber());

	// write to ini file
	writeGameOptionsFile();

	SyncGraphicsUI();
}

void FrontEnd::eventOptionsVoipReset(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(args);
	r3d_assert(argCount == 0);

	// update those to match defaults in Vars.h
	g_voip_enable->SetBool(true);
	g_voip_showChatBubble->SetBool(true);
	g_voip_InputDeviceInd->SetInt(-1);
	g_voip_OutputDeviceInd->SetInt(-1);
	g_voip_volume->SetFloat(1.0f);

	initVoipOptions();

	// write to ini file
	writeGameOptionsFile();
	SyncGraphicsUI();
}

void FrontEnd::eventMsgBoxCallback(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	if (loginMsgBoxOK_Exit)
	{
		loginMsgBoxOK_Exit = false;
		LoginMenuExitFlag = -1;
	}
	if (needReturnFromQuickJoin)
	{
		gfxFrontEnd.Invoke("_root.api.Main.showScreen", "PlayGame");
		needReturnFromQuickJoin = false;
	}
}

void FrontEnd::eventBrowseGamesRequestFilterStatus(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(args);
	r3d_assert(argCount == 0);

	// setBrowseGamesOptions(regus:Boolean, regeu:Boolean, regru:Boolean, regsa:Boolean, filt_gw:Boolean, filt_sh:Boolean, filt_empt:Boolean, filt_full:Boolean, opt_trac:Boolean, opt_nm:Boolean, opt_ch:Boolean, opt_enable:Boolean)
	Scaleform::GFx::Value var[14];
	var[0].SetBoolean(gUserSettings.BrowseGames_Filter.region_us);
	var[1].SetBoolean(gUserSettings.BrowseGames_Filter.region_eu);
	var[2].SetBoolean(gUserSettings.BrowseGames_Filter.region_ru);
	var[3].SetBoolean(gUserSettings.BrowseGames_Filter.region_sa);
	var[4].SetBoolean(false);
	var[5].SetBoolean(false);
	var[6].SetBoolean(gUserSettings.BrowseGames_Filter.hideempty);
	var[7].SetBoolean(gUserSettings.BrowseGames_Filter.hidefull);
	var[8].SetBoolean(false);
	var[9].SetBoolean(gUserSettings.BrowseGames_Filter.tracers2);
	var[10].SetBoolean(gUserSettings.BrowseGames_Filter.nameplates2);
	var[11].SetBoolean(gUserSettings.BrowseGames_Filter.crosshair2);
	var[12].SetBoolean(gUserSettings.BrowseGames_Filter.enable_options);
	var[13].SetBoolean(gUserSettings.BrowseGames_Filter.password);
	gfxFrontEnd.Invoke("_root.api.setBrowseGamesOptions", var, 14);
}

void FrontEnd::eventBrowseGamesSetFilter(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	//regus:Boolean, regeu:Boolean, regru:Boolean, regsa:Boolean, filt_gw:Boolean, filt_sh:Boolean, filt_empt:Boolean, filt_full:Boolean, opt_trac:Boolean, opt_nm:Boolean, opt_ch:Boolean, timeLimit:uint
	r3d_assert(args);
	r3d_assert(argCount == 16);
	gUserSettings.BrowseGames_Filter.region_us = args[0].GetBool();
	gUserSettings.BrowseGames_Filter.region_eu = args[1].GetBool();
	gUserSettings.BrowseGames_Filter.region_ru = args[2].GetBool();
	gUserSettings.BrowseGames_Filter.region_sa = args[3].GetBool();
	//gUserSettings.BrowseGames_Filter.gameworld = args[4].GetBool();
	//gUserSettings.BrowseGames_Filter.stronghold = args[5].GetBool();
	gUserSettings.BrowseGames_Filter.hideempty = args[6].GetBool();
	gUserSettings.BrowseGames_Filter.hidefull = args[7].GetBool();
	//gUserSettings.BrowseGames_Filter.showPrivateServers = args[8].GetBool();
	gUserSettings.BrowseGames_Filter.tracers2 = args[9].GetBool();
	gUserSettings.BrowseGames_Filter.nameplates2 = args[10].GetBool();
	gUserSettings.BrowseGames_Filter.crosshair2 = args[11].GetBool();
	r3dscpy(m_browseGamesNameFilter, args[12].GetString());
	gUserSettings.BrowseGames_Filter.enable_options = args[13].GetBool();
	gUserSettings.BrowseGames_Filter.password = args[14].GetBool();
	gUserSettings.BrowseGames_Filter.timeLimit = args[15].GetUInt();

	gUserSettings.saveSettings();
}

class callbackEnterPassword : public Scaleform::GFx::FunctionHandler
{
public:
	virtual void Call(const Params& params)
	{
		FrontEnd* This = (FrontEnd*)params.pUserData;
		bool res = params.pArgs[0].GetBool();
		if (res) // user pressed OK
		{
			const char* inputText = params.pArgs[1].GetString();
			r3dscpy(This->m_joinGamePwd, inputText);

			Scaleform::GFx::Value var[3];
			var[0].SetString(gLangMngr.getString("WaitConnectingToServer"));
			var[1].SetBoolean(false);
			var[2].SetString("");
			This->gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 3);

			This->StartAsyncOperation(&FrontEnd::as_JoinGameThread);
		}
	}
};
static callbackEnterPassword g_callbackEnterPassword;

void FrontEnd::eventBrowseGamesJoin(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	// gameID:int
	r3d_assert(args);
	r3d_assert(argCount == 1);

	if (gUserProfile.ProfileData.NumSlots == 0)
		return;

	m_joinGameServerId = args[0].GetInt();
	m_joinGamePwd[0] = 0;
	r3d_assert(m_joinGameServerId > 0);

	// check if we need to input password
	bool needPassword = false;
	bool foundGameInMaster = false;

	// search in active games
	const GBPKT_M2C_GameData_s* gdata = gMasterServerLogic.FindGameById(m_joinGameServerId);
	if (gdata)
	{
		foundGameInMaster = true;
		if (gdata->info.flags & GBGameInfo::SFLAGS_Passworded)
			needPassword = true;

		if (gUserProfile.ProfileData.isDevAccount == 0) // only check non dev accounts
		{
			if (gdata->info.gameTimeLimit > int(gUserProfile.ProfileData.TimePlayed / 60 / 60)) // check if we can join this game (gametime limit)
			{
				Scaleform::GFx::Value var[3];
				char tmpStr[512];
				sprintf(tmpStr, gLangMngr.getString("CannotJoinGameTimeLimit"), gdata->info.gameTimeLimit);
				var[0].SetString(tmpStr);
				var[1].SetBoolean(true);
				var[2].SetString("");
				gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 3);
				return;
			}
		}
		// check if server doesn't allow trial accounts (exclude premium servers as trials can connect to it as well if they have premium account)
		if (gdata->info.channel != 4 && !(gdata->info.flags & GBGameInfo::SFLAGS_TrialsAllowed) && gUserProfile.IsTrialAccount())
		{
			Scaleform::GFx::Value var[3];
			var[0].SetString(gLangMngr.getString("CannotJoinGameTrialLimit"));
			var[1].SetBoolean(true);
			var[2].SetString("");
			gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 3);
			return;
		}
		//AlexRedd:: BR mode
#ifdef ENABLE_BATTLE_ROYALE
		if (gUserProfile.ProfileData.isDevAccount == 0) // only check non dev accounts
		{
			if (gdata->info.IsGameBR() && gUserProfile.ProfileData.GameDollars < gUserProfile.GD_REWARD_BR)
			{
				Scaleform::GFx::Value var[3];
				char tmpStr[512];
				sprintf(tmpStr, gLangMngr.getString("NotEnougMoneyToJoinGame"), gUserProfile.GD_REWARD_BR);
				var[0].SetString(tmpStr);
				var[1].SetBoolean(true);
				var[2].SetString("");
				gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 3);
				return;
			}
		}
#endif //ENABLE_BATTLE_ROYALE

		if (gdata->info.IsNoDropGame())
		{
			wiCharDataFull& slot = gUserProfile.ProfileData.ArmorySlots[gUserProfile.SelectedCharID];
			for (int a = 0; a < slot.BackpackSize; a++)
			{
				if (slot.Items[a].itemID != 0)
				{
					const WeaponConfig* wc = g_pWeaponArmory->getWeaponConfig(slot.Items[a].itemID);
					if (wc && wc->category == storecat_SUPPORT)
					{
						Scaleform::GFx::Value var[3];
						char tmpStr[512];
						sprintf(tmpStr, gLangMngr.getString("InThisGameAreForbiddenSupportItems"));
						var[0].SetString(tmpStr);
						var[1].SetBoolean(true);
						var[2].SetString("");
						gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 3);
						return;
					}
				}
			}
		}

		//AlexRedd:: check for vip items (no drop)
		/*if(gdata->info.IsNoDropGame())
		{
		wiCharDataFull& slot = gUserProfile.ProfileData.ArmorySlots[gUserProfile.SelectedCharID];
		for (int a = 0; a < slot.BackpackSize; a++)
		{
		//wiInventoryItem& wi = slot.Items[i];
		if (slot.Items[a].itemID != 0)
		{
		if(slot.Items[a].itemID == WeaponConfig::ITEMID_NoDrop_Weapon || slot.Items[a].itemID == WeaponConfig::ITEMID_NoDrop_Backpack)
		{
		Scaleform::GFx::Value var[3];
		char tmpStr[512];
		sprintf(tmpStr, gLangMngr.getString("InThisGameAreForbiddenVipItems"));
		var[0].SetString(tmpStr);
		var[1].SetBoolean(true);
		var[2].SetString("");
		gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 3);
		return;
		}
		}
		}
		if(slot.BackpackID == WeaponConfig::ITEMID_NoDrop_Backpack)
		{
		Scaleform::GFx::Value var[3];
		char tmpStr[512];
		sprintf(tmpStr, gLangMngr.getString("InThisGameAreForbiddenNoDropBackpack"));
		var[0].SetString(tmpStr);
		var[1].SetBoolean(true);
		var[2].SetString("");
		gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 3);
		return;
		}
		}*/

		//AlexRedd:: check for hero id (not used for now)
		/*if(gdata->info.IsGameBR())
		{
		wiCharDataFull& slot = gUserProfile.ProfileData.ArmorySlots[gUserProfile.SelectedCharID];
		if(slot.HeroItemID!=20174)
		{
		Scaleform::GFx::Value var[3];
		char tmpStr[512];
		sprintf(tmpStr, gLangMngr.getString("InThisGameAreNeededBasicHero"));//todo: add to langpack
		var[0].SetString(tmpStr);
		var[1].SetBoolean(true);
		var[2].SetString("");
		gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 3);
		return;
		}
		}*/

		//AlexRedd:: check for cases
		if (gdata->info.IsNoDropGame())
		{
			wiCharDataFull& slot = gUserProfile.ProfileData.ArmorySlots[gUserProfile.SelectedCharID];
			for (int a = 0; a < slot.BackpackSize; a++)
			{
				//wiInventoryItem& wi = slot.Items[i];
				if (slot.Items[a].itemID != 0)
				{
					if (slot.Items[a].itemID >= WeaponConfig::ITEMID_AssaultCase && slot.Items[a].itemID <= WeaponConfig::ITEMID_MedicCase)
					{
						Scaleform::GFx::Value var[3];
						char tmpStr[512];
						sprintf(tmpStr, gLangMngr.getString("InThisGameAreForbiddenCases"));
						var[0].SetString(tmpStr);
						var[1].SetBoolean(true);
						var[2].SetString("");
						gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 3);
						return;
					}
				}
			}
		}
	}

	// check if we joining to our own server (joining from my server menu via eventMyServerJoinServer callback)
	const CUserServerRental::myServer_s* srv = gUserProfile.serverRent->GetServerById(m_joinGameServerId);
	if (srv)
	{
		needPassword = srv->pwd[0] ? true : false;
	}

	if (!foundGameInMaster)
	{
		// check if we joining recent or favorites
		for (UserSettings::TGameList::iterator it = gUserSettings.FavoriteGames.begin(); it != gUserSettings.FavoriteGames.end(); ++it)
		{
			if (it->gameServerId == m_joinGameServerId && it->flags & GBGameInfo::SFLAGS_Passworded)
			{
				needPassword = true;
				break;
			}
		}
		for (UserSettings::TGameList::iterator it = gUserSettings.RecentGames.begin(); it != gUserSettings.RecentGames.end(); ++it)
		{
			if (it->gameServerId == m_joinGameServerId && it->flags & GBGameInfo::SFLAGS_Passworded)
			{
				needPassword = true;
				break;
			}
		}
	}

	if (needPassword)
	{
		Scaleform::GFx::Value pfunc;
		gfxFrontEnd.GetMovie()->CreateFunction(&pfunc, &g_callbackEnterPassword, this);
		Scaleform::GFx::Value var[3];
		var[0].SetString(gLangMngr.getString("$FR_EnterPasswordToJoinGame"));
		var[1].SetString(gLangMngr.getString("$FR_PASSWORD_PROTECTED"));
		var[2] = pfunc;
		gfxFrontEnd.Invoke("_root.api.Main.MsgBox.showPasswordInputMsg", var, 3);
		return;
	}

	Scaleform::GFx::Value var[3];
	var[0].SetString(gLangMngr.getString("WaitConnectingToServer"));
	var[1].SetBoolean(false);
	var[2].SetString("");
	gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 3);

	StartAsyncOperation(&FrontEnd::as_JoinGameThread);
}
void FrontEnd::eventBrowseGamesOnAddToFavorites(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	// gameID:int
	r3d_assert(args);
	r3d_assert(argCount == 1);

	uint32_t gameID = (uint32_t)args[0].GetInt();
	const GBPKT_M2C_GameData_s* gdata = gMasterServerLogic.FindGameById(gameID);
	if (gdata)
	{
		gUserSettings.addGameToFavorite(gdata->info);
		gUserSettings.saveSettings();
	}
}

void FrontEnd::eventBrowseGamesRequestList(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	// type:String, sort:String, order:int, oper:int
	// type (browse, recent, favorites)
	// sort (name, mode, map, ping)
	// order (1, -1)
	// oper (0 - top, -1 - left 100 , +1 - right 100)
	r3d_assert(args);
	r3d_assert(argCount == 4);

	Scaleform::GFx::Value var[3];
	var[0].SetString(gLangMngr.getString("FetchingGamesListFromServer"));
	var[1].SetBoolean(false);
	var[2].SetString("");
	gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 3);

	const char* btype = args[0].GetString();
	const char* sort = args[1].GetString();
	m_browseGamesSortOrder = args[2].GetInt();
	m_browseGamesRequestedOper = args[3].GetInt();

	if (strcmp(sort, "name") == 0)
		m_browseGamesSortMode = 0;
	else if (strcmp(sort, "mode") == 0)
		m_browseGamesSortMode = 1;
	else if (strcmp(sort, "map") == 0)
		m_browseGamesSortMode = 2;
	else if (strcmp(sort, "ping") == 0)
		m_browseGamesSortMode = 3;

	if (strcmp(btype, "lobby_all") == 0)
		m_browseGamesMode = 0;
	else if (strcmp(btype, "lobby_ranked") == 0)
		m_browseGamesMode = 1;
	else if (strcmp(btype, "lobby_arcade") == 0)
		m_browseGamesMode = 2;
	else
		m_browseGamesMode = 3;

	if (m_browseGamesMode == 0 && m_browseGamesRequestedOper == 0) // fetch new list from server only when requesting TOP list (activate and each refresh list click)
	{
		StartAsyncOperation(&FrontEnd::as_BrowseGamesThread, &FrontEnd::OnGameListReceived);
	}
	else
	{
		// this works only if we already have a list of games from server. but, browse games shows by default in mode 0, so we should always have a list
		gfxFrontEnd.Invoke("_root.api.hideInfoMsg", "");
		processNewGameList();
		gfxFrontEnd.Invoke("_root.api.Main.BrowseGamesAnim.showGameList", "");
	}
}

unsigned int WINAPI FrontEnd::as_BrowseGamesThread(void* in_data)
{
	r3dThreadAutoInstallCrashHelper crashHelper;
	FrontEnd* This = (FrontEnd*)in_data;

	This->DelayServerRequest();
	if (!This->ConnectToMasterServer())
		return 0;

	GBPKT_C2M_RefreshList_s n;

	// copy filters
	n.region = GBNET_REGION_US_West;
	n.browseChannel = (BYTE)This->CurrentBrowseChannel;
	n.hideempty = gUserSettings.BrowseGames_Filter.hideempty;
	n.hidefull = gUserSettings.BrowseGames_Filter.hidefull;
	n.enable_options = gUserSettings.BrowseGames_Filter.enable_options;
	n.tracers2 = gUserSettings.BrowseGames_Filter.tracers2;
	n.nameplates2 = gUserSettings.BrowseGames_Filter.nameplates2;
	n.crosshair2 = gUserSettings.BrowseGames_Filter.crosshair2;
	n.password = gUserSettings.BrowseGames_Filter.password;
	n.timeLimit = gUserSettings.BrowseGames_Filter.timeLimit;

	gMasterServerLogic.RequestGameList(n);

	const float endTime = r3dGetTime() + 120.0f;
	while (r3dGetTime() < endTime)
	{
		::Sleep(10);
		if (gMasterServerLogic.gameListReceived_)
		{
			This->ProcessSupervisorPings();
			return 1;
		}

		if (!gMasterServerLogic.IsConnected())
			break;
	}

	This->SetAsyncError(0, gLangMngr.getString("FailedReceiveGameList"));
	return 0;
}

void FrontEnd::OnGameListReceived()
{
	gfxFrontEnd.Invoke("_root.api.hideInfoMsg", "");
	processNewGameList();
	gfxFrontEnd.Invoke("_root.api.Main.BrowseGamesAnim.showGameList", "");
}

static bool SortGamesByNameAsc(const GBPKT_M2C_GameData_s& g1, const GBPKT_M2C_GameData_s& g2) {
	return stricmp(g1.info.name, g2.info.name) < 0;
}
static bool SortGamesByNameDec(const GBPKT_M2C_GameData_s& g1, const GBPKT_M2C_GameData_s& g2) {
	return stricmp(g2.info.name, g1.info.name) < 0;
}

static bool SortGamesByMapAsc(const GBPKT_M2C_GameData_s& g1, const GBPKT_M2C_GameData_s& g2) {
	return g1.info.mapId < g2.info.mapId;
}
static bool SortGamesByMapDec(const GBPKT_M2C_GameData_s& g1, const GBPKT_M2C_GameData_s& g2) {
	return g2.info.mapId < g1.info.mapId;
}

static bool SortGamesByModeAsc(const GBPKT_M2C_GameData_s& g1, const GBPKT_M2C_GameData_s& g2) {
	return g1.info.IsGameworld() == g2.info.IsGameworld();
}
static bool SortGamesByModeDec(const GBPKT_M2C_GameData_s& g1, const GBPKT_M2C_GameData_s& g2) {
	return g2.info.IsGameworld() == g1.info.IsGameworld();
}

static bool SortGamesByPingAsc(const GBPKT_M2C_GameData_s& g1, const GBPKT_M2C_GameData_s& g2) {
	int ping1 = GetGamePing(g1.superId);
	int ping2 = GetGamePing(g2.superId);
	return ping1 < ping2;
}
static bool SortGamesByPingDec(const GBPKT_M2C_GameData_s& g1, const GBPKT_M2C_GameData_s& g2) {
	int ping1 = GetGamePing(g1.superId);
	int ping2 = GetGamePing(g2.superId);
	return ping2 < ping1;
}

void FrontEnd::processNewGameList()
{
	r3dgameVector(GBPKT_M2C_GameData_s) filteredGames;

	if (m_browseGamesMode == 0)
	{
		// note: all additional filters are server side now
		for (size_t i = 0; i < gMasterServerLogic.games_.size(); i++)
		{
			const GBPKT_M2C_GameData_s& gd = gMasterServerLogic.games_[i];
			const GBGameInfo& ginfo = gd.info;
			// simple name filter
			if (strlen(m_browseGamesNameFilter) > 1)
			{
				if (!stristr(ginfo.name, m_browseGamesNameFilter))
					continue;
			}
			filteredGames.push_back(gd);
		}
	}
	else if (m_browseGamesMode == 1) // recent
	{
		GBPKT_M2C_GameData_s gd;
		gd.curPlayers = 0;
		gd.status = 0;
		gd.superId = 0;

		UserSettings::TGameList& list = gUserSettings.RecentGames;
		for (UserSettings::TGameList::iterator it = list.begin(); it != list.end(); ++it)
		{
			gd.info = *it;
#ifdef ENABLE_BATTLE_ROYALE
			//AlexRedd:: BR mode
			if (!gd.info.IsGameBR())
#endif //ENABLE_BATTLE_ROYALE
				filteredGames.push_back(gd);
		}
	}
	else if (m_browseGamesMode == 2) // favorite
	{
		GBPKT_M2C_GameData_s gd;
		gd.curPlayers = 0;
		gd.status = 0;
		gd.superId = 0;

		UserSettings::TGameList& list = gUserSettings.FavoriteGames;
		for (UserSettings::TGameList::iterator it = list.begin(); it != list.end(); ++it)
		{
			gd.info = *it;
#ifdef ENABLE_BATTLE_ROYALE
			//AlexRedd:: BR mode
			if (!gd.info.IsGameBR())
#endif //ENABLE_BATTLE_ROYALE
				filteredGames.push_back(gd);
		}
	}
	else
		r3d_assert(false); // shouldn't happen


	// sort
	if (m_browseGamesSortMode == 0 && m_browseGamesMode != 1) // do not sort recent games by name, kill whole idea
	{
		if (m_browseGamesSortOrder == 1)
			std::sort(filteredGames.begin(), filteredGames.end(), SortGamesByNameAsc);
		else
			std::sort(filteredGames.begin(), filteredGames.end(), SortGamesByNameDec);
	}
	if (m_browseGamesSortMode == 1)
	{
		if (m_browseGamesSortOrder == 1)
			std::sort(filteredGames.begin(), filteredGames.end(), SortGamesByMapAsc);
		else
			std::sort(filteredGames.begin(), filteredGames.end(), SortGamesByMapDec);
	}
	if (m_browseGamesSortMode == 2)
	{
		if (m_browseGamesSortOrder == 1)
			std::sort(filteredGames.begin(), filteredGames.end(), SortGamesByModeAsc);
		else
			std::sort(filteredGames.begin(), filteredGames.end(), SortGamesByModeDec);
	}
	if (m_browseGamesSortMode == 3)
	{
		if (m_browseGamesSortOrder == 1)
			std::sort(filteredGames.begin(), filteredGames.end(), SortGamesByPingAsc);
		else
			std::sort(filteredGames.begin(), filteredGames.end(), SortGamesByPingDec);
	}

	int numGames = (int)filteredGames.size();

	if (m_browseGamesRequestedOper == -1)
		m_browseGamesCurrentCur -= 100;
	else if (m_browseGamesRequestedOper == 1)
		m_browseGamesCurrentCur += 100;
	else if (m_browseGamesRequestedOper == 0)
		m_browseGamesCurrentCur = 0;

	m_browseGamesCurrentCur = R3D_CLAMP(m_browseGamesCurrentCur, 0, numGames - 100);

	for (int i = 0; i < numGames; i++)
	{
		if (i < m_browseGamesCurrentCur || i>(m_browseGamesCurrentCur + 100))
			continue;

		const GBPKT_M2C_GameData_s& gd = filteredGames[i];
		const GBGameInfo& ginfo = gd.info;

		int ping = GetGamePing(gd.superId);
		if (ping > 0)
			ping = R3D_CLAMP(ping + random(10) - 5, 1, 1000);
		ping = R3D_CLAMP(ping / 10, 1, 100); // UI accepts ping from 0 to 100 and shows bars instead of actual number

		const char* mapName = getMapName(ginfo.mapId);
#ifdef ENABLE_BATTLE_ROYALE
		bool isGameHasStarted = ginfo.IsGameBR() && gd.status == 1;
#endif //ENABLE_BATTLE_ROYALE
		//addGameToList(id:Number, name:String, mode:String, map:String, tracers:Boolean, nametags:Boolean, crosshair:Boolean, players:String, ping:int, favorite:Boolean, isPassword:Boolean, isTimeLimit:Boolean, trialsAllowed:Boolean, donate:Boolean, disableWeapon:String, ingame:Boolean, channel:int, isPVP:Boolean)
		Scaleform::GFx::Value var[18];
		var[0].SetNumber(ginfo.gameServerId);
		var[1].SetString(ginfo.name);
		//var[2].SetString(ginfo.IsGameworld() ? "GAMEWORLD" : "STRONGHOLD");
		const char* serverType;
		if (ginfo.channel == 5)
			serverType = gLangMngr.getString("1");
		else if (ginfo.IsDevEvent())	//AlexRedd:: Event mode	
			serverType = gLangMngr.getString("2");
#ifdef ENABLE_BATTLE_ROYALE
		else if (ginfo.IsGameBR())	//AlexRedd:: BR mode	
			serverType = gLangMngr.getString("3");
#endif //ENABLE_BATTLE_ROYALE
		//else if (ginfo.channel==7)//AlexRedd:: Trade mode
		//serverType = gLangMngr.getString("MapType_Trade");
		else
			serverType = gLangMngr.getString("ARCADE");
		var[2].SetString(serverType);
		var[3].SetString(mapName);
		var[4].SetBoolean((ginfo.flags & GBGameInfo::SFLAGS_Tracers) ? true : false);
		var[5].SetBoolean((ginfo.flags & GBGameInfo::SFLAGS_Nameplates) ? true : false);
		var[6].SetBoolean((ginfo.flags & GBGameInfo::SFLAGS_CrossHair) ? true : false);
		char players[16];
		sprintf(players, "%d/%d", R3D_MIN(gd.curPlayers, ginfo.maxPlayers), ginfo.maxPlayers); // hide if dev connected
		var[7].SetString(players);
		var[8].SetInt(ping);
#ifdef ENABLE_BATTLE_ROYALE
		var[9].SetBoolean(gUserSettings.isInFavoriteGamesList(ginfo.gameServerId));
#else
		var[9].SetBoolean(gUserSettings.isInFavoriteGamesList(ginfo.gameServerId));
#endif //ENABLE_BATTLE_ROYALE
		var[10].SetBoolean((ginfo.flags & GBGameInfo::SFLAGS_Passworded) ? true : false
#ifdef ENABLE_BATTLE_ROYALE
			//AlexRedd:: BR mode
			|| isGameHasStarted
#endif //ENABLE_BATTLE_ROYALE
			);
		var[11].SetBoolean(ginfo.gameTimeLimit > 0 ? true : false);
		var[12].SetBoolean((ginfo.flags & GBGameInfo::SFLAGS_TrialsAllowed) ? true : false);
		var[13].SetBoolean(ginfo.channel == 3 && m_browseGamesMode == 0); // allow donating GC only to private servers		
		if (ginfo.flags & GBGameInfo::SFLAGS_DisableASR)
			var[14].SetString("no_auto");
		else if (ginfo.flags & GBGameInfo::SFLAGS_DisableSNP)
			var[14].SetString("no_sniper");
		else
			var[14].SetString("no_icon");
#ifdef ENABLE_BATTLE_ROYALE
		var[15].SetBoolean(isGameHasStarted);
#else
		var[15].SetBoolean(false);
#endif //ENABLE_BATTLE_ROYALE
		var[16].SetInt(ginfo.channel);
		var[17].SetInt((ginfo.flags & GBGameInfo::SFLAGS_PVE) ? true : false);

		if (ginfo.mapId != GBGameInfo::MAPID_ServerTest)
		{
			gfxFrontEnd.Invoke("_root.api.Main.BrowseGamesAnim.addGameToList", var, 18);
		}
		else {
			if (gUserProfile.ProfileData.isDevAccount > 0)
				gfxFrontEnd.Invoke("_root.api.Main.BrowseGamesAnim.addGameToList", var, 18);
		}
	}
}

int FrontEnd::GetSupervisorPing(DWORD ip)
{
	HANDLE hIcmpFile = IcmpCreateFile();
	if (hIcmpFile == INVALID_HANDLE_VALUE) {
		r3dOutToLog("IcmpCreatefile returned error: %d\n", GetLastError());
		return -1;
	}

	char  sendData[32] = "Data Buffer";
	DWORD replySize = sizeof(ICMP_ECHO_REPLY) + sizeof(sendData);
	void* replyBuf = (void*)_alloca(replySize);

	// send single ping with 1000ms, without payload as it alert most firewalls
	DWORD sendResult = IcmpSendEcho(hIcmpFile, ip, sendData, 0, NULL, replyBuf, replySize, 1000);
#ifndef FINAL_BUILD	
	if (sendResult == 0) {
		DWORD lastE = GetLastError();
		char ips[128];
		r3dscpy(ips, inet_ntoa(*(in_addr*)&ip));
		r3dOutToLog("PING failed to %s : %d\n", ips, lastE);
	}
#endif

	IcmpCloseHandle(hIcmpFile);

	if (sendResult == 0) {
		//r3dOutToLog("IcmpSendEcho returned error: %d\n", GetLastError());
		return -2;
	}

	PICMP_ECHO_REPLY pEchoReply = (PICMP_ECHO_REPLY)replyBuf;
	if (pEchoReply->Status == IP_SUCCESS)
	{
		return pEchoReply->RoundTripTime;
	}

	//r3dOutToLog("IcmpSendEcho returned status %d\n", pEchoReply->Status);
	return -3;
}

void FrontEnd::ProcessSupervisorPings()
{
	for (size_t i = 0; i < gMasterServerLogic.supers_.size(); ++i)
	{
		const GBPKT_M2C_SupervisorData_s& super = gMasterServerLogic.supers_[i];
		if (super.ID >= R3D_ARRAYSIZE(superPings_))
		{
#ifndef FINAL_BUILD		
			r3dError("Too Many servers, please contact support@infestationmmo.com");
#endif
			continue;
		}

		// if we already have stored ping, don't query again
		if (superPings_[super.ID] > 1)
			continue;

		int ping = GetSupervisorPing(super.ip);
		superPings_[super.ID] = ping ? ping : 1;

		// fill all other supervisors in /24 IP mask with same value to prevent pinging machines in same DC
		for (size_t j = i + 1; j < gMasterServerLogic.supers_.size(); ++j)
		{
			const GBPKT_M2C_SupervisorData_s& s2 = gMasterServerLogic.supers_[j];
			if (s2.ID >= R3D_ARRAYSIZE(superPings_))
				continue;

			if ((s2.ip & 0x00FFFFFF) == (super.ip & 0x00FFFFFF)) // network order is big engianess
				superPings_[s2.ID] = superPings_[super.ID];
		}
	}
}

void FrontEnd::eventRequestMyClanInfo(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount == 0);

	setClanInfo();
	gfxFrontEnd.Invoke("_root.api.Main.Clans.showClanList", "");
}

void FrontEnd::setClanInfo()
{
	wiCharDataFull& slot = gUserProfile.ProfileData.ArmorySlots[gUserProfile.SelectedCharID];
	CUserClans* clans = gUserProfile.clans[gUserProfile.SelectedCharID];

	// fill clanCurData_.clanID and invites/application list.
	// TODO: implement timer to avoid API spamming? or check async every N seconds...
	clans->ApiGetClanStatus();

	if (clans->clanCurData_.ClanID != slot.ClanID)
	{
		slot.ClanID = clans->clanCurData_.ClanID;
		// we joined or left clan. do something
	}


	// if we don't have clan data yet - retrieve it. NOTE: need switch to async call
	if (slot.ClanID && clans->clanInfo_.ClanID == 0)
	{
		clans->ApiClanGetInfo(slot.ClanID, &clans->clanInfo_, &clans->clanMembers_);
	}

	{
		clans->ApiClanGetInfo(slot.ClanID, &clans->clanInfo_, &clans->clanMembers_);
		//		public function setClanInfo(clanID:uint, isAdmin:Boolean, name:String, availableSlots:uint, clanReserve:uint, logoID:uint, FriendlyFire:uint, ClanPoints:uint)
		Scaleform::GFx::Value var[8];
		var[0].SetUInt(slot.ClanID); // if ClanID is zero, then treated by UI as user is not in clan
		var[1].SetBoolean(slot.ClanRank <= 1); // should be true only for admins of the clan (creator=0 and officers=1)
		var[2].SetString(clans->clanInfo_.ClanName);
		var[3].SetUInt(clans->clanInfo_.MaxClanMembers - clans->clanInfo_.NumClanMembers);
		var[4].SetUInt(clans->clanInfo_.ClanGP);
		var[5].SetUInt(clans->clanInfo_.ClanEmblemID);
		var[6].SetInt(clans->clanInfo_.FriendlyFire);
		var[7].SetInt(clans->clanInfo_.ClanPoints);
		gfxFrontEnd.Invoke("_root.api.setClanInfo", var, 8);
	}

	{
		Scaleform::GFx::Value var[10];
		for (CUserClans::TClanMemberList::iterator iter = clans->clanMembers_.begin(); iter != clans->clanMembers_.end(); ++iter)
		{
			CUserClans::ClanMember_s& memberInfo = *iter;
			//public function addClanMemberInfo(customerID:uint, name:String, exp:uint, time:String, rep:String, kzombie:uint, ksurvivor:uint, kbandits:uint, donatedgc:uint)
			var[0].SetUInt(memberInfo.CharID);
			var[1].SetString(memberInfo.gamertag);
			var[2].SetUInt(memberInfo.stats.XP);
			var[3].SetString(getTimePlayedString(memberInfo.stats.TimePlayed));
			var[4].SetString(getReputationString(memberInfo.stats.Reputation));
			var[5].SetUInt(memberInfo.stats.KilledZombies);
			var[6].SetUInt(memberInfo.stats.KilledSurvivors);
			var[7].SetUInt(memberInfo.stats.KilledBandits);
			var[8].SetUInt(memberInfo.ContributedGP);
			var[9].SetUInt(memberInfo.ClanRank);
			gfxFrontEnd.Invoke("_root.api.addClanMemberInfo", var, 10);
		}
	}

	checkForInviteFromClan();
}

void FrontEnd::eventRequestClanList(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount == 2);
	//sort:int, mode:int
	int sortID = args[0].GetInt();
	int mode = args[1].GetInt(); // 0-top, 1-left, 2-right
	//r3dOutToLog("######## eventRequestClanList: sortID %d, mode %d\n", sortID, mode);
	clanListRequest_SortID = sortID;
	if (mode == 0)
		clanListRequest_StartPos = 0;
	else if (mode == 1)
		clanListRequest_StartPos = R3D_CLAMP(clanListRequest_StartPos - 100, 0, clanListRequest_ClanListSize - 100);
	else if (mode == 2)
		clanListRequest_StartPos = R3D_CLAMP(clanListRequest_StartPos + 100, 0, clanListRequest_ClanListSize - 100);
	else
		r3d_assert(false);

	Scaleform::GFx::Value var[2];
	var[0].SetString(gLangMngr.getString("OneMomentPlease"));
	var[1].SetBoolean(false);
	gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);

	StartAsyncOperation(&FrontEnd::as_RequestClanListThread, &FrontEnd::OnRequestClanListSuccess);
}

unsigned int WINAPI FrontEnd::as_RequestClanListThread(void* in_data)
{
	r3dThreadAutoInstallCrashHelper crashHelper;
	FrontEnd* This = (FrontEnd*)in_data;

	CUserClans* clans = game_new CUserClans;
	int api = clans->ApiClanGetLeaderboard2(This->clanListRequest_SortID, This->clanListRequest_StartPos, &This->clanListRequest_ClanListSize);
	This->cachedClanList = clans->leaderboard_;
	delete clans;

	if (api != 0)
	{
		if (!This->processClanError(api))
		{
			This->SetAsyncError(api, gLangMngr.getString("FailedToGetClanLeaderboard"));
		}
		return 0;
	}
	return 1;
}

void FrontEnd::OnRequestClanListSuccess()
{
	gfxFrontEnd.Invoke("_root.api.hideInfoMsg", "");

	//r3d_assert(!cachedClanList.empty());
	{
		gfxFrontEnd.Invoke("_root.api.Main.Clans.clearClanInfo", "");

		Scaleform::GFx::Value var[8];
		for (ClanList::iterator iter = cachedClanList.begin(); iter != cachedClanList.end(); ++iter)
		{
			CUserClans::ClanInfo_s& clanInfo = *iter;
			//public function addClanInfo(clanID:uint, name:String, creator:String, xp:uint, numMembers:uint, description:String, icon:String, Points:uint)
			var[0].SetUInt(clanInfo.ClanID);
			var[1].SetString(clanInfo.ClanName);
			var[2].SetString(clanInfo.OwnerGamertag);
			var[3].SetUInt(clanInfo.ClanXP);
			var[4].SetUInt(clanInfo.NumClanMembers);
			var[5].SetString(clanInfo.ClanLore);
			var[6].SetUInt(clanInfo.ClanEmblemID);
			var[7].SetUInt(clanInfo.ClanPoints);
			gfxFrontEnd.Invoke("_root.api.Main.Clans.addClanInfo", var, 8);
		}
		gfxFrontEnd.Invoke("_root.api.Main.Clans.populateClanList", "");
	}
}

void FrontEnd::eventCreateClan(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount == 6);

	if (gUserProfile.ProfileData.NumSlots == 0)
	{
		Scaleform::GFx::Value var[2];
		var[0].SetString(gLangMngr.getString("YouNeedCharBeforeCreatingClan"));
		var[1].SetBoolean(true);
		gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);
		return;
	}

	if (gUserProfile.IsTrialAccount())
	{
		Scaleform::GFx::Value var[2];
		var[0].SetString(gLangMngr.getString("TrialAccountLimit_NoClans"));
		var[1].SetString(gLangMngr.getString("$TrialAccountLimit"));
		gfxFrontEnd.Invoke("_root.api.showTrialAccountLimit", var, 2);
		return;
	}

	//eventCreateClan(name:String, tag:String, desc:String, nameColor:int, tagColor:int, iconID:int)
	r3dscpy(clanCreateParams.ClanName, args[0].GetString());
	r3dscpy(clanCreateParams.ClanTag, args[1].GetString());
	clanCreateParam_Desc = args[2].GetString();
	clanCreateParams.ClanNameColor = args[3].GetInt();
	clanCreateParams.ClanTagColor = args[4].GetInt();
	clanCreateParams.ClanEmblemID = args[5].GetInt();

	if (strpbrk(clanCreateParams.ClanName, "!@#$%^&*()-=+_<>,./?'\":;|{}[]") != NULL) // do not allow this symbols
	{
		Scaleform::GFx::Value var[2];
		var[0].SetString(gLangMngr.getString("ClanNameNoSpecSymbols"));
		var[1].SetBoolean(true);
		gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);
		return;
	}
	if (strpbrk(clanCreateParams.ClanTag, "!@#$%^&*()-=+_<>,./?'\":;|{}[]") != NULL) // do not allow this symbols
	{
		Scaleform::GFx::Value var[2];
		var[0].SetString(gLangMngr.getString("ClanTagNoSpecSymbols"));
		var[1].SetBoolean(true);
		gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);
		return;
	}

	/*int pos = 0;
	while((pos= clanCreateParam_Desc.find('<'))!=-1)
	clanCreateParam_Desc.replace(pos, 1, "&lt;");
	while((pos = clanCreateParam_Desc.find('>'))!=-1)
	clanCreateParam_Desc.replace(pos, 1, "&gt;");*/

	Scaleform::GFx::Value var[2];
	var[0].SetString(gLangMngr.getString("OneMomentPlease"));
	var[1].SetBoolean(false);
	gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);

	StartAsyncOperation(&FrontEnd::as_CreateClanThread, &FrontEnd::OnCreateClanSuccess);
}

bool FrontEnd::processClanError(int api)
{
	if (api >= 20 && api <= 29)
	{
		char tmpStr[64];
		sprintf(tmpStr, "ClanError_Code%d", api);
		SetAsyncError(0, gLangMngr.getString(tmpStr));
		return true;
	}
	return false;
}

unsigned int WINAPI FrontEnd::as_CreateClanThread(void* in_data)
{
	r3dThreadAutoInstallCrashHelper crashHelper;
	FrontEnd* This = (FrontEnd*)in_data;

	CUserClans* clans = gUserProfile.clans[gUserProfile.SelectedCharID];
	int api = clans->ApiClanCreate(This->clanCreateParams);
	if (api != 0)
	{
		if (!This->processClanError(api))
		{
			if (api == 7)
				This->SetAsyncError(0, gLangMngr.getString("NotEnoughTimePlayedToCreateClan"));
			else
				This->SetAsyncError(api, gLangMngr.getString("FailedToCreateClan"));
		}
		return 0;
	}
	return 1;
}

void FrontEnd::OnCreateClanSuccess()
{
	gfxFrontEnd.Invoke("_root.api.hideInfoMsg", "");
	CUserClans* clans = gUserProfile.clans[gUserProfile.SelectedCharID];
	int api = clans->ApiClanSetLore(clanCreateParam_Desc.c_str());
	if (api != 0)
	{
		r3dOutToLog("failed to set clan desc, api=%d\n", api);
		r3d_assert(false);
	}

	cachedClanList.clear(); // to refresh and get newly created clan
	setClanInfo();
	gfxFrontEnd.Invoke("_root.api.Main.showScreen", "MyClan");
}

void FrontEnd::eventClanAdminDonateGC(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount == 2);
	uint32_t charID = args[0].GetUInt();
	uint32_t numGC = args[1].GetUInt();
	CUserClans* clans = gUserProfile.clans[gUserProfile.SelectedCharID];

	if (clans->clanInfo_.ClanGP < int(numGC))
	{
		Scaleform::GFx::Value var[2];
		var[0].SetString(gLangMngr.getString("ClanReserveNotEnoughGC"));
		var[1].SetBoolean(true);
		gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);
		return;
	}

	int api = clans->ApiClanDonateGPToMember(charID, numGC);
	if (api != 0)
	{
		r3dOutToLog("Failed to donate to member, api=%d\n", api);

		Scaleform::GFx::Value var[2];
		if (api == 9) // one week timeout
		{
			var[0].SetString(gLangMngr.getString("FailToDonateGCToClanMemberTime"));
			var[1].SetBoolean(true);
		}
		else
		{
			var[0].SetString(gLangMngr.getString("FailToDonateGCToClanMember"));
			var[1].SetBoolean(true);
		}
		gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);
		return;
	}

	char tmpStr[32]; sprintf(tmpStr, "%d GC", clans->clanInfo_.ClanGP);
	gfxFrontEnd.SetVariable("_root.api.Main.ClansMyClan.MyClan.OptionsBlock3.GC.text", tmpStr);
}

void FrontEnd::refreshClanUIMemberList()
{
	setClanInfo();
	gfxFrontEnd.Invoke("_root.api.Main.ClansMyClan.showClanMembers", "");
}

void FrontEnd::eventClanAdminAction(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount == 2);
	uint32_t charID = args[0].GetUInt();
	const char* actionType = args[1].GetString(); // promote, demote, kick
	CUserClans* clans = gUserProfile.clans[gUserProfile.SelectedCharID];
	if (strcmp(actionType, "promote") == 0)
	{
		CUserClans::ClanMember_s* member = clans->GetMember(charID);
		r3d_assert(member);
		if (member->ClanRank > 0)
		{
			int newRank = member->ClanRank;
			if (newRank > 2)
				newRank = 1;
			else
				newRank = newRank - 1;
			int api = clans->ApiClanSetRank(charID, newRank);
			if (api != 0)
			{
				r3dOutToLog("Failed to promote rank, api=%d\n", api);

				Scaleform::GFx::Value var[2];
				var[0].SetString(gLangMngr.getString("FailToPromote"));
				var[1].SetBoolean(true);
				gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);
			}
			else
			{
				if (newRank == 0) // promoted someone else to leader -> demote itself
				{
					wiCharDataFull& slot = gUserProfile.ProfileData.ArmorySlots[gUserProfile.SelectedCharID];
					slot.ClanRank = 1;
					CUserClans::ClanMember_s* m = clans->GetMember(slot.LoadoutID);
					if (m)
						m->ClanRank = 1;
				}
				refreshClanUIMemberList();
			}
		}
		else
		{
			Scaleform::GFx::Value var[2];
			var[0].SetString(gLangMngr.getString("MemberAlreadyHasHighestRank"));
			var[1].SetBoolean(true);
			gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);
		}
	}
	if (strcmp(actionType, "demote") == 0)
	{
		CUserClans::ClanMember_s* member = clans->GetMember(charID);
		r3d_assert(member);
		if (member->ClanRank < 2)
		{
			int api = clans->ApiClanSetRank(charID, member->ClanRank + 1);
			if (api != 0)
			{
				r3dOutToLog("Failed to demote rank, api=%d\n", api);

				Scaleform::GFx::Value var[2];
				var[0].SetString(gLangMngr.getString("FailToDemote"));
				var[1].SetBoolean(true);
				gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);
			}
			else
				refreshClanUIMemberList();
		}
		else
		{
			Scaleform::GFx::Value var[2];
			var[0].SetString(gLangMngr.getString("MemberAlreadyHasLowestRank"));
			var[1].SetBoolean(true);
			gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);
		}
	}
	if (strcmp(actionType, "kick") == 0)
	{
		if (clans->GetMember(charID) == NULL)
		{
			Scaleform::GFx::Value var[2];
			var[0].SetString(gLangMngr.getString("NoSuchClanMember"));
			var[1].SetBoolean(true);
			gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);
			return;
		}

		int api = clans->ApiClanKick(charID);
		if (api != 0)
		{
			if (api == 6)
			{
				Scaleform::GFx::Value var[2];
				var[0].SetString(gLangMngr.getString("YouCannotKickYourself"));
				var[1].SetBoolean(true);
				gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);
			}
			else
			{
				r3dOutToLog("Failed to kick, api=%d\n", api);

				Scaleform::GFx::Value var[2];
				var[0].SetString(gLangMngr.getString("FailToKickMember"));
				var[1].SetBoolean(true);
				gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);
			}
		}
		else
		{
			Scaleform::GFx::Value var[2];
			var[0].SetString(gLangMngr.getString("ClanMemberWasKickedFromClan"));
			var[1].SetBoolean(true);
			gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);
			refreshClanUIMemberList();
		}
	}
}

void FrontEnd::eventClanLeaveClan(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount == 0);
	CUserClans* clans = gUserProfile.clans[gUserProfile.SelectedCharID];

	// do not allow leader to leave clan if clan has GC balance
	if (clans->clanInfo_.NumClanMembers == 1 && clans->clanInfo_.ClanGP > 0)
	{
		Scaleform::GFx::Value var[2];
		var[0].SetString(gLangMngr.getString("CannotLeaveClanWithGCBalance"));
		var[1].SetBoolean(true);
		gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);
		return;
	}

	int api = clans->ApiClanLeave();
	if (api != 0)
	{
		r3dOutToLog("Failed to leave clan, api=%d\n", api);

		Scaleform::GFx::Value var[2];
		var[0].SetString(gLangMngr.getString("FailToLeaveClan"));
		var[1].SetBoolean(true);
		gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);
	}
	else
	{
		//TheHexa:: BUG FIX: LEAVE CLAN AND NEED TO CLOSE GAME TO RELOAD INFO
		wiCharDataFull& slot = gUserProfile.ProfileData.ArmorySlots[gUserProfile.SelectedCharID];
		updateClientSurvivor(slot);

		gfxFrontEnd.Invoke("_root.api.Main.showScreen", "Clans");
	}
}

void FrontEnd::eventClanDonateGCToClan(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount == 1);
	uint32_t amount = args[0].GetUInt();

	if (amount == 0)
	{
		Scaleform::GFx::Value var[2];
		var[0].SetString(gLangMngr.getString("ClanError_AmountToDonateShouldBeMoreThanZero"));
		var[1].SetBoolean(true);
		gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);
		return;
	}
	if (amount > (uint32_t)gUserProfile.ProfileData.GamePoints)
	{
		Scaleform::GFx::Value var[2];
		var[0].SetString(gLangMngr.getString("ClanError_NotEnoughGPToDonate"));
		var[1].SetBoolean(true);
		gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);
		return;
	}

	CUserClans* clans = gUserProfile.clans[gUserProfile.SelectedCharID];
	int api = clans->ApiClanDonateGPToClan(amount);
	if (api != 0)
	{
		r3dOutToLog("Failed to donate to clan, api=%d\n", api);

		Scaleform::GFx::Value var[2];
		if (api == 9) // one week timeout
		{
			var[0].SetString(gLangMngr.getString("FailToDonateGCToClanTime"));
			var[1].SetBoolean(true);
		}
		else
		{
			var[0].SetString(gLangMngr.getString("FailToDonateGCToClan"));
			var[1].SetBoolean(true);
		}
		gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);
		return;
	}
	char tmpStr[32]; sprintf(tmpStr, "%d GC", clans->clanInfo_.ClanGP);
	gfxFrontEnd.SetVariable("_root.api.Main.ClansMyClan.MyClan.OptionsBlock3.GC.text", tmpStr);
}

void FrontEnd::eventRequestClanApplications(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount == 0);

	CUserClans* clans = gUserProfile.clans[gUserProfile.SelectedCharID];

	clans->ApiGetClanStatus();

	Scaleform::GFx::Value var[9];
	for (CUserClans::ClanApplicationList::iterator it = clans->clanApplications_.begin(); it != clans->clanApplications_.end(); ++it)
	{
		//public function addApplication(appID:uint, appText:String, name:String, exp:uint, stime:String, rep:String, kz:uint, ks:uint, kb:uint)
		CUserClans::ClanApplication_s& appl = *it;
		var[0].SetUInt(appl.ClanApplID);
		var[1].SetString(appl.Note.c_str());
		var[2].SetString(appl.Gamertag.c_str());
		var[3].SetUInt(appl.stats.XP);
		var[4].SetString(getTimePlayedString(appl.stats.TimePlayed));
		var[5].SetString(getReputationString(appl.stats.Reputation));
		var[6].SetUInt(appl.stats.KilledZombies);
		var[7].SetUInt(appl.stats.KilledSurvivors);
		var[8].SetUInt(appl.stats.KilledBandits);
		gfxFrontEnd.Invoke("_root.api.Main.ClansMyClanApps.addApplication", var, 9);
	}

	gfxFrontEnd.Invoke("_root.api.Main.ClansMyClanApps.showApplications", "");
}

void FrontEnd::eventClanApplicationAction(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount == 2);
	uint32_t applicationID = args[0].GetUInt();
	bool isAccepted = args[1].GetBool();

	CUserClans* clans = gUserProfile.clans[gUserProfile.SelectedCharID];
	int api = clans->ApiClanApplyAnswer(applicationID, isAccepted);
	if (api != 0)
	{
		r3dOutToLog("Failed to answer application, api=%d\n", api);

		Scaleform::GFx::Value var[2];
		var[0].SetString(gLangMngr.getString("FailToAnswerApplication"));
		var[1].SetBoolean(true);
		gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);
	}
}

void FrontEnd::eventClanInviteToClan(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount == 1);
	const char* playerNameToInvite = args[0].GetString();

	CUserClans* clans = gUserProfile.clans[gUserProfile.SelectedCharID];

	int api = clans->ApiClanSendInvite(playerNameToInvite);
	if (api != 0)
	{
		r3dOutToLog("Failed to send invite, api=%d\n", api);

		Scaleform::GFx::Value var[2];
		var[0].SetString(gLangMngr.getString("FailToSendInvite"));
		var[1].SetBoolean(true);
		gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);
	}
	else
	{
		Scaleform::GFx::Value var[2];
		var[0].SetString(gLangMngr.getString("InviteSentSuccess"));
		var[1].SetBoolean(true);
		gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);
	}
}

void FrontEnd::eventClanRespondToInvite(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount == 2);
	uint32_t inviteID = args[0].GetUInt();
	bool isAccepted = args[1].GetBool();

	CUserClans* clans = gUserProfile.clans[gUserProfile.SelectedCharID];
	int api = clans->ApiClanAnswerInvite(inviteID, isAccepted);
	// remove this invite from the list
	{
		struct clanInviteSearch
		{
			uint32_t inviteID;

			clanInviteSearch(uint32_t id) : inviteID(id) {};

			bool operator()(const CUserClans::ClanInvite_s &a)
			{
				return a.ClanInviteID == inviteID;
			}
		};

		clanInviteSearch prd(inviteID);
		clans->clanInvites_.erase(std::find_if(clans->clanInvites_.begin(), clans->clanInvites_.end(), prd));
	}
	if (api != 0)
	{
		r3dOutToLog("Failed to accept invite, api=%d\n", api);

		Scaleform::GFx::Value var[2];
		var[0].SetString(gLangMngr.getString("FailAcceptInvite"));
		var[1].SetBoolean(true);
		gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);
	}
	else if (isAccepted)
	{
		setClanInfo();
		gfxFrontEnd.Invoke("_root.api.Main.showScreen", "MyClan");
		//TheHexa:: BUG FIX: NEED TO CLOSE GAME TO RELOAD INFO
		wiCharDataFull& slot = gUserProfile.ProfileData.ArmorySlots[gUserProfile.SelectedCharID];
		updateClientSurvivor(slot);
	}
	else if (!isAccepted)
	{
		checkForInviteFromClan();
		//TheHexa:: BUG FIX: NEED TO CLOSE GAME TO RELOAD INFO
		wiCharDataFull& slot = gUserProfile.ProfileData.ArmorySlots[gUserProfile.SelectedCharID];
		updateClientSurvivor(slot);
	}
}

void FrontEnd::checkForInviteFromClan()
{
	CUserClans* clans = gUserProfile.clans[gUserProfile.SelectedCharID];

	if (!clans->clanInvites_.empty())
	{
		CUserClans::ClanInvite_s& invite = clans->clanInvites_[0];
		//		public function showClanInvite(inviteID:uint, clanName:String, numMembers:uint, desc:String, iconID:uint)
		Scaleform::GFx::Value var[5];
		var[0].SetUInt(invite.ClanInviteID);
		var[1].SetString(invite.ClanName.c_str());
		var[2].SetUInt(0); // todo: need data
		var[3].SetString(""); // todo: need data
		var[4].SetUInt(invite.ClanEmblemID);
		gfxFrontEnd.Invoke("_root.api.showClanInvite", var, 5);
	}
}

void FrontEnd::eventClanBuySlots(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount == 1);
	uint32_t buyIdx = args[0].GetUInt();

	CUserClans* clans = gUserProfile.clans[gUserProfile.SelectedCharID];

	int api = clans->ApiClanBuyAddMembers(buyIdx);
	if (api != 0)
	{
		r3dOutToLog("Failed to buy slots, api=%d\n", api);

		Scaleform::GFx::Value var[2];
		var[0].SetString(gLangMngr.getString("FailToBuyMoreSlots"));
		var[1].SetBoolean(true);
		gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);
		return;
	}

	gfxFrontEnd.SetVariable("_root.api.Main.ClansMyClan.MyClan.OptionsBlock2.Slots.text", clans->clanInfo_.MaxClanMembers);
}

void FrontEnd::eventClanApplyToJoin(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount == 2);
	uint32_t clanID = args[0].GetUInt();
	const char* applText = args[1].GetString();
	CUserClans* clans = gUserProfile.clans[gUserProfile.SelectedCharID];

	wiCharDataFull& slot = gUserProfile.ProfileData.ArmorySlots[gUserProfile.SelectedCharID];
	if (slot.ClanID != 0)
	{
		Scaleform::GFx::Value var[2];
		var[0].SetString(gLangMngr.getString("YouAreAlreadyInClan"));
		var[1].SetBoolean(true);
		gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);
		return;
	}

	int api = clans->ApiClanApplyToJoin(clanID, applText);
	if (api != 0)
	{
		r3dOutToLog("Failed to apply to clan, api=%d\n", api);

		Scaleform::GFx::Value var[2];
		var[0].SetString(gLangMngr.getString("FailApplyToClan"));
		var[1].SetBoolean(true);
		gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);
	}
	else
	{
		//TheHexa:: BUG FIX: NEED TO CLOSE GAME TO RELOAD INFO
		wiCharDataFull& slot = gUserProfile.ProfileData.ArmorySlots[gUserProfile.SelectedCharID];
		updateClientSurvivor(slot);

		Scaleform::GFx::Value var[2];
		var[0].SetString(gLangMngr.getString("SuccessApplyToClan"));
		var[1].SetBoolean(true);
		gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);
	}
}

static int GetGPConversionRateFromPrice(int price)
{
	for (int i = 0; i < 32; i++)
	{
		if (price <= gUserProfile.m_GCPriceTable[i * 2 + 0])
			return gUserProfile.m_GCPriceTable[i * 2 + 1];
	}

	return 0;
}

void FrontEnd::eventStorePurchaseGPRequest(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount == 0);
	//if(gUserProfile.IsRussianAccount() || gUserProfile.ProfileData.AccountType == 21)
	//{
	//Scaleform::GFx::Value vars[1];
	//vars[0].SetInt(0);
	//eventStorePurchaseGP(pMovie, vars, 1);
	//}
	//else
	//{
	gfxFrontEnd.Invoke("_root.api.Main.PurchaseGC.showPopUp", 1); // 1 - to indicate that we want GC dialog, otherwise it will be GD dialog
	//}
}

void FrontEnd::eventStorePurchaseGP(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount == 1);

	int priceInCents = args[0].GetInt();
	//int howMuchGC = priceInCents * GetGPConversionRateFromPrice(priceInCents) / 100; // calc in cents to avoid rounding issues
	int howMuchUSD = priceInCents / 100; // calc in cents to avoid rounding issues

	// web site purchase
	{
		// open browser with web store open
		//char authKey[128];
		//gUserProfile.GenerateSessionKey(authKey);
		char url[256];
		char sessionId[128];
		sprintf(sessionId, "%d", gUserProfile.SessionID);
		sprintf(url, "https://zombie-hunters.com/buy.php?session=%s&amount=%d", sessionId, howMuchUSD);
		ShellExecute(NULL, "open", url, "", NULL, SW_SHOW);
	}

	// minimize our window
	ShowWindow(win::hWnd, SW_MINIMIZE);
}

void FrontEnd::eventStorePurchaseGD(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount == 1);
	m_buyGDForGC = args[0].GetInt();

	Scaleform::GFx::Value var[2];
	var[0].SetString(gLangMngr.getString("OneMomentPlease"));
	var[1].SetBoolean(false);
	gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);

	StartAsyncOperation(&FrontEnd::as_BuyGDForGCThread, &FrontEnd::OnBuyGDForGCSuccess);
}

unsigned int FrontEnd::as_BuyGDForGCThread(void* in_data)
{
	r3dThreadAutoInstallCrashHelper crashHelper;
	FrontEnd* This = (FrontEnd*)in_data;

	This->DelayServerRequest();

	int apiCode = gUserProfile.ApiConvertGamePoints(This->m_buyGDForGC);
	if (apiCode != 0)
	{
		This->SetAsyncError(apiCode, gLangMngr.getString("FailedToConvertGCtoGD"));
		return 0;
	}

	return 1;
}

void FrontEnd::OnBuyGDForGCSuccess()
{
	gfxFrontEnd.Invoke("_root.api.hideInfoMsg", "");

	Scaleform::GFx::Value var[1];
	var[0].SetInt(gUserProfile.ProfileData.GamePoints);
	gfxFrontEnd.Invoke("_root.api.setGC", var, 1);

	var[0].SetInt(gUserProfile.ProfileData.GameDollars);
	gfxFrontEnd.Invoke("_root.api.setDollars", var, 1);

	var[0].SetInt(gUserProfile.ProfileData.XP50Boosts);
	gfxFrontEnd.Invoke("_root.api.setXPBoosts", var, 1);
}


void FrontEnd::eventStorePurchaseGDCallback(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount == 1);
	int priceInGC = args[0].GetInt();

	int conv = 0;
	int GDResult = gUserProfile.GetConvertedGP2GD(priceInGC, conv);

	char tmpStr[64];
	sprintf(tmpStr, "1GC = %d", conv);

	gfxFrontEnd.SetVariable("_root.api.Main.PurchaseGC.Bar.Rate.text", tmpStr);

	gfxFrontEnd.Invoke("_root.api.Main.PurchaseGC.setGCValue", GDResult);
}

void FrontEnd::eventStorePurchaseGPCallback(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount == 1);
	int priceInCents = args[0].GetInt();

	int conv = GetGPConversionRateFromPrice(priceInCents);

	char tmpStr[64];
	sprintf(tmpStr, "1 USD = %d GC", conv);

	gfxFrontEnd.SetVariable("_root.api.Main.PurchaseGC.Bar.Rate.text", tmpStr);

	int howMuchGC = priceInCents * conv / 100; // calc in cents to avoid rounding issues
	gfxFrontEnd.Invoke("_root.api.Main.PurchaseGC.setGCValue", howMuchGC);
}

unsigned int WINAPI FrontEnd::as_SteamBuyGPThread(void* in_data)
{
	r3dThreadAutoInstallCrashHelper crashHelper;
	FrontEnd* This = (FrontEnd*)in_data;

	int apiCode = gUserProfile.ApiSteamStartBuyGP(This->m_buyGpPriceCents);
	if (apiCode != 0)
	{
		if (apiCode == 7)
		{
			This->SetAsyncError(0, "Your purchase could not be completed because it looks like the currency of funds in your Steam Wallet does not match the currency of this purchase (USD)");
			return 0;
		}

		This->SetAsyncError(0, gLangMngr.getString("BuyItemFail"));
		return 0;
	}

	// wait for authorization result - for 10min and pray that callback successfully passed to client
	const float endTime = r3dGetTime() + 600.0f;
	while (true)
	{
		::Sleep(10);
		if (gUserProfile.steamAuthResp.gotResp)
			break;

		// if we didn't got steam callback
		if (r3dGetTime() > endTime) {
			This->SetAsyncError(0, gLangMngr.getString("BuyItemFail"));
			return 0;
		}
	}

	// if authorization was denied, just silently quit
	r3d_assert(gUserProfile.steamAuthResp.gotResp);
	if (gUserProfile.steamAuthResp.bAuthorized != 1)
		return 1;

	// auth ok, finish transaction.
	apiCode = gUserProfile.ApiSteamFinishBuyGP(gUserProfile.steamAuthResp.ulOrderID);
	if (apiCode != 0)
	{
		This->SetAsyncError(0, gLangMngr.getString("BuyItemFail"));
		return 0;
	}

	return 1;
}

void FrontEnd::OnSteamBuyGPSuccess()
{
	gfxFrontEnd.Invoke("_root.api.hideInfoMsg", "");

	Scaleform::GFx::Value var[1];
	var[0].SetInt(gUserProfile.ProfileData.GamePoints);
	gfxFrontEnd.Invoke("_root.api.setGC", var, 1);
	return;
}

void FrontEnd::eventRequestLeaderboardData(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount == 3);
	int boardType = args[0].GetInt(); // 0-soft, 1-hard
	int boardSelect = args[1].GetInt(); // which board to display, starts with 0
	int mode = args[2].GetInt(); // 0 - default list when you activate leaderboard, 1-top100, 2-move left, 3-move right

	// static is safe here
	static int prevType = 0;

	if (prevType != boardType)
	{
		prevType = boardType;
		leaderboard_requestStartPos = -1; // reset
	}
	if (leaderboard_BoardSelected != boardSelect)
	{
		leaderboard_BoardSelected = boardSelect;
		leaderboard_requestStartPos = -1; // reset
	}
	if (mode == 1)
		leaderboard_requestStartPos = 0;
	else if (mode == 0)
		leaderboard_requestStartPos = -1;
	else if (mode == 2)
		leaderboard_requestStartPos = R3D_CLAMP(leaderboard_requestStartPos - 100, 0, leaderboardSize - 100);
	else if (mode == 3)
		leaderboard_requestStartPos = R3D_CLAMP(leaderboard_requestStartPos + 100, 0, leaderboardSize - 100);

	leaderboard_requestTableID = boardSelect + boardType * 50;

	Scaleform::GFx::Value var[2];
	var[0].SetString(gLangMngr.getString("OneMomentPlease"));
	var[1].SetBoolean(false);
	gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);

	StartAsyncOperation(&FrontEnd::as_RequestLeaderboardThread, &FrontEnd::OnRequestLeaderboardSuccess);
}

unsigned int FrontEnd::as_RequestLeaderboardThread(void* in_data)
{
	r3dThreadAutoInstallCrashHelper crashHelper;
	FrontEnd* This = (FrontEnd*)in_data;

	int outPos = 0;
	int lbSize = 0;
	int apiCode = gUserProfile.ApiGetLeaderboard(This->leaderboard_requestTableID, This->leaderboard_requestStartPos, &outPos, &lbSize);
	if (apiCode != 0)
	{
		This->SetAsyncError(apiCode, gLangMngr.getString("FailedLeaderboardData"));
		return 0;
	}
	if (lbSize)
		This->leaderboardSize = lbSize;

	if (This->leaderboard_requestStartPos == -1)
		This->leaderboard_requestStartPos = 0; // for proper pos index

	return 1;
}

void FrontEnd::OnRequestLeaderboardSuccess()
{
	gfxFrontEnd.Invoke("_root.api.hideInfoMsg", "");

	gfxFrontEnd.Invoke("_root.api.Main.LeaderboardAnim.clearLeaderboardList", "");
	{
		Scaleform::GFx::Value args[4];
		char tmpStr[32] = { 0 };
		char tmpPlrName[256] = { 0 };
		for (size_t i = 0; i < gUserProfile.m_lbData[leaderboard_requestTableID].size(); ++i)
		{
			args[0].SetInt(leaderboard_requestStartPos + i + 1);
			if (gUserProfile.m_lbData[leaderboard_requestTableID][i].ClanID > 0)
			{
				const char* tagColor = "#000000";
				int ClanTagColor = gUserProfile.m_lbData[leaderboard_requestTableID][i].ClanTagColor;
				switch (ClanTagColor)
				{
				case 1: tagColor = "#aa0000"; break;
				case 2: tagColor = "#a6780c"; break;
				case 3: tagColor = "#10a49e"; break;
				case 4: tagColor = "#20a414"; break;
				case 5: tagColor = "#104ea4"; break;
				case 6: tagColor = "#9610a4"; break;
				case 7: tagColor = "#444444"; break;
				case 8: tagColor = "#a4a4a4"; break;
				default:
					break;
				}
				sprintf(tmpPlrName, "<font color='%s'>[%s]</font> <font color='#D42F2F'>%s</font>", tagColor, gUserProfile.m_lbData[leaderboard_requestTableID][i].ClanTag, gUserProfile.m_lbData[leaderboard_requestTableID][i].gamertag);
			}
			else
				sprintf(tmpPlrName, "<font color='#D42F2F'>%s</font>", gUserProfile.m_lbData[leaderboard_requestTableID][i].gamertag);
			args[1].SetString(tmpPlrName);
			args[2].SetBoolean(gUserProfile.m_lbData[leaderboard_requestTableID][i].Alive > 0);
			switch (leaderboard_BoardSelected)
			{
			case 0:
				sprintf(tmpStr, "%d", gUserProfile.m_lbData[leaderboard_requestTableID][i].stats.XP);
				break;
			case 1:
				r3dscpy(tmpStr, getTimePlayedString(gUserProfile.m_lbData[leaderboard_requestTableID][i].stats.TimePlayed));
				break;
			case 2:
				sprintf(tmpStr, "%d", gUserProfile.m_lbData[leaderboard_requestTableID][i].stats.KilledZombies);
				break;
			case 3:
				sprintf(tmpStr, "%d", gUserProfile.m_lbData[leaderboard_requestTableID][i].stats.KilledSurvivors);
				break;
			case 4:
				sprintf(tmpStr, "%d", gUserProfile.m_lbData[leaderboard_requestTableID][i].stats.KilledBandits);
				break;
			case 5:
			case 6:
				sprintf(tmpStr, "%d", gUserProfile.m_lbData[leaderboard_requestTableID][i].stats.Reputation);
				break;
			case 7:
				sprintf(tmpStr, "%d", gUserProfile.m_lbData[leaderboard_requestTableID][i].stats.VictorysBattleRoyale); //AlexRedd:: BR mode
				break;
			default:
				r3d_assert(false);
				break;
			}

			args[3].SetString(tmpStr);
			gfxFrontEnd.Invoke("_root.api.Main.LeaderboardAnim.addLeaderboardData", args, 4);
		}
	}

	switch (leaderboard_BoardSelected)
	{
	case 0:
		gfxFrontEnd.Invoke("_root.api.Main.LeaderboardAnim.setLeaderboardText", gLangMngr.getString("$FR_LB_TOP_XP"));
		break;
	case 1:
		gfxFrontEnd.Invoke("_root.api.Main.LeaderboardAnim.setLeaderboardText", gLangMngr.getString("$FR_LB_TOP_ST"));
		break;
	case 2:
		gfxFrontEnd.Invoke("_root.api.Main.LeaderboardAnim.setLeaderboardText", gLangMngr.getString("$FR_LB_TOP_KZ"));
		break;
	case 3:
		gfxFrontEnd.Invoke("_root.api.Main.LeaderboardAnim.setLeaderboardText", gLangMngr.getString("$FR_LB_TOP_KS"));
		break;
	case 4:
		gfxFrontEnd.Invoke("_root.api.Main.LeaderboardAnim.setLeaderboardText", gLangMngr.getString("$FR_LB_TOP_KB"));
		break;
	case 5:
	case 6:
		gfxFrontEnd.Invoke("_root.api.Main.LeaderboardAnim.setLeaderboardText", gLangMngr.getString("$FR_LB_TOP_RT"));
		break;
	case 7:
		gfxFrontEnd.Invoke("_root.api.Main.LeaderboardAnim.setLeaderboardText", gLangMngr.getString("$FR_LB_TOP_BR")); //AlexRedd:: BR mode
		break;
	default:
		r3d_assert(false);
		break;
	}

	gfxFrontEnd.Invoke("_root.api.Main.LeaderboardAnim.populateLeaderboard", "");
}

void FrontEnd::eventRequestGCTransactionData(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount == 0);

	Scaleform::GFx::Value var[2];
	var[0].SetString(gLangMngr.getString("OneMomentPlease"));
	var[1].SetBoolean(false);
	gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);

	StartAsyncOperation(&FrontEnd::as_RequestGCTransactionThread, &FrontEnd::OnRequestGCTransactionSuccess);
}

unsigned int FrontEnd::as_RequestGCTransactionThread(void* in_data)
{
	r3dThreadAutoInstallCrashHelper crashHelper;
	FrontEnd* This = (FrontEnd*)in_data;

	This->DelayServerRequest();

	int apiCode = gUserProfile.ApiGetGPTransactions();
	if (apiCode != 0)
	{
		This->SetAsyncError(apiCode, gLangMngr.getString("FailedToGetGCTransactions"));
		return 0;
	}

	return 1;
}

void FrontEnd::OnRequestGCTransactionSuccess()
{
	gfxFrontEnd.Invoke("_root.api.hideInfoMsg", "");

	// update GamePoints in UI
	{
		Scaleform::GFx::Value var[1];
		var[0].SetInt(gUserProfile.ProfileData.GamePoints);
		gfxFrontEnd.Invoke("_root.api.setGC", var, 1);
	}

	// fill log
	for (size_t i = 0; i < gUserProfile.GPLog_.size(); i++)
	{
		const CClientUserProfile::GPLog_s& gl = gUserProfile.GPLog_[i];

		Scaleform::GFx::Value args[5];

		char strAmount[64];
		sprintf(strAmount, "%+d", gl.Amount);

		struct tm* tm = _gmtime32(&gl.Time);
		char strTime[64];
		sprintf(strTime, "%d/%d/%d", tm->tm_mon + 1, tm->tm_mday, 1900 + tm->tm_year);

		char strBalance[64];
		sprintf(strBalance, "%d", gl.Previous + gl.Amount);

		char strDesc[256 * 2] = "";
		r3dscpy(strDesc, gl.Description.c_str());

		// shop override, display bought item
		if (strncmp(gl.Description.c_str(), "Shop: ", 6) == 0)
		{
			int ItemID = atoi(gl.Description.c_str() + 6);
			const BaseItemConfig* itemCfg = g_pWeaponArmory->getConfig(ItemID);
			if (itemCfg)
			{
				if (ItemID == 301159 || ItemID == 301257 || ItemID == 301399) // early revival item or premium acc
					sprintf(strDesc, "%s", itemCfg->m_StoreName);
				else
					sprintf(strDesc, "Marketplace: %s", itemCfg->m_StoreName);
			}
		}

		//public function addTransactionData(id:int, date:String, name:String, amount:String, balance:String)
		args[0].SetInt(gl.TransactionID);
		args[1].SetString(strTime);
		args[2].SetString(strDesc);
		args[3].SetString(strAmount);
		args[4].SetString(strBalance);

		gfxFrontEnd.Invoke("_root.api.Main.Marketplace.addTransactionData", args, 5);
	}

	gUserProfile.GPLog_.clear();

	gfxFrontEnd.Invoke("_root.api.Main.Marketplace.showTransactionsPopup", "");
}

//AlexRedd:: teleport system
unsigned int FrontEnd::as_RequestTeleportThread(void* in_data)
{
	r3dThreadAutoInstallCrashHelper crashHelper;
	FrontEnd* This = (FrontEnd*)in_data;

	This->DelayServerRequest();

	int apiCode = gUserProfile.ApiGetTeleportPoints();
	if (apiCode != 0)
	{
		This->SetAsyncError(apiCode, gLangMngr.getString("FailedToGetTeleportPoints"));
		return 0;
	}

	return 1;
}

void FrontEnd::OnRequestTeleportSuccess()
{
	gfxFrontEnd.Invoke("_root.api.hideInfoMsg", "");

	wiCharDataFull& w = gUserProfile.ProfileData.ArmorySlots[gUserProfile.SelectedCharID];

	// we need to load level xml to read minimap size...
	r3dPoint3D minimapOrigin(0, 0, 0), minimapSize(0, 0, 0);
	{
		char fname[MAX_PATH];
		sprintf(fname, "Levels\\%s\\LevelData.xml", getLevelDirBasedOnLevelID(w.GameMapId));
		r3d_assert(r3dFileExists(fname));

		pugi::xml_document xmlLevelFile;
		char *levelData = 0;
		ParseXMLFile(fname, xmlLevelFile, levelData);

		pugi::xml_node xmlLevel = xmlLevelFile.child("level");

		minimapOrigin.x = xmlLevel.attribute("minimapOrigin.x").as_float();
		minimapOrigin.z = xmlLevel.attribute("minimapOrigin.z").as_float();
		minimapSize.x = xmlLevel.attribute("minimapSize.x").as_float();
		minimapSize.z = xmlLevel.attribute("minimapSize.z").as_float();

		delete[] levelData;
	}

	// fill teleport log
	for (size_t i = 0; i < gUserProfile.TeleportLog_.size(); i++)
	{
		const CClientUserProfile::TeleportLog_s& tlp = gUserProfile.TeleportLog_[i];

		r3dPoint3D Pos = r3dPoint3D(tlp.TeleportPos.x, tlp.TeleportPos.y, tlp.TeleportPos.z);
		r3dPoint2D mapPos = getMinimapPosWithExternalSize(Pos, minimapOrigin, minimapSize);
		if (w.GamePos != Pos)
		{
			Scaleform::GFx::Value var[5];
			var[0].SetNumber(mapPos.x);
			var[1].SetNumber(mapPos.y);
			var[2].SetNumber(Pos.x);
			var[3].SetNumber(Pos.y);
			var[4].SetNumber(Pos.z);
			gfxFrontEnd.Invoke("_root.api.Main.SurvivorsAnim.addTeleportPointToMap", var, 5);
		}
	}
	gUserProfile.TeleportLog_.clear();

	for (int i = 0; i < wiUserProfile::MAX_LOADOUT_SLOTS; ++i)
	{
		wiCharDataFull& w2 = gUserProfile.ProfileData.ArmorySlots[i];
		if (w2.Alive == 1 && w2.GameMapId == w.GameMapId)
		{
			r3dPoint2D mapPos = getMinimapPosWithExternalSize(w2.GamePos, minimapOrigin, minimapSize);
			Scaleform::GFx::Value var[3];
			var[0].SetNumber(mapPos.x);
			var[1].SetNumber(mapPos.y);
			var[2].SetString(w2.Gamertag);
			gfxFrontEnd.Invoke("_root.api.Main.SurvivorsAnim.addSurvivorPinToMap", var, 3);
		}
	}
}
//////////////////////////////////

void FrontEnd::eventRequestLotteryData(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount == 0);

	Scaleform::GFx::Value var[2];
	var[0].SetString(gLangMngr.getString("OneMomentPlease"));
	var[1].SetBoolean(false);
	gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);

	StartAsyncOperation(&FrontEnd::as_RequestLotteryThread, &FrontEnd::OnRequestLotterySuccess);
}

unsigned int FrontEnd::as_RequestLotteryThread(void* in_data)
{
	r3dThreadAutoInstallCrashHelper crashHelper;
	FrontEnd* This = (FrontEnd*)in_data;

	This->DelayServerRequest();

	int apiCode = gUserProfile.ApiGetLottery();
	if (apiCode != 0)
	{
		This->SetAsyncError(apiCode, gLangMngr.getString("FailedToGetLottery"));
		return 0;
	}

	return 1;
}

void FrontEnd::OnRequestLotterySuccess()
{
	gfxFrontEnd.Invoke("_root.api.hideInfoMsg", "");

	// fill Lottery log
	for (size_t i = 0; i < gUserProfile.LotLog_.size(); i++)
	{
		const CClientUserProfile::LotLog_s& gl = gUserProfile.LotLog_[i];

		Scaleform::GFx::Value args[5];

		struct tm* tm = _gmtime32(&gl.Time);
		char strTime[64];
		sprintf(strTime, "%d/%d/%d", tm->tm_mon + 1, tm->tm_mday, 1900 + tm->tm_year);

		char strUID[256] = "";
		r3dscpy(strUID, gl.UserID.c_str());
		char Bank[64] = { 0 };
		sprintf(Bank, "%s <font color = \"#3DB7E5\">%d GD</font>", gLangMngr.getString("$FR_TOTAL_BANK"), gl.Bank);
		char strChance[32] = "";
		sprintf(strChance, "~ %s %%", gl.Chance.c_str());

		args[0].SetInt(gl.ID);
		args[1].SetString(strTime);
		args[2].SetString(strUID);
		args[3].SetInt(gl.CountTickets);
		args[4].SetString(strChance);
		gfxFrontEnd.Invoke("_root.api.Main.Marketplace.addLotteryData", args, 5);
		gfxFrontEnd.SetVariable("_root.api.Main.Marketplace.Marketplace.PopupLottery.Bank.Text.htmlText", Bank);
		//r3dOutToLog("!!!! addLotteryData: %d %s %s\n", gl.ID, strTime, strUID);
	}
	gUserProfile.LotLog_.clear();

	// fill Lottery winners log
	for (size_t i = 0; i < gUserProfile.WinLog_.size(); i++)
	{
		const CClientUserProfile::WinLog_s& wl = gUserProfile.WinLog_[i];

		Scaleform::GFx::Value args[3];

		struct tm* tm = _gmtime32(&wl.Time);
		char strTime[64];
		sprintf(strTime, "<font color=\"#777777\">%d/%d/%d </font>", tm->tm_mon + 1, tm->tm_mday, 1900 + tm->tm_year);

		char strWUID[256] = "";
		r3dscpy(strWUID, wl.WUserID.c_str());

		char strJackpot[64];
		sprintf(strJackpot, "<font color=\"#0da5df\"> +%d</font>", wl.Jackpot);

		args[0].SetString(strTime);
		args[1].SetString(strWUID);
		args[2].SetString(strJackpot);
		gfxFrontEnd.Invoke("_root.api.Main.Marketplace.addWinLotteryData", args, 3);
		//r3dOutToLog("!!!! addWinLotteryData: %s %s %d\n", strTime, strWUID, wl.Jackpot);
	}
	gUserProfile.WinLog_.clear();

	gfxFrontEnd.Invoke("_root.api.Main.Marketplace.showLotteryPopup", "");
}

void FrontEnd::eventRequestMyServerList(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount == 0);

	Scaleform::GFx::Value var[2];
	var[0].SetString(gLangMngr.getString("OneMomentPlease"));
	var[1].SetBoolean(false);
	gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);

	StartAsyncOperation(&FrontEnd::as_MyServerListThread, &FrontEnd::OnMyServerListSuccess);
}

unsigned int WINAPI FrontEnd::as_MyServerListThread(void* in_data)
{
	r3dThreadAutoInstallCrashHelper crashHelper;
	FrontEnd* This = (FrontEnd*)in_data;

	// grab server prices
	int apiCode = gUserProfile.serverRent->ApiGetServerRentPrices();
	if (apiCode != 0)
	{
		This->SetAsyncError(0, gLangMngr.getString("ServerRent_FailGetPrice"));
		return 0;
	}

	// grab my servers
	apiCode = gUserProfile.serverRent->ApiGetServersList();
	if (apiCode != 0)
	{
		This->SetAsyncError(0, gLangMngr.getString("ServerRent_FailGetList"));
		return 0;
	}

	// ask masterserver about each server
	for (size_t i = 0; i < gUserProfile.serverRent->serverList.size(); i++)
	{
		This->DelayServerRequest();
		if (!This->ConnectToMasterServer())
			return 0;

		CUserServerRental::myServer_s& srv = gUserProfile.serverRent->serverList[i];

		gMasterServerLogic.SendMyServerGetInfo(srv.ginfo.gameServerId, srv.AdminKey);

		const float endTime = r3dGetTime() + 10.0f;
		while (r3dGetTime() < endTime)
		{
			::Sleep(10);

			if (!gMasterServerLogic.IsConnected())
				return 0;

			if (gMasterServerLogic.serverInfoAnswered_)
			{
				srv.status = gMasterServerLogic.serverInfoAnswer_.status;
				srv.curPlayers = (int)gMasterServerLogic.serverInfoPlayers_.size();
				break;
			}
		}
	}

	return 1;
}

void FrontEnd::OnMyServerListSuccess()
{
	gfxFrontEnd.Invoke("_root.api.hideInfoMsg", "");

	static bool shownSetupServerInfo = false;
	bool needtoShowSetupServerInfo = false;

	// fill server list
	for (size_t i = 0; i < gUserProfile.serverRent->serverList.size(); i++)
	{
		const CUserServerRental::myServer_s& srv = gUserProfile.serverRent->serverList[i];

		// addServerInfo(0, "first", "east", "us", "online", 50, 100, "99D 45H 53M", "GAME SERVER", "COLORADO");
		// addServerInfo(serverID:uint, name:String, location:String, region:String, status:String, playersOnline:uint, maxPlayers:uint, timeLeft:String, type:String, map:String)

		const char* location = "";
		const char* region = "??";

		if (srv.ginfo.region == GBNET_REGION_US_West)
			region = "US";
		else if (srv.ginfo.region == GBNET_REGION_Europe)
			region = "EU";
		else if (srv.ginfo.region == GBNET_REGION_Russia)
			region = "RU";
		else if (srv.ginfo.region == GBNET_REGION_SouthAmerica)
			region = "SA";

		char timeLeft[64];
		bool serverExpired = false;
		if (srv.WorkHours >= srv.RentHours)
		{
			sprintf(timeLeft, gLangMngr.getString("ServerStatus_Expired"));
			serverExpired = true;
		}
		else
			sprintf(timeLeft, gLangMngr.getString("ServerRental_TimeLeft"), (srv.RentHours - srv.WorkHours) / 24, (srv.RentHours - srv.WorkHours) % 24);

		const char* status = gLangMngr.getString("ServerStatus_Unknown");
		switch (srv.status)
		{
		case 1: status = gLangMngr.getString("ServerStatus_Pending"); if (!serverExpired) needtoShowSetupServerInfo = true; break;
		case 2: //FAKE: all servers are always ONLINE. 
		case 3: status = gLangMngr.getString("ServerStatus_Online"); break;
		case 4: status = gLangMngr.getString("ServerStatus_Starting"); break;
		}

		if (serverExpired)
			status = gLangMngr.getString("ServerStatus_Offline");

		const char* mapName = getMapName(srv.ginfo.mapId);

		Scaleform::GFx::Value vars[11];
		vars[0].SetInt(srv.ginfo.gameServerId);
		vars[1].SetString(srv.ginfo.name);
		vars[2].SetString(location);
		vars[3].SetString(region);
		vars[4].SetString(status);
		vars[5].SetInt(srv.curPlayers);
		vars[6].SetInt(srv.ginfo.maxPlayers);
		vars[7].SetString(timeLeft);
		//vars[8].SetString(srv.ginfo.IsGameworld() ? gLangMngr.getString("MapType_Gameworld") : gLangMngr.getString("MapType_Stronghold"));
		const char* serverType;
		if (gClientLogic().m_gameInfo.channel == 5)
			serverType = gLangMngr.getString("MapType_Stronghold");
		else if (gClientLogic().m_gameInfo.IsDevEvent())//AlexRedd:: Event mode	
			serverType = gLangMngr.getString("MapType_DevEvent");
#ifdef ENABLE_BATTLE_ROYALE
		else if (gClientLogic().m_gameInfo.IsGameBR())	//AlexRedd:: BR mode	
			serverType = gLangMngr.getString("MapType_BR");
#endif //ENABLE_BATTLE_ROYALE
		//else if (gClientLogic().m_gameInfo.channel == 7)//AlexRedd:: Trade mode
		//serverType = gLangMngr.getString("MapType_Trade");
		else
			serverType = gLangMngr.getString("MapType_Gameworld");
		vars[8].SetString(serverType);
		vars[9].SetString(mapName);
		vars[10].SetBoolean(true); // is Renew button visible
		gfxFrontEnd.Invoke("_root.api.Main.PlayGameMyServers.addServerInfo", vars, 11);
	}

	gfxFrontEnd.Invoke("_root.api.Main.PlayGameMyServers.showServerList", "");

	if (needtoShowSetupServerInfo && !shownSetupServerInfo)
	{
		shownSetupServerInfo = true;
		Scaleform::GFx::Value var[2];
		var[0].SetString(gLangMngr.getString("ServerRental_SetupStatusInfo"));
		var[1].SetBoolean(true);
		gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);
	}
}

void FrontEnd::eventRequestMyServerInfo(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount == 1);
	m_myGameServerId = args[0].GetUInt();

	const CUserServerRental::myServer_s* srv = gUserProfile.serverRent->GetServerById(m_myGameServerId);
	m_myGameServerAdminKey = srv->AdminKey;

	Scaleform::GFx::Value var[2];
	var[0].SetString(gLangMngr.getString("OneMomentPlease"));
	var[1].SetBoolean(false);
	gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);

	StartAsyncOperation(&FrontEnd::as_MyServerInfoThread, &FrontEnd::OnMyServerInfoSuccess);
}

unsigned int WINAPI FrontEnd::as_MyServerInfoThread(void* in_data)
{
	r3dThreadAutoInstallCrashHelper crashHelper;
	FrontEnd* This = (FrontEnd*)in_data;

	This->DelayServerRequest();
	if (!This->ConnectToMasterServer())
		return 0;

	gMasterServerLogic.SendMyServerGetInfo(This->m_myGameServerId, This->m_myGameServerAdminKey);

	const float endTime = r3dGetTime() + 60.0f;
	while (r3dGetTime() < endTime)
	{
		::Sleep(10);

		if (!gMasterServerLogic.IsConnected())
			return 0;

		if (gMasterServerLogic.serverInfoAnswered_)
			return 1;
	}

	This->SetAsyncError(0, gLangMngr.getString("FailedReceiveGameList"));
	return 0;
}

void FrontEnd::OnMyServerInfoSuccess()
{
	gfxFrontEnd.Invoke("_root.api.hideInfoMsg", "");

	// add all players
	for (size_t i = 0; i < gMasterServerLogic.serverInfoPlayers_.size(); i++)
	{
		const GBPKT_M2C_MyServerAddPlayer_s& plr = gMasterServerLogic.serverInfoPlayers_[i];

		// addPlayerInfo(name:String, status:String, rep:String, xp:int)
		Scaleform::GFx::Value vars[4];
		vars[0].SetString(plr.gamertag);
		vars[1].SetString(gLangMngr.getString("PlayerStatus_Alive"));
		vars[2].SetString(getReputationString(plr.reputation));
		vars[3].SetInt(plr.XP);
		gfxFrontEnd.Invoke("_root.api.Main.PlayGameMyServers.addPlayerInfo", vars, 4);
	}
	gfxFrontEnd.Invoke("_root.api.Main.PlayGameMyServers.showPlayerList", "");

	const CUserServerRental::myServer_s* srv = gUserProfile.serverRent->GetServerById(m_myGameServerId);

	// update player count in server info
	{
		Scaleform::GFx::Value vars[2];
		vars[0].SetUInt(m_myGameServerId);
		vars[1].SetUInt(gMasterServerLogic.serverInfoPlayers_.size());
		gfxFrontEnd.Invoke("_root.api.Main.PlayGameMyServers.updatePlayerOnlineForServerID", vars, 2);
	}

	{
		Scaleform::GFx::Value var[9];
		var[0].SetBoolean(true);
		var[1].SetInt((srv->ginfo.flags & GBGameInfo::SFLAGS_Nameplates) ? 1 : 0); // nameplate
		var[2].SetInt((srv->ginfo.flags & GBGameInfo::SFLAGS_CrossHair) ? 1 : 0); // crosshair
		var[3].SetInt((srv->ginfo.flags & GBGameInfo::SFLAGS_Tracers) ? 1 : 0); // tracers
		var[4].SetUInt(srv->ginfo.gameTimeLimit); // gametime limit, if any
		var[5].SetInt((srv->ginfo.flags & GBGameInfo::SFLAGS_TrialsAllowed) ? 1 : 0); // trials
		var[6].SetInt((srv->ginfo.flags & GBGameInfo::SFLAGS_DisableASR) ? 1 : 0);
		var[7].SetInt((srv->ginfo.flags & GBGameInfo::SFLAGS_DisableSNP) ? 1 : 0);
		var[8].SetInt((srv->ginfo.flags & GBGameInfo::SFLAGS_PVE) ? 1 : 0);
		gfxFrontEnd.Invoke("_root.api.Main.PlayGameMyServers.setServerInfoSettingsVisibility", var, 9);
	}

	gfxFrontEnd.SetVariable("_root.api.Main.PlayGameMyServers.MyServers.Settings.Password.text", srv->pwd);
}

void FrontEnd::eventMyServerKickPlayer(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount == 2);
	uint32_t serverID = args[0].GetUInt();
	const char* plrName = args[1].GetString();

	m_myKickCharID = 0;
	for (size_t i = 0; i < gMasterServerLogic.serverInfoPlayers_.size(); i++)
	{
		const GBPKT_M2C_MyServerAddPlayer_s& plr = gMasterServerLogic.serverInfoPlayers_[i];
		if (strcmp(plrName, plr.gamertag) == 0)
		{
			m_myKickCharID = plr.CharID;
			break;
		}
	}
	r3d_assert(m_myKickCharID);

	Scaleform::GFx::Value var[2];
	var[0].SetString(gLangMngr.getString("OneMomentPlease"));
	var[1].SetBoolean(false);
	gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);

	StartAsyncOperation(&FrontEnd::as_MyServerKickPlayerThread, &FrontEnd::OnMyServerKickPlayerSuccess);
}

unsigned int WINAPI FrontEnd::as_MyServerKickPlayerThread(void* in_data)
{
	r3dThreadAutoInstallCrashHelper crashHelper;
	FrontEnd* This = (FrontEnd*)in_data;

	This->DelayServerRequest();
	if (!This->ConnectToMasterServer())
		return 0;

	gMasterServerLogic.SendMyServerKickPlayer(This->m_myGameServerId, This->m_myGameServerAdminKey, This->m_myKickCharID);

	// there will be no answer, so just wait a few seconds and return to reload server info
	::Sleep(1000);
	return 1;
}

void FrontEnd::OnMyServerKickPlayerSuccess()
{
	gfxFrontEnd.Invoke("_root.api.hideInfoMsg", "");
}

//
// WARNING: following price functions must be in sync with backend api_ServersMgr.aspx calcServerRentPrice
//
static int getServerRentOptionPrice(int base, int opt)
{
	// get percent of base price
	int add = base * opt / 100;
	return add;
}

static int calcServerRentPrice(const Scaleform::GFx::Value* args)
{
	//isGameServer:int, mapID:int, regionID:int, slotID:int, rentID:int, name:String, password:String, pveID:int, nameplates:int, crosshair:int, tracers:int
	int isGameServer = args[0].GetInt();
	int mapID = args[1].GetInt();
	int regionID = args[2].GetInt();
	int slotID = args[3].GetInt();
	int rentID = args[4].GetInt();
	const char* name = args[5].GetString();
	const char* password = args[6].GetString();
	int pveID = args[7].GetInt();
	int nameplates = args[8].GetInt();
	int crosshair = args[9].GetInt();
	int tracers = args[10].GetInt();

	CUserServerRental::priceOps_s& opt = isGameServer ? gUserProfile.serverRent->optGameServer : gUserProfile.serverRent->optStronghold;

	// calc base
	int base = 0;
	switch (regionID)
	{
	case 1:  base = opt.Base_US; break;
	case 10: base = opt.Base_EU; break;
	case 20: base = opt.Base_RU; break;
	case 30: base = opt.Base_SA; break;
	default: r3dError("bad regionID");
	}
	if (base == 0) return 0;

	// calc all other options
	int price = base;

	//[TH] price is set for slot, not by percents
	switch (slotID)
	{
	case 0: price = opt.Slot1; break;
	case 1: price = opt.Slot2; break;
	case 2: price = opt.Slot3; break;
	case 3: price = opt.Slot4; break;
	case 4: price = opt.Slot5; break;
	default: r3dError("bad slotID");
	}

	if (*password)
		price += getServerRentOptionPrice(base, opt.Passworded);

	if (pveID)
		price += getServerRentOptionPrice(base, opt.PVE);
	else
		price += getServerRentOptionPrice(base, opt.PVP);

	if (nameplates)
		price += getServerRentOptionPrice(base, opt.OptNameplates);
	if (crosshair)
		price += getServerRentOptionPrice(base, opt.OptCrosshair);
	if (tracers)
		price += getServerRentOptionPrice(base, opt.OptTracers);

	// hourly rent
	if (rentID >= CUserServerRental::HOURLY_RENTID_START)
	{
		// hourly rent is used for renew ONLY.
		// so we can ignore negative prices (not enough capacity)
		// that'll fix rounding issue with API
		if (price < 0) price = -price;

		// 10% add for hourly rent
		price += price / 10;
		price = (int)ceilf((float)price / 24.0f / 31.0f);
		price = (rentID - CUserServerRental::HOURLY_RENTID_START)*price;
		return price;
	}

	// [TH] adjust per day coeff
	switch (rentID)
	{
	case 0: price *= 3; break;
	case 1: price *= 7; break;
	case 2: price *= 15; break;
	case 3: price *= 30; break;
	case 4: price *= 60; break;
	default: r3dError("bad rentID");
	}

	return price;
}

// some weird conversion functions, legacy.
static void gameInfoTo_calcServerRentPrice_Args(Scaleform::GFx::Value* targs, const GBGameInfo& ginfo, int slotID, int rentID)
{
	targs[0].SetInt(ginfo.IsGameworld() ? 1 : 0);	// int isGameServer = args[0].GetInt();
	targs[1].SetInt(ginfo.mapId);			// int mapID        = args[1].GetInt();
	targs[2].SetInt(ginfo.region);			// int regionID     = args[2].GetInt();
	targs[3].SetInt(slotID);			// int slotID       = args[3].GetInt();
	targs[4].SetInt(rentID);			// int rentID       = args[4].GetInt();
	targs[5].SetString("temp_name");		// const char* name
	targs[6].SetString("");				// const char* password = args[6].GetString();
	targs[7].SetInt(ginfo.flags & GBGameInfo::SFLAGS_PVE ? 1 : 0);	// int pveID        = args[7].GetInt();
	targs[8].SetInt(0);				// int nameplates   = args[8].GetInt();
	targs[9].SetInt(0);				// int crosshair    = args[9].GetInt();
	targs[10].SetInt(0);				// int tracers      = args[10].GetInt();
}

static void calcServerRentPrice_toRentParams(Scaleform::GFx::Value* targs, CUserServerRental::rentParams_s& params)
{
	params.isGameServer = targs[0].GetInt();
	params.mapID = targs[1].GetInt();
	params.regionID = GBNET_REGION_US_West; //[TH] targs[2].GetInt();
	params.slotID = targs[3].GetInt();
	params.rentID = targs[4].GetInt();
	r3dscpy(params.name, targs[5].GetString());
	r3dscpy(params.password, targs[6].GetString());
	params.pveID = targs[7].GetInt();
	params.nameplates = targs[8].GetInt();
	params.crosshair = targs[9].GetInt();
	params.tracers = targs[10].GetInt();
}

void FrontEnd::eventRenewServerUpdatePrice(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount == 2);
	//serverIDToRenew:uint, rentID:int
	DWORD gameServerId = args[0].GetUInt();
	int   rentID = args[1].GetInt();

	CUserServerRental::myServer_s* srv = gUserProfile.serverRent->GetServerById(gameServerId);
	r3d_assert(srv);

	// detect slotID from slot numbers
	int slotID = CUserServerRental::GetSlotID(srv->ginfo);
	if (slotID == -1)
	{
		gfxFrontEnd.Invoke("_root.api.Main.RentServerPopup.updateServerPrice", 0);

		// we wasn't able to determine slots
		Scaleform::GFx::Value vars[3];
		vars[0].SetString(gLangMngr.getString("ServerRent_BadRenewSlots"));
		vars[1].SetBoolean(true);
		vars[2].SetString("$ERROR");
		gfxFrontEnd.Invoke("_root.api.showInfoMsg", vars, 3);
		return;
	}

	// fill temporary array to be in sync with calcServerRentPrice arguments
	Scaleform::GFx::Value targs[11];
	gameInfoTo_calcServerRentPrice_Args(targs, srv->ginfo, slotID, rentID);

	// fill params array as in eventRentServer
	CUserServerRental::rentParams_s params;
	calcServerRentPrice_toRentParams(targs, params);

	m_rentServerParams = params;
	m_myGameServerId = gameServerId;
	m_rentServerPrice = calcServerRentPrice(targs);

	// hack: if we do not have server capacity base price will be *negative* here.
	// but we should always renew, so invert and process
	if (m_rentServerPrice < 0) m_rentServerPrice = -m_rentServerPrice;

	if (m_rentServerPrice == 0)
	{
		Scaleform::GFx::Value vars[3];
		vars[0].SetString(gLangMngr.getString("ServerRent_RegionDisabled"));
		vars[1].SetBoolean(true);
		vars[2].SetString("$ERROR");
		gfxFrontEnd.Invoke("_root.api.showInfoMsg", vars, 3);
		return;
	}

	gfxFrontEnd.Invoke("_root.api.Main.RentServerPopup.updateServerPrice", m_rentServerPrice);
}

void FrontEnd::eventRenewServer(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount == 2);

	Scaleform::GFx::Value var[2];
	var[0].SetString(gLangMngr.getString("OneMomentPlease"));
	var[1].SetBoolean(false);
	gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);

	StartAsyncOperation(&FrontEnd::as_RenewServerThread, &FrontEnd::OnRenewServerSuccess);
}

unsigned int WINAPI FrontEnd::as_RenewServerThread(void* in_data)
{
	r3dThreadAutoInstallCrashHelper crashHelper;
	FrontEnd* This = (FrontEnd*)in_data;

	int apiCode = gUserProfile.serverRent->ApiRenewServer(This->m_myGameServerId, This->m_rentServerParams, This->m_rentServerPrice);
	if (apiCode != 0)
	{
		This->SetAsyncError(apiCode, gLangMngr.getString("ServerRent_RentFail"));
		return 0;
	}

	return 1;
}

void FrontEnd::OnRenewServerSuccess()
{
	gfxFrontEnd.Invoke("_root.api.hideInfoMsg", "");

	Scaleform::GFx::Value var[1];
	var[0].SetInt(gUserProfile.ProfileData.GamePoints);
	gfxFrontEnd.Invoke("_root.api.setGC", var, 1);

	// return back to server screen
	gfxFrontEnd.Invoke("_root.api.Main.showScreen", "MyServers");

	// and force to reread servers list
	eventRequestMyServerList(&gfxFrontEnd, NULL, 0);
}

void FrontEnd::eventRentServerUpdatePrice(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount == 11);
	//isGameServer:int, mapID:int, regionID:int, slotID:int, rentID:int, name:String, password:String, pveID:int, nameplates:int, crosshair:int, tracers:int

	m_rentServerPrice = calcServerRentPrice(args);
	if (m_rentServerPrice <= 0)
	{
		Scaleform::GFx::Value vars[3];
		vars[0].SetString(gLangMngr.getString("ServerRent_RegionDisabled"));
		vars[1].SetBoolean(true);
		vars[2].SetString("$ERROR");
		gfxFrontEnd.Invoke("_root.api.showInfoMsg", vars, 3);
		return;
	}

	gfxFrontEnd.Invoke("_root.api.Main.RentServerPopup.updateServerPrice", m_rentServerPrice);
}

void FrontEnd::eventRentServer(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount == 11);
	//isGameServer:int, mapID:int, regionID:int, slotID:int, rentID:int, name:String, password:String, pveID:int, nameplates:int, crosshair:int, tracers:int

	CUserServerRental::rentParams_s params;
	params.isGameServer = args[0].GetInt();
	params.mapID = args[1].GetInt();
	params.regionID = GBNET_REGION_US_West; //[TH] args[2].GetInt();
	params.slotID = args[3].GetInt();
	params.rentID = args[4].GetInt();
	r3dscpy(params.name, args[5].GetString());
	r3dscpy(params.password, args[6].GetString());
	params.pveID = args[7].GetInt();
	params.nameplates = args[8].GetInt();
	params.crosshair = args[9].GetInt();
	params.tracers = args[10].GetInt();

	if (strlen(params.name) == 0)
	{
		Scaleform::GFx::Value vars[3];
		vars[0].SetString(gLangMngr.getString("ServerRent_NameTooShort"));
		vars[1].SetBoolean(true);
		vars[2].SetString("$ERROR");
		gfxFrontEnd.Invoke("_root.api.showInfoMsg", vars, 3);
		return;
	}
	if (strlen(params.name) >= 32)
	{
		Scaleform::GFx::Value vars[3];
		vars[0].SetString(gLangMngr.getString("ServerRent_NameTooBig"));
		vars[1].SetBoolean(true);
		vars[2].SetString("$ERROR");
		gfxFrontEnd.Invoke("_root.api.showInfoMsg", vars, 3);
		return;
	}

	if (strlen(params.password) >= 16)
	{
		Scaleform::GFx::Value vars[3];
		vars[0].SetString(gLangMngr.getString("ServerRent_PasswordTooBig"));
		vars[1].SetBoolean(true);
		vars[2].SetString("$ERROR");
		gfxFrontEnd.Invoke("_root.api.showInfoMsg", vars, 3);
		return;
	}

	m_rentServerParams = params;
	if (m_rentServerPrice <= 0)
	{
		Scaleform::GFx::Value vars[3];
		vars[0].SetString(gLangMngr.getString("ServerRent_RegionDisabled"));
		vars[1].SetBoolean(true);
		vars[2].SetString("$ERROR");
		gfxFrontEnd.Invoke("_root.api.showInfoMsg", vars, 3);
		return;
	}

	if (m_rentServerPrice > gUserProfile.ProfileData.GamePoints)
	{
		Scaleform::GFx::Value vars[3];
		vars[0].SetString(gLangMngr.getString("NotEnoughGP"));
		vars[1].SetBoolean(true);
		vars[2].SetString("$ERROR");
		gfxFrontEnd.Invoke("_root.api.showInfoMsg", vars, 3);
		return;
	}

	Scaleform::GFx::Value var[2];
	var[0].SetString(gLangMngr.getString("OneMomentPlease"));
	var[1].SetBoolean(false);
	gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);

	StartAsyncOperation(&FrontEnd::as_RentServerThread, &FrontEnd::OnRentServerSuccess);
}

unsigned int WINAPI FrontEnd::as_RentServerThread(void* in_data)
{
	r3dThreadAutoInstallCrashHelper crashHelper;
	FrontEnd* This = (FrontEnd*)in_data;

	int apiCode = gUserProfile.serverRent->ApiRentServer(This->m_rentServerParams, This->m_rentServerPrice);
	if (apiCode != 0)
	{
		if (apiCode == 9)
			This->SetAsyncError(0, gLangMngr.getString("ServerRent_NameAlreadyInUse"));
		else if (apiCode == 3)
			This->SetAsyncError(0, gLangMngr.getString("ServerRent_PriceDesync"));
		else
			This->SetAsyncError(apiCode, gLangMngr.getString("ServerRent_RentFail"));
		return 0;
	}

	return 1;
}

void FrontEnd::OnRentServerSuccess()
{
	gfxFrontEnd.Invoke("_root.api.hideInfoMsg", "");

	Scaleform::GFx::Value var[1];
	var[0].SetInt(gUserProfile.ProfileData.GamePoints);
	gfxFrontEnd.Invoke("_root.api.setGC", var, 1);

	// return back to server screen
	gfxFrontEnd.Invoke("_root.api.Main.showScreen", "MyServers");

	// and force to reread servers list
	eventRequestMyServerList(&gfxFrontEnd, NULL, 0);
}

void FrontEnd::eventMyServerJoinServer(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount == 1);
	uint32_t serverID = args[0].GetUInt();

	// join server
	Scaleform::GFx::Value var[1];
	var[0].SetInt(serverID);
	eventBrowseGamesJoin(pMovie, var, 1);
}

void FrontEnd::eventMyServerUpdateSettings(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount == 10);
	uint32_t serverID = args[0].GetUInt();
	const char* newPassword = args[1].GetString();
	int isNameplateOn = args[2].GetInt();
	int isCrosshairOn = args[3].GetInt();
	int isTracersOn = args[4].GetInt();
	uint32_t gameTimeLimit = args[5].GetUInt();
	if (gameTimeLimit > 99999)
		gameTimeLimit = 0;
	int isTrialsAllowed = args[6].GetInt();
	int disableASR = args[7].GetInt();
	int disableSNP = args[8].GetInt();
	int disablePVP = args[9].GetInt();

	if (strlen(newPassword) >= 16)
	{
		Scaleform::GFx::Value vars[3];
		vars[0].SetString(gLangMngr.getString("ServerRent_PasswordTooBig"));
		vars[1].SetBoolean(true);
		vars[2].SetString("$ERROR");
		gfxFrontEnd.Invoke("_root.api.showInfoMsg", vars, 3);
		return;
	}

	CUserServerRental::myServer_s* srv = gUserProfile.serverRent->GetServerById(m_myGameServerId);
	m_myServerFlags = srv->ginfo.flags;
	m_myServerFlags &= ~GBGameInfo::SFLAGS_Nameplates;
	m_myServerFlags &= ~GBGameInfo::SFLAGS_CrossHair;
	m_myServerFlags &= ~GBGameInfo::SFLAGS_Tracers;
	m_myServerFlags &= ~GBGameInfo::SFLAGS_TrialsAllowed;
	m_myServerFlags &= ~GBGameInfo::SFLAGS_DisableASR;
	m_myServerFlags &= ~GBGameInfo::SFLAGS_DisableSNP;
	m_myServerFlags &= ~GBGameInfo::SFLAGS_PVE;
	if (isNameplateOn) m_myServerFlags |= GBGameInfo::SFLAGS_Nameplates;
	if (isCrosshairOn) m_myServerFlags |= GBGameInfo::SFLAGS_CrossHair;
	if (isTracersOn)   m_myServerFlags |= GBGameInfo::SFLAGS_Tracers;
	if (isTrialsAllowed)   m_myServerFlags |= GBGameInfo::SFLAGS_TrialsAllowed;
	if (disableASR)   m_myServerFlags |= GBGameInfo::SFLAGS_DisableASR;
	if (disableSNP)   m_myServerFlags |= GBGameInfo::SFLAGS_DisableSNP;
	if (disablePVP)   m_myServerFlags |= GBGameInfo::SFLAGS_PVE;

	r3dscpy(m_myServerPwd, newPassword);

	m_myServerGameTimeLimit = gameTimeLimit;

	Scaleform::GFx::Value var[2];
	var[0].SetString(gLangMngr.getString("OneMomentPlease"));
	var[1].SetBoolean(false);
	gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);

	StartAsyncOperation(&FrontEnd::as_MyServerSetSettingsThread, &FrontEnd::OnMyServerSetSettingsSuccess);
}

unsigned int WINAPI FrontEnd::as_MyServerSetSettingsThread(void* in_data)
{
	r3dThreadAutoInstallCrashHelper crashHelper;
	FrontEnd* This = (FrontEnd*)in_data;

	int apiCode = gUserProfile.serverRent->ApiSetServerParams(This->m_myGameServerId, This->m_myServerPwd, This->m_myServerFlags, This->m_myServerGameTimeLimit);
	if (apiCode != 0)
	{
		This->SetAsyncError(apiCode, gLangMngr.getString("ServerRent_SetPwdFail"));
		return 0;
	}

	// send password change request to server
	This->DelayServerRequest();
	if (!This->ConnectToMasterServer())
		return 0;

	gMasterServerLogic.SendMyServerSetParams(This->m_myGameServerId, This->m_myGameServerAdminKey, This->m_myServerPwd, This->m_myServerFlags, This->m_myServerGameTimeLimit);

	return 1;
}

void FrontEnd::OnMyServerSetSettingsSuccess()
{
	gfxFrontEnd.Invoke("_root.api.hideInfoMsg", "");

	CUserServerRental::myServer_s* srv = gUserProfile.serverRent->GetServerById(m_myGameServerId);
	r3dscpy(srv->pwd, m_myServerPwd);
	srv->ginfo.flags = (BYTE)m_myServerFlags;
	srv->ginfo.gameTimeLimit = m_myServerGameTimeLimit;

	// set again
	gfxFrontEnd.SetVariable("_root.api.Main.PlayGameMyServers.MyServers.Settings.Password.text", srv->pwd);

	return;
}

//AlexRedd:: FriendlyFire system
void FrontEnd::eventFriendlyFireSet(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount == 1);
	int isFriendlyFire = args[0].GetInt();

	m_FriendlyFire = isFriendlyFire;

	Scaleform::GFx::Value var[2];
	var[0].SetString(gLangMngr.getString("OneMomentPlease"));
	var[1].SetBoolean(false);
	gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);

	StartAsyncOperation(&FrontEnd::as_FriendlyFireThread, &FrontEnd::OnFriendlyFireSuccess);
}

unsigned int WINAPI FrontEnd::as_FriendlyFireThread(void* in_data)
{
	r3dThreadAutoInstallCrashHelper crashHelper;
	FrontEnd* This = (FrontEnd*)in_data;

	This->DelayServerRequest();

	CUserClans* clans = gUserProfile.clans[gUserProfile.SelectedCharID];
	int apiCode = clans->ApiSetFriendlyFire(clans->clanCurData_.ClanID, This->m_FriendlyFire);
	if (apiCode != 0)
	{
		This->SetAsyncError(apiCode, gLangMngr.getString("FailedToSetFriendlyFire"));
		return 0;
	}

	return 1;
}

void FrontEnd::OnFriendlyFireSuccess()
{
	gfxFrontEnd.Invoke("_root.api.hideInfoMsg", "");
	return;
}

void FrontEnd::eventLearnSkill(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount == 1);
	m_SkillID = args[0].GetUInt();

	Scaleform::GFx::Value var[2];
	var[0].SetString(gLangMngr.getString("OneMomentPlease"));
	var[1].SetBoolean(false);
	gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);

	StartAsyncOperation(&FrontEnd::as_LearnSkillThread, &FrontEnd::OnLearnSkillSuccess);
}

unsigned int WINAPI FrontEnd::as_LearnSkillThread(void* in_data)
{
	r3dThreadAutoInstallCrashHelper crashHelper;
	FrontEnd* This = (FrontEnd*)in_data;

	int apiCode = gUserProfile.ApiSkillLearn(This->m_SkillID, 1);
	if (apiCode != 0)
	{
		This->SetAsyncError(apiCode, gLangMngr.getString("SkillLearnError"));
		return 0;
	}

	return 1;
}

void FrontEnd::OnLearnSkillSuccess()
{
	gfxFrontEnd.Invoke("_root.api.hideInfoMsg", "");

	wiCharDataFull& slot = gUserProfile.ProfileData.ArmorySlots[gUserProfile.SelectedCharID];

	{
		Scaleform::GFx::Value var[2];
		char tmpGamertag[128];
		if (slot.ClanID != 0)
			sprintf(tmpGamertag, "[%s] %s", slot.ClanTag, slot.Gamertag);
		else
			r3dscpy(tmpGamertag, slot.Gamertag);
		var[0].SetString(tmpGamertag);
		var[1].SetInt(m_SkillID);
		gfxFrontEnd.Invoke("_root.api.setSkillLearnedSurvivor", var, 2);
	}

	updateClientSurvivor(slot);

	gfxFrontEnd.Invoke("_root.api.Main.SkillTree.refreshSkillTree", "");

	return;
}

void FrontEnd::eventChangeOutfit(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount == 5);
	uint32_t slotID = args[0].GetUInt();
	m_ChangeOutfit_newHead = args[1].GetUInt();
	m_ChangeOutfit_newBody = args[2].GetUInt();
	m_ChangeOutfit_newLegs = args[3].GetUInt();
	m_ChangeOutfit_newHero = args[4].GetUInt();

	r3d_assert(gUserProfile.SelectedCharID == slotID);

	Scaleform::GFx::Value var[2];
	var[0].SetString(gLangMngr.getString("OneMomentPlease"));
	var[1].SetBoolean(false);
	gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);

	StartAsyncOperation(&FrontEnd::as_ChangeOutfitThread, &FrontEnd::OnChangeOutfitSuccess);
}

unsigned int WINAPI FrontEnd::as_ChangeOutfitThread(void* in_data)
{
	r3dThreadAutoInstallCrashHelper crashHelper;
	FrontEnd* This = (FrontEnd*)in_data;

	int apiCode = gUserProfile.ApiCharChangeSkin(This->m_ChangeOutfit_newHead, This->m_ChangeOutfit_newBody, This->m_ChangeOutfit_newLegs, This->m_ChangeOutfit_newHero);
	if (apiCode != 0)
	{
		This->SetAsyncError(apiCode, gLangMngr.getString("ChangeOutfitError"));
		return 0;
	}

	return 1;
}

void FrontEnd::OnChangeOutfitSuccess()
{
	gfxFrontEnd.Invoke("_root.api.hideInfoMsg", "");

	wiCharDataFull& slot = gUserProfile.ProfileData.ArmorySlots[gUserProfile.SelectedCharID];

	updateClientSurvivor(slot);

	m_needPlayerRenderingRequest = 1;

	// during change outfit we show player without items, so once we are returning back, re-load player with all his items
	m_Player->uberAnim_->anim.StopAll();	// prevent animation blending on loadout switch
	m_Player->UpdateLoadoutSlot(slot, -1);

	gfxFrontEnd.Invoke("_root.api.Main.ChangeOutfit.EventChangeOnSuccess", "");

	return;
}

unsigned int WINAPI FrontEnd::as_CharRenameThread(void* in_data)
{
	r3dThreadAutoInstallCrashHelper crashHelper;
	FrontEnd* This = (FrontEnd*)in_data;

	if (gUserProfile.ProfileData.GamePoints < gUserProfile.ShopCharRename)
	{
		This->SetAsyncError(0, gLangMngr.getString("NotEnoughGC"));
		return 0;
	}

	int minutesLeft = 0;
	int apiCode = gUserProfile.ApiCharRename(This->m_CreateGamerTag, &minutesLeft);
	if (apiCode == 4)
	{
		char buf[1024];
		// temp sring. maybe create a valid date and show it
		int daysLeft = minutesLeft / 1440;
		int hoursLeft = minutesLeft / 60;
		sprintf(buf, gLangMngr.getString("ChangeNameTimeError"), (daysLeft > 0) ? (daysLeft) : (hoursLeft), (daysLeft > 0) ? gLangMngr.getString("$FR_Store_PluralDay") : gLangMngr.getString("$FR_PAUSE_INVENTORY_HOURS"));
		This->SetAsyncError(0, buf);
		return 0;
	}
	if (apiCode != 0)
	{
		if (apiCode == 9)
			This->SetAsyncError(0, gLangMngr.getString("ThisNameIsAlreadyInUse"));
		else
			This->SetAsyncError(apiCode, gLangMngr.getString("CharRenameError"));
		return 0;
	}

	return 1;
}

void FrontEnd::OnCharRenameSuccess()
{
	gfxFrontEnd.Invoke("_root.api.hideInfoMsg", "");

	wiCharDataFull& slot = gUserProfile.ProfileData.ArmorySlots[gUserProfile.SelectedCharID];

	{
		char tmpGamertag_old[128];
		char tmpGamertag[128];
		if (slot.ClanID != 0)
		{
			sprintf(tmpGamertag_old, "[%s] %s", slot.ClanTag, CharRename_PreviousName);
			sprintf(tmpGamertag, "[%s] %s", slot.ClanTag, slot.Gamertag);
		}
		else
		{
			r3dscpy(tmpGamertag_old, CharRename_PreviousName);
			r3dscpy(tmpGamertag, slot.Gamertag);
		}

		Scaleform::GFx::Value var[2];
		var[0].SetString(tmpGamertag_old);
		var[1].SetString(tmpGamertag);
		gfxFrontEnd.Invoke("_root.api.changeSurvivorName", var, 2);
	}

	updateClientSurvivor(slot);

	Scaleform::GFx::Value var[2];
	var[0].SetInt(gUserProfile.ProfileData.GamePoints);
	gfxFrontEnd.Invoke("_root.api.setGC", var, 1);

	return;
}

void FrontEnd::eventSetCurrentBrowseChannel(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount == 2);
	uint32_t channel = args[0].GetUInt();
	bool quickGameRequest = args[1].GetBool();

	if (channel == 4 && gUserProfile.ProfileData.PremiumAcc == 0) // premium required
	{
		gfxFrontEnd.Invoke("_root.api.Main.PremiumNeededPopUp.showPopUp", "");
		return;
	}

	CurrentBrowseChannel = channel;
	if (quickGameRequest/* || channel == 7*/)
		gfxFrontEnd.Invoke("_root.api.Main.showScreen", "PlayGameQuick");
	else
		gfxFrontEnd.Invoke("_root.api.Main.showScreen", "ServerBrowse");
}

void FrontEnd::eventTrialRequestUpgrade(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount == 0);

	/*// web site purchase
	{
	char url[256];
	if(gUserProfile.ProfileData.AccountType == 21)
	sprintf(url, "http://infestationmmo.ru/buy");
	else
	sprintf(url, "https://127.0.0.1.com/#getgame");
	ShellExecute(NULL, "open", url, "", NULL, SW_SHOW);
	}

	// minimize our window
	ShowWindow(win::hWnd, SW_MINIMIZE);*/

	// show upgrade dialog?
	//gfxFrontEnd.Invoke("_root.api.showTrialUpgradeWindow", "");

	//AlexRedd:: new upgrade trial popup	
	Scaleform::GFx::Value var[2];
	var[0].SetInt(gUserProfile.ProfileData.AccountType);
	var[1].SetString(gUserProfile.username);
	gfxFrontEnd.Invoke("_root.api.Main.UpgradeAccountPopUp.setCurrentAccountTexts", var, 2);
	gfxFrontEnd.Invoke("_root.api.Main.UpgradeAccountPopUp.showPopUp", "");
}

void FrontEnd::eventTrialUpgradeAccount(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount == 1);
	const char* upgradeCode = args[0].GetString();

	if (strlen(upgradeCode) != 19 && strlen(upgradeCode) != 29) {
		Scaleform::GFx::Value var[2];
		var[0].SetString(gLangMngr.getString("UpgradeTrialAccount_WrongSerialFormat"));
		var[1].SetBoolean(true);
		gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);
		return;
	}

	int apiCode = gUserProfile.ApiUpgradeTrialAccount(upgradeCode);
	const char* msg = "";
	if (apiCode == 0) {
		msg = gLangMngr.getString("UpgradeTrialAccount_Success");
		//SoundSys.PlayAndForget(SoundSys.GetEventIDByPath("Sounds/UI Events/Misc/Buy_Premium"), r3dPoint3D(0, 0, 0));
	}
	else if (apiCode == 2) {
		msg = gLangMngr.getString("UpgradeTrialAccount_CannotUpgrade");
		SoundSys.PlayAndForget(SoundSys.GetEventIDByPath("Sounds/UI Events/Misc/UI_Error_MsgSound"), r3dPoint3D(0, 0, 0));
	}
	else if (apiCode == 3) {
		msg = gLangMngr.getString("UpgradeTrialAccount_SerialNotValid");
		SoundSys.PlayAndForget(SoundSys.GetEventIDByPath("Sounds/UI Events/Misc/UI_Error_MsgSound"), r3dPoint3D(0, 0, 0));
	}
	else if (apiCode == 4) {
		msg = gLangMngr.getString("UpgradeTrialAccount_SerialUsed");
		SoundSys.PlayAndForget(SoundSys.GetEventIDByPath("Sounds/UI Events/Misc/UI_Error_MsgSound"), r3dPoint3D(0, 0, 0));
	}
	else {
		msg = gLangMngr.getString("UpgradeTrialAccount_Failed");
		SoundSys.PlayAndForget(SoundSys.GetEventIDByPath("Sounds/UI Events/Misc/UI_Error_MsgSound"), r3dPoint3D(0, 0, 0));
	}

	if (apiCode == 0)
	{
		//gfxFrontEnd.SetVariable("_root.api.Main.SurvivorsAnim.Survivors.BtnUpgtrial.visible", false);
		gfxFrontEnd.SetVariable("_root.api.isTrialAccount", false);
	}

	Scaleform::GFx::Value var[2];
	var[0].SetString(msg);
	var[1].SetBoolean(true);
	gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);

	return;
}

void FrontEnd::eventBuyPremiumAccount(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount == 0);

	mStore_BuyItemID = 301257; // premium account itemID
	mStore_BuyPrice = gUserProfile.ShopPremiumAcc;
	mStore_BuyPriceGD = 0;

	if (gUserProfile.ProfileData.GamePoints < mStore_BuyPrice)
	{
		Scaleform::GFx::Value var[2];
		var[0].SetString(gLangMngr.getString("NotEnougMoneyToBuyItem"));
		var[1].SetBoolean(true);
		gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);
		return;
	}

	Scaleform::GFx::Value var[2];
	var[0].SetString(gLangMngr.getString("OneMomentPlease"));
	var[1].SetBoolean(false);
	gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);

	StartAsyncOperation(&FrontEnd::as_BuyItemThread, &FrontEnd::OnBuyPremiumSuccess);
}

void FrontEnd::OnBuyPremiumSuccess()
{
	Scaleform::GFx::Value var[2];
	var[0].SetInt(gUserProfile.ProfileData.GamePoints);
	gfxFrontEnd.Invoke("_root.api.setGC", var, 1);

	var[0].SetInt(gUserProfile.ProfileData.GameDollars);
	gfxFrontEnd.Invoke("_root.api.setDollars", var, 1);

	var[0].SetInt(gUserProfile.ProfileData.XP50Boosts);
	gfxFrontEnd.Invoke("_root.api.setXPBoosts", var, 1);

	gfxFrontEnd.Invoke("_root.api.hideInfoMsg", "");

	gUserProfile.ProfileData.PremiumAcc = 43200; // 30 days of premium
	gfxFrontEnd.SetVariable("_root.api.Main.SurvivorsAnim.Survivors.PremiumAcc.visible", true);

	//SoundSys.PlayAndForget(SoundSys.GetEventIDByPath("Sounds/UI Events/Misc/Buy_Premium"), r3dPoint3D(0, 0, 0));

	var[0].SetString(gLangMngr.getString("PremiumAccountBoughtPleaseRestart"));
	var[1].SetBoolean(true);
	gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);


	return;
}

void FrontEnd::eventBuyAccount(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount == 1);

	int buyIdx = args[0].GetInt();
	if (buyIdx == 0) {
		mStore_BuyItemID = 301402; // legend account itemID
		mStore_BuyPrice = gUserProfile.ShopLegendAcc;
	}
	else if (buyIdx == 1) {
		mStore_BuyItemID = 301403; // pioneer account itemID
		mStore_BuyPrice = gUserProfile.ShopPioneerAcc;
	}

	mStore_BuyPriceGD = 0;

	if (gUserProfile.ProfileData.GamePoints < mStore_BuyPrice)
	{
		Scaleform::GFx::Value var[2];
		var[0].SetString(gLangMngr.getString("NotEnougMoneyToBuyItem"));
		var[1].SetBoolean(true);
		gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);
		return;
	}

	if (gUserProfile.ProfileData.AccountType == 0 && buyIdx == 0 || gUserProfile.ProfileData.AccountType == 54 && buyIdx == 1)
	{
		Scaleform::GFx::Value var[2];
		var[0].SetString(gLangMngr.getString("UpgradeTrialAccount_CannotUpgradeSameAccount"));
		var[1].SetBoolean(true);
		gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);
		return;
	}

	Scaleform::GFx::Value var[2];
	var[0].SetString(gLangMngr.getString("OneMomentPlease"));
	var[1].SetBoolean(false);
	gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);

	StartAsyncOperation(&FrontEnd::as_BuyItemThread, &FrontEnd::OnBuyAccountSuccess);
}

void FrontEnd::OnBuyAccountSuccess()
{
	gUserProfile.GetProfile();

	updateInventoryAndSkillItems();

	Scaleform::GFx::Value var[2];
	var[0].SetInt(gUserProfile.ProfileData.GamePoints);
	gfxFrontEnd.Invoke("_root.api.setGC", var, 1);
	gfxFrontEnd.Invoke("_root.api.Main.UpgradeAccountPopUp.setNewGC", "");

	var[0].SetInt(gUserProfile.ProfileData.GameDollars);
	gfxFrontEnd.Invoke("_root.api.setDollars", var, 1);

	var[0].SetInt(gUserProfile.ProfileData.XP50Boosts);
	gfxFrontEnd.Invoke("_root.api.setXPBoosts", var, 1);

	gfxFrontEnd.Invoke("_root.api.hideInfoMsg", "");

	Scaleform::GFx::Value var2[7];
	// clear inventory DB
	gfxFrontEnd.Invoke("_root.api.clearInventory", NULL, 0);

	// add all items
	for (uint32_t i = 0; i < gUserProfile.ProfileData.NumItems; ++i)
	{
		var2[0].SetUInt(uint32_t(gUserProfile.ProfileData.Inventory[i].InventoryID));
		var2[1].SetUInt(gUserProfile.ProfileData.Inventory[i].itemID);
		var2[2].SetNumber(gUserProfile.ProfileData.Inventory[i].quantity);
		var2[3].SetNumber(gUserProfile.ProfileData.Inventory[i].Var1);
		var2[4].SetNumber(gUserProfile.ProfileData.Inventory[i].Var2);
		bool isConsumable = false;
		{
			const WeaponConfig* wc = g_pWeaponArmory->getWeaponConfig(gUserProfile.ProfileData.Inventory[i].itemID);
			if (wc && wc->category == storecat_UsableItem && wc->m_isConsumable)
				isConsumable = true;
		}
		var2[5].SetBoolean(isConsumable);
		var2[6].SetString(getAdditionalDescForItem(gUserProfile.ProfileData.Inventory[i].itemID, gUserProfile.ProfileData.Inventory[i].Var1, gUserProfile.ProfileData.Inventory[i].Var2, gUserProfile.ProfileData.Inventory[i].Var3));
		gfxFrontEnd.Invoke("_root.api.addInventoryItem", var2, 7);
	}

	//SoundSys.PlayAndForget(SoundSys.GetEventIDByPath("Sounds/UI Events/Misc/Buy_Premium"), r3dPoint3D(0, 0, 0));

	var[0].SetString(gLangMngr.getString("UpgradeTrialAccount_Success"));
	var[1].SetBoolean(true);
	gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);
	return;
}

void FrontEnd::eventMarketplaceActive(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount == 0);
	//g_shown_marketplace_tutorial->SetBool(true);
	writeGameOptionsFile();
	// Blackmarket System Begin
	if (cMarketplaceScreen == eMarketplaceScreen::Blackmarket)
	{
		// use frontend to request currently selected categories
		Scaleform::GFx::Value vars[1];
		vars[0].SetInt((int)(eMarketplaceScreen::Blackmarket));
		gfxFrontEnd.Invoke("_root.api.Main.Marketplace.DoLoadBlackmarketWithSelectedTab", vars, 1);
	}
	// Blackmarket System End
}

void FrontEnd::eventRequestShowDonateGCtoServer(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount == 1);
	int serverID = args[0].GetInt();

	const GBPKT_M2C_GameData_s* gdata = gMasterServerLogic.FindGameById(serverID);
	if (gdata == NULL)
	{
		// server not found by some reason
		r3d_assert(gdata);
		return;
	}

	// detect slotID
	int slotID = CUserServerRental::GetSlotID(gdata->info);
	if (slotID == -1)
	{
		// we wasn't able to determine slots
		Scaleform::GFx::Value vars[3];
		vars[0].SetString(gLangMngr.getString("ServerRent_BadRenewSlots"));
		vars[1].SetBoolean(true);
		vars[2].SetString("$ERROR");
		gfxFrontEnd.Invoke("_root.api.showInfoMsg", vars, 3);
		return;
	}

	// grab server prices
	static bool haveServerPrices = false;
	if (!haveServerPrices)
	{
		int apiCode = gUserProfile.serverRent->ApiGetServerRentPrices();
		if (apiCode != 0)
		{
			Scaleform::GFx::Value vars[3];
			vars[0].SetString(gLangMngr.getString("ServerRent_FailGetPrice"));
			vars[1].SetBoolean(true);
			vars[2].SetString("$ERROR");
			gfxFrontEnd.Invoke("_root.api.showInfoMsg", vars, 3);
			return;
		}
		haveServerPrices = true;
	}

	// check if region is enabled, check with 1 hour rent
	int rentID = CUserServerRental::HOURLY_RENTID_START + 1;

	// fill temporary array to be in sync with calcServerRentPrice arguments
	Scaleform::GFx::Value targs[11];
	gameInfoTo_calcServerRentPrice_Args(targs, gdata->info, slotID, rentID);

	int rentPrice = calcServerRentPrice(targs);
	if (rentPrice == 0)
	{
		Scaleform::GFx::Value vars[3];
		vars[0].SetString(gLangMngr.getString("ServerRent_RegionDisabled"));
		vars[1].SetBoolean(true);
		vars[2].SetString("$ERROR");
		gfxFrontEnd.Invoke("_root.api.showInfoMsg", vars, 3);
		return;
	}

	gfxFrontEnd.Invoke("_root.api.Main.DonateGCSrvPopUp.showPopUp", serverID);
}

void FrontEnd::eventDonateGCtoServerCallback(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount == 2);
	int numHours = args[0].GetInt();
	int serverID = args[1].GetInt();

	const GBPKT_M2C_GameData_s* gdata = gMasterServerLogic.FindGameById(serverID);
	r3d_assert(gdata);

	// WARNING: all following is somehow modified copy-paste from eventRenewServerUpdatePrice

	// detect slotID from slot numbers
	int slotID = CUserServerRental::GetSlotID(gdata->info);
	r3d_assert(slotID != -1);

	// rentID as hourly rent
	int rentID = CUserServerRental::HOURLY_RENTID_START + numHours;

	// fill temporary array to be in sync with calcServerRentPrice arguments
	Scaleform::GFx::Value targs[11];
	gameInfoTo_calcServerRentPrice_Args(targs, gdata->info, slotID, rentID);

	// fill params array as in eventRentServer
	CUserServerRental::rentParams_s params;
	calcServerRentPrice_toRentParams(targs, params);

	m_rentServerParams = params;
	m_myGameServerId = serverID;
	m_rentServerPrice = calcServerRentPrice(targs);

	// hack: if we do not have server capacity base price will be *negative* here.
	// but we should always renew, so invert and process
	if (m_rentServerPrice < 0) m_rentServerPrice = -m_rentServerPrice;

	gfxFrontEnd.Invoke("_root.api.Main.DonateGCSrvPopUp.setGCValue", m_rentServerPrice);
}


void FrontEnd::eventDonateGCtoServer(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount == 2);
	int numHours = args[0].GetInt();
	int serverID = args[1].GetInt();

	Scaleform::GFx::Value var[2];
	var[0].SetString(gLangMngr.getString("OneMomentPlease"));
	var[1].SetBoolean(false);
	gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);

	r3d_assert(m_rentServerParams.rentID > CUserServerRental::HOURLY_RENTID_START);
	r3d_assert(m_rentServerPrice > 0);

	StartAsyncOperation(&FrontEnd::as_DonateToServerThread, &FrontEnd::OnDonateToServerSuccess);
}

unsigned int WINAPI FrontEnd::as_DonateToServerThread(void* in_data)
{
	r3dThreadAutoInstallCrashHelper crashHelper;
	FrontEnd* This = (FrontEnd*)in_data;

	int HoursLeft = 0;
	int apiCode = gUserProfile.serverRent->ApiRenewServer(This->m_myGameServerId, This->m_rentServerParams, This->m_rentServerPrice, &HoursLeft);
	if (apiCode == 4)
	{
		char buf[512];
		sprintf(buf, gLangMngr.getString("ServerRent_DonateNeedWait"), HoursLeft);
		This->SetAsyncError(0, buf);
		return 0;
	}
	if (apiCode != 0)
	{
		This->SetAsyncError(apiCode, gLangMngr.getString("ServerRent_RentFail"));
		return 0;
	}

	return 1;
}

void FrontEnd::OnDonateToServerSuccess()
{
	gfxFrontEnd.Invoke("_root.api.hideInfoMsg", "");

	Scaleform::GFx::Value var[1];
	var[0].SetInt(gUserProfile.ProfileData.GamePoints);
	gfxFrontEnd.Invoke("_root.api.setGC", var, 1);

	// display some success message?
	{
		Scaleform::GFx::Value vars[3];
		vars[0].SetString(gLangMngr.getString("ServerRent_DonateOk"));
		vars[1].SetBoolean(true);
		vars[2].SetString("$FR_OK");
		gfxFrontEnd.Invoke("_root.api.showInfoMsg", vars, 3);
	}

	return;
}

void FrontEnd::eventMoveAllItems(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	//move all items from backpack to inventory
	const wiCharDataFull& slot = gUserProfile.ProfileData.ArmorySlots[gUserProfile.SelectedCharID];

	int itemsToMove = 0;
	for (int i = 0; i < slot.BackpackSize; ++i)
	{
		const wiInventoryItem& wi = slot.Items[i];
		if (wi.itemID > 0)
		{
			itemsToMove++;
		}
	}

	if (itemsToMove)
	{
		if ((gUserProfile.ProfileData.NumItems + itemsToMove) > wiUserProfile::INVENTORY_SIZE_LIMIT)
		{
			Scaleform::GFx::Value vars[3];
			vars[0].SetString(gLangMngr.getString("InGameUI_ErrorMsg_NoInventorySpace"));
			vars[1].SetBoolean(true);
			vars[2].SetString("$ERROR");
			gfxFrontEnd.Invoke("_root.api.showInfoMsg", vars, 3);
			return;
		}
	}
	else
	{
		return;
	}

	Scaleform::GFx::Value var[2];
	var[0].SetString(gLangMngr.getString("OneMomentPlease"));
	var[1].SetBoolean(false);
	gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);

	StartAsyncOperation(&FrontEnd::as_MoveAllItemsThread, &FrontEnd::OnMoveAllItemsSucessfully);
}

unsigned int WINAPI FrontEnd::as_MoveAllItemsThread(void* in_data)
{
	r3dThreadAutoInstallCrashHelper crashHelper;
	FrontEnd* This = (FrontEnd*)in_data;

	This->DelayServerRequest();

	int BackpackSlot = 0;
	int Quantity = 0;
	int apiCode = 0;
	const wiCharDataFull& slot = gUserProfile.ProfileData.ArmorySlots[gUserProfile.SelectedCharID];
	for (int i = 0; i < slot.BackpackSize; ++i)
	{
		const wiInventoryItem& wi = slot.Items[i];
		if (wi.itemID > 0)
		{
			BackpackSlot = i;
			Quantity = wi.quantity;
			apiCode = gUserProfile.ApiBackpackToInventory(BackpackSlot, Quantity);
		}

		if (apiCode != 0)
		{
			if (apiCode == 7)
			{
				This->SetAsyncError(0, gLangMngr.getString("GameSessionHasNotClosedYet"));
			}
			else if (apiCode == 9)
			{
				This->SetAsyncError(0, gLangMngr.getString("InGameUI_ErrorMsg_NoInventorySpace"));
			}
			else if (apiCode == 6)
			{
				return 1;
			}
			else
			{
				This->SetAsyncError(apiCode, gLangMngr.getString("BackpackToInventoryFail"));
			}
			return 0;
		}
	}

	return 1;
}

void FrontEnd::OnMoveAllItemsSucessfully()
{
	Scaleform::GFx::Value var[8];
	gfxFrontEnd.Invoke("_root.api.clearBackpack", "");
	int    slot = gUserProfile.SelectedCharID;

	addBackpackItems(gUserProfile.ProfileData.ArmorySlots[slot], slot);

	updateInventoryAndSkillItems();

	updateSurvivorTotalWeight(slot);

	gfxFrontEnd.Invoke("_root.api.hideInfoMsg", "");
	gfxFrontEnd.Invoke("_root.api.backpackToInventorySuccess", "");
}

void FrontEnd::eventOpenURL(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount == 1);
	const char* urlID = args[0].GetString();
	r3d_assert(urlID);

	/*
	if(strcmp(urlID, "clickLink_TOS")==0)
	{
	ShellExecute(NULL, "open", "http://forums.com/index.php?/topic/133636-regarding-bans-for-cheating/", "", NULL, SW_SHOW);

	// minimize our window
	ShowWindow(win::hWnd, SW_MINIMIZE);
	}
	*/
}

void FrontEnd::eventShowRewardPopPUp(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	Scaleform::GFx::Value var[2];
	var[0].SetString(gLangMngr.getString("OneMomentPlease"));
	var[1].SetBoolean(false);
	gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);

	StartAsyncOperation(&FrontEnd::as_ShowRewardPopUpThread, &FrontEnd::OnShowRewardPopUpSuccess);
}

unsigned int WINAPI FrontEnd::as_ShowRewardPopUpThread(void* in_data)
{
	r3dThreadAutoInstallCrashHelper crashHelper;
	FrontEnd* This = (FrontEnd*)in_data;

	This->DelayServerRequest();

	int apiCode = gUserProfile.ApiRetBonusGetInfo();
	if (apiCode != 0)
	{
		This->SetAsyncError(apiCode, "Failed to show reward popup");
		return 0;
	}

	return 1;
}

void FrontEnd::OnShowRewardPopUpSuccess()
{
	gfxFrontEnd.Invoke("_root.api.hideInfoMsg", "");

	if (gUserProfile.GetProfile() == 0)// get profile
	{
		// get texts
		int hours = (int)floorf(float(gUserProfile.minutesToNextRetDay_) / 60.0f);
		int mins = gUserProfile.minutesToNextRetDay_ - hours * 60;
		char tempStr[128];
		if (mins > 0)
			sprintf(tempStr, "%02d:%02d", hours, mins);
		else
			sprintf(tempStr, "24:00");
		char tempStr2[256];
		sprintf(tempStr2, "%s %s", gLangMngr.getString("$FR_NextDailyRewardIn"), tempStr);
		gfxFrontEnd.SetVariable("_root.api.Main.PopUpDailyRewards.TitleNextDrop.text", tempStr2);
		char tempStr3[128] = "";
		//sprintf(tempStr3, "%s %d", gLangMngr.getString("$FR_CurDay"), gUserProfile.curRetentionDays_);
		//sprintf(tempStr3, "1.7.2018 - 30.7.2018");
		gfxFrontEnd.SetVariable("_root.api.Main.PopUpDailyRewards.TitleCurDay.text", tempStr3);

		// add data to UI
		for (uint32_t i = 0; i < gUserProfile.retentionBonusByDays_.size(); ++i)
		{
			Scaleform::GFx::Value var[3];
			var[0].SetInt(gUserProfile.curRetentionDays_);
			var[1].SetInt(gUserProfile.retentionBonusByDays_[i]);
			if (gUserProfile.ProfileData.PremiumAcc > 0)// set x2 for premium
				var[2].SetInt(gUserProfile.retentionQuantity_[i] * 2);
			else
				var[2].SetInt(gUserProfile.retentionQuantity_[i]);
			gfxFrontEnd.Invoke("_root.api.Main.PopUpDailyRewards.addDailyRewardsData", var, 3);

			//play animation and sound
			if (gUserProfile.ProfileData.showRewardPopUp > 0)
				gfxFrontEnd.Invoke("_root.api.Main.PopUpDailyRewards.ShowNewRewardAnim", "");
		}

		// show screen
		gfxFrontEnd.Invoke("_root.api.Main.PopUpDailyRewards.showPopUp", "");

		// close message job
		StartAsyncOperation(&FrontEnd::as_CloseRewardPopUpThread, &FrontEnd::OnCloseRewardPopUpSuccess);
	}
}

unsigned int WINAPI FrontEnd::as_CloseRewardPopUpThread(void* in_data)
{
	r3dThreadAutoInstallCrashHelper crashHelper;
	FrontEnd* This = (FrontEnd*)in_data;

	This->DelayServerRequest();

	int apiCode = gUserProfile.ApiCloseRewardPopUp(This->m_closeRewardMessageActionID);
	if (apiCode != 0)
	{
		This->SetAsyncError(apiCode, "Failed to change show popup status");
		return 0;
	}

	return 1;
}

void FrontEnd::OnCloseRewardPopUpSuccess()
{
	gfxFrontEnd.Invoke("_root.api.hideInfoMsg", "");
	m_closeRewardMessageActionID = 0;
}

void FrontEnd::eventTeleportPlayers(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	float m_PosX = (float)args[0].GetNumber();
	float m_PosY = (float)args[1].GetNumber();
	float m_PosZ = (float)args[2].GetNumber();

	char positionstring[128];
	sprintf(positionstring, "%f %f %f", m_PosX, m_PosY, m_PosZ);
	r3dscpy(m_Pos, positionstring);

	Scaleform::GFx::Value var[2];
	var[0].SetString(gLangMngr.getString("OneMomentPlease"));
	var[1].SetBoolean(false);
	gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);

	StartAsyncOperation(&FrontEnd::as_TeleportCharThread, &FrontEnd::OnTeleportCharSuccess);
}

unsigned int WINAPI FrontEnd::as_TeleportCharThread(void* in_data)
{
	r3dThreadAutoInstallCrashHelper crashHelper;
	FrontEnd* This = (FrontEnd*)in_data;
	wiCharDataFull& w = gUserProfile.ProfileData.ArmorySlots[gUserProfile.SelectedCharID];

	//if(!gUserProfile.ProfileData.PremiumAcc)
	//{
	if (gUserProfile.ShopCharTeleportGD > 0)
	{
		if (gUserProfile.ProfileData.GameDollars < gUserProfile.ShopCharTeleportGD)
		{
			This->SetAsyncError(0, gLangMngr.getString("NotEnoughGD"));
			return 0;
		}
	}
	else if (gUserProfile.ShopCharTeleportGC > 0)
	{
		if (gUserProfile.ProfileData.GamePoints < gUserProfile.ShopCharTeleportGC)
		{
			This->SetAsyncError(0, gLangMngr.getString("NotEnoughGC"));
			return 0;
		}
	}
	//}

	if (w.GameMapId != GBGameInfo::MAPID_ZP_Test)
	{
		This->SetAsyncError(0, gLangMngr.getString("ForbiddenForThisMap"));
		return 0;
	}

	This->DelayServerRequest();

	int apiCode = gUserProfile.ApiTeleportChar(This->m_Pos);
	if (apiCode != 0)
	{
		if (apiCode == 9)
			This->SetAsyncError(0, "Failed to teleport character\n Reason: Cooldown is active!");
		else
			This->SetAsyncError(apiCode, "Failed to teleport character");
		return 0;
	}

	return 1;
}

void FrontEnd::OnTeleportCharSuccess()
{
	Scaleform::GFx::Value var[3];

	eventShowSurvivorsMapRefresh();

	var[0].SetInt(gUserProfile.ProfileData.GamePoints);
	gfxFrontEnd.Invoke("_root.api.setGC", var, 1);

	var[0].SetInt(gUserProfile.ProfileData.GameDollars);
	gfxFrontEnd.Invoke("_root.api.setDollars", var, 1);

	var[0].SetInt(gUserProfile.ProfileData.XP50Boosts);
	gfxFrontEnd.Invoke("_root.api.setXPBoosts", var, 1);

	var[0].SetInt(0);
	gfxFrontEnd.Invoke("_root.api.setCells", var, 1);
}

void FrontEnd::eventRequestUpgradeAccBonusLootData(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount == 1);
	m_AccBonusType = args[0].GetInt();

	Scaleform::GFx::Value var[2];
	var[0].SetString(gLangMngr.getString("OneMomentPlease"));
	var[1].SetBoolean(false);
	gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);

	StartAsyncOperation(&FrontEnd::as_RequestUpgradeAccBonusLootDataThread, &FrontEnd::OnRequestUpgradeAccBonusLootDataSuccess);
}

unsigned int FrontEnd::as_RequestUpgradeAccBonusLootDataThread(void* in_data)
{
	r3dThreadAutoInstallCrashHelper crashHelper;
	FrontEnd* This = (FrontEnd*)in_data;

	This->DelayServerRequest();

	int apiCode = gUserProfile.ApiGetUpgradeAccBonusLootData(This->m_AccBonusType);
	if (apiCode != 0)
	{
		This->SetAsyncError(apiCode, gLangMngr.getString("FailedToGetUpgradeAccBonusLootData"));
		return 0;
	}

	return 1;
}

void FrontEnd::OnRequestUpgradeAccBonusLootDataSuccess()
{
	gfxFrontEnd.Invoke("_root.api.hideInfoMsg", "");

	// fill bonus items
	for (size_t i = 0; i < gUserProfile.BonusList_.size(); i++)
	{
		const CClientUserProfile::BonusList_s& bl = gUserProfile.BonusList_[i];
		Scaleform::GFx::Value args[2];
		//public function addUpgradeAccBonusLootData(ItemID:int, Amount:int)
		args[0].SetInt(bl.ItemID);
		args[1].SetInt(bl.Amount);
		gfxFrontEnd.Invoke("_root.api.Main.UpgradeAccountPopUp.addUpgradeAccBonusLootData", args, 2);
	}

	gUserProfile.BonusList_.clear();
	gfxFrontEnd.Invoke("_root.api.Main.UpgradeAccountPopUp.showBonusPopUp", "");
}

void FrontEnd::scan_loadout()
{
	clear_loadout();
	clear_loadout_pictures();

	WIN32_FIND_DATA ffblk;
	HANDLE hFind;

	hFind = FindFirstFile(Va("*%s", LOADOUT_SYSTEM_FILE_EXTENSION), &ffblk);
	if (hFind == INVALID_HANDLE_VALUE) {
		return;
	}

	do
	{
		if (ffblk.nFileSizeHigh != 0)
			continue;

		add_loadout(ffblk.cFileName);
	} while (FindNextFile(hFind, &ffblk) != 0);

	FindClose(hFind);

	refresh_loadout();
}

void FrontEnd::clear_loadout()
{
	gfxFrontEnd.Invoke("_root.api.clear_loadout", "");
}

void FrontEnd::add_loadout(const char* filename)
{
	const wiCharDataFull& slot = gUserProfile.ProfileData.ArmorySlots[gUserProfile.SelectedCharID];

	loadout l;
	l.load_loadout(slot, filename);

	Scaleform::GFx::Value args[2];
	args[0].SetString(l.name);
	args[1].SetString(filename);
	gfxFrontEnd.Invoke("_root.api.add_loadout", args, 2);

	for (int i = 0; i < 8; i++)
	{
		if (l.item[i].ItemID > 0)
		{
			const GearConfig* gc = g_pWeaponArmory->getGearConfig(l.item[i].ItemID);
			const BaseItemConfig* ic = g_pWeaponArmory->getWeaponConfig(l.item[i].ItemID);
			const WeaponAttachmentConfig* atmCfg = g_pWeaponArmory->getAttachmentConfig(l.item[i].ItemID);

			if (gc)
				add_loadout_picture(filename, gc->m_StoreIcon);
			else if (ic)
				add_loadout_picture(filename, ic->m_StoreIcon);
			else if (atmCfg)
				add_loadout_picture(filename, atmCfg->m_StoreIcon);
		}
		else
			add_loadout_picture(filename, ""); // we need this to finish a logic
	}

	const BackpackConfig* backpack = g_pWeaponArmory->getBackpackConfig(l.BackpackID);
	if (backpack)
		add_loadout_picture(filename, backpack->m_StoreIcon);
	else
		add_loadout_picture(filename, ""); // we need this to finish a logic
}

void FrontEnd::refresh_loadout()
{
	gfxFrontEnd.Invoke("_root.api.refresh_loadout", "");
}

void FrontEnd::eventLoad_Loadout(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount == 1);

	const char* name = args[0].GetString();
	load_loadout(name);
}

void FrontEnd::load_loadout(const char* name)
{
	const wiCharDataFull& slot = gUserProfile.ProfileData.ArmorySlots[gUserProfile.SelectedCharID];

	loadout l;
	l.load_loadout(slot, name);

	Scaleform::GFx::Value var[2];
	var[0].SetString(gLangMngr.getString("OneMomentPlease"));
	var[1].SetBoolean(false);
	gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);

	m_inventoryID = 0;
	m_gridTo = 0;
	m_Amount = 0;

	for (int i = 0; i < l.Backpacksize; i++)
	{
		if (l.item[i].ItemID <= 0)
			continue;

		m_gridTo = l.item[i].SlotID;
		m_Amount = l.item[i].Quantity;

		m_gridFrom = m_gridTo;
		m_Amount2 = slot.Items[m_gridTo].quantity;

		// check weight
		float totalWeight = slot.getTotalWeight();
		const BaseItemConfig* bic = g_pWeaponArmory->getConfig(slot.Items[m_gridTo].itemID);
		if (bic)
		{
			if (i != 0 && i != 1 && i != 6 && i != 7)
				totalWeight -= bic->m_Weight*slot.Items[m_gridTo].quantity;
		}

		bic = g_pWeaponArmory->getConfig(l.item[i].ItemID);
		if (bic)
		{
			if (i != 0 && i != 1 && i != 6 && i != 7)
				totalWeight += bic->m_Weight*m_Amount;

			if (slot.Skills[CUserSkills::SKILL_Physical3])
				totalWeight *= 0.95f;
			if (slot.Skills[CUserSkills::SKILL_Physical7])
				totalWeight *= 0.9f;
		}

		const BackpackConfig* bc = g_pWeaponArmory->getBackpackConfig(slot.BackpackID);
		r3d_assert(bc);
		if (totalWeight > bc->m_maxWeight)
		{
			Scaleform::GFx::Value var[3];
			var[0].SetString(gLangMngr.getString("FR_PAUSE_TOO_MUCH_WEIGHT"));
			var[1].SetBoolean(true);
			var[2].SetString("");
			gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 3);
			return;
		}
	}

	loadout_ = l;
	StartAsyncOperation(&FrontEnd::as_LOADOUT_BackpackFromInventoryThread_MAIN, &FrontEnd::OnLOADOUT_BackpackFromInventorySuccess);
}

bool FrontEnd::find_free_loadout_name(const char* fname)
{
	std::ifstream file(fname);
	return file.is_open();
}

void FrontEnd::eventSave_Loadout(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount == 2);

	char* name = (char*)args[0].GetString();
	int count = args[1].GetInt();
	char* fname;
	bool found = false;
	for (int i = 0; i < count; i++)
	{
		fname = (char*)Va("Loadout%i%s", i, LOADOUT_SYSTEM_FILE_EXTENSION);
		if (!find_free_loadout_name(fname))
		{
			found = true;
			break;
		}
	}

	if (!found)
		fname = (char*)Va("Loadout%i%s", count + 1, LOADOUT_SYSTEM_FILE_EXTENSION);

	save_loadout(fname, name);
	scan_loadout();
}

void FrontEnd::save_loadout(const char* fname, const char* name)
{
	const wiCharDataFull& slot = gUserProfile.ProfileData.ArmorySlots[gUserProfile.SelectedCharID];
	loadout l;
	l.save_loadout(slot, fname, name);
}

void FrontEnd::clear_loadout_pictures()
{
	gfxFrontEnd.Invoke("_root.api.clear_loadout_picture", "");
}

void FrontEnd::add_loadout_picture(const char* filename, const char* picture)
{
	Scaleform::GFx::Value args[2];
	args[0].SetString(filename);
	args[1].SetString(picture);
	gfxFrontEnd.Invoke("_root.api.add_loadout_picture", args, 2);
}

void FrontEnd::eventDelete_Loadout(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount == 1);

	char* fname = (char*)args[0].GetString();
	DeleteFile(fname);
	scan_loadout();
}
// Blackmarket System Begin
unsigned int WINAPI FrontEnd::async_DoLoadBlackmarket(void* in_data)
{
	r3dThreadAutoInstallCrashHelper crashHelper;
	FrontEnd* This = (FrontEnd*)in_data;

	// do only request data if screen is blackmarket
	if (This->cMarketplaceScreen != eMarketplaceScreen::Blackmarket)
	{
		return 1;
	}

	This->DelayServerRequest();

	int apiCode = gUserProfile.ApiFetchBlackmarket(This->m_BlackmarketCategoriesRequest);
	if (apiCode != 0)
	{
		This->SetAsyncError(apiCode, gLangMngr.getString("$FR_BLACKMARKET_UNABLE_TO_FETCH"));
		return 0;
	}

	return 1;
}

void FrontEnd::OnDoLoadBlackmarket()
{
	gfxFrontEnd.Invoke("_root.api.hideInfoMsg", "");

	clearBlackmarketUI(gfxFrontEnd);
	addBlackmarketToUI(gfxFrontEnd);

	Scaleform::GFx::Value var[1];
	var[0].SetInt((int)cMarketplaceScreen);
	gfxFrontEnd.Invoke("_root.api.SetMarketplaceScreen", var, 1);

	// refresh
	gfxFrontEnd.Invoke("_root.api.Main.Marketplace.RefreshSelectedTab", "");

	if (m_BlackmarketListWasOutdated)
	{
		Scaleform::GFx::Value var[3];
		var[0].SetString("BLACKMARKET CHANGED, ACTION COULD NOT BE PERFORMED");
		var[1].SetBoolean(true);
		var[2].SetString("$ERROR");
		gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 3);
		m_BlackmarketListWasOutdated = false;
	}
}

void FrontEnd::eventMarketplaceShowBlackmarket(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	// use frontend to request currently selected categories
	Scaleform::GFx::Value vars[1];
	vars[0].SetInt((int)(cMarketplaceScreen == eMarketplaceScreen::Blackmarket ? eMarketplaceScreen::Market : eMarketplaceScreen::Blackmarket));
	gfxFrontEnd.Invoke("_root.api.Main.Marketplace.DoLoadBlackmarketWithSelectedTab", vars, 1);
}

unsigned int WINAPI FrontEnd::async_DoBuyItemFromBlackmarket(void* in_data)
{
	r3dThreadAutoInstallCrashHelper crashHelper;
	FrontEnd* This = (FrontEnd*)in_data;

	// screen is not blackmarket - do not perform buy request
	if (This->cMarketplaceScreen != eMarketplaceScreen::Blackmarket)
	{
		return 0;
	}

	This->DelayServerRequest();

	int apiCode = gUserProfile.ApiBuyItemFromBlackmarket(This->m_BlackmarketBuyerPK_ID, This->m_BlackmarketBuyerQuantity, This->m_BlackmarketBuyerInventoryID, This->m_BlackmarketBuyerItemID, This->m_BlackmarketBuyerOutQuantity, This->m_BlackmarketBuyerItemVar1, This->m_BlackmarketBuyerItemVar2, This->m_BlackmarketBuyerItemVar3);
	if (apiCode != 0)
	{
		// something fucked up or item isnt valid anymore - perform refresh
		This->m_BlackmarketRefreshDirty = true;

		This->SetAsyncError(apiCode, gLangMngr.getString("$FR_BLACKMARKET_UNABLE_TO_BUY"));
		return 0;
	}

	This->m_BlackmarketRefreshDirty = false;
	return 1;
}

void FrontEnd::OnBoughtItemFromBlackmarket()
{
	bool RequestQuantityTooBig = false;

	// my buy request quantity is bigger than on stock
	if (m_BlackmarketBuyerQuantity != m_BlackmarketBuyerOutQuantity)
	{
		// bought it all
		m_BlackmarketBuyerQuantity = m_BlackmarketBuyerOutQuantity;
		RequestQuantityTooBig = true;
	}

	for (auto it = vBlackmarketItems.begin(); it != vBlackmarketItems.end(); ++it)
	{
		auto& itm = *it;
		if (itm.PK_BlackmarketID == m_BlackmarketBuyerPK_ID)
		{
			// my buy request quantity is bigger than on stock
			if (RequestQuantityTooBig)
			{
				// bought it all
				vBlackmarketItems.erase(it);
				break;
			}

			if ((int)((int)itm.Quantity - (int)m_BlackmarketBuyerQuantity) <= 0)
			{
				vBlackmarketItems.erase(it);
				break;
			}

			itm.Quantity -= m_BlackmarketBuyerQuantity;
		}
	}

	clearBlackmarketUI(gfxFrontEnd);
	addBlackmarketToUI(gfxFrontEnd);

	// refresh
	gfxFrontEnd.Invoke("_root.api.Main.Marketplace.RefreshSelectedTab", "");

	// try find existing slot in inventory
	bool bHaveFoundInInventory = false;
	for (uint32_t i = 0; i < gUserProfile.ProfileData.NumItems; i++)
	{
		if (gUserProfile.ProfileData.Inventory[i].itemID == m_BlackmarketBuyerItemID)
		{
			if (gUserProfile.ProfileData.Inventory[i].InventoryID == m_BlackmarketBuyerInventoryID)
			{
				if (gUserProfile.ProfileData.Inventory[i].Var1 == m_BlackmarketBuyerItemVar1
					&& gUserProfile.ProfileData.Inventory[i].Var2 == m_BlackmarketBuyerItemVar2
					&& gUserProfile.ProfileData.Inventory[i].Var3 == m_BlackmarketBuyerItemVar3)
				{
					gUserProfile.ProfileData.Inventory[i].quantity += m_BlackmarketBuyerQuantity;
					bHaveFoundInInventory = true;
					break;
				}
			}
		}
	}

	if (!bHaveFoundInInventory)
	{
		// add item to our inventory
		wiInventoryItem* itm = gUserProfile.getFreeInventorySlot();
		if (itm)
		{
			itm->InventoryID = m_BlackmarketBuyerInventoryID;
			itm->itemID = m_BlackmarketBuyerItemID;
			itm->quantity = m_BlackmarketBuyerQuantity;
			itm->Var1 = m_BlackmarketBuyerItemVar1;
			itm->Var2 = m_BlackmarketBuyerItemVar2;
			itm->Var3 = m_BlackmarketBuyerItemVar3;
		}
	}

	updateInventoryAndSkillItems();

	Scaleform::GFx::Value var[1];
	var[0].SetInt(gUserProfile.ProfileData.GamePoints);
	gfxFrontEnd.Invoke("_root.api.setGC", var, 1);

	var[0].SetInt(gUserProfile.ProfileData.GameDollars);
	gfxFrontEnd.Invoke("_root.api.setDollars", var, 1);

	gfxFrontEnd.Invoke("_root.api.hideInfoMsg", "");

	// only bought part of it
	if (RequestQuantityTooBig)
	{
		char msg[255];
		sprintf_s(msg, "Bought %d amount", m_BlackmarketBuyerOutQuantity);

		Scaleform::GFx::Value var[3];
		var[0].SetString(msg);
		var[1].SetBoolean(true);
		var[2].SetString("$ERROR");
		gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 3);
	}
}

unsigned int WINAPI FrontEnd::async_DoRemoveItemFromBlackmarket(void* in_data)
{
	r3dThreadAutoInstallCrashHelper crashHelper;
	FrontEnd* This = (FrontEnd*)in_data;

	// screen is not blackmarket - do not perform buy request
	if (This->cMarketplaceScreen != eMarketplaceScreen::Blackmarket)
	{
		return 0;
	}

	This->DelayServerRequest();

	int apiCode = gUserProfile.ApiRemoveItemFromBlackmarket(This->m_BlackmarketBuyerPK_ID, This->m_BlackmarketBuyerQuantity, This->m_BlackmarketBuyerInventoryID, This->m_BlackmarketBuyerItemID, This->m_BlackmarketBuyerOutQuantity, This->m_BlackmarketBuyerItemVar1, This->m_BlackmarketBuyerItemVar2, This->m_BlackmarketBuyerItemVar3);
	if (apiCode != 0)
	{
		// something fucked up or item isnt valid anymore - perform refresh
		This->m_BlackmarketRefreshDirty = true;

		This->SetAsyncError(apiCode, gLangMngr.getString("$FR_BLACKMARKET_UNABLE_TO_REMOVE"));
		return 0;
	}

	This->m_BlackmarketRefreshDirty = false;
	return 1;
}

void FrontEnd::OnRemovedItemFromBlackmarket()
{
	bool RequestQuantityTooBig = false;

	// my buy request quantity is bigger than on stock
	if (m_BlackmarketBuyerQuantity != m_BlackmarketBuyerOutQuantity)
	{
		// remove it all
		m_BlackmarketBuyerQuantity = m_BlackmarketBuyerOutQuantity;
		RequestQuantityTooBig = true;
	}

	for (auto it = vBlackmarketItems.begin(); it != vBlackmarketItems.end(); ++it)
	{
		auto& itm = *it;
		if (itm.PK_BlackmarketID == m_BlackmarketBuyerPK_ID)
		{
			// my remove request quantity is bigger than on stock
			if (RequestQuantityTooBig)
			{
				// remove it all
				vBlackmarketItems.erase(it);
				break;
			}

			if ((int)((int)itm.Quantity - (int)m_BlackmarketBuyerQuantity) <= 0)
			{
				vBlackmarketItems.erase(it);
				break;
			}

			itm.Quantity -= m_BlackmarketBuyerQuantity;
		}
	}

	clearBlackmarketUI(gfxFrontEnd);
	addBlackmarketToUI(gfxFrontEnd);

	// refresh
	gfxFrontEnd.Invoke("_root.api.Main.Marketplace.RefreshSelectedTab", "");

	// try find existing slot in inventory
	bool bHaveFoundInInventory = false;
	for (uint32_t i = 0; i < gUserProfile.ProfileData.NumItems; i++)
	{
		if (gUserProfile.ProfileData.Inventory[i].itemID == m_BlackmarketBuyerItemID)
		{
			if (gUserProfile.ProfileData.Inventory[i].InventoryID == m_BlackmarketBuyerInventoryID)
			{
				if (gUserProfile.ProfileData.Inventory[i].Var1 == m_BlackmarketBuyerItemVar1
					&& gUserProfile.ProfileData.Inventory[i].Var2 == m_BlackmarketBuyerItemVar2
					&& gUserProfile.ProfileData.Inventory[i].Var3 == m_BlackmarketBuyerItemVar3)
				{
					gUserProfile.ProfileData.Inventory[i].quantity += m_BlackmarketBuyerQuantity;
					bHaveFoundInInventory = true;
					break;
				}
			}
		}
	}

	if (!bHaveFoundInInventory)
	{
		// add item to our inventory
		wiInventoryItem* itm = gUserProfile.getFreeInventorySlot();
		if (itm)
		{
			itm->InventoryID = m_BlackmarketBuyerInventoryID;
			itm->itemID = m_BlackmarketBuyerItemID;
			itm->quantity = m_BlackmarketBuyerQuantity;
			itm->Var1 = m_BlackmarketBuyerItemVar1;
			itm->Var2 = m_BlackmarketBuyerItemVar2;
			itm->Var3 = m_BlackmarketBuyerItemVar3;
		}
	}

	updateInventoryAndSkillItems();

	gfxFrontEnd.Invoke("_root.api.hideInfoMsg", "");

	// only bought part of it
	if (RequestQuantityTooBig)
	{
		char msg[255];
		sprintf_s(msg, "Removed %d amount", m_BlackmarketBuyerOutQuantity);

		Scaleform::GFx::Value var[3];
		var[0].SetString(msg);
		var[1].SetBoolean(true);
		var[2].SetString("$ERROR");
		gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 3);
	}
}

unsigned int WINAPI FrontEnd::async_DoSellItemOnBlackmarket(void* in_data)
{
	r3dThreadAutoInstallCrashHelper crashHelper;
	FrontEnd* This = (FrontEnd*)in_data;

	This->DelayServerRequest();

	int apiCode = gUserProfile.ApiSellItemOnBlackmarket(This->m_BlackmarketBuyerInventoryID, This->m_BlackmarketSellerSinglePrice, This->m_BlackmarketBuyerQuantity, This->m_BlackmarketCurrencyType, This->m_BlackmarketBuyerInventoryID, This->m_BlackmarketBuyerQuantity);
	if (apiCode != 0)
	{
		This->SetAsyncError(apiCode, gLangMngr.getString("$FR_BLACKMARKET_UNABLE_TO_SELL"));
		return 0;
	}

	return 1;
}

void FrontEnd::OnSellItemOnBlackmarket()
{
	// adjust inventory change
	for (uint32_t i = 0; i < gUserProfile.ProfileData.NumItems; i++)
	{
		if (gUserProfile.ProfileData.Inventory[i].InventoryID == m_BlackmarketBuyerInventoryID)
		{
			gUserProfile.ProfileData.Inventory[i].quantity -= m_BlackmarketBuyerQuantity;
			if (gUserProfile.ProfileData.Inventory[i].quantity <= 0)
				gUserProfile.ProfileData.Inventory[i].Reset();

			break;
		}
	}

	for (size_t i = 0; i < gUserProfile.ProfileData.ArmorySlots[gUserProfile.SelectedCharID].BackpackSize; ++i)
	{
		if (gUserProfile.ProfileData.ArmorySlots[gUserProfile.SelectedCharID].Items[i].InventoryID == m_BlackmarketBuyerInventoryID)
		{
			gUserProfile.ProfileData.ArmorySlots[gUserProfile.SelectedCharID].Items[i].quantity -= m_BlackmarketBuyerQuantity;
			if (gUserProfile.ProfileData.ArmorySlots[gUserProfile.SelectedCharID].Items[i].quantity <= 0)
				gUserProfile.ProfileData.ArmorySlots[gUserProfile.SelectedCharID].Items[i].Reset();

			break;
		}
	}

	Scaleform::GFx::Value var[8];
	gfxFrontEnd.Invoke("_root.api.clearBackpack", "");
	int	slot = gUserProfile.SelectedCharID;

	addBackpackItems(gUserProfile.ProfileData.ArmorySlots[slot], slot);

	updateInventoryAndSkillItems();

	var[0].SetInt(gUserProfile.ProfileData.GamePoints);
	gfxFrontEnd.Invoke("_root.api.setGC", var, 1);

	var[0].SetInt(gUserProfile.ProfileData.GameDollars);
	gfxFrontEnd.Invoke("_root.api.setDollars", var, 1);

	gfxFrontEnd.Invoke("_root.api.backpackFromInventorySuccess", "");
	gfxFrontEnd.Invoke("_root.api.backpackToInventorySuccess", "");
	gfxFrontEnd.Invoke("_root.api.hideInfoMsg", "");
}

void FrontEnd::eventBuyItemFromBlackmarket(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount == 2);

	auto PK_BlackmarketID = args[0].GetUInt();
	auto Quantity = args[1].GetUInt();

	m_BlackmarketBuyerPK_ID = PK_BlackmarketID;
	m_BlackmarketBuyerQuantity = Quantity;

	bool bFoundBlackmarketItem = false;
	wiBlackmarketItem BlackmarketItem;
	for (auto it = vBlackmarketItems.begin(); it != vBlackmarketItems.end(); ++it)
	{
		auto& itm = *it;
		if (itm.PK_BlackmarketID == m_BlackmarketBuyerPK_ID)
		{
			BlackmarketItem = itm;
			bFoundBlackmarketItem = true;
			break;
		}
	}

	if (!bFoundBlackmarketItem)
	{
		Scaleform::GFx::Value var[3];
		var[0].SetString("BLACKMARKET ITEM IS NOT VALID");
		var[1].SetBoolean(true);
		var[2].SetString("$ERROR");
		gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 3);
		return;
	}

	if (gUserProfile.CustomerID == BlackmarketItem.FK_CustomerID)
	{
		Scaleform::GFx::Value var[3];
		var[0].SetString("CANNOT BUY MY OWN ITEM");
		var[1].SetBoolean(true);
		var[2].SetString("$ERROR");
		gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 3);
		return;
	}

	if (BlackmarketItem.CurrencyType == 4) // GC
	{
		if ((int)(gUserProfile.ProfileData.GamePoints - (int)(BlackmarketItem.SinglePrice * m_BlackmarketBuyerQuantity)) < 0)
		{
			// not enough money
			Scaleform::GFx::Value var[3];
			var[0].SetString("NOT ENOUGH GC");
			var[1].SetBoolean(true);
			var[2].SetString("$ERROR");
			gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 3);
			return;
		}
	}
	else if (BlackmarketItem.CurrencyType == 8) // GD
	{
		if ((int)(gUserProfile.ProfileData.GameDollars - (int)(BlackmarketItem.SinglePrice * m_BlackmarketBuyerQuantity)) < 0)
		{
			// not enough money
			Scaleform::GFx::Value var[3];
			var[0].SetString("NOT ENOUGH GD");
			var[1].SetBoolean(true);
			var[2].SetString("$ERROR");
			gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 3);
			return;
		}
	}
	else
	{
		Scaleform::GFx::Value var[3];
		var[0].SetString("INVALID BUYIDX");
		var[1].SetBoolean(true);
		var[2].SetString("$ERROR");
		gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 3);
		return;
	}

	Scaleform::GFx::Value var[2];
	var[0].SetString(gLangMngr.getString("OneMomentPlease"));
	var[1].SetBoolean(false);
	gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);

	StartAsyncOperation(&FrontEnd::async_DoBuyItemFromBlackmarket, &FrontEnd::OnBoughtItemFromBlackmarket);
}

void FrontEnd::eventRemoveItemFromBlackmarket(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount == 2);

	auto PK_BlackmarketID = args[0].GetUInt();
	auto Quantity = args[1].GetUInt();

	m_BlackmarketBuyerPK_ID = PK_BlackmarketID;
	m_BlackmarketBuyerQuantity = Quantity;

	bool bFoundBlackmarketItem = false;
	wiBlackmarketItem BlackmarketItem;
	for (auto it = vBlackmarketItems.begin(); it != vBlackmarketItems.end(); ++it)
	{
		auto& itm = *it;
		if (itm.PK_BlackmarketID == m_BlackmarketBuyerPK_ID)
		{
			BlackmarketItem = itm;
			bFoundBlackmarketItem = true;
			break;
		}
	}

	if (!bFoundBlackmarketItem)
	{
		Scaleform::GFx::Value var[3];
		var[0].SetString("BLACKMARKET ITEM IS NOT VALID");
		var[1].SetBoolean(true);
		var[2].SetString("$ERROR");
		gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 3);
		return;
	}

	if (gUserProfile.CustomerID != BlackmarketItem.FK_CustomerID)
	{
		Scaleform::GFx::Value var[3];
		var[0].SetString("CANNOT REMOVE AN ITEM I DO NOT OWN!");
		var[1].SetBoolean(true);
		var[2].SetString("$ERROR");
		gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 3);
		return;
	}

	Scaleform::GFx::Value var[2];
	var[0].SetString(gLangMngr.getString("OneMomentPlease"));
	var[1].SetBoolean(false);
	gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);

	StartAsyncOperation(&FrontEnd::async_DoRemoveItemFromBlackmarket, &FrontEnd::OnRemovedItemFromBlackmarket);
}

void FrontEnd::eventSellItemOnBlackmarket(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount == 4);

	auto InventoryID = args[0].GetUInt();
	auto Price = args[1].GetUInt();
	auto Quantity = args[2].GetUInt();
	auto CurrencyType = args[3].GetUInt();

	// check for currency
	if (CurrencyType != 4 && CurrencyType != 8)
	{
		Scaleform::GFx::Value var[3];
		var[0].SetString("CURRENCY NOT SUPPORTED!");
		var[1].SetBoolean(true);
		var[2].SetString("$ERROR");
		gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 3);
		return;
	}

	auto fee_enabled = GetBlackmarketSetting("opt_fee_enabled");
	if (fee_enabled)
	{
		if (fee_enabled->as_boolean())
		{
			// fee is enabled
			// now get perc value
			auto fee_perc = GetBlackmarketSetting("opt_fee_perc");
			if (fee_perc)
			{
				// calc total fee
				auto fee = Price * fee_perc->opt_value;
				fee = (int)fee * Quantity;

				// check enough money
				if (CurrencyType == 4) // GC
				{
					if ((gUserProfile.ProfileData.GamePoints - (int)fee) < 0)
					{
						Scaleform::GFx::Value var[3];
						var[0].SetString("FEE: NOT ENOUGH GC");
						var[1].SetBoolean(true);
						var[2].SetString("$ERROR");
						gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 3);
						return;
					}
				}
				else if (CurrencyType == 8) // GD
				{
					if ((gUserProfile.ProfileData.GameDollars - (int)fee) < 0)
					{
						Scaleform::GFx::Value var[3];
						var[0].SetString("FEE: NOT ENOUGH GD");
						var[1].SetBoolean(true);
						var[2].SetString("$ERROR");
						gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 3);
						return;
					}
				}
			}
		}
	}

	m_BlackmarketBuyerInventoryID = InventoryID;
	m_BlackmarketSellerSinglePrice = Price;
	m_BlackmarketBuyerQuantity = Quantity;
	m_BlackmarketCurrencyType = CurrencyType;

	Scaleform::GFx::Value var[2];
	var[0].SetString(gLangMngr.getString("OneMomentPlease"));
	var[1].SetBoolean(false);
	gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);

	StartAsyncOperation(&FrontEnd::async_DoSellItemOnBlackmarket, &FrontEnd::OnSellItemOnBlackmarket);
}

void FrontEnd::eventBlackmarketRefresh(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	// use frontend to request currently selected categories
	Scaleform::GFx::Value vars[1];
	vars[0].SetInt((int)(eMarketplaceScreen::Blackmarket));
	gfxFrontEnd.Invoke("_root.api.Main.Marketplace.DoLoadBlackmarketWithSelectedTab", vars, 1);
}

void FrontEnd::eventDoLoadBlackmarketWithSelectedCategory(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount == 2);

	m_BlackmarketCategoriesRequest.clear();

	auto Categories = args[0];
	for (size_t i = 0; i < Categories.GetArraySize(); ++i)
	{
		Scaleform::GFx::Value val;
		Categories.GetElement(i, &val);
		auto cat = val.GetNumber();
		m_BlackmarketCategoriesRequest.emplace_back((uint32_t)cat);
	}

	auto screen = args[1].GetInt();
	cMarketplaceScreen = (eMarketplaceScreen)screen;

	Scaleform::GFx::Value var[2];
	var[0].SetString(gLangMngr.getString("OneMomentPlease"));
	var[1].SetBoolean(false);
	gfxFrontEnd.Invoke("_root.api.showInfoMsg", var, 2);

	StartAsyncOperation(&FrontEnd::async_DoLoadBlackmarket, &FrontEnd::OnDoLoadBlackmarket);
}
// Blackmarket System End