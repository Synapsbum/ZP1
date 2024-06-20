#include "r3dPCH.h"
#include "r3dProtect.h"

#include "HUDDisplay.h"

#include "ObjectsCode/Gameplay/BasePlayerSpawnPoint.h"
#include "../multiplayer/clientgamelogic.h"
#include "../ObjectsCode/ai/AI_Player.H"
#include "ObjectsCode/Gameplay/obj_Zombie.h"
#include "../ObjectsCode/weapons/Weapon.h"
#include "../ObjectsCode/weapons/WeaponArmory.h"
#include "TeamSpeakClient.h"
#include "../GameLevel.h"

#include "HUDPause.h"
#include "HUDSafelock.h"
#include "HUDAttachments.h"
#include "HUDVault.h"
#include "HUDStore.h"
#include "HUDTrade.h"
#include "HUDRepair.h"
#include "FrontEndShared.h"
#include "HUDSkins.h"//new add skin

#include "..\GameEngine\gameobjects\obj_Vehicle.h"

#ifndef FINAL_BUILD
#include "../../../GameEngine/ai/AutodeskNav/AutodeskNavMesh.h"
#endif // FINAL_BUILD

void writeGameOptionsFile();

extern HUDPause*	hudPause;
extern HUDSafelock* hudSafelock;
extern HUDVault*	hudVault;
extern HUDAttachments*	hudAttm;
extern HUDTrade* hudTrade;
extern HUDStore*	hudStore;
extern HUDRepair* hudRepair;
extern HUDSkins* hudSkins;//new add skin
#ifndef FINAL_BUILD
extern AutodeskNavMesh gAutodeskNavMesh;
#endif // FINAL_BUILD

HUDDisplay :: HUDDisplay()
	: Inited( false )
	, chatVisible( true ) //Cynthia:1752, initialize chatVisible to true
	, chatInputActive( false )
	, lastChatMessageSent( 0 )
	, currentChatChannel( 0 )
	, playersListVisible( false )
	, bloodAlpha( 0.0f )
	, writeNoteSavedSlotIDFrom( 0 )
	, timeoutForNotes( 0 )
	, timeoutNoteReadAbuseReportedHideUI( 0 )
	, RangeFinderUIVisible( false )
	, weaponInfoVisible( -1 )
	, SafeZoneWarningVisible( false )
	, TPSReticleVisible( -1 )
	, isReadNoteVisible( false )
	, isWriteNoteVisible( false )
	, isReadGravestoneVisible( false )
	, m_PrevBreathValue(-1)
	, chatHUDVisible( false )
	, isFoundSafeZone( false )
	, m_spawnSelectedType_( 0 )
	, canRespawnPlayer( false )	
#ifdef MISSIONS
	, nextMissionIndex( 0 )
	, missionHUDVisible( false )
	, radioctiveHUDVisible( false )
#ifdef MISSION_TRIGGERS
	, isReadMissionVisible( false )
#endif
#endif
	, isSafelockPinVisible( false )
	, isSafeLockInputEnabled ( 0 )
	, localPlayer_inGroup( false )
	, localPlayer_groupLeader( false )
#ifdef VEHICLES_ENABLED
	, isShowingYouAreDead(false)
#endif
{
}

HUDDisplay :: ~HUDDisplay()
{
}

bool HUDDisplay::Init()
{
	if(!gfxHUD.Load("Data\\Menu\\HUD.swf", true)) 
		return false;
	if(!gfxBloodStreak.Load("Data\\Menu\\WarZ_BloodStreak.swf", false))
		return false;
	if(!gfxRangeFinder.Load("Data\\Menu\\WarZ_HUD_RangeFinder.swf", false))
		return false;

	r_render_in_game_HUD->SetBool(true);

	gfxHUD.Invoke("_root.api.setChatVisibility", true);
	chatHUDVisible = true;

	ShowWaitingForPlayersMsg = false;//AlexRedd:: BR mode
	ShowBRGDMsg = false;//AlexRedd:: BR mode
	canRespawnPlayer = true;

#define MAKE_CALLBACK(FUNC) game_new r3dScaleformMovie::TGFxEICallback<HUDDisplay>(this, &HUDDisplay::FUNC)
	gfxHUD.RegisterEventHandler("eventChatMessage", MAKE_CALLBACK(eventChatMessage));
	gfxHUD.RegisterEventHandler("eventNoteWritePost", MAKE_CALLBACK(eventNoteWritePost));
	gfxHUD.RegisterEventHandler("eventNoteClosed", MAKE_CALLBACK(eventNoteClosed));
	gfxHUD.RegisterEventHandler("eventNoteReportAbuse", MAKE_CALLBACK(eventNoteReportAbuse));
	gfxHUD.RegisterEventHandler("eventPlayerListAction", MAKE_CALLBACK(eventPlayerListAction));
	gfxHUD.RegisterEventHandler("eventShowPlayerListContextMenu", MAKE_CALLBACK(eventShowPlayerListContextMenu));
	gfxHUD.RegisterEventHandler("eventSafelockPass", MAKE_CALLBACK(eventSafelockPass));
	gfxHUD.RegisterEventHandler("eventGraveNoteClosed", MAKE_CALLBACK(eventGravestoneClosed));
	gfxHUD.RegisterEventHandler("eventDeadToMenu", MAKE_CALLBACK(eventDeadToMenu));
	gfxHUD.RegisterEventHandler("eventRespawnPlayer", MAKE_CALLBACK(eventRespawnPlayer));
	gfxHUD.RegisterEventHandler("eventSetAutoRespawn", MAKE_CALLBACK(eventSetAutoRespawn));
	gfxHUD.RegisterEventHandler("eventSetSelectedRespawn", MAKE_CALLBACK(eventSetSelectedRespawn));
#if defined(MISSIONS) && defined(MISSION_TRIGGERS)
	gfxHUD.RegisterEventHandler("eventMissionAccepted", MAKE_CALLBACK(eventMissionAccepted));
	gfxHUD.RegisterEventHandler("eventMissionClosed", MAKE_CALLBACK(eventMissionClosed));
#endif

	{
		Scaleform::GFx::Value var[4];
		var[0].SetInt(0);
		var[1].SetString(gLangMngr.getString("$HUD_Chat_Channel1"));
		var[2].SetBoolean(true);
		var[3].SetBoolean(true);
		gfxHUD.Invoke("_root.api.setChatTab", var, 4);
 		var[0].SetInt(1);
		var[1].SetString(gLangMngr.getString("$HUD_Chat_Channel2"));
		var[2].SetBoolean(false);
		var[3].SetBoolean(true);
		gfxHUD.Invoke("_root.api.setChatTab", var, 4);
		var[0].SetInt(2);
		var[1].SetString(gLangMngr.getString("$HUD_Chat_Channel3"));
		var[2].SetBoolean(false);
		var[3].SetBoolean(true);
		gfxHUD.Invoke("_root.api.setChatTab", var, 4);
		var[0].SetInt(3);
		var[1].SetString(gLangMngr.getString("$HUD_Chat_Channel4"));
		var[2].SetBoolean(false);
		var[3].SetBoolean(true);
		gfxHUD.Invoke("_root.api.setChatTab", var, 4);

		currentChatChannel = 0;
		var[0].SetInt(0);
		gfxHUD.Invoke("_root.api.setChatTabActive", var, 1);
	}

	setChatTransparency(R3D_CLAMP(g_ui_chat_alpha->GetFloat()/100.0f, 0.0f, 1.0f));

	Inited = true;

	weaponInfoVisible = -1;
	SafeZoneWarningVisible = false;
	TPSReticleVisible = -1;
	m_PrevBreathValue = -1;

	isReadNoteVisible = false;
	isWriteNoteVisible = false;

	isReadGravestoneVisible = false;

#if defined(MISSIONS) && defined(MISSION_TRIGGERS)
	isReadMissionVisible = false;
#endif

	isSafelockPinVisible = false;
	isSafeLockInputEnabled = 0;

	localPlayer_inGroup = false;
	localPlayer_groupLeader = false;

	lastDisplayedHudMessageTime = 0;
	hudMessagesQueue2.clear();
	hudMessagesQueue.clear();
	
	hudGameMessagesQueue.clear();
	lastDisplayedHudGameMessageTime = 0;	

	{
		char sFullPath[512];
		char sFullPathImg[512];
		sprintf(sFullPath, "%s\\%s", r3dGameLevel::GetHomeDir(), "minimap.dds");
		sprintf(sFullPathImg, "$%s", sFullPath); // use '$' char to indicate absolute path

		if(r3dFileExists(sFullPath))
			gfxHUD.Invoke("_root.api.setMapIcon", sFullPathImg);
	}

	return true;
}

bool HUDDisplay::Unload()
{
	for (int i = 0; i<32; ++i)
	{
		if (m_HUDIcons[i].enabled)
		{
			m_HUDIcons[i].enabled = false;
			removeUserIcon(m_HUDIcons[i].icon);
		}
	}

	gfxHUD.Unload();
	gfxBloodStreak.Unload();
	gfxRangeFinder.Unload();

	Inited = false;
	return true;
}

void HUDDisplay::enableClanChannel()
{
	Scaleform::GFx::Value var[4];
	var[0].SetInt(2);
	var[1].SetString("CLAN");
	var[2].SetBoolean(false);
	var[3].SetBoolean(true);
	gfxHUD.Invoke("_root.api.setChatTab", var, 4);
}

int HUDDisplay::Update()
{
	if(!Inited)
		return 1;
		
	const ClientGameLogic& CGL = gClientLogic();

	//if(CGL.m_gameInfo.mapId==GBGameInfo::MAPID_WZ_California)
	//{
	//	gfxHUD.SetVariable("_root.api.Main.CaliTestText.visible", 1);
	//}

	gfxHUD.SetVariable("_root.api.Main.Weapon.Weapon.DuraScale.visible", false);//AlexRedd:: durability disabled for now

	gfxHUD.SetVariable("_root.api.Main.Condition.visible", !isShowingYouAreDead);
	gfxHUD.SetVariable("_root.api.Main.Slots.visible", !isShowingYouAreDead);
	gfxHUD.SetVariable("_root.api.Main.PingFps.visible", !isShowingYouAreDead);
	gfxHUD.SetVariable("_root.api.Main.Weapon.visible", !isShowingYouAreDead);
#ifdef ENABLE_BATTLE_ROYALE
	gfxHUD.SetVariable("_root.api.Main.ThreatIndicatorBR.visible", (CGL.m_gameInfo.IsGameBR() && CGL.m_gameHasStarted && !isShowingYouAreDead));
	gfxHUD.SetVariable("_root.Main.BRKilled.visible", (CGL.m_gameInfo.IsGameBR() && CGL.m_gameHasStarted && playersListVisible && !isShowingYouAreDead));
#else
	gfxHUD.SetVariable("_root.api.Main.ThreatIndicatorBR.visible", false);
	gfxHUD.SetVariable("_root.Main.BRKilled.visible", false);
#endif //ENABLE_BATTLE_ROYALE
	

	//AlexRedd:: for spectator stream mode
	if (gUserProfile.ProfileData.isDevAccount && d_video_spectator_mode->GetBool()
#ifdef ENABLE_BATTLE_ROYALE
		&& !CGL.m_gameInfo.IsGameBR()
#endif //ENABLE_BATTLE_ROYALE
		)
	{		
		gfxHUD.SetVariable("_root.api.Main.Condition.visible", !r_render_friends_name->GetBool());
		gfxHUD.SetVariable("_root.api.Main.PingFps.visible", !r_render_friends_name->GetBool());
		gfxHUD.SetVariable("_root.api.Main.Weapon.visible", !r_render_friends_name->GetBool());
		gfxHUD.SetVariable("_root.api.Main.Slots.visible", !r_render_friends_name->GetBool());
		gfxHUD.SetVariable("_root.api.Main.MiniMap.visible", !r_render_friends_name->GetBool());
		gfxHUD.SetVariable("_root.api.Main.ThreatIndicator.visible", !r_render_friends_name->GetBool());
		setTPSReticleVisibility(!r_render_friends_name->GetBool());		
	}

	if(r3dGetTime() > timeoutNoteReadAbuseReportedHideUI && timeoutNoteReadAbuseReportedHideUI != 0)
	{
		r3dMouse::Hide();
		writeNoteSavedSlotIDFrom = 0;
		timeoutNoteReadAbuseReportedHideUI = 0;
		timeoutForNotes = r3dGetTime() + 0.5f;
		Scaleform::GFx::Value var[2];
		var[0].SetBoolean(false);
		var[1].SetString("");
		gfxHUD.Invoke("_root.api.showNoteRead", var, 2);
	}
	if(RangeFinderUIVisible)
	{
		r3dPoint3D dir;
		r3dScreenTo3D(r3dRenderer->ScreenW2, r3dRenderer->ScreenH2, &dir);

		PxRaycastHit hit;
		PhysicsCallbackObject* target = NULL;
		PxSceneQueryFilterData filter(PxFilterData(COLLIDABLE_STATIC_MASK|(1<<PHYSCOLL_NETWORKPLAYER), 0, 0, 0), PxSceneQueryFilterFlag::eSTATIC|PxSceneQueryFilterFlag::eDYNAMIC);
		g_pPhysicsWorld->raycastSingle(PxVec3(gCam.x, gCam.y, gCam.z), PxVec3(dir.x, dir.y, dir.z), 2000.0f, PxSceneQueryFlag::eDISTANCE, hit, filter);

		float distance = -1;
		if(hit.shape)
		{
			// sergey's design (range finder shows not real distance... have no idea what it actually shows)
			distance = hit.distance * (1.0f + R3D_MIN(1.0f, (R3D_MAX(0.0f, (hit.distance-200.0f)/1800.0f)))*0.35f);
		}
		gfxRangeFinder.Invoke("_root.Main.Distance.gotoAndStop", distance!=-1?"on":"off");	
		char tmpStr[16];
		sprintf(tmpStr, "%.1f", distance);
		gfxRangeFinder.SetVariable("_root.Main.Distance.Distance.Distance.text", tmpStr);

		const ClientGameLogic& CGL = gClientLogic();
		float compass = atan2f(CGL.localPlayer_->m_vVision.z, CGL.localPlayer_->m_vVision.x)/R3D_PI;
		compass = R3D_CLAMP(compass, -1.0f, 1.0f);

		//float cmpVal = -(compass * 820);
		//gfxRangeFinder.SetVariable("_root.Main.compass.right.x", cmpVal);
		//gfxRangeFinder.SetVariable("_root.Main.compass.left.x", cmpVal-1632);

		//if(!CGL.localPlayer_->m_isAiming)
		//	showRangeFinderUI(false); // in case if player switched weapon or anything happened
		//Cynthia:fixed bugID 1111 force to close rangefinderUI everytime at checkfireweapon(), not here, or rangefinderUI would be forced to close when crossing into safe zone
	}

	for (int i = 0; i<32; ++i)
	{
		if (m_HUDIcons[i].enabled)
		{
			if ((r3dGetTime() - m_HUDIcons[i].spawnTime)>m_HUDIcons[i].lifetime)
			{
				m_HUDIcons[i].enabled = false;
				removeUserIcon(m_HUDIcons[i].icon);
			}
			else
			{
				moveUserIcon(m_HUDIcons[i].icon, m_HUDIcons[i].pos, true);
			}
		}
	}

	//AlexRedd:: Compass
	{
		float compass;
		if(CGL.localPlayer_->IsInVehicle())
		{
			obj_Vehicle* vehicle = g_pPhysicsWorld->m_VehicleManager->GetDrivenCar()->owner;				
			compass = atan2f(vehicle->GetvForw().z, vehicle->GetvForw().x)/R3D_PI;
			compass = R3D_CLAMP(compass, -1.0f, 1.0f);
		} 
		else if(CGL.localPlayer_->bDead)
		{
			compass = atan2f(CGL.localPlayer_->GetvForw().z, CGL.localPlayer_->GetvForw().x)/R3D_PI;
			compass = R3D_CLAMP(compass, -1.0f, 1.0f);
		}
		else 
		{				
			compass = atan2f(CGL.localPlayer_->m_vVision.z, CGL.localPlayer_->m_vVision.x)/R3D_PI;
			compass = R3D_CLAMP(compass, -1.0f, 1.0f);
		}

		r3dPoint3D d = CGL.localPlayer_->GetvForw();
		d.y = 0;
		d.Normalize();
		float dot1 = d.Dot(r3dPoint3D(0,0,1)); // north
		float dot2 = d.Dot(r3dPoint3D(1,0,0));
		float deg = acosf(dot1);
		deg = R3D_RAD2DEG(deg);
		if(dot2<0) 
			deg = 360 - deg;
		deg = R3D_CLAMP(deg, 0.0f, 360.0f);	

		float cmpVal2 = -(compass * 820);

		char tmpStr[32];
		sprintf(tmpStr, "%.f", deg);

		gfxHUD.SetVariable("_root.Main.CompassAnim.compass.right.x", cmpVal2);
		gfxHUD.SetVariable("_root.Main.CompassAnim.compass.left.x", cmpVal2-1632);
		gfxHUD.SetVariable("_root.Main.CompassAnim.compass.Current.text", tmpStr);
	}

	if(!hudMessagesQueue.empty() && (r3dGetTime()-lastDisplayedHudMessageTime)>1.5f)
	{
		std::string str = hudMessagesQueue.front();
		hudMessagesQueue.pop_front();
		gfxHUD.Invoke("_root.api.showMsg", str.c_str());
		lastDisplayedHudMessageTime = r3dGetTime();
	}

	if(!hudMessagesQueue2.empty() && (r3dGetTime()-lastDisplayedHudMessageTime)>1.5f)
	{
		std::string str2 = hudMessagesQueue2.front();
		hudMessagesQueue2.pop_front();
		gfxHUD.Invoke("_root.api.showMsg2", str2.c_str());
		lastDisplayedHudMessageTime = r3dGetTime();
	}	
	
	if((r3dGetTime()-lastDisplayedHudGameMessageTime)>5.0f)
	{		
		gfxHUD.Invoke("_root.api.removeGameMsgList", "");
	}	

	// enable safelock pin input only when interact key is released (need to enable it on next frame, otherwise key will be passed to flash)
	if(isSafelockPinVisible)
	{
		if(isSafeLockInputEnabled == 0 && !InputMappingMngr->isPressed(r3dInputMappingMngr::KS_INTERACT))
		{
			isSafeLockInputEnabled++;
		}
		else if(isSafeLockInputEnabled > 0 && isSafeLockInputEnabled <= 2)
		{
			if(++isSafeLockInputEnabled == 2)
				gfxHUD.Invoke("_root.api.enableSafelockInput", true);
		}
	}

	return 1;
}


int HUDDisplay::Draw()
{
	if(!Inited)
		return 1;

	gfxHUD.SetCurentRTViewport(Scaleform::GFx::Movie::SM_NoScale);
	gfxHUD.SetCurentRTViewport(Scaleform::GFx::Movie::SM_ExactFit);

	gfxBloodStreak.SetCurentRTViewport(Scaleform::GFx::Movie::SM_NoScale);
	gfxBloodStreak.SetCurentRTViewport(Scaleform::GFx::Movie::SM_ExactFit);

	{
		R3DPROFILE_FUNCTION("gfxBloodStreak.UpdateAndDraw");
		if(bloodAlpha > 0.0f)
			gfxBloodStreak.UpdateAndDraw();
	}
	{
		R3DPROFILE_FUNCTION("gfxRangeFinder.UpdateAndDraw");
		if(RangeFinderUIVisible)
			gfxRangeFinder.UpdateAndDraw();
	}
	if(r_render_in_game_HUD->GetBool())
	{
		R3DPROFILE_FUNCTION("gfxHUD.UpdateAndDraw");
#ifndef FINAL_BUILD
		gfxHUD.UpdateAndDraw(d_disable_render_hud->GetBool());
#else
		gfxHUD.UpdateAndDraw();
#endif
	}

	return 1;
}

void HUDDisplay::setBloodAlpha(float alpha)
{
	if(!Inited) return;
	if(R3D_ABS(bloodAlpha-alpha)<0.01f) return;

	bloodAlpha = alpha;
	gfxBloodStreak.SetVariable("_root.blood.alpha", alpha);
}

void HUDDisplay::eventDeadToMenu(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	if(!Inited) return;
	PKT_C2S_DisconnectReq_s n;
	p2pSendToHost(gClientLogic().localPlayer_, &n, sizeof(n), true);	
}

void HUDDisplay::eventRespawnPlayer(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	if(!Inited) return;
	r3d_assert(argCount == 1);
	int spawnSelectedID = args[0].GetInt();	

	obj_Player* plr = gClientLogic().localPlayer_;
	r3d_assert(plr);	
	
	if(spawnSelectedID == SPAWN_SAFE && gUserProfile.ProfileData.GameDollars  < gUserProfile.ShopRespawnSafeZone ||
		spawnSelectedID == SPAWN_WORLD && gUserProfile.ProfileData.GameDollars  < gUserProfile.ShopRespawnWorld ||
		spawnSelectedID == SPAWN_NEAR && gUserProfile.ProfileData.GameDollars  < gUserProfile.ShopRespawnNear ||
		spawnSelectedID == SPAWN_GROUP && gUserProfile.ProfileData.GameDollars  < gUserProfile.ShopRespawnGroup || 
		spawnSelectedID == SPAWN_SAFE && !isFoundSafeZone || spawnSelectedID == SPAWN_GROUP && plr->GroupID == 0)
	{
		char tmpStr[64];
		if(spawnSelectedID == SPAWN_SAFE && !isFoundSafeZone)
		{			
			sprintf(tmpStr, gLangMngr.getString("InfoMsg_NoSafeZoneInMap")); 
			showMessage2(tmpStr);
			return;
		}
		else if(spawnSelectedID == SPAWN_GROUP && plr->GroupID == 0)
		{
			sprintf(tmpStr, gLangMngr.getString("InfoMsg_NotInGroup"));
			showMessage2(tmpStr);
			return;
		}
		else
		{
			sprintf(tmpStr, gLangMngr.getString("InfoMsg_NotEnoughGD"));
			showMessage2(tmpStr);			
			return;
		}
	}
	else// respawn
	{
		int priceGD = 0;
		//char tmpStr[128];

		PKT_C2S_SetRespawnData_s n;	
		switch( spawnSelectedID )
		{
		case SPAWN_SAFE:	priceGD = gUserProfile.ShopRespawnSafeZone;	break;
		case SPAWN_WORLD:	priceGD = gUserProfile.ShopRespawnWorld; break;
		case SPAWN_NEAR:	priceGD = gUserProfile.ShopRespawnNear;	break;
		case SPAWN_GROUP:	priceGD = gUserProfile.ShopRespawnGroup; plr->m_numRespawnsToGroup++; break;
		}
#ifdef ENABLE_BATTLE_ROYALE
		if(gClientLogic().m_gameInfo.IsGameBR())
			n.spawnSelected = SPAWN_BATTLEROYALE;
		else
#endif //ENABLE_BATTLE_ROYALE
			n.spawnSelected = spawnSelectedID;
		n.PlayerID = toP2pNetId(plr->GetNetworkID());
		p2pSendToHost(plr, &n, sizeof(n), true);
	}	
}

void HUDDisplay::eventSetAutoRespawn(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	if(!Inited) return;
	r3d_assert(argCount == 1);	
	g_enable_auto_respawn->SetInt(args[0].GetInt());
	writeGameOptionsFile();
}

void HUDDisplay::eventSetSelectedRespawn(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	if(!Inited) return;
	r3d_assert(argCount == 1);	
	m_spawnSelectedType_ = args[0].GetInt();	
}

// this will leak on app exit
//char* s_ChatIgnoreList[256] = { 0 };
//int s_CharIgnoreListNum = 0;

void HUDDisplay::eventChatMessage(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	chatInputActive = false;
	lastChatMessageSent = r3dGetTime();

	static char s_chatMsg[2048];
	int currentTabIndex = args[0].GetInt();
	r3dscpy(s_chatMsg, args[1].GetString());

	bool has_anything = false;

	size_t start_text=0;
	size_t argLen = strlen(s_chatMsg);
	if(argLen < 3)
		return;
		
	// MMZ
	if(strncmp(s_chatMsg, "/backpack", 9)==0 && gUserProfile.ProfileData.isDevAccount)
	{
		d_disable_backpacks_draw->SetBool(!d_disable_backpacks_draw->GetBool());
		return;
	}
//#ifndef FINAL_BUILD		
	if(strncmp(s_chatMsg, "/stime", 6) == NULL && gUserProfile.ProfileData.isDevAccount)
	{
		char buf[256];
		int hour, min;
		if(3 != sscanf(s_chatMsg, "%s %d %d", buf, &hour, &min))
		{
			addChatMessage(0, "<system>", "/stime {hour} {min}", 0);
			return;
		}

		__int64 gameUtcTime = gClientLogic().GetServerGameTime();
		struct tm* tm = _gmtime64(&gameUtcTime);
		r3d_assert(tm);
		
		// adjust server time to match supplied hour
		gClientLogic().gameStartUtcTime_ -= tm->tm_sec;
		gClientLogic().gameStartUtcTime_ -= (tm->tm_min) * 60;
		gClientLogic().gameStartUtcTime_ += (hour - tm->tm_hour) * 60 * 60;
		gClientLogic().gameStartUtcTime_ += (min) * 60;
		gClientLogic().lastShadowCacheReset_ = -1;
		
		addChatMessage(0, "<system>", "time changed", 0);
		return;
	}
	//AlexRedd:: spectator mouse sensitivity
	if(strncmp(s_chatMsg, "/sens", 5) == NULL && gUserProfile.ProfileData.isDevAccount)
	{
		char buf[256];
		int sens;		
		if(2 != sscanf(s_chatMsg, "%s %d", buf, &sens))
		{
			addChatMessage(0, "<system>", "/sens {0-100}", 0);
			return;
		}

		r_spectator_mouse_sens->SetFloat( R3D_CLAMP((float)sens/100, 0.0f, 1.0f) );
		writeGameOptionsFile();

		addChatMessage(0, "<system>", "sensitivity changed", 0);		
		return;
	}
	/*if(strncmp(s_chatMsg, "/nm", 3) == 0 )
	{
		char buf[128];
		int status = 0;

		if (2 != sscanf(s_chatMsg, "%s %d", buf, &status))
		{
			addChatMessage(0, "<system>", "/nm {0|1}", 0);
			return;
		}
		d_navmesh_debug->SetBool(status != 0);
		// intentional fall-through
	}*/
//#endif	

	/*if (strncmp(s_chatMsg, "/ignore", 7) == 0)
	{
		char plrName[256] = { 0 };
		const char* beginNameStr = strchr(s_chatMsg, '"');
		const char* endNameStr = strrchr(s_chatMsg, '"');
		if (!beginNameStr || !endNameStr || beginNameStr == endNameStr)
		{
			addChatMessage(0, "<system>", gLangMngr.getString("$HUD_Msg_ChatIgnoreFormat"), 0);
			return;
		}

		memcpy(plrName, beginNameStr + 1, int((endNameStr)-(beginNameStr)) - 1);		

		bool found_user = false;
		const ClientGameLogic& CGL = gClientLogic();
		char plrUserName[64];
		// try to find a user with that name		
		for (int i = 0; i < MAX_NUM_PLAYERS; ++i)
		{
			obj_Player* plr = CGL.GetPlayer(i);
			if (plr)
			{
				plr->GetUserName(plrUserName);
				//r3dOutToLog("#### plrUserName %s\n", plrUserName); 

				if (stricmp(plrUserName, plrName) == 0)
				{
					found_user = true;
					if (plr == CGL.localPlayer_)
					{
						addChatMessage(0, "<system>", gLangMngr.getString("$HUD_Msg_ChatIgnoreYourself"), 0);
					}
					else
					{
						s_ChatIgnoreList[s_CharIgnoreListNum] = new char[strlen(s_chatMsg) + 1];
						r3dscpy(s_ChatIgnoreList[s_CharIgnoreListNum], s_chatMsg);
						s_CharIgnoreListNum++;
						addChatMessage(0, "<system>", gLangMngr.getString("$HUD_Msg_ChatIgnoreUserAdded"), 0);
					}
					break;
				}
			}
		}
		if (!found_user)
		{
			addChatMessage(0, "<system>", gLangMngr.getString("$HUD_Msg_ChatNoSuchUser"), 0);
		}
		return;
	}

	if (strncmp(s_chatMsg, "/unignore", 9) == 0)
	{		
		char plrName[256] = { 0 };
		const char* beginNameStr = strchr(s_chatMsg, '"');
		const char* endNameStr = strrchr(s_chatMsg, '"');
		if (!beginNameStr || !endNameStr || beginNameStr == endNameStr)
		{
			addChatMessage(0, "<system>", gLangMngr.getString("$HUD_Msg_ChatUnignoreFormat"), 0);
			return;
		}

		memcpy(plrName, beginNameStr + 1, int((endNameStr)-(beginNameStr)) - 1);
		
		bool found_user = false;
		// try to find a user with that name
		for (int i = 0; i < s_CharIgnoreListNum; ++i)
		{
			if (stricmp(s_ChatIgnoreList[i], plrName) == 0)
			{
				found_user = true;
				delete[] s_ChatIgnoreList[i]; s_ChatIgnoreList[i] = 0;
				s_ChatIgnoreList[i] = s_ChatIgnoreList[s_CharIgnoreListNum - 1];
				s_CharIgnoreListNum--;
				addChatMessage(0, "<system>", gLangMngr.getString("$HUD_Msg_ChatIgnoreUserRemoved"), 0);				
			}
			break;
		}
		if (!found_user)
		{
			addChatMessage(0, "<system>", gLangMngr.getString("$HUD_Msg_ChatNoSuchUserInIgnoreList"), 0);			
		}
		return;
	}*/

#ifdef AUTO_BAN_CHAT	
	//AlexRedd:: Ban chat
	// check profanity filter
	{
		bool hasProfanity = false;
		dictionaryHash_.IterateStart();
		while ( dictionaryHash_.IterateNext() )
		{
			const char* profanityEntry = dictionaryHash_.IterateGetKey().c_str();
			if(stristr(s_chatMsg, profanityEntry))
			{
				hasProfanity = true;
				break;
			}
		}
		if(hasProfanity)
		{
			if(gUserProfile.ProfileData.BanTimeChat==0 && currentTabIndex!=2 && currentTabIndex!=3)// skip allready banned, group and clan tabs
			{
				PKT_C2C_PlayerSwearing_s n;
				p2pSendToHost(gClientLogic().localPlayer_, &n, sizeof(n), true);
			}
		}
	}
#endif//AUTO_BAN_CHAT

	char userName[64];	
	gClientLogic().localPlayer_->GetUserName(userName);	

	if(gUserProfile.ProfileData.BanTimeChat==0)//AlexRedd:: Ban chat
	{
		PKT_C2C_ChatMessage_s n;
		n.userFlag = 0; // server will init it for others
		n.msgChannel = currentTabIndex;
		r3dscpy(n.msg, &s_chatMsg[start_text]);
		r3dscpy(n.gamertag, userName);
		p2pSendToHost(gClientLogic().localPlayer_, &n, sizeof(n));
	}
	else{		
		addChatMessage(0, "<system>", gLangMngr.getString("ChatTemporarilyBanned"), 0);
		char TimeLeftMsg[64]={0};		
		sprintf(TimeLeftMsg, gLangMngr.getString("ChatBanTimeLeft"), (gUserProfile.ProfileData.BanTimeChat/60));		
		addChatMessage(0, "<system>", TimeLeftMsg, 0);
		if(gUserProfile.ProfileData.BanChatCount>=3)
			addChatMessage(0, "<system>", gLangMngr.getString("ChatBanWarningMsg"), 0);		

		if (gClientLogic().localPlayer_)
			SoundSys.PlayAndForget(SoundSys.GetEventIDByPath("Sounds/UI Events/Misc/UI_Error_MsgSound"), gClientLogic().localPlayer_->GetPosition());		
	}

	uint32_t flags = 0;
	if(gUserProfile.ProfileData.AccountType==0)
		flags|=1;
	if(gUserProfile.ProfileData.isDevAccount & wiUserProfile::DAA_DEV_ICON)
		flags|=2;
	if(gUserProfile.ProfileData.AccountType==54)
		flags|=4;


	if(gUserProfile.ProfileData.BanTimeChat==0)//AlexRedd:: Ban chat
		addChatMessage(currentTabIndex, userName, &s_chatMsg[start_text], flags);

	memset(s_chatMsg, 0, sizeof(s_chatMsg));
}

char* HUDDisplay::applyProfanityFilter(const char* input)
{
	r3d_assert(input);
	r3d_assert(strlen(input)<2048);

	// really simple profanity filter
	int counter = 0;
	char profanityFilter[2048]={0};
	static char clearString[2048]={0};
	r3dscpy(profanityFilter, input);
	const char* delim = " ,.!?-(){}[];'\":*+/@#$%^&_=`~\n\r";
 	char* word = strtok(profanityFilter, delim);
	int regularCounter = 0; // to copy proper delimiter

	int spn = strspn(&input[regularCounter], delim);				//fixed bugID 782, always inspect delimiters prior processing following words.
	for(int k=0; k<spn; ++k)
		clearString[counter++] = input[regularCounter++];

	while(word)
	{
		regularCounter += strlen(word);
		char tmpWordBuf[512] = {0};
		r3dscpy(tmpWordBuf, word);
		{
			char *p = tmpWordBuf;
			while (*p)
			{
				*p = (char) tolower(*p);
				p++;
			}
		}

		if(dictionaryHash_.IsExists(tmpWordBuf))
		{
			r3dscpy(&clearString[counter], "***");
			counter +=3;
		}
		else
		{
			r3dscpy(&clearString[counter], word);
			counter +=strlen(word);
		}
		spn = strspn(&input[regularCounter], delim);
		
		for(int k=0; k<spn; ++k)
			clearString[counter++] = input[regularCounter++];
		word = strtok(NULL, delim);
	}
	clearString[counter++] = 0;

	return clearString;
}

void HUDDisplay::addChatMessage(int tabIndex, const char* user, const char* text, uint32_t flags)
{
	if(!Inited) return;
	Scaleform::GFx::Value var[3];

	char tmpMsg[1024];
	static char tabNames[4][128] = {0};
	if(tabNames[0][0]==0)
	{
		r3dscpy(tabNames[0], (gLangMngr.getString("$HUD_Chat_Channel1")));
		r3dscpy(tabNames[1], (gLangMngr.getString("$HUD_Chat_Channel2")));
		r3dscpy(tabNames[2], (gLangMngr.getString("$HUD_Chat_Channel3")));
		r3dscpy(tabNames[3], (gLangMngr.getString("$HUD_Chat_Channel4")));
	}

	static const char* tabNamesColor[] = {"#00A000", "#13bbeb", "#de13eb", "#ebcf13"};	
	const char* userColor = "#ffffff";
	const char* textColor = "#d0d0d0";
	const char* namePrefix = "";

	switch(flags)
	{	
	case 1: // Legend
		userColor = "#ffa800";
		break;
	case 2: // DEV
		userColor = "#ff0000";
		textColor = "#ff0000";
		namePrefix = "&lt;DEV&gt;";
		break;
	case 4: // Punisher
		userColor = "#0099ff";
		break;
	case 5: // Battle Royale
		userColor = "#ff0000";
		textColor = "#ffffff";	
		break;
	default:
		break;
	}	

	// check ignore list	
	/*{		
		for (int i = 0; i<s_CharIgnoreListNum; ++i)
		{
			if (stricmp(s_ChatIgnoreList[i], user) == 0)
				return; // ignore msg
		}		
	}*/

	// dirty stl :)
	r3dSTLString sUser = user;
	int pos = 0;
	while((pos= sUser.find('<'))!=-1)
		sUser.replace(pos, 1, "&lt;");
	while((pos = sUser.find('>'))!=-1)
		sUser.replace(pos, 1, "&gt;");

	r3dSTLString sMsg = text;
	while((pos = sMsg.find('<'))!=-1)
		sMsg.replace(pos, 1, "&lt;");
	while((pos = sMsg.find('>'))!=-1)
		sMsg.replace(pos, 1, "&gt;");
	
	sMsg = applyProfanityFilter(sMsg.c_str());

	sprintf(tmpMsg, "<font color=\"%s\">[%s]</font> <font color=\"%s\">%s%s:</font> <font color=\"%s\">%s</font>", tabNamesColor[tabIndex], tabNames[tabIndex], userColor, namePrefix, sUser.c_str(), textColor, sMsg.c_str());

	var[0].SetString(tmpMsg);
	gfxHUD.Invoke("_root.api.receiveChat", var, 1);
}

void HUDDisplay::addChatMessageBox(int tabIndex, const char* user, const char* text, DWORD weapon)
{
	if (!Inited) return;
	Scaleform::GFx::Value var[3];

	char tmpMsg[1024];
	static char tabNames[4][128] = { 0 };
	if (tabNames[0][0] == 0)
	{
		r3dscpy(tabNames[0], (gLangMngr.getString("$HUD_Chat_Channel1")));
		r3dscpy(tabNames[1], (gLangMngr.getString("$HUD_Chat_Channel2")));
		r3dscpy(tabNames[2], (gLangMngr.getString("$HUD_Chat_Channel3")));
		r3dscpy(tabNames[3], (gLangMngr.getString("$HUD_Chat_Channel4")));
	}

	static const char* tabNamesColor[] = { "#00A000", "#13bbeb", "#de13eb", "#ebcf13" };
	static const char* userNameColor[] = { "#ffffff", "#ffa800" };


	const char* userColor = "#00b5ff";
	const char* textColor = "#d0d0d0";
	const char* weaponColor = "#a000ff";
	const char* namePrefix = "[BOX]";

	// dirty stl :)
	r3dSTLString sUser = user;
	int pos = 0;
	while ((pos = sUser.find('<')) != -1)
		sUser.replace(pos, 1, "&lt;");
	while ((pos = sUser.find('>')) != -1)
		sUser.replace(pos, 1, "&gt;");

	r3dSTLString sMsg = text;
	while ((pos = sMsg.find('<')) != -1)
		sMsg.replace(pos, 1, "&lt;");
	while ((pos = sMsg.find('>')) != -1)
		sMsg.replace(pos, 1, "&gt;");

	char WinWeapon[64 * 2] = { 0 };
	const BaseItemConfig* itemCfg = g_pWeaponArmory->getConfig(weapon);
	sprintf(WinWeapon, itemCfg->m_StoreName);

	sprintf(tmpMsg, "<font color=\"%s\">[%s]</font> <font color=\"%s\">%s</font><font color=\"%s\"> %s:</font> <font color=\"%s\">%s</font> <font color=\"%s\">%s</font>",
		tabNamesColor[tabIndex], tabNames[tabIndex], textColor, namePrefix, userColor, sUser.c_str(), textColor, sMsg.c_str(), weaponColor, WinWeapon);

	var[0].SetString(tmpMsg);
	gfxHUD.Invoke("_root.api.receiveChat", var, 1);
}

void HUDDisplay::showPingFPS(int valueping, int valuefps)
{
	if (!Inited) return;

	//PING
	int Ping = valueping;
	bool pingnormal = Ping < 100;
	bool pinglow = (Ping > 100 && Ping < 200);
	bool pinghight = Ping > 200;
	char PingStr[64] = { 0 };

	if (pingnormal)
		sprintf(PingStr, "<font color=\"#FFFFFF\">%d</font>", valueping);
	else if (pinglow)
		sprintf(PingStr, "<font color=\"#FF9900\">%d</font>", valueping);
	else if (pinghight)
		sprintf(PingStr, "<font color=\"#CC0000\">%d</font>", valueping);

	//FPS
	float fps = r3dGetAvgFPS();
	bool fpsvaluemax = fps > 40.0f;
	bool fpsvaluemed = (fps > 20.0f && fps < 40.0f);
	bool fpsvaluemin = fps < 20.0f;
	char FpsStr[64] = { 0 };

	if (fpsvaluemax)
		sprintf(FpsStr, "<font color=\"#FFFFFF\">%d</font>", valuefps);
	else if (fpsvaluemed)
		sprintf(FpsStr, "<font color=\"#FF9900\">%d</font>", valuefps);
	else if (fpsvaluemin)
		sprintf(FpsStr, "<font color=\"#CC0000\">%d</font>", valuefps);	
	
	Scaleform::GFx::Value var[2];
	var[0].SetString(PingStr);
	var[1].SetString(FpsStr);
	gfxHUD.Invoke("_root.api.showPingFPS", var, 2);
}

void HUDDisplay::showVOIPIconTalking(bool visible)
{
	if(!Inited) return;
	gfxHUD.SetVariable("_root.api.Main.VoipIcon.visible", visible);
}

void HUDDisplay::setThreatValue(int percent)
{
	if(!Inited) return;
	gfxHUD.Invoke("_root.api.setThreatValue", percent);
}

void HUDDisplay::setLifeParams(int food, int water, int health, int toxicity, int stamina)
{
	if(!Inited) return;
	Scaleform::GFx::Value var[5];

	// temp, for testing
#ifndef FINAL_BUILD
	if(d_ui_health->GetInt() >= 0)
		health = d_ui_health->GetInt();
	if(d_ui_toxic->GetInt() >= 0)
		toxicity = d_ui_toxic->GetInt();
	if(d_ui_water->GetInt() >= 0)
		water = d_ui_water->GetInt();
	if(d_ui_food->GetInt() >= 0)
		food = d_ui_food->GetInt();
	if(d_ui_stamina->GetInt() >= 0)
		stamina = d_ui_stamina->GetInt();
#endif

	// UI expects inverse values, so do 100-X (exception is toxicity)
	var[0].SetInt(100-food);
	var[1].SetInt(100-water);
	var[2].SetInt(100-health);
	var[3].SetInt(toxicity);
	var[4].SetInt(100-stamina);
	gfxHUD.Invoke("_root.api.setHeroCondition", var, 5);
}

void HUDDisplay::setLifeConditions(const char* type, bool visible)
{
	if(!Inited) return;
	Scaleform::GFx::Value var[2];
	var[0].SetString(type);
	var[1].SetBoolean(visible);
	gfxHUD.Invoke("_root.api.setConditionIconVisibility", var, 2);
}

void HUDDisplay::setWeaponInfo(const char* name, int ammo, int clips, int firemode, int durability)
{
	if (!Inited) return;
	Scaleform::GFx::Value var[5];
	var[0].SetString(name);
	var[1].SetInt(ammo);
	var[2].SetInt(clips);
	if (firemode == 1)
		var[3].SetString("one");
	else if (firemode == 2)
		var[3].SetString("three");
	else
		var[3].SetString("auto");
	var[4].SetInt(durability);
	gfxHUD.Invoke("_root.api.setWeaponInfo", var, 5);
}

void HUDDisplay::showWeaponInfo(int state)
{
	if(!Inited) return;
	if(state != weaponInfoVisible)
		gfxHUD.Invoke("_root.api.showWeaponInfo", state);
	weaponInfoVisible = state;
}

void HUDDisplay::setSlotCooldown(int slotID, int progress, float timeLeft)//Odozi fix cd time "int timeLeft"
{
	char cooldowntime[64] = { 0 };
	if(!Inited) return;
	Scaleform::GFx::Value var[3];
	var[0].SetInt(slotID);
	var[1].SetInt(progress);
	var[2].SetString(cooldowntime);
	sprintf(cooldowntime, "%.1f", timeLeft);
	gfxHUD.Invoke("_root.api.setSlotCooldown", var, 3);
}

void HUDDisplay::setSlotInfo(int slotID, const char* name, int quantity, const char* icon)
{
	if(!Inited) return;
	Scaleform::GFx::Value var[4];
	var[0].SetInt(slotID);
	var[1].SetString(name);
	var[2].SetInt(quantity);
	var[3].SetString(icon);
	gfxHUD.Invoke("_root.api.setSlot", var, 4);
}

void HUDDisplay::updateSlotInfo(int slotID, int quantity)
{
	if(!Inited) return;
	Scaleform::GFx::Value var[2];
	var[0].SetInt(slotID);
	var[1].SetInt(quantity);
	gfxHUD.Invoke("_root.api.updateSlot", var, 2);
}

void HUDDisplay::showSlots(bool state)
{
	if(!Inited) return;
	gfxHUD.Invoke("_root.api.showSlots", state);
}

void HUDDisplay::setActiveSlot(int slotID)
{
	if(!Inited) return;
	gfxHUD.Invoke("_root.api.setActiveSlot", slotID);
}

void HUDDisplay::setActivatedSlot(int slotID)
{
	if(!Inited) return;
	gfxHUD.Invoke("_root.api.setActivatedSlot", slotID);
}

void HUDDisplay::showMessage(const char* text)
{
	if(!Inited) return;
	if(r_render_in_game_HUD->GetBool())
		hudMessagesQueue.push_back(text);
}

void HUDDisplay::showMessage2(const char* text)
{
	if(!Inited) return;
	if(r_render_in_game_HUD->GetBool())
		hudMessagesQueue2.push_back(text);
}

void HUDDisplay::showChat(bool showChat, bool force, const char* defaultInputText)
{
	if(!Inited) return;
	if(chatVisible != showChat || force)
	{
		chatVisible = showChat;
		Scaleform::GFx::Value var[3];
		var[0].SetBoolean(showChat);
		var[1].SetBoolean(chatInputActive);
		var[2].SetString(defaultInputText);
		gfxHUD.Invoke("_root.api.showChat", var, 3);
	}
}

void HUDDisplay::showChatInput(const char* defaultInputText)
{
	if(!Inited) return;
	chatInputActive = true;
	showChat(true, true, defaultInputText);
	gfxHUD.Invoke("_root.api.setChatActive", "");
}

void HUDDisplay::setChatTransparency(float alpha)
{
	if(!Inited) return;
	gfxHUD.Invoke("_root.api.setChatTransparency", alpha);
}

void HUDDisplay::setChatChannel(int index)
{
	if(!Inited) return;
	if(index <0 || index > 3) return;

	if(currentChatChannel != index)
	{
		currentChatChannel = index;
		Scaleform::GFx::Value var[1];
		var[0].SetInt(index);
		gfxHUD.Invoke("_root.api.setChatTabActive", var, 1);

		showChatInput("");
	}
}

void HUDDisplay::setChatVisibility(bool vis)
{
	if(!Inited) return;
	gfxHUD.Invoke("_root.api.setChatVisibility", vis);	
}

void HUDDisplay::showChatHUD()
{
	if(!Inited) return;
	if(!r_render_in_game_HUD->GetBool())
		return;
	if(chatHUDVisible)
		return;

	Scaleform::GFx::Value var[1];
	var[0].SetBoolean(true);
	gfxHUD.Invoke("_root.api.setChatVisibility", var, 1);

	chatHUDVisible = true;
}

void HUDDisplay::hideChatHUD()
{
	if(!Inited) return;
	if(!chatHUDVisible)
		return;

	Scaleform::GFx::Value var[1];
	var[0].SetBoolean(false);
	gfxHUD.Invoke("_root.api.setChatVisibility", var, 1);

	chatHUDVisible = false;
}

void HUDDisplay::clearPlayersList()
{
	if(!Inited) return;
	gfxHUD.Invoke("_root.api.clearPlayersList", "");
}

extern const char* getReputationString(int reputation);
void HUDDisplay::addPlayerToList(int index, int num, void* PlayerName_s_data)
{
	if(!Inited) return;
	Scaleform::GFx::Value var[11];
	var[0].SetInt(index);
	var[1].SetInt(num);

	ClientGameLogic::PlayerName_s* plrInfo = (ClientGameLogic::PlayerName_s*)PlayerName_s_data;
	
	// dirty stl :)
	r3dSTLString sUser = plrInfo->Gamertag;
	int pos = 0;
	while((pos= sUser.find('<'))!=-1)
		sUser.replace(pos, 1, "&lt;");
	while((pos = sUser.find('>'))!=-1)
		sUser.replace(pos, 1, "&gt;");
	
	var[2].SetString(sUser.c_str());

	const char* algnmt = getReputationString(plrInfo->reputation);
	if(plrInfo->accountFlags&2) // if isDev, remove reputation string
		algnmt = "";
	var[3].SetString(algnmt);
	var[4].SetBoolean((plrInfo->accountFlags&1)?true:false);
	var[5].SetBoolean((plrInfo->accountFlags&2)?true:false);
	var[6].SetBoolean((plrInfo->accountFlags&4)?true:false);
	var[7].SetBoolean((plrInfo->groupFlags&1)?true:false);
	var[8].SetBoolean(plrInfo->voiceIsMuted);
	var[9].SetBoolean((plrInfo->accountFlags&8)?true:false);
	char tmpStr[256]={0};
	gClientLogic().localPlayer_->GetUserName(tmpStr);
	var[10].SetBoolean(strcmp(plrInfo->Gamertag, tmpStr)==0); // if this is local player
	gfxHUD.Invoke("_root.api.addPlayerToList", var, 11);
}

void HUDDisplay::setPlayerInListVoipIconVisible(int index, bool visible, bool isMuted)
{
	if(!Inited) return;
	Scaleform::GFx::Value var[3];
	var[0].SetInt(index);
	var[1].SetBoolean(visible);
	var[2].SetBoolean(isMuted);
	gfxHUD.Invoke("_root.api.setPlayerInListVoipVisible", var, 3);
}

void HUDDisplay::showPlayersList(int flag)
{
	if(!Inited) return;
	playersListVisible = flag;

	Scaleform::GFx::Value var[1];
	var[0].SetBoolean(flag?true:false);
	gfxHUD.Invoke("_root.api.showPlayersList", var, 1);

	if(playersListVisible || isShowingYouAreDead)
		r3dMouse::Show();
	else
		r3dMouse::Hide();

	// check for any player that might be speaking right now
	const ClientGameLogic& CGL = gClientLogic();
	for(int i=0; i<MAX_NUM_PLAYERS; ++i)
	{
		obj_Player* plr = CGL.GetPlayer(i);
		if(plr && plr->isVoiceActive())
		{
			setPlayerInListVoipIconVisible(i, true, false);
		}
	}
	//AlexRedd:: fix holding breath
	if(gClientLogic().localPlayer_)
	{
		gClientLogic().localPlayer_->m_isAiming = false;
		gClientLogic().localPlayer_->m_isFinishedAiming = false;
	}
}

void HUDDisplay::showWriteNote(int slotIDFrom)
{
	if(!Inited) return;
	if(isWriteNoteVisible)
		return;

	r3dMouse::Show();
	
	writeNoteSavedSlotIDFrom = slotIDFrom;

	Scaleform::GFx::Value var[1];
	var[0].SetBoolean(true);
	gfxHUD.Invoke("_root.api.showNoteWrite", var, 1);

	isWriteNoteVisible = true;
}

void HUDDisplay::eventNoteWritePost(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount == 1);

	r3dMouse::Hide();

	const char* Message = args[0].GetString();

	obj_Player* plr = gClientLogic().localPlayer_;
	r3d_assert(plr);

	PKT_C2S_CreateNote_s n;
	n.SlotFrom = (BYTE)writeNoteSavedSlotIDFrom;
	n.pos      = plr->GetPosition() + plr->GetvForw()*0.2f;
	r3dscpy(n.TextFrom, plr->CurLoadout.Gamertag);
	r3dscpy(n.TextSubj, Message);
	p2pSendToHost(gClientLogic().localPlayer_, &n, sizeof(n));

	// local logic
	wiInventoryItem& wi = plr->CurLoadout.Items[writeNoteSavedSlotIDFrom];
	r3d_assert(wi.itemID && wi.quantity > 0);
	//local logic
	wi.quantity--;
	if(wi.quantity <= 0) {
		wi.Reset();
	}

	plr->OnBackpackChanged(writeNoteSavedSlotIDFrom);

	writeNoteSavedSlotIDFrom = 0;

	timeoutForNotes = r3dGetTime() + .5f;

	isReadNoteVisible = false;
	isWriteNoteVisible = false;
}

void HUDDisplay::eventNoteClosed(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3dMouse::Hide();

	writeNoteSavedSlotIDFrom = 0;
	timeoutForNotes = r3dGetTime() + .5f;

	isReadNoteVisible = false;
	isWriteNoteVisible = false;
}

void HUDDisplay::showReadNote(const char* msg)
{
	if(!Inited) return;
	if(!r_render_in_game_HUD->GetBool())
		return;
	if(isReadNoteVisible)
		return;

	r3dMouse::Show();
	writeNoteSavedSlotIDFrom = 1; // temp, to prevent mouse from hiding
	Scaleform::GFx::Value var[2];
	var[0].SetBoolean(true);
	var[1].SetString(msg);
	gfxHUD.Invoke("_root.api.showNoteRead", var, 2);
	isReadNoteVisible = true;
}

void HUDDisplay::hideReadNote()
{
	if(!Inited) return;
	if(!isReadNoteVisible) return;

	r3dMouse::Hide();

	Scaleform::GFx::Value var[2];
	var[0].SetBoolean(false);
	var[1].SetString("");
	gfxHUD.Invoke("_root.api.showNoteRead", var, 2);
	
	isReadNoteVisible = false;
	writeNoteSavedSlotIDFrom = 0;
}

void HUDDisplay::hideWriteNote()
{
	if(!Inited) return;
	if(!isWriteNoteVisible) return;

	r3dMouse::Hide();

	Scaleform::GFx::Value var[1];
	var[0].SetBoolean(false);
	gfxHUD.Invoke("_root.api.showNoteWrite", var, 1);

	isWriteNoteVisible = false;
	writeNoteSavedSlotIDFrom = 0;
}

void HUDDisplay::eventNoteReportAbuse(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	// ptumik: not sure what to do with this yet... need design

	//r3dMouse::Hide();
	//writeNoteSavedSlotIDFrom = 0;
	//timeoutForNotes = r3dGetTime() + 1.0f;

	timeoutNoteReadAbuseReportedHideUI = r3dGetTime() + 0.5f;

	isReadNoteVisible = false;
	isWriteNoteVisible = false;
}

void HUDDisplay::eventGravestoneClosed(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3dMouse::Hide();

	writeNoteSavedSlotIDFrom = 0;

	isReadGravestoneVisible = false;
}

void HUDDisplay::showGravestone(const char* msg, const char* victim, const char*aggressor )
{
	if(!Inited) return;
	if(!r_render_in_game_HUD->GetBool())
		return;
	if(isReadGravestoneVisible)
		return;

	r3dMouse::Show();
	writeNoteSavedSlotIDFrom = 1; // temp, to prevent mouse from hiding
	Scaleform::GFx::Value var[4];
	var[0].SetBoolean(true);
	var[1].SetString(msg);
	var[2].SetString(victim);
	var[3].SetString(aggressor);
	gfxHUD.Invoke("_root.api.showGraveNote", var, 4);

	isReadGravestoneVisible = true;
}

void HUDDisplay::hideGravestone()
{
	if(!Inited) return;
	if(!isReadGravestoneVisible) return;

	r3dMouse::Hide();

	Scaleform::GFx::Value var[4];
	var[0].SetBoolean(false);
	var[1].SetString("");
	var[2].SetString("");
	var[3].SetString("");
	gfxHUD.Invoke("_root.api.showGraveNote", var, 4);

	isReadGravestoneVisible = false;
	writeNoteSavedSlotIDFrom = 0;
}

void HUDDisplay::showRadioactiveHUD()
{
	if (!Inited) return;
	if (!r_render_in_game_HUD->GetBool())return;
	if (gClientLogic().localPlayer_->bDead)return;
	if (radioctiveHUDVisible)return;	
	
	gfxHUD.SetVariable("_root.api.Main.RadWarning.visible", true);
	radioctiveHUDVisible = true;	
}

void HUDDisplay::hideRadioactiveHUD()
{
	if (!Inited) return;	
	if (!radioctiveHUDVisible)return;

	gfxHUD.SetVariable("_root.api.Main.RadWarning.visible", false);
	radioctiveHUDVisible = false;
}

#ifdef MISSIONS
void HUDDisplay::showMissionHUD()
{
	if(!Inited) return;
	if(!r_render_in_game_HUD->GetBool())
		return;
	if(missionHUDVisible)
		return;

	Scaleform::GFx::Value var[1];
	var[0].SetBoolean(true);
	gfxHUD.Invoke("_root.api.showMissionInfo", var, 1);

	missionHUDVisible = true;
}

void HUDDisplay::hideMissionHUD()
{
	if(!Inited) return;
	if(!missionHUDVisible)
		return;

	Scaleform::GFx::Value var[1];
	var[0].SetBoolean(false);
	gfxHUD.Invoke("_root.api.showMissionInfo", var, 1);

	missionHUDVisible = false;
}

void HUDDisplay::addMissionInfo(uint32_t missionID, const char* missionName)
{
	// Does the Mission already have an entry?
	std::map<uint32_t, uint32_t>::iterator mapIter = missionIDtoIndex.find( missionID );
	if( mapIter != missionIDtoIndex.end() )
		return;

	// No, so add it.
	missionIDtoIndex[ missionID ] = nextMissionIndex;
	missionIDtoActionIDs[ missionID ].push_back( 0 );
	Scaleform::GFx::Value var[1];
	var[0].SetString(missionName);
	gfxHUD.Invoke("_root.api.addMissionInfo", var, 1);

	++nextMissionIndex;
}

void HUDDisplay::removeMissionInfo(uint32_t missionID)
{
	// Does the Mission have an entry?
	std::map<uint32_t, uint32_t>::const_iterator mapIter = missionIDtoIndex.find( missionID );
	if( mapIter == missionIDtoIndex.end() )
		return;

	// Yes, so get rid of it.
	uint32_t index = mapIter->second;
	Scaleform::GFx::Value var[1];
	var[0].SetUInt(index);
	gfxHUD.Invoke("_root.api.removeMissionInfo", var, 1);

	// Remove the Mission and Action/Objective map entries;
	missionIDtoIndex.erase( mapIter );
	bool bSkippedNextActionIndex = false;
	for( std::vector< uint32_t >::iterator iter = missionIDtoActionIDs[ missionID ].begin();
		 iter != missionIDtoActionIDs[ missionID ].end(); ++iter )
	{
		// Skip the first entry, because it is the NextActionIndex.
		if( bSkippedNextActionIndex )
			actionIDtoIndex.erase( *iter );
		else
			bSkippedNextActionIndex = true;
	}
	missionIDtoActionIDs[ missionID ].clear();
	missionIDtoActionIDs.erase( missionID );

	// Now update the mission indexes, because they must be sequential, and must start at 0.
	for( std::map<uint32_t, uint32_t>::iterator iter = missionIDtoIndex.begin();
		 iter!= missionIDtoIndex.end(); ++iter )
	{
		if( iter->second > index )
			--iter->second;
	}

	--nextMissionIndex;
}

void HUDDisplay::addMissionAction(uint32_t missionID, uint32_t actionID, bool isCompleted, const char* actionText, const char* progress, bool hasAreaRestriction)
{
	// Does the Mission have an entry?
	std::map<uint32_t, uint32_t>::const_iterator mapIter = missionIDtoIndex.find( missionID );
	if( mapIter == missionIDtoIndex.end() )
		return;

	// Yes, so add the action/objective.
	uint32_t actionIndex = missionIDtoActionIDs[ missionID ][ 0 ]; // first element contains NextActionIndex to be used.
	missionIDtoActionIDs[ missionID ].push_back( actionID );
	actionIDtoIndex[ actionID ] = actionIndex;
	++(missionIDtoActionIDs[ missionID ][ 0 ]);

	Scaleform::GFx::Value var[5];
	var[0].SetUInt( mapIter->second );
	var[1].SetString( actionText );
	var[2].SetBoolean( isCompleted );
	var[3].SetString( progress );
	var[4].SetBoolean( hasAreaRestriction );
	gfxHUD.Invoke("_root.api.addMissionObjective", var, 5);
}

void HUDDisplay::setMissionActionInArea(uint32_t missionID, uint32_t actionID, bool isInArea)
{
	// Does the Mission have an entry?
	std::map<uint32_t, uint32_t>::const_iterator mapMissionIter = missionIDtoIndex.find( missionID );
	if( mapMissionIter == missionIDtoIndex.end() )
		return;
	// Does the Action/Objective have an entry?
	std::map<uint32_t, uint32_t>::const_iterator mapActionIter = actionIDtoIndex.find( actionID );
	if( mapActionIter == actionIDtoIndex.end() )
		return;

	// Yes, so update its information.
	Scaleform::GFx::Value var[3];
	var[0].SetUInt( mapMissionIter->second );
	var[1].SetUInt( mapActionIter->second );
	var[2].SetBoolean( isInArea );
	gfxHUD.Invoke("_root.api.setMissionObjectiveInArea", var, 3);
}

void HUDDisplay::setMissionActionProgress(uint32_t missionID, uint32_t actionID, const char* progress)
{
	// Does the Mission have an entry?
	std::map<uint32_t, uint32_t>::const_iterator mapMissionIter = missionIDtoIndex.find( missionID );
	if( mapMissionIter == missionIDtoIndex.end() )
		return;
	// Does the Action/Objective have an entry?
	std::map<uint32_t, uint32_t>::const_iterator mapActionIter = actionIDtoIndex.find( actionID );
	if( mapActionIter == actionIDtoIndex.end() )
		return;

	// Yes, so update its information.
	Scaleform::GFx::Value var[3];
	var[0].SetUInt( mapMissionIter->second );
	var[1].SetUInt( mapActionIter->second );
	var[2].SetString( progress );
	gfxHUD.Invoke("_root.api.setMissionObjectiveNumbers", var, 3);
}

void HUDDisplay::setMissionActionComplete(uint32_t missionID, uint32_t actionID, bool isComplete)
{
	// Does the Mission have an entry?
	std::map<uint32_t, uint32_t>::const_iterator mapMissionIter = missionIDtoIndex.find( missionID );
	if( mapMissionIter == missionIDtoIndex.end() )
		return;
	// Does the Action/Objective have an entry?
	std::map<uint32_t, uint32_t>::const_iterator mapActionIter = actionIDtoIndex.find( actionID );
	if( mapActionIter == actionIDtoIndex.end() )
		return;

	// Yes, so update its information.
	Scaleform::GFx::Value var[2];
	var[0].SetUInt( mapMissionIter->second );
	var[1].SetUInt( mapActionIter->second );
	//r3dOutToLog("Mission(%d): Set Mission Action/Objective(%d) Completed, MissionIndex(%d), ActionIndex(%d)\n", missionID, actionID, mapMissionIter->second, mapActionIter->second );
	gfxHUD.Invoke("_root.api.setMissionObjectiveCompleted", var, 2);
}


#ifdef MISSION_TRIGGERS
void HUDDisplay::eventMissionAccepted(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3dMouse::Hide();
	writeNoteSavedSlotIDFrom = 0;
	isReadMissionVisible = false;

	r3d_assert(argCount > 0 && "Missing the accepted missionID.");

	PKT_C2S_AcceptMission_s n;
	n.missionID = args[0].GetUInt();
	p2pSendToHost(gClientLogic().localPlayer_, &n, sizeof(n));
}
void HUDDisplay::eventMissionClosed(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3dMouse::Hide();
	writeNoteSavedSlotIDFrom = 0;
	isReadMissionVisible = false;
}

void HUDDisplay::showMission(uint32_t missionID, const char* name, const char* desc)
{
	if(!Inited) return;
	if(!r_render_in_game_HUD->GetBool())
		return;
	if(isReadMissionVisible)
		return;

	r3dMouse::Show();
	writeNoteSavedSlotIDFrom = 1; // temp, to prevent mouse from hiding
	Scaleform::GFx::Value var[5];
	var[0].SetBoolean(true);
	var[1].SetString(desc);
	var[2].SetString(name);
	var[3].SetString("");
	var[4].SetUInt( missionID );
	gfxHUD.Invoke("_root.api.showMission", var, 5);

	isReadMissionVisible = true;
}

void HUDDisplay::hideMission()
{
	if(!Inited) return;
	if(isReadMissionVisible)
		return;

	r3dMouse::Hide();

	Scaleform::GFx::Value var[5];
	var[0].SetBoolean(false);
	var[1].SetString("");
	var[2].SetString("");
	var[3].SetString("");
	var[5].SetUInt( 0 );
	gfxHUD.Invoke("_root.api.showMission", var, 5);

	isReadMissionVisible = false;
	writeNoteSavedSlotIDFrom = 0;
}
#endif
#endif

void HUDDisplay::eventSafelockPass(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount==1);
	r3dMouse::Hide();
	isSafelockPinVisible = false;

	isSafeLockInputEnabled = 0;
	gfxHUD.Invoke("_root.api.enableSafelockInput", false);

	if(isSafelockPinResetOp)
	{
		isSafelockPinResetOp = false;
		PKT_C2S_LockboxKeyReset_s n;
		r3dscpy(n.old_AccessCodeS, hudSafelock->CurrentAccessCodeString);
		r3dscpy(n.new_AccessCodeS, args[0].GetString());
		n.lockboxID  = currentSafelockNetID;
		p2pSendToHost(gClientLogic().localPlayer_, &n, sizeof(n));
	}
	else
	{
		PKT_C2S_LockboxOpAns_s n;
		r3dscpy(n.AccessCodeS, args[0].GetString());
		r3dscpy(hudSafelock->CurrentAccessCodeString, args[0].GetString());
		n.lockboxID  = currentSafelockNetID;		
		p2pSendToHost(gClientLogic().localPlayer_, &n, sizeof(n));
	}
}

void HUDDisplay::showSafelockPin(bool isUnlock, int lockboxID, bool isDoingLockboxReset)
{
	if(!Inited) return;
	if(!r_render_in_game_HUD->GetBool())
		return;
	if(isSafelockPinVisible)
		return;

	r3dMouse::Show();
	Scaleform::GFx::Value var[2];
	var[0].SetBoolean(isUnlock);
	var[1].SetBoolean(true);
	gfxHUD.Invoke("_root.api.showSafelock", var, 2);
	isSafelockPinVisible = true;
	currentSafelockNetID = lockboxID;
	isSafelockPinResetOp = false;

	if(!isUnlock && isDoingLockboxReset)
		isSafelockPinResetOp = true;
	
}

void HUDDisplay::hideSafelockPin()
{
	if(!Inited) return;
	if(!isSafelockPinVisible) return;

	r3dMouse::Hide();

	Scaleform::GFx::Value var[2];
	var[0].SetBoolean(false);
	var[1].SetBoolean(false);
	gfxHUD.Invoke("_root.api.showSafelock", var, 2);

	isSafelockPinVisible = false;
}

static const int HUDDisplay_ContextMenu_LeaveGroupID = 10;
static const int HUDDisplay_ContextMenu_InviteToGroupID = 11;
static const int HUDDisplay_ContextMenu_AcceptGroupInviteID = 12;
static const int HUDDisplay_ContextMenu_KickFromGroupID = 13;
static const int HUDDisplay_ContextMenu_VOIP_MuteID=20;
static const int HUDDisplay_ContextMenu_VOIP_UnmuteID=21;
static const int HUDDisplay_ContextMenu_ReportPlayerID = 50;
static const int HUDDisplay_ContextMenu_AdminKickPlayerID = 51;
static const int HUDDisplay_ContextMenu_AdminBanPlayerID = 52;
static const int HUDDisplay_ContextMenu_AdminTeleportToPlayerID = 53;
static const int HUDDisplay_ContextMenu_AdminTeleportPlayerToAdminID = 54;
static const int HUDDisplay_ContextMenu_AdminBanChatPlayerID = 55;
static const int HUDDisplay_ContextMenu_AdminKillPlayerID = 56;
//static const int HUDDisplay_ContextMenu_IgnorePlayerID = 57;
//static const int HUDDisplay_ContextMenu_UnIgnorePlayerID = 58;

void HUDDisplay::eventShowPlayerListContextMenu(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount==1);
	const char* gamertag = args[0].GetString();

	// todo: depending on who we selected, show different context menu
	struct ContextCommand
	{
		int cmdID;
		const char* cmdTxt;
		ContextCommand() : cmdID(0), cmdTxt("") {}
		ContextCommand(int id, const char* txt) : cmdID(id), cmdTxt(txt) {}
	};
	ContextCommand cntxCmds[10];
	int curCntxCmd=0;

	const ClientGameLogic& CGL = gClientLogic();

	bool selectedSelf = false;
	{
		char tmpStr[128] = {0};
		CGL.localPlayer_->GetUserName(tmpStr);
		if(stricmp(tmpStr, gamertag)==0)
			selectedSelf = true;
	}

	const ClientGameLogic::PlayerName_s* plrNamePtr = NULL;
	for(int i=0; i<R3D_ARRAYSIZE(CGL.playerNames); ++i)
	{
		if(CGL.playerNames[i].Gamertag[0] && stricmp(CGL.playerNames[i].Gamertag, gamertag)==0)
		{
			plrNamePtr = &CGL.playerNames[i];
			break;
		}
	}

	if(plrNamePtr) // it can be null if player left the game while localplayer kept player list open
	{
		// group logic
		if(selectedSelf && localPlayer_inGroup)
		{
			cntxCmds[curCntxCmd++] = ContextCommand(HUDDisplay_ContextMenu_LeaveGroupID, "$HUD_PlayerAction_LeaveGroup");
		}
		else if(!selectedSelf)
		{
			if(((localPlayer_inGroup && localPlayer_groupLeader) || !localPlayer_inGroup) && !(plrNamePtr->groupFlags&2) && !(plrNamePtr->groupFlags&1) )
				cntxCmds[curCntxCmd++] = ContextCommand(HUDDisplay_ContextMenu_InviteToGroupID, "$HUD_PlayerAction_InviteGroup");
			else if(!localPlayer_inGroup && (plrNamePtr->groupFlags&1))
				cntxCmds[curCntxCmd++] = ContextCommand(HUDDisplay_ContextMenu_AcceptGroupInviteID, "$HUD_PlayerAction_AcceptGroupInvite");
			else if(localPlayer_inGroup && localPlayer_groupLeader && (plrNamePtr->groupFlags&2))
				cntxCmds[curCntxCmd++] = ContextCommand(HUDDisplay_ContextMenu_KickFromGroupID, "$HUD_PlayerAction_KickFromGroup");

			{
				if(!plrNamePtr->voiceIsMuted)
					cntxCmds[curCntxCmd++] = ContextCommand(HUDDisplay_ContextMenu_VOIP_MuteID, "$HUD_PlayerAction_Mute");
				else
					cntxCmds[curCntxCmd++] = ContextCommand(HUDDisplay_ContextMenu_VOIP_UnmuteID, "$HUD_PlayerAction_Unmute");
			}
		}
		
		bool isAdmin = gUserProfile.ProfileData.isDevAccount > 0;
		if(isAdmin)
		{
			if(gUserProfile.ProfileData.isDevAccount & wiUserProfile::DAA_KICK)
				cntxCmds[curCntxCmd++] = ContextCommand(HUDDisplay_ContextMenu_AdminKickPlayerID, "$HUD_PlayerAction_Kick");
			if(gUserProfile.ProfileData.isDevAccount & wiUserProfile::DAA_BAN || gUserProfile.ProfileData.IsMod >= wiUserProfile::MOD)
				cntxCmds[curCntxCmd++] = ContextCommand(HUDDisplay_ContextMenu_AdminBanPlayerID, "$HUD_PlayerAction_Ban");
			//if (gUserProfile.ProfileData.isDevAccount & wiUserProfile::DAA_BAN)
				//cntxCmds[curCntxCmd++] = ContextCommand(HUDDisplay_ContextMenu_AdminKillPlayerID, "$HUD_PlayerAction_Kill");
			if(gUserProfile.ProfileData.isDevAccount & wiUserProfile::DAA_BAN || gUserProfile.ProfileData.IsMod >= wiUserProfile::TRIAL_MOD)
				cntxCmds[curCntxCmd++] = ContextCommand(HUDDisplay_ContextMenu_AdminBanChatPlayerID, "$HUD_PlayerAction_BanChat");
			if(gUserProfile.ProfileData.isDevAccount & wiUserProfile::DAA_TELEPORT)
				cntxCmds[curCntxCmd++] = ContextCommand(HUDDisplay_ContextMenu_AdminTeleportToPlayerID, "TELEPORT TO");
			if(gUserProfile.ProfileData.isDevAccount & wiUserProfile::DAA_TELEPORT)
				cntxCmds[curCntxCmd++] = ContextCommand(HUDDisplay_ContextMenu_AdminTeleportPlayerToAdminID, "TELEPORT TO YOU");
		}
		else
		{
			if(gUserProfile.CustomerID == CGL.m_gameCreatorCustomerID)
				cntxCmds[curCntxCmd++] = ContextCommand(HUDDisplay_ContextMenu_AdminKickPlayerID, "$HUD_PlayerAction_Kick");

			if (!selectedSelf)
			{
				cntxCmds[curCntxCmd++] = ContextCommand(HUDDisplay_ContextMenu_ReportPlayerID, "$HUD_PlayerAction_Report");
				//cntxCmds[curCntxCmd++] = ContextCommand(HUDDisplay_ContextMenu_IgnorePlayerID, "$HUD_PlayerAction_Ignore");
				//cntxCmds[curCntxCmd++] = ContextCommand(HUDDisplay_ContextMenu_UnIgnorePlayerID, "$HUD_PlayerAction_UnIgnore");
			}
		}
	}

	r3d_assert(curCntxCmd <=9); // everything after 8 IS DEBUG ONLY FOR DEVS!!!!

	Scaleform::GFx::Value var[3];
	for(int i=0; i<9; ++i)
	{
		var[0].SetInt(i+1);
		var[1].SetString(cntxCmds[i].cmdTxt);
		var[2].SetInt(cntxCmds[i].cmdID);
		gfxHUD.Invoke("_root.api.setPlayerListContextMenuButton", var, 3);
	}

#ifdef ENABLE_BATTLE_ROYALE
	if(!gClientLogic().m_gameInfo.IsGameBR())//AlexRedd:: BR mode
#endif //ENABLE_BATTLE_ROYALE
		gfxHUD.Invoke("_root.api.showPlayerListContextMenu", "");
#ifdef ENABLE_BATTLE_ROYALE
	else if(gClientLogic().m_gameInfo.IsGameBR() && gUserProfile.ProfileData.isDevAccount)//AlexRedd:: BR mode
		gfxHUD.Invoke("_root.api.showPlayerListContextMenu", "");
#endif //ENABLE_BATTLE_ROYALE
}

void HUDDisplay::eventPlayerListAction(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount == 2);
	int actionID = args[0].GetInt();
	const char* gamertag = args[1].GetString();

	ClientGameLogic& CGL = gClientLogic();
	ClientGameLogic::PlayerName_s* plrNamePtr = NULL;
	for(int i=0; i<R3D_ARRAYSIZE(CGL.playerNames); ++i)
	{
		if(CGL.playerNames[i].Gamertag[0] && stricmp(CGL.playerNames[i].Gamertag, gamertag)==0)
		{
			plrNamePtr = &CGL.playerNames[i];
			break;
		}
	}

	showPlayersList(0);
	
	if (actionID == HUDDisplay_ContextMenu_ReportPlayerID)
	{
		char tmpStr[256];
		sprintf(tmpStr, "/report \"%s\" reason:", gamertag);
		showChatInput(tmpStr);
	}
	else if(actionID == HUDDisplay_ContextMenu_AdminKickPlayerID)
	{
		/*char tmpStr[256];
		sprintf(tmpStr, "/kick \"%s\" reason:", gamertag);
		showChatInput(tmpStr);*/

		PKT_C2C_ChatMessage_s n;
		char ffkick[128];
		sprintf(ffkick, "/kick \"%s\" reason:", gamertag);
		r3dscpy(n.gamertag, "system");
		r3dscpy(n.msg, ffkick);
		n.msgChannel = 1;
		n.userFlag = 0;
		p2pSendToHost(gClientLogic().localPlayer_, &n, sizeof(n));
		showPlayersList(0);
	}
	else if(actionID == HUDDisplay_ContextMenu_AdminBanPlayerID)
	{
		char tmpStr[256];
		sprintf(tmpStr, "/ban \"%s\" reason:", gamertag);
		showChatInput(tmpStr);
	}
	else if (actionID == HUDDisplay_ContextMenu_AdminKillPlayerID)
	{
		char tmpStr[256];
		sprintf(tmpStr, "/pkill \"%s\" reason:", gamertag);
		showChatInput(tmpStr);
	}
	else if(actionID == HUDDisplay_ContextMenu_AdminBanChatPlayerID)
	{
		char tmpStr[256];
		sprintf(tmpStr, "/chatban \"%s\" reason:", gamertag);
		showChatInput(tmpStr);
	}
	else if(actionID == HUDDisplay_ContextMenu_AdminTeleportToPlayerID)
	{
		/*char tmpStr[256];
		sprintf(tmpStr, "/ttp \"%s\"", gamertag);
		showChatInput(tmpStr);*/

		PKT_C2C_ChatMessage_s n;
		char tmpStr[256];
		sprintf(tmpStr, "/ttp \"%s\"", gamertag);
		r3dscpy(n.gamertag, "system");
		r3dscpy(n.msg, tmpStr);
		n.msgChannel = 1;
		n.userFlag = 0;
		p2pSendToHost(gClientLogic().localPlayer_, &n, sizeof(n));
		showPlayersList(0);
	}
	else if(actionID == HUDDisplay_ContextMenu_AdminTeleportPlayerToAdminID)
	{
		/*char tmpStr[256];
		sprintf(tmpStr, "/ttyl \"%s\"", gamertag);
		showChatInput(tmpStr);*/

		PKT_C2C_ChatMessage_s n;
		char tmpStr[256];
		sprintf(tmpStr, "/ttyl \"%s\"", gamertag);
		r3dscpy(n.gamertag, "system");
		r3dscpy(n.msg, tmpStr);
		n.msgChannel = 1;
		n.userFlag = 0;
		p2pSendToHost(gClientLogic().localPlayer_, &n, sizeof(n));
		showPlayersList(0);
	}
	else if(actionID == HUDDisplay_ContextMenu_LeaveGroupID)
	{
#ifdef ENABLE_BATTLE_ROYALE
		if (gClientLogic().m_gameInfo.IsGameBR() && gUserProfile.ProfileData.isDevAccount == 0)//AlexRedd:: BR mode
			return;
#endif //ENABLE_BATTLE_ROYALE
		PKT_C2S_GroupLeaveGroup_s n;
		p2pSendToHost(CGL.localPlayer_, &n, sizeof(n));
	}
	else if(actionID == HUDDisplay_ContextMenu_InviteToGroupID)
	{
#ifdef ENABLE_BATTLE_ROYALE
		if (gClientLogic().m_gameInfo.IsGameBR() && gUserProfile.ProfileData.isDevAccount == 0)//AlexRedd:: BR mode
			return;
#endif //ENABLE_BATTLE_ROYALE
		PKT_C2S_GroupInvitePlayer_s n;
		r3dscpy(n.gamertag, gamertag);
		p2pSendToHost(CGL.localPlayer_, &n, sizeof(n));
	}
	else if(actionID == HUDDisplay_ContextMenu_AcceptGroupInviteID)
	{
#ifdef ENABLE_BATTLE_ROYALE
		if (gClientLogic().m_gameInfo.IsGameBR() && gUserProfile.ProfileData.isDevAccount == 0)//AlexRedd:: BR mode
			return;
#endif //ENABLE_BATTLE_ROYALE
		PKT_C2S_GroupAcceptInvite_s n;
		r3dscpy(n.gamertag, gamertag);
		p2pSendToHost(CGL.localPlayer_, &n, sizeof(n));
	}
	else if(actionID == HUDDisplay_ContextMenu_KickFromGroupID)
	{
#ifdef ENABLE_BATTLE_ROYALE
		if (gClientLogic().m_gameInfo.IsGameBR() && gUserProfile.ProfileData.isDevAccount == 0)//AlexRedd:: BR mode
			return;
#endif //ENABLE_BATTLE_ROYALE
		PKT_C2S_GroupKickPlayer_s n;
		r3dscpy(n.gamertag, gamertag);
		p2pSendToHost(CGL.localPlayer_, &n, sizeof(n));
	}
	else if(actionID == HUDDisplay_ContextMenu_VOIP_MuteID)//Cynthia
	{
		//if(plrNamePtr->voiceClientID > 0 && plrNamePtr->voiceEnabled)
		if(plrNamePtr->voiceClientID > 0)
		{
			gTeamSpeakClient.MuteClient(plrNamePtr->voiceClientID);
			plrNamePtr->voiceIsMuted = 1;
		}
	}
	else if(actionID == HUDDisplay_ContextMenu_VOIP_UnmuteID)
	{
		//if(plrNamePtr->voiceClientID > 0 && plrNamePtr->voiceEnabled)
		if(plrNamePtr->voiceClientID > 0)
		{
			gTeamSpeakClient.UnmuteClient(plrNamePtr->voiceClientID);
			plrNamePtr->voiceIsMuted = 0;
		}
	}
	/*else if(actionID == HUDDisplay_ContextMenu_IgnorePlayerID)
	{
		char tmpStr[256];
		sprintf(tmpStr, "/ignore \"%s\"", gamertag);
		showChatInput(tmpStr);		
	}
	else if (actionID == HUDDisplay_ContextMenu_UnIgnorePlayerID)
	{
		char tmpStr[256];
		sprintf(tmpStr, "/unignore \"%s\"", gamertag);
		showChatInput(tmpStr);
	}*/
}

void HUDDisplay::showYouAreDead(const char* killerName, const char* pic, int dist, int killerHealth, bool headShot, int xp, int gd)
{
	if(!Inited) return;
	r3dMouse::Show();
#ifdef VEHICLES_ENABLED
	isShowingYouAreDead = true;
#endif

	//check for safe zone (NOT WORKING WITH FASTLOAD!)
	/*ObjectManager& GW = GameWorld();	
	for (GameObject *targetObj = GW.GetFirstObject(); targetObj; targetObj = GW.GetNextObject(targetObj))
	{		
		if(targetObj->isObjType(OBJTYPE_PostBox))
		{
			isFoundSafeZone = true;			
			break;
		}
	}*/
	isFoundSafeZone = gClientLogic().m_gameInfo.mapId == GBGameInfo::MAPID_ZP_Test;
#ifdef ENABLE_BATTLE_ROYALE
	SendDisconnectRequest = gClientLogic().m_gameInfo.IsGameBR() && gClientLogic().m_gameHasStarted;//gClientLogic().m_gameInfo.mapId == GBGameInfo::MAPID_ZP_Test;
#else
	SendDisconnectRequest = false;
#endif //ENABLE_BATTLE_ROYALE

	//check for lock
	bool isLockedSafe = !isFoundSafeZone; // || gClientLogic().m_gameInfo.mapId != GBGameInfo::MAPID_ZP_Test;
#ifdef ENABLE_BATTLE_ROYALE
	bool isLockedWorld = gClientLogic().m_gameInfo.IsGameBR() && gClientLogic().m_gameHasStarted;
	bool isLockedNear = gClientLogic().m_gameInfo.IsGameBR() || gClientLogic().m_gameInfo.IsDevEvent() && !gClientLogic().m_gameInfo.IsNoDropGame() || gClientLogic().m_gameInfo.channel == 7;
#else
	bool isLockedNear = gClientLogic().m_gameInfo.IsDevEvent() && !gClientLogic().m_gameInfo.IsNoDropGame() || gClientLogic().m_gameInfo.channel == 7;
	bool isLockedWorld = false;
#endif //ENABLE_BATTLE_ROYALE
	
	bool isLockedGroup = !localPlayer_inGroup || gClientLogic().m_gameInfo.IsDevEvent() && !gClientLogic().m_gameInfo.IsNoDropGame() || gClientLogic().m_gameInfo.channel == 7;
	bool isLockedInventory = true; // gClientLogic().m_gameInfo.IsDevEvent() || gClientLogic().m_gameInfo.IsGameBR();
	bool isPemiumAcc = false;// gUserProfile.ProfileData.PremiumAcc > 0 || gUserProfile.ProfileData.isDevAccount > 0;
	
	if (gClientLogic().m_gameInfo.IsDevEvent() && !gClientLogic().m_gameInfo.IsNoDropGame() || gClientLogic().m_gameInfo.channel == 7 
#ifdef ENABLE_BATTLE_ROYALE
		|| gClientLogic().m_gameInfo.IsGameBR() && !gClientLogic().m_gameHasStarted
#endif //ENABLE_BATTLE_ROYALE
		)
		m_spawnSelectedType_ = SPAWN_WORLD;
	else
		m_spawnSelectedType_ = SPAWN_NEAR;

#ifdef ENABLE_BATTLE_ROYALE
	if (gClientLogic().m_gameInfo.IsGameBR() && gClientLogic().m_gameHasStarted && gClientLogic().m_is_has_winner_player)	
		gfxHUD.SetVariable("_root.Main.PlayerDead.DeadMsg.TopText.text", gLangMngr.getString("$HUD_YouAreWinner"));
	else
#endif //ENABLE_BATTLE_ROYALE
		gfxHUD.SetVariable("_root.Main.PlayerDead.DeadMsg.TopText.text", gLangMngr.getString("$HUD_YouAreDead"));

	char victimName[64];
	if(gClientLogic().localPlayer_)
		gClientLogic().localPlayer_->GetUserName(victimName);

	Scaleform::GFx::Value var[22];
	var[0].SetString(killerName);
	var[1].SetString(victimName);
	var[2].SetString(pic);
	var[3].SetInt(dist);
	var[4].SetInt(killerHealth);
	var[5].SetBoolean(headShot);
	var[6].SetInt(xp);
	var[7].SetInt(gClientLogic().localPlayer_->m_Kills);
	var[8].SetInt(gd);
	var[9].SetBoolean(isLockedSafe);
	var[10].SetBoolean(isLockedWorld);
	var[11].SetBoolean(isLockedNear);
	var[12].SetBoolean(isLockedGroup);
	var[13].SetBoolean(isLockedInventory);
	var[14].SetBoolean(isPemiumAcc);
	var[15].SetInt(gUserProfile.ShopRespawnSafeZone);	
	var[16].SetInt(gUserProfile.ShopRespawnWorld);
	var[17].SetInt(gUserProfile.ShopRespawnNear);
	var[18].SetInt(gUserProfile.ShopRespawnGroup);
	var[19].SetBoolean(SendDisconnectRequest);
	var[20].SetInt(m_spawnSelectedType_);
	var[21].SetInt(g_enable_auto_respawn->GetInt());
	gfxHUD.Invoke("_root.api.showYouAreDead", var, 22);	
	ShowMsgDeath("in");	
}

#ifdef VEHICLES_ENABLED
bool HUDDisplay::isYouAreDead()
{
	return isShowingYouAreDead;
}
#endif

void HUDDisplay::updateDeadTimer(int timer, int progress, int timer2, int progress2)
{
	if(!Inited) return;	

	Scaleform::GFx::Value var[2];
	var[0].SetInt(timer);
	var[1].SetInt(progress);	
	gfxHUD.Invoke("_root.api.updateDeadTimer", var, 2);

	if(hudVault && hudVault->isActive())
	{
		if(timer > 0 || timer2 > 0)
			hudVault->m_DeadTimer_ = timer;
	}

	if(localPlayer_inGroup)
	{
		Scaleform::GFx::Value var2[2];
		var2[0].SetInt(timer2);
		var2[1].SetInt(progress2);		
		gfxHUD.Invoke("_root.api.updateDeadGroupTimer", var2, 2);
	}
	
	// auto respawn logic
	if(timer < 1 && canRespawnPlayer && !SendDisconnectRequest && g_enable_auto_respawn->GetInt() == 1 && m_spawnSelectedType_ != SPAWN_GROUP)
	{
		canRespawnPlayer = false;		
		char tmpStr[128];		
		if(m_spawnSelectedType_ == SPAWN_SAFE && gUserProfile.ProfileData.GameDollars  < gUserProfile.ShopRespawnSafeZone ||
		   m_spawnSelectedType_ == SPAWN_WORLD && gUserProfile.ProfileData.GameDollars  < gUserProfile.ShopRespawnWorld ||
		   m_spawnSelectedType_ == SPAWN_NEAR && gUserProfile.ProfileData.GameDollars  < gUserProfile.ShopRespawnNear ||		  
		   m_spawnSelectedType_ == SPAWN_SAFE && !isFoundSafeZone)
		{			
			if(m_spawnSelectedType_ == SPAWN_SAFE && !isFoundSafeZone)
			{			
				sprintf(tmpStr, gLangMngr.getString("InfoMsg_NoSafeZoneInMap"));
				showMessage2(tmpStr);
				return;
			}			
			else
			{			
				sprintf(tmpStr, gLangMngr.getString("InfoMsg_NotEnoughGD"));
				showMessage2(tmpStr);			
				return;
			}
		}
		else // respawn
		{
			PKT_C2S_SetRespawnData_s n;	
#ifdef ENABLE_BATTLE_ROYALE
			if(gClientLogic().m_gameInfo.IsGameBR())
				n.spawnSelected = SPAWN_BATTLEROYALE;
			else
#endif //ENABLE_BATTLE_ROYALE
				n.spawnSelected = m_spawnSelectedType_;
			n.PlayerID = toP2pNetId(gClientLogic().localPlayer_->GetNetworkID());
			p2pSendToHost(gClientLogic().localPlayer_, &n, sizeof(n), true);
		}
	}
	 // group auto respawn logic
	else if(timer2 < 1 && canRespawnPlayer && !SendDisconnectRequest && g_enable_auto_respawn->GetInt() == 1 && m_spawnSelectedType_ == SPAWN_GROUP && localPlayer_inGroup)
	{
		canRespawnPlayer = false;
		int priceXP = 0;
		char tmpStr[128];
		wiCharDataFull& slot = gClientLogic().localPlayer_->CurLoadout;
		if(slot.Stats.XP  < gUserProfile.ShopRespawnGroup || gClientLogic().localPlayer_->GroupID == 0)
		{			
			if(gClientLogic().localPlayer_->GroupID == 0)
			{
				sprintf(tmpStr, gLangMngr.getString("InfoMsg_NotInGroup"));
				showMessage2(tmpStr);
				return;
			}
			else
			{			
				sprintf(tmpStr, gLangMngr.getString("InfoMsg_NotEnoughXP"));
				showMessage2(tmpStr);			
				return;
			}
		}
		else // respawn
		{
			PKT_C2S_SetRespawnData_s n;
			n.spawnSelected = SPAWN_GROUP;
			n.PlayerID = toP2pNetId(gClientLogic().localPlayer_->GetNetworkID());
			p2pSendToHost(gClientLogic().localPlayer_, &n, sizeof(n), true);
		}
	}
	 // disconnect
	else if(timer < 1 && SendDisconnectRequest)
	{
		PKT_C2S_DisconnectReq_s n;
		p2pSendToHost(gClientLogic().localPlayer_, &n, sizeof(n), true);
	}
}

void HUDDisplay::updateDeadExitTimer(int timer)
{
	if(!Inited) return;

	if(hudVault && hudVault->isActive())
	{
		if(timer > 0)
			hudVault->m_DeadExitTimer_ = timer;
	}

	char tmpstr[64];
	char finalText[128];
	sprintf(tmpstr, gLangMngr.getString("$HUD_ExitingIn"), timer);
	sprintf(finalText, "<font color=\"#ff0000\">%s</font>", tmpstr);
	gfxHUD.SetVariable("_root.Main.PlayerDead.DeadMsg.Buttons.QuitTittle.htmlText", finalText);

	if(timer < 1)
	{		
		PKT_C2S_DisconnectReq_s n;
		p2pSendToHost(gClientLogic().localPlayer_, &n, sizeof(n), true);
	}
}

void HUDDisplay::showSafeZoneWarning(bool flag)
{
	if(!Inited) return;

	if(SafeZoneWarningVisible != flag)
	{
		SafeZoneWarningVisible = flag;
		gfxHUD.Invoke("_root.Main.Condition.gotoAndStop", flag?"safe":"regular");
	}
}

const char* getReputationIconName(int reputation);
void HUDDisplay::addCharTag(const char* name, bool isSameClan, int rep, Scaleform::GFx::Value& result)
{
	if(!Inited) return;
	r3d_assert(result.IsUndefined());

	const char* repS = getReputationIconName(rep);	

	Scaleform::GFx::Value var[3];
	var[0].SetString(name);
	var[1].SetBoolean(isSameClan);
	var[2].SetString(repS);	
	gfxHUD.Invoke("_root.api.addCharTag", &result, var, 3);	
}

void HUDDisplay::setCharTagHealth(Scaleform::GFx::Value& bar, int health, bool isVisible)
{
	if(!Inited) return;
	r3d_assert(!bar.IsUndefined());	

	Scaleform::GFx::Value var[3];
	var[0] = bar;	
	var[1].SetInt(health);
	var[2].SetBoolean(isVisible);
	gfxHUD.Invoke("_root.api.setCharTagHealth", var, 3);	
}

void HUDDisplay::removeUserIcon(Scaleform::GFx::Value& icon)
{
	if(!Inited) return;
	r3d_assert(!icon.IsUndefined());

	Scaleform::GFx::Value var[1];
	var[0] = icon;
	gfxHUD.Invoke("_root.api.removeUserIcon", var, 1);

	icon.SetUndefined();
}

// optimized version
void HUDDisplay::moveUserIcon(Scaleform::GFx::Value& icon, const r3dPoint3D& pos, bool alwaysShow, bool force_invisible /* = false */, bool pos_in_screen_space/* =false */)
{
	if(!Inited)
		return;
	r3d_assert(!icon.IsUndefined());

	r3dPoint3D scrCoord;
	float x, y;
	int isVisible = 1;
	if(!pos_in_screen_space)
	{
		if(alwaysShow)
			isVisible = r3dProjectToScreenAlways(pos, &scrCoord, 20, 20);
		else
			isVisible = r3dProjectToScreen(pos, &scrCoord);
	}
	else
		scrCoord = pos;

	// convert screens into UI space
	float mulX = 1920.0f/r3dRenderer->ScreenW;
	float mulY = 1080.0f/r3dRenderer->ScreenH;
	x = scrCoord.x * mulX;
	y = scrCoord.y * mulY;

	Scaleform::GFx::Value::DisplayInfo displayInfo;
	icon.GetDisplayInfo(&displayInfo);
	displayInfo.SetVisible(isVisible && !force_invisible);
	displayInfo.SetX(x);
	displayInfo.SetY(y);
	icon.SetDisplayInfo(displayInfo);
}

void HUDDisplay::setCharTagTextVisible(Scaleform::GFx::Value& icon, bool isVisible, bool isSameGroup, bool isVoipTalking, bool isSpawnIcon)
{
	if(!Inited) return;
	r3d_assert(!icon.IsUndefined());

	Scaleform::GFx::Value var[5];
	var[0] = icon;
	var[1].SetBoolean(isVisible);
	var[2].SetBoolean(isSameGroup);
	var[3].SetBoolean(isVoipTalking);
	var[4].SetBoolean(isSpawnIcon);	
	gfxHUD.Invoke("_root.api.setCharTagTextVisible", var, 5);
}

void HUDDisplay::setTPSReticleVisibility(int set)
{
	if (!Inited) return;

	if (!(gClientLogic().m_gameInfo.flags & GBGameInfo::SFLAGS_CrossHair))
		set = 0;

	if (set == TPSReticleVisible) return;

	TPSReticleVisible = set;

	gfxHUD.SetVariable("_root.Main.reticles.visible", set);		
}

void HUDDisplay::refreshReticle()
{
	if (!Inited)
		return;

	char reticleColor[16] = { 0 };
	if (g_crosshaire_color->GetInt() == 0)
		sprintf(reticleColor, "none");
	else if (g_crosshaire_color->GetInt() == 1)
		sprintf(reticleColor, "red");
	else if (g_crosshaire_color->GetInt() == 2)
		sprintf(reticleColor, "blue");
	else if (g_crosshaire_color->GetInt() == 3)
		sprintf(reticleColor, "green");

	setReticleColor(reticleColor);

	gfxHUD.Invoke("_root.api.setReticle", g_crosshaire_mode->GetInt());
}	

void HUDDisplay::setReticleScale(float scale)
{
	if (!Inited)
		return;
	static float prevScale = 1.0f;
	if (R3D_ABS(prevScale - scale) < 0.01f)
		return;
	prevScale = scale;
	
	gfxHUD.Invoke("_root.api.scaleReticle", scale);
}

void HUDDisplay::setReticleColor(const char* color)
{
	if (!Inited)
		return;	

	gfxHUD.Invoke("_root.api.setReticleColor", color);
}

void HUDDisplay::addPlayerToGroupList(const char* name, bool isLeader, bool isLeaving)
{
	if(!Inited) return;
	Scaleform::GFx::Value var[3];

	char tmpStr[128] = {0};
	const ClientGameLogic& CGL = gClientLogic();
	CGL.localPlayer_->GetUserName(tmpStr);
	if(stricmp(tmpStr, name)==0)
	{
		localPlayer_groupLeader = isLeader;
		localPlayer_inGroup = true;
	}

	var[0].SetString(name);
	var[1].SetBoolean(isLeader);
	var[2].SetBoolean(isLeaving);
	gfxHUD.Invoke("_root.api.addPlayerToGroup", var, 3);
}

void HUDDisplay::removePlayerFromGroupList(const char* name)
{
	if(!Inited) return;

	char tmpStr[128] = {0};
	const ClientGameLogic& CGL = gClientLogic();
	CGL.localPlayer_->GetUserName(tmpStr);
	if(stricmp(tmpStr, name)==0)
	{
		localPlayer_groupLeader = false;
		localPlayer_inGroup = false;
	}

	gfxHUD.Invoke("_root.api.removePlayerFromGroup", name);

	if(isShowingYouAreDead && CGL.localPlayer_->bDead)
	{
#ifdef ENABLE_BATTLE_ROYALE
		if(gClientLogic().m_gameInfo.IsGameBR() && !gClientLogic().m_gameHasStarted)
			m_spawnSelectedType_ = SPAWN_WORLD;
		else
#endif //ENABLE_BATTLE_ROYALE
			m_spawnSelectedType_ = SPAWN_NEAR;

		gfxHUD.Invoke("_root.api.lockDeadMenuGroupButton", "");
	}
}

void HUDDisplay::aboutToLeavePlayerFromGroup(const char* name)
{
	if(!Inited) return;

	gfxHUD.Invoke("_root.api.aboutToLeavePlayerFromGroup", name);
}

void HUDDisplay::addPlayerToVoipList(const char* name)
{
	if(!Inited) return;
	gfxHUD.Invoke("_root.api.addPlayerToVoipList", name);
}

void HUDDisplay::removePlayerFromVoipList(const char* name)
{
	if(!Inited) return;
	gfxHUD.Invoke("_root.api.removePlayerFromVoipList", name);
}


void HUDDisplay::setCarInfo(int durability, int speed, int speedText, int gas, int rpm)
{
	if(!Inited) return;
	Scaleform::GFx::Value var[5];
	var[0].SetInt(durability);
	var[1].SetInt(speed);
	var[2].SetInt(speedText);
	var[3].SetInt(gas);
	var[4].SetInt(rpm);
	gfxHUD.Invoke("_root.api.setCarInfo", var, 5);
}

void HUDDisplay::showCarInfo(bool visible)
{
	if(!Inited) return;
	gfxHUD.Invoke("_root.api.setCarInfoVisibility", visible);
}

void HUDDisplay::setCarTypeInfo(const char* type)
{
	if(!Inited) return;
	Scaleform::GFx::Value var[1];
	var[0].SetString(type);
	gfxHUD.Invoke("_root.api.setCarTypeInfo", var, 1);
}

void HUDDisplay::setCarSeatInfo(int seatID, const char* type)
{
	if(!Inited) return;
	Scaleform::GFx::Value var[2];
	var[0].SetInt(seatID);
	var[1].SetString(type);
	gfxHUD.Invoke("_root.api.setCarSeatInfo", var, 2);	
}

void HUDDisplay::SetReloadingProgress(int progress, const char* time)
{
	if (!Inited)
		return;
	if(!r_render_in_game_HUD->GetBool())
		return;
	
	gfxHUD.Invoke("_root.api.updateReload", progress);	
	gfxHUD.SetVariable("_root.Main.container_reload.NumNum.Text.text", time);
}

void HUDDisplay::showReloading(bool set)
{
	if (!Inited)
		return;
	if(!r_render_in_game_HUD->GetBool())
		return;
	if (set)
		gfxHUD.Invoke("_root.api.showReload", "");
	else
		gfxHUD.Invoke("_root.api.hideReload", "");
}

void HUDDisplay::SetBreathValue(float value)
{
	if (!Inited)
		return;
	if (m_PrevBreathValue != value)
	{
		m_PrevBreathValue = value;
		gfxHUD.Invoke("_root.api.setBreathBarValue", value);
	}
}

void HUDDisplay::showKills(int kills)
{
	if (!Inited) return;

	if(!r_render_in_game_HUD->GetBool())
		return;
	
	// flash part from who made that :)
	if (kills>0){	
		gfxHUD.SetVariable("_root.api.Main.KillStreak.visible", true);	
		gfxHUD.Invoke("_root.Main.KillStreak.gotoAndStop", kills);
	}
	else
		gfxHUD.SetVariable("_root.api.Main.KillStreak.visible", false);

	if (kills>5)
		gfxHUD.SetVariable("_root.api.Main.KillStreak.Kills.Text.text", kills);	
	
	//show animated killstreak message
	//if (kills == 15 || kills == 25 || kills == 50 || kills == 100)
		//showKillStreakMsg(kills);
}

void HUDDisplay::showKillStreakMsg(int kills)
{
	if(!Inited)	return;
	if(hudPause->isActive())return;
	if(!g_enable_killstreak_sounds->GetBool())return;
	if(!r_render_in_game_HUD->GetBool())return;

	if (gClientLogic().localPlayer_)
		SoundSys.PlayAndForget(SoundSys.GetEventIDByPath("Sounds/UI Events/Misc/Town_Phase3_Siege_Failure"), gClientLogic().localPlayer_->GetPosition());
	
	gfxHUD.Invoke("_root.api.AddKillStreakMessage", kills);
}

void HUDDisplay::AddKillMessage(obj_Player* killer, obj_Player* victim, obj_Zombie* zombie, STORE_CATEGORIES damageType, int lastTimeHitItemID, bool isHeadShot)
{
	if(!Inited)	return;	
	if(hudPause->isActive())return;	

	if(r_render_in_game_HUD->GetBool())
	{
		char victimUserName[64] = "";
		char killerUserName[64] = "";		
		char patchIcon[256] = "";		

		const BaseItemConfig* baseItem = g_pWeaponArmory->getConfig(lastTimeHitItemID);
		if(killer)killer->GetUserName(killerUserName);
		if(victim)victim->GetUserName(victimUserName);

		bool isLocalKill = false;
		if(damageType != storecat_INVALID)
			isLocalKill = gClientLogic().localPlayer_==killer?true:false;

		if (victim)
		{
			if (baseItem && damageType != storecat_MELEE && baseItem->m_itemID != WeaponConfig::ITEMID_UnarmedMelee && !zombie && killer != victim)
			{
				sprintf(patchIcon, "%s", baseItem->m_HudIcon);
			}
			else if (damageType == storecat_MELEE && !zombie && (baseItem && baseItem->m_itemID != WeaponConfig::ITEMID_UnarmedMelee))//by melee
			{
				sprintf(patchIcon, "%s", "$Data/Weapons/HudIcons/Knife_01.dds");
			}
			else if (killer == victim)//suicide
			{
				isHeadShot = false;
				sprintf(killerUserName, "%s", gLangMngr.getString("$HUD_Suicide"));
				sprintf(patchIcon, "%s", "$Data/Weapons/HudIcons/killfeed_suicide_icon.dds");
			}
			else if (baseItem && baseItem->m_itemID == WeaponConfig::ITEMID_UnarmedMelee)//by unarmed melee
			{
				sprintf(patchIcon, "%s", "$Data/Weapons/HudIcons/killfeed_unarmed_icon.dds");
			}
			else if (zombie)//by zombie
			{
				isHeadShot = false;
				sprintf(killerUserName, "%s", gLangMngr.getString("$HUD_KilledByZombie"));
				sprintf(patchIcon, "%s", "$Data/Weapons/HudIcons/killfeed_zombie_icon.dds");
			}
			else if (damageType == storecat_Vehicle)//by vehicle
			{
				isHeadShot = false;
				sprintf(killerUserName, "%s", gLangMngr.getString("$ShotOnTheVehicle"));
				sprintf(patchIcon, "%s", "$Data/Weapons/HudIcons/hud_killfeed_car_pic.dds");
			}
			else
			{
				isHeadShot = false;
				sprintf(killerUserName, "%s", gLangMngr.getString("$HUD_Suicide"));
				sprintf(patchIcon, "%s", "$Data/Weapons/HudIcons/killfeed_suicide_icon.dds");
			}

			Scaleform::GFx::Value var[5];
			var[0].SetString(victimUserName);
			var[1].SetString(killerUserName);
			var[2].SetBoolean(isHeadShot);
			var[3].SetString(patchIcon);
			var[4].SetBoolean(isLocalKill);
			gfxHUD.Invoke("_root.api.addGameMsgList", var, 5);	
			lastDisplayedHudGameMessageTime = r3dGetTime();			
		}		
	}
}

void HUDDisplay::showSpawnProtect(float protect)
{
	if (!Inited) return;
	obj_Player* plr = gClientLogic().localPlayer_;	

	if (protect>0.0f)
	{
		gfxHUD.Invoke("_root.Main.Condition.gotoAndStop", "spawn");
		gfxHUD.Invoke("_root.Main.Condition.ProtectBar.gotoAndStop", 100 - protect);
	}
	else if (plr->CurLoadout.GameFlags & wiCharDataFull::GAMEFLAG_NearPostBox)
	{
		gfxHUD.Invoke("_root.Main.Condition.gotoAndStop", "safe");
	}
	else {
		gfxHUD.Invoke("_root.Main.Condition.gotoAndStop", "regular");
	}
}

void HUDDisplay::setWeaponStat(int dmg, const char* spread, const char* recoil)
{
	if (!Inited) return;
	if(!r_render_in_game_HUD->GetBool())return;

	char Stats[512];
	sprintf(Stats, "DMG: <font color=\"#FFFFFF\">%d</font>  |  SPR: <font color=\"#FFFFFF\">%s</font>  |  REC: <font color=\"#FFFFFF\">%s</font>", dmg, spread, recoil);
	gfxHUD.Invoke("_root.api.setWeaponStat", Stats);
}

void HUDDisplay::showBRMessage(const char* text)
{
	if (!Inited) return;
	if(!r_render_in_game_HUD->GetBool())return;

	// show message
	//if(!ShowWaitingForPlayersMsg)
	//{			
		ShowWaitingForPlayersMsg = true;			
		Scaleform::GFx::Value var[2];
		var[0].SetBoolean(true);
		var[1].SetString(text);
		gfxHUD.Invoke("_root.api.setWaitingForPlayersMsg", var, 2);	
	//}
}
void HUDDisplay::hideBRMessage()
{
	if (!Inited) return;
	if(!r_render_in_game_HUD->GetBool())return;

	// hide message
	if(ShowWaitingForPlayersMsg)
	{
		ShowWaitingForPlayersMsg = false;
		Scaleform::GFx::Value var[2];
		var[0].SetBoolean(false);
		var[1].SetString("");
		gfxHUD.Invoke("_root.api.setWaitingForPlayersMsg", var, 2);		
	}	
}

void HUDDisplay::showBRGDMessage(const char* text)
{
	if (!Inited) return;
	if(!r_render_in_game_HUD->GetBool())return;

	// show message
	//if(!ShowBRGDMsg)
	//{			
		ShowBRGDMsg = true;			
		Scaleform::GFx::Value var[2];
		var[0].SetBoolean(true);
		var[1].SetString(text);
		gfxHUD.Invoke("_root.api.setGDRewardMsg", var, 2);	
	//}
}
void HUDDisplay::hideBRGDMessage()
{
	if (!Inited) return;
	if(!r_render_in_game_HUD->GetBool())return;

	// hide message
	if(ShowBRGDMsg)
	{
		ShowBRGDMsg = false;
		Scaleform::GFx::Value var[2];
		var[0].SetBoolean(false);
		var[1].SetString("");
		gfxHUD.Invoke("_root.api.setGDRewardMsg", var, 2);			
	}	
}

void HUDDisplay::showBRSZDistance(int distance)
{
	if(!Inited) return;
	if(!r_render_in_game_HUD->GetBool())return;
	
	char disStr[128]={0};	
	sprintf(disStr, "<font color=\"#FFFFFF\">%d</font>", distance);
	gfxHUD.Invoke("_root.api.setBRSZDistance", disStr);
}

void HUDDisplay::showBRPlayersLeft(int playersLeft)
{
	if(!Inited) return;	
	if(!r_render_in_game_HUD->GetBool())return;	
	
	gfxHUD.SetVariable("_root.Main.BRKilled.Num.Text.text", gClientLogic().localPlayer_->m_Kills);
	gfxHUD.SetVariable("_root.Main.BRKilled.Text.Text.text", gLangMngr.getString("$HUD_Killed"));

	gfxHUD.SetVariable("_root.Main.BRAlive.Num.Text.text", playersLeft);
	gfxHUD.SetVariable("_root.Main.BRAlive.Text.Text.text", gLangMngr.getString("$HUD_Alive"));
	gfxHUD.SetVariable("_root.Main.BRAlive.visible", !isShowingYouAreDead);
	gfxHUD.SetVariable("_root.Main.ThreatIndicator.y", 110);
	gfxHUD.SetVariable("_root.Main.MiniMap.y", 50);
}

void HUDDisplay::showBRStartEndOfRoundAnim(int actionId)
{
	if(!Inited) return;	
	if(!r_render_in_game_HUD->GetBool())return;	
	if(isShowingYouAreDead)return;
	
	if(actionId == 1)//start
	{
		gfxHUD.Invoke("_root.Main.BRMessage.End.gotoAndPlay", "out");
		gfxHUD.Invoke("_root.Main.BRMessage.Start.gotoAndPlay", "in");
	}
	else if(actionId == 2)//end
	{
		gfxHUD.Invoke("_root.Main.BRMessage.Start.gotoAndPlay", "out");
		gfxHUD.Invoke("_root.Main.BRMessage.End.gotoAndPlay", "in");
	}

	gfxHUD.SetVariable("_root.api.Main.BRMessage.visible", actionId>0);
}

void HUDDisplay::setThreatBRValue(int percent)
{
	if(!Inited) return;
	if(!r_render_in_game_HUD->GetBool())return;
	gfxHUD.Invoke("_root.api.setThreatBRValue", percent);
}

void HUDDisplay::ShowMsgDeath(const char* enable)
{
	if(!Inited) return;
	gfxHUD.Invoke("_root.Main.PlayerDead.gotoAndPlay", enable);
}

void HUDDisplay::showLeavingBattlezone(bool show)//AlexRedd:: BZ
{
	if(!Inited)	return;	

	gfxHUD.Invoke("_root.api.showLeavingBattleZone", show);	
}

void HUDDisplay::setLeavingBattlezoneTimer(float progress)//AlexRedd:: BZ
{
	if(!Inited)	return;	

	Scaleform::GFx::Value var[1];
	var[0].SetInt(int(progress));
	gfxHUD.Invoke("_root.api.setLeavingBattleZoneBar", var, 1);	
}

void HUDDisplay::ToggleCompass(bool enable) 
{
	if(!Inited)	return;	

	if(!r_render_in_game_HUD->GetBool() || isShowingYouAreDead)
		enable = false;

	Scaleform::GFx::Value var[1];
	var[0].SetBoolean(enable);
	gfxHUD.Invoke("_root.api.MoveCompass", var,1);
}

void HUDDisplay::ToggleMiniMap(bool enable) 
{
	if(!Inited)	return;	

	if(!r_render_in_game_HUD->GetBool() || isShowingYouAreDead)
		enable = true;

	Scaleform::GFx::Value var[1];
	var[0].SetBoolean(enable);
	gfxHUD.Invoke("_root.api.MoveMiniMap", var,1);
}

void HUDDisplay::ToggleThreatIndicator(bool enable) 
{
	if(!Inited)	return;	

	if(!r_render_in_game_HUD->GetBool() || isShowingYouAreDead)
		enable = true;

	Scaleform::GFx::Value var[1];
	var[0].SetBoolean(enable);
	gfxHUD.Invoke("_root.api.MoveThreatIndicator", var,1);
}

r3dPoint2D getHUDMinimapPosWithExternalSize(const r3dPoint3D& pos, const r3dPoint3D& worldOrigin, const r3dPoint3D& worldSize)
{
	float left_corner_x = worldOrigin.x;
	float bottom_corner_y = worldOrigin.z;
	float x_size = worldSize.x;
	float y_size = worldSize.z;

	float x = R3D_CLAMP((pos.x - left_corner_x) / x_size, 0.0f, 1.0f);
	float y = 1.0f - R3D_CLAMP((pos.z - bottom_corner_y) / y_size, 0.0f, 1.0f);

	return r3dPoint2D(x, y);
}

r3dPoint2D getHUDMinimapPos(const r3dPoint3D& pos)
{
	r3dPoint3D worldOrigin = GameWorld().m_MinimapOrigin;
	r3dPoint3D worldSize = GameWorld().m_MinimapSize;
	return getHUDMinimapPosWithExternalSize(pos, worldOrigin, worldSize);
}

void HUDDisplay::setMinimapPosition(const r3dPoint3D& pos, const r3dPoint3D& dir)
{
	r3dPoint2D mapPos = getHUDMinimapPos(pos);

	// calculate rotation around Y axis
	r3dPoint3D d = dir;
	d.y = 0;
	d.Normalize();
	float dot1 = d.Dot(r3dPoint3D(0, 0, 1)); // north
	float dot2 = d.Dot(r3dPoint3D(1, 0, 0));
	float deg = acosf(dot1);
	deg = R3D_RAD2DEG(deg);
	if (dot2<0)
		deg = 360 - deg;
	deg = R3D_CLAMP(deg, 0.0f, 360.0f);

	Scaleform::GFx::Value var[3];
	var[0].SetNumber(mapPos.x);
	var[1].SetNumber(mapPos.y);
	var[2].SetNumber(deg);
	gfxHUD.Invoke("_root.api.setMapPos", var, 3);
}

void HUDDisplay::showMinimapFunc()
{
	const ClientGameLogic& CGL = gClientLogic();
	obj_Player* plr = CGL.localPlayer_;
	r3d_assert(plr);	
	setMinimapPosition(plr->GetPosition(), plr->GetvForw());
}

void HUDDisplay::setScreenIconAlpha(Scaleform::GFx::Value& icon, float alpha)
{
	if (!Inited)
		return;
	r3d_assert(!icon.IsUndefined());

	Scaleform::GFx::Value::DisplayInfo dinfo;
	icon.GetDisplayInfo(&dinfo);
	dinfo.SetAlpha(R3D_CLAMP(alpha * 100.0f, 0.0f, 100.0f)); // 0..100
	icon.SetDisplayInfo(dinfo);
}

void HUDDisplay::setScreenIconScale(Scaleform::GFx::Value& icon, float scale)
{
	if (!Inited)
		return;
	r3d_assert(!icon.IsUndefined());
	Scaleform::GFx::Value::DisplayInfo dinfo;
	icon.GetDisplayInfo(&dinfo);
	scale = R3D_CLAMP(scale*100.0f, 0.0f, 100.0f);
	dinfo.SetScale(scale, scale);
	icon.SetDisplayInfo(dinfo);
}

void HUDDisplay::setScreenIconDistance(Scaleform::GFx::Value& icon, const r3dPoint3D& pos)
{
	if (!Inited) return;
	r3d_assert(!icon.IsUndefined());

	float distance = (pos - gClientLogic().localPlayer_->GetPosition()).Length();
	char tmpstr[64];
	sprintf(tmpstr, "%d", int(distance));
	Scaleform::GFx::Value var[2];
	var[0] = icon;
	var[1].SetString(tmpstr);
	gfxHUD.Invoke("_root.api.setScreenIconDistance", var, 2);
}

void HUDDisplay::addScreenIcon(Scaleform::GFx::Value& result, const char* iconname)
{
	if (!Inited)
		return;
	r3d_assert(result.IsUndefined());
	
	Scaleform::GFx::Value var[1];
	var[0].SetString(iconname);
	gfxHUD.Invoke("_root.api.addScreenIcon", &result, var, 1);
}

void HUDDisplay::addHUDIcon(HUDIconType type, float lifetime, const r3dVector& pos)
{
	if (!Inited)
		return;

	int i = 0;
	bool found = false;
	// search for icon within X radius firstly
	for (i = 0; i<32; ++i)
	{
		if (m_HUDIcons[i].enabled && m_HUDIcons[i].type == type && (pos - m_HUDIcons[i].pos).Length() < 10.0f) // same point, just update lifetime
		{
			m_HUDIcons[i].spawnTime = r3dGetTime();
			return;
		}
	}
	for (i = 0; i<32; ++i)
	{
		if (!m_HUDIcons[i].enabled)
		{
			found = true;
			break;
		}
	}
	if (!found)
		return;

	r3d_assert(m_HUDIcons[i].icon.IsUndefined());

	Scaleform::GFx::Value var[1];	
	if (type == HUDIcon_Attack)
		var[0].SetString("attack");
	else if (type == HUDIcon_Defend)
		var[0].SetString("defend");
	else if (type == HUDIcon_Spotted)
		var[0].SetString("spotted");
	else
		r3d_assert(false);
	gfxHUD.Invoke("_root.api.addScreenIcon", &m_HUDIcons[i].icon, var, 1);

	//r3dOutToLog("### type %i lifetime %.2f\n", type, lifetime);

	m_HUDIcons[i].enabled = true;
	m_HUDIcons[i].type = type;
	m_HUDIcons[i].lifetime = lifetime;
	m_HUDIcons[i].spawnTime = r3dGetTime();
	m_HUDIcons[i].pos = pos;

	moveUserIcon(m_HUDIcons[i].icon, pos, true, false);
}

void HUDDisplay::addCommandsData(const char* text)
{
	if (!Inited)
		return;

	Scaleform::GFx::Value var[1];
	var[0].SetString(text);
	gfxHUD.Invoke("_root.api.addCommandsData", var, 1);
}

void HUDDisplay::toggleCommandsPopUp(bool enable)
{
	if (!Inited)
		return;

	if (enable)
		gfxHUD.Invoke("_root.api.showCommandsPopUp", "");
	else
		gfxHUD.Invoke("_root.api.hideCommandsPopUp", "");
}