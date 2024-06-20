#include "r3dPCH.h"
#include "r3d.h"

#include "Main_Network.h"

#include "r3dBackgroundTaskDispatcher.h"

#include "r3dNetwork.h"

#include "Particle.h"

#pragma warning (disable: 4244)
#pragma warning (disable: 4305)
#pragma warning (disable: 4101)

#include "cvar.h"
#include "fmod/soundsys.h"

#include "APIScaleformGFX.h"
#include "GameCommon.h"
#include "GameLevel.h"

#include "r3dNetwork.h"
#include "multiplayer/ClientGameLogic.h"
#include "multiplayer/MasterServerLogic.h"
#include "multiplayer/LoginSessionPoller.h"

#include "ObjectsCode/Gameplay/BasePlayerSpawnPoint.h"
#include "ObjectsCode/Gameplay/BaseItemSpawnPoint.h"

#include "UI\FrontEndWarZ.h"
#include "UI\LoadingScreen.h"

#include "Editors/CollectionsManager.h"

#include "TeamSpeakClient.h"

// temporary externals from game.cpp
extern void GameStateGameLoop();
extern void InitGame();
extern void DestroyGame();
extern void GameFrameStart();

extern bool IsNeedExit();
extern void InputUpdate();

extern EGameResult PlayNetworkGame();

void tempDoMsgLoop()
{
	r3dProcessWindowMessages();

	gClientLogic().Tick();
	gMasterServerLogic.Tick();
}

static void SetNewLogFile()
{
	extern void r3dChangeLogFile(const char* fname);
	char buf[512];
	sprintf(buf, "logs\\r3dlog_client_%d.txt", GetTickCount());
	r3dChangeLogFile(buf);
}

static bool ConnectToGameServer()
{
	r3d_assert(_p2p_gamePort);
	r3d_assert(_p2p_gameHost[0]);
	r3d_assert(_p2p_gameSessionId);

	gClientLogic().Reset();
	return gClientLogic().Connect(_p2p_gameHost, _p2p_gamePort);
}

#ifndef FINAL_BUILD
static void MasterServerQuckJoin()
{
	if(!DoConnectScreen(&gMasterServerLogic, &MasterServerLogic::IsConnected, gLangMngr.getString("WaitConnectingToServer"), 30.f ) )
		r3dError("can't connect to master server\n");

	NetPacketsMaster::GBPKT_C2M_QuickGameReq_s n;
	n.CustomerID     = gUserProfile.CustomerID;
	n.gameMap        = d_use_test_map->GetInt();
	n.region         = 0xFF;
	n.browseChannel  = 6;
	n.playerGameTime = 0;
		
	gMasterServerLogic.SendJoinQuickGame(n);
	
	const float endTime = r3dGetTime() + 60.0f;
	while(r3dGetTime() < endTime)
	{
		::Sleep(10);
		gMasterServerLogic.Tick();

		if(!gMasterServerLogic.IsConnected())
			break;

		if(gMasterServerLogic.gameJoinAnswered_)
		{
			break;
		}
	}

	if(gMasterServerLogic.gameJoinAnswer_.result != GBPKT_M2C_JoinGameAns_s::rOk)
		r3dError("failed to join game: res %d\n", gMasterServerLogic.gameJoinAnswer_.result);

	gMasterServerLogic.GetJoinedGameServer(_p2p_gameHost, &_p2p_gamePort, &_p2p_gameSessionId);

	gMasterServerLogic.Disconnect();
	
	extern bool g_bDisableP2PSendToHost;
	g_bDisableP2PSendToHost = false;
}
#endif

FrontEnd* frontend = NULL;
void loadFrontend()
{
	if (frontend)
		return;

	r3d_assert(frontend == NULL);

	frontend = game_new FrontEnd();
	frontend->Load();
}

const char* getLevelDirBasedOnLevelID(int mapID)
{
	switch(mapID) 
	{
	case GBGameInfo::MAPID_Editor_Particles: 
		return "WorkInProgress\\Editor_Particles"; 
	case GBGameInfo::MAPID_ServerTest:
		return "WorkInProgress\\ServerTest";
	case GBGameInfo::MAPID_ZP_Test: 
		return "ZP_Test"; 
	case GBGameInfo::MAPID_WZ_California: 
		return "California_V2"; 
	case GBGameInfo::MAPID_WZ_Cliffside: 
		return "WZ_Cliffside"; 
	case GBGameInfo::MAPID_WZ_Caliwood: 
		return "Caliwood"; 
	case GBGameInfo::MAPID_WZ_AircraftCarrier: 
		return "WZ_AircraftCarrier";
	case GBGameInfo::MAPID_WZ_Trade_Map: 
		return "WZ_Trade_Map";
	case GBGameInfo::MAPID_ZH_Warehouse: 
		return "ZH_Warehouse";
	case GBGameInfo::MAPID_WZ_BRmap: 
		return "WZ_BRmap";
	case GBGameInfo::MAPID_WZ_Clearview_V2: 
		return "WZ_Clearview_V2";
	case GBGameInfo::MAPID_WZ_Nevada: 
		return "WZ_Nevada";
	case GBGameInfo::MAPID_WZ_RockyFord: 
		return "WZ_RockyFord";
	case GBGameInfo::MAPID_WZ_Quarantine:
		return "WZ_Quarantine";
	}

	r3dError("invalid map id\n");
	return "";
}

void PlayMusic()
{
	mainmenuTheme = SoundSys.GetEventIDByPath("Sounds/MainMenu GUI/UI_MENU_MUSIC");
	MainMenuSoundEvent = SoundSys.Play(mainmenuTheme, r3dPoint3D(0, 0, 0));
}

void StopMusic()
{
	mainmenuTheme = -1;
	SoundSys.Stop(MainMenuSoundEvent);
	MainMenuSoundEvent = NULL;
}

void ExecuteNetworkGame(bool quickJoin)
{
	const char* showLoginErrorMsg = 0;
	EGameResult gameResult = GRESULT_Unknown;

repeat_the_login:
	loadFrontend();

	gLoginSessionPoller.Stop();

	int res = 0;
	frontend->initLoginStep(showLoginErrorMsg);
	while (res == 0) {
		res = frontend->Draw();
	}
	showLoginErrorMsg = 0;
	if (res == FrontEndShared::RET_Exit)
		return;

	gLoginSessionPoller.Start(gUserProfile.CustomerID, gUserProfile.SessionID);

	gMasterServerLogic.Disconnect();
	if (!gMasterServerLogic.StartConnect(g_serverip->GetString(), GBNET_CLIENT_PORT))
		r3dOutToLog("Error connecting to Master Server: couldn't connect to Servers.");

repeat_the_menu:
	loadFrontend();

	if(!quickJoin)
	{
		int res=0;
		frontend->postLoginStepInit(gameResult);
		while(res == 0) {
			res = frontend->Draw();
		}

		frontend->Unload();
		SAFE_DELETE(frontend);

		if(res == FrontEndShared::RET_Exit || res == 0)
			return;
		if( res == FrontEndShared::RET_Diconnected)
		{
			showLoginErrorMsg = gLangMngr.getString("LoginMenu_Disconnected");
			goto repeat_the_login;
		}
		else if( res == FrontEndShared::RET_Banned)
		{
			showLoginErrorMsg = gLangMngr.getString("LoginMenu_AccountFrozen");
			goto repeat_the_login;
		}
		else if( res == FrontEndShared::RET_DoubleLogin)
		{
			showLoginErrorMsg = gLangMngr.getString("LoginMenu_DoubleLogin");
			goto repeat_the_login;
		}
		r3d_assert(res == FrontEndShared::RET_JoinGame);
		gMasterServerLogic.GetJoinedGameServer(_p2p_gameHost, &_p2p_gamePort, &_p2p_gameSessionId);
	}
	else
	{
#ifndef FINAL_BUILD
		if(gUserProfile.CustomerID == 0)
			r3dError("bad login");

		if(gUserProfile.GetProfile() != 0)
			r3dError("unable to get profile");
		gUserProfile.SelectedCharID = 0;

		gLoadingScreen->StartLoadingScreen();
		MasterServerQuckJoin();
		r3dOutToLog("connecting to %s\n", _p2p_gameHost);
		gLoadingScreen->StopLoadingScreen();
#endif		
	}

	r3dEnsureDeviceAvailable();

	gLoadingScreen->StartLoadingScreen();

	if(!ConnectToGameServer())
	{
		gClientLogic().Disconnect();
		gLoadingScreen->StopLoadingScreen();
		showLoginErrorMsg = gLangMngr.getString("LoginMenu_CannotConnectServer");
		goto repeat_the_login;
	}

	r3dEnsureDeviceAvailable();

	if(gClientLogic().ValidateServerVersion(_p2p_gameSessionId) == 0)
	{
		gClientLogic().Disconnect();

		gLoadingScreen->StopLoadingScreen();
		if(gClientLogic().serverVersionStatus_ == 0) // timeout on validating version
		{
			gameResult = GRESULT_Disconnect;
			goto repeat_the_menu;
		}
		else
		{
			showLoginErrorMsg = gLangMngr.getString("LoginMenu_ClientUpdateRequired");
			goto repeat_the_login;
		}
	}

	r3dEnsureDeviceAvailable();

	if(!gClientLogic().RequestToJoinGame())
	{
		gClientLogic().Disconnect();

		gLoadingScreen->StopLoadingScreen();
		gameResult = GRESULT_Disconnect;
		goto repeat_the_menu;
	}

	gLoginSessionPoller.ForceTick(); // force to update that we joined the game

	r3dGameLevel::SetHomeDir(getLevelDirBasedOnLevelID(gClientLogic().m_gameInfo.mapId));

#ifdef DISABLE_GI_ACCESS_FOR_DEV_EVENT_SERVER
	if (gClientLogic().m_gameInfo.IsDevEvent())
	{	
		if (gClientLogic().m_gameInfo.flags & GBGameInfo::SFLAGS_DisableASR)	      	
			gClientLogic().LoadDevEventLoadoutSnipersMode();
		else if (gClientLogic().m_gameInfo.flags & GBGameInfo::SFLAGS_DisableSNP)	
			gClientLogic().LoadDevEventLoadoutNoSnipersMode();
		else
			gClientLogic().LoadDevEventLoadout();
	}
#endif
#ifdef ENABLE_BATTLE_ROYALE
	//AlexRedd:: BR mode
	if (gClientLogic().m_gameInfo.IsGameBR())		
		gClientLogic().LoadDevEventLoadoutBR();
#endif //ENABLE_BATTLE_ROYALE
	

	// start the game
	gameResult = PlayNetworkGame();
	
	if
	(
		gameResult == GRESULT_Exit ||
		gameResult == GRESULT_Disconnect ||
		gameResult == GRESULT_Finished
	)
	{
		r3dRenderer->ChangeForceAspect(16.0f / 9);
	}

	gLoadingScreen->StopLoadingScreen();

	gLoginSessionPoller.ForceTick(); // force to update that we left the game
	
	if(gameResult == GRESULT_DoubleLogin) {
		showLoginErrorMsg = gLangMngr.getString("LoginMenu_DoubleLogin");
		goto repeat_the_login;
	}
	if(gameResult == GRESULT_Unsync) {
		showLoginErrorMsg = gLangMngr.getString("ClientMustBeUpdated");
		goto repeat_the_login;
	}

	if(gameResult != GRESULT_Exit)
		goto repeat_the_menu;

	// clean up dictionary at the end of the game
	extern HashTableDynamic<const char*, FixedString256, NameHashFunc_T, 1024> dictionaryHash_;
	dictionaryHash_.Clear();
}

static EGameResult PlayNetworkGame()
{
#pragma omp parallel /*for*/ num_threads(omp_get_num_threads())
	r3d_assert(GameWorld().bInited == 0);

	r_hud_filter_mode->SetInt(0); // turn off NVG

	r3dEnsureDeviceAvailable();

	InitGame();

	extern void TPSGameHUD_OnStartGame();
	TPSGameHUD_OnStartGame();

	GameWorld().Update();

	EGameResult gameResult = GRESULT_Playing;
	if(gClientLogic().RequestToStartGame())
	{
		extern void SetHud(int iHud);
		SetHud(0);

		GameWorld().Update();

		// physics warm up
		g_pPhysicsWorld->StartSimulation();
		g_pPhysicsWorld->EndSimulation();

		if(MainMenuSoundEvent)
		{
			SoundSys.Stop(MainMenuSoundEvent);
			SoundSys.Release(MainMenuSoundEvent);
			MainMenuSoundEvent = 0;
		}
		
		// start team speak here, supply login credentials
		if(g_voip_enable->GetBool() == true)
		{
			gTeamSpeakClient.OpenDevices(g_voip_OutputDeviceInd->GetInt(), g_voip_InputDeviceInd->GetInt());
			gTeamSpeakClient.SetVolume(g_voip_volume->GetFloat());

			gTeamSpeakClient.StartConnect(
				_p2p_gameHost, 
				_p2p_gamePort + SBNET_VOICE_PORT_ADD, 
				gClientLogic().m_gameVoicePwd, 
				gClientLogic().m_gameVoiceId);
		}

		// enable those two lines to be able to profile first frames of game
		//r_show_profiler->SetBool(true);
		//r_profiler_paused->SetBool(false);

		r3dStartFrame();
		while(gameResult == GRESULT_Playing) 
		{
			r3dEndFrame();
			r3dStartFrame();

			ClearBackBufferFringes();

			R3DPROFILE_START("Game Frame");

			InputUpdate();

			GameFrameStart();

			if(!gClientLogic().serverConnected_)
			{
				if(gClientLogic().disconnectStatus_ == 2)
				{
					// disconnect was acked, game is finished
					gameResult = GRESULT_Finished;
				}
				else
				{
					gameResult = GRESULT_Disconnect;
				}
				continue;
			}

			GameStateGameLoop();

			//@ might not be a good place to tick (move to GameStateGameLoop), but that'll do for now.
			gTeamSpeakClient.Tick();

			R3DPROFILE_END("Game Frame");

			if(IsNeedExit())
			{
				// we should not allow quick exit from game
				// we can't just exit, because RakNet will properly disconnect immidiately.
				//- gameResult = GRESULT_Exit;

				// so we either need to request exit with PKT_C2S_DisconnectReq and wait
				// or terminate application so player will be disconnected on timeout
				TerminateProcess(r3d_CurrentProcess, 0);
			}

			// check for double login.
			#ifdef FINAL_BUILD
			if(!gLoginSessionPoller.IsConnected())
				gameResult = GRESULT_DoubleLogin;
			#endif

			FileTrackDoWork();
		} 
	}
	else // failed to connect
	{
		switch(gClientLogic().gameStartResult_)
		{
		case PKT_S2C_StartGameAns_s::RES_Timeout:
			gameResult = GRESULT_Timeout;
			break;
		case PKT_S2C_StartGameAns_s::RES_Failed:
			gameResult = GRESULT_Failed_To_Join_Game;
			break;
		case PKT_S2C_StartGameAns_s::RES_UNSYNC:
			gameResult = GRESULT_Unsync;
			break;
		case PKT_S2C_StartGameAns_s::RES_InvalidLogin:
			gameResult = GRESULT_DoubleLogin;
			break;
		case PKT_S2C_StartGameAns_s::RES_StillInGame:
			gameResult = GRESULT_StillInGame;
			break;
		default:
			gameResult = GRESULT_Disconnect;
			break;
		}
	}
	
	gTeamSpeakClient.Disconnect();
	
	gClientLogic().Disconnect();

	DestroyGame();

	if(gameResult != GRESULT_Exit)
		StopMusic();

	return gameResult;
}
