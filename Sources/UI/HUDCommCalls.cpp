#include "r3dPCH.h"
#include "r3d.h"

#include "HUDCommCalls.h"

#include "LangMngr.h"
#include "..\multiplayer\ClientGameLogic.h"
#include "..\ObjectsCode\AI\AI_Player.H"

#include "HUDDisplay.h"
#include "HUDPause.h"
#include "HUDSafelock.h"
#include "HUDAttachments.h"
#include "HUDVault.h"
#include "HUDStore.h"
#include "HUDTrade.h"
#include "HUDRepair.h"
#include "HUDCrafting.h"
#include "HUDSell.h"

extern HUDDisplay*	hudMain;
extern HUDPause*	hudPause;
extern HUDSafelock* hudSafelock;
extern HUDVault*	hudVault;
extern HUDAttachments*	hudAttm;
extern HUDTrade* hudTrade;
extern HUDStore*	hudStore;
extern HUDRepair* hudRepair;
extern HUDCrafting* hudCraft;
extern HUDSell*	    hudSell;

HUDCommCalls::HUDCommCalls()
: m_bInited ( false )
{
}

HUDCommCalls::~HUDCommCalls()
{
}

bool HUDCommCalls::Init()
{
	//if(!gfxMovie.Load("Data\\Menu\\WarZ_HUD_CommCalls.swf", false)) // Not used
		//return false;

	//gfxMovie.SetCurentRTViewport( Scaleform::GFx::Movie::SM_NoScale );
	//gfxMovie.GetMovie()->SetViewAlignment(Scaleform::GFx::Movie::Align_CenterRight);

	RUS_LANG = strcmp(g_user_language->GetString(), "russian") == 0 ? true : false;

	m_bInited = true;
	m_bVisible = false;
	m_CurrentMsgType = 0;
	for (int i = 0; i<10; ++i)
	{
		m_CommandsG[i].id = -1;
		m_CommandsH[i].id = -1;
	}	

	return true;
}

bool HUDCommCalls::Unload()
{
	/*if(m_bInited)
	{
		gfxMovie.Unload();		
	}*/
	m_bInited = false;
	return true;
}

void HUDCommCalls::Update()
{
	if (gClientLogic().localPlayer_ && m_CommandsG[0].id == -1) // G - key commands
	{
		// fill in command list		
		//m_CommandsG[0] = CallCommand(Command_G0, "", -1,-1,-1,-1);
		m_CommandsG[1] = CallCommand(Command_G1, gLangMngr.getString("CommandG1"), SoundSys.GetEventIDByPath(RUS_LANG ? "Sounds/VoiceOver_New/RUS/M/1st/Comm_G1_M_RUS" : "Sounds/VoiceOver_New/ENG/M/1st/Comm_G1_M_ENG"), 
																				   SoundSys.GetEventIDByPath(RUS_LANG ? "Sounds/VoiceOver_New/RUS/M/3st/Comm_G1_M_RUS" : "Sounds/VoiceOver_New/ENG/M/3st/Comm_G1_M_ENG"), 
																				   SoundSys.GetEventIDByPath(RUS_LANG ? "Sounds/VoiceOver_New/RUS/F/1st/Comm_G1_F_RUS" : "Sounds/VoiceOver_New/ENG/F/1st/Comm_G1_F_ENG"), 
																				   SoundSys.GetEventIDByPath(RUS_LANG ? "Sounds/VoiceOver_New/RUS/F/3st/Comm_G1_F_RUS" : "Sounds/VoiceOver_New/ENG/F/3st/Comm_G1_F_ENG"));

		m_CommandsG[2] = CallCommand(Command_G2, gLangMngr.getString("CommandG2"), SoundSys.GetEventIDByPath(RUS_LANG ? "Sounds/VoiceOver_New/RUS/M/1st/Comm_G2_M_RUS" : "Sounds/VoiceOver_New/ENG/M/1st/Comm_G2_M_ENG"),
																				   SoundSys.GetEventIDByPath(RUS_LANG ? "Sounds/VoiceOver_New/RUS/M/3st/Comm_G2_M_RUS" : "Sounds/VoiceOver_New/ENG/M/3st/Comm_G2_M_ENG"), 
																				   SoundSys.GetEventIDByPath(RUS_LANG ? "Sounds/VoiceOver_New/RUS/F/1st/Comm_G2_F_RUS" : "Sounds/VoiceOver_New/ENG/F/1st/Comm_G2_F_ENG"), 
																				   SoundSys.GetEventIDByPath(RUS_LANG ? "Sounds/VoiceOver_New/RUS/F/3st/Comm_G2_F_RUS" : "Sounds/VoiceOver_New/ENG/F/3st/Comm_G2_F_ENG"));

		m_CommandsG[3] = CallCommand(Command_G3, gLangMngr.getString("CommandG3"), SoundSys.GetEventIDByPath(RUS_LANG ? "Sounds/VoiceOver_New/RUS/M/1st/Comm_G3_M_RUS" : "Sounds/VoiceOver_New/ENG/M/1st/Comm_G3_M_ENG"),
																				   SoundSys.GetEventIDByPath(RUS_LANG ? "Sounds/VoiceOver_New/RUS/M/3st/Comm_G3_M_RUS" : "Sounds/VoiceOver_New/ENG/M/3st/Comm_G3_M_ENG"), 
																				   SoundSys.GetEventIDByPath(RUS_LANG ? "Sounds/VoiceOver_New/RUS/F/1st/Comm_G3_F_RUS" : "Sounds/VoiceOver_New/ENG/F/1st/Comm_G3_F_ENG"), 
																				   SoundSys.GetEventIDByPath(RUS_LANG ? "Sounds/VoiceOver_New/RUS/F/3st/Comm_G3_F_RUS" : "Sounds/VoiceOver_New/ENG/F/3st/Comm_G3_F_ENG"));

		m_CommandsG[4] = CallCommand(Command_G4, gLangMngr.getString("CommandG4"), SoundSys.GetEventIDByPath(RUS_LANG ? "Sounds/VoiceOver_New/RUS/M/1st/Comm_G4_M_RUS" : "Sounds/VoiceOver_New/ENG/M/1st/Comm_G4_M_ENG"),
																				   SoundSys.GetEventIDByPath(RUS_LANG ? "Sounds/VoiceOver_New/RUS/M/3st/Comm_G4_M_RUS" : "Sounds/VoiceOver_New/ENG/M/3st/Comm_G4_M_ENG"), 
																				   SoundSys.GetEventIDByPath(RUS_LANG ? "Sounds/VoiceOver_New/RUS/F/1st/Comm_G4_F_RUS" : "Sounds/VoiceOver_New/ENG/F/1st/Comm_G4_F_ENG"), 
																				   SoundSys.GetEventIDByPath(RUS_LANG ? "Sounds/VoiceOver_New/RUS/F/3st/Comm_G4_F_RUS" : "Sounds/VoiceOver_New/ENG/F/3st/Comm_G4_F_ENG"));

		m_CommandsG[5] = CallCommand(Command_G5, gLangMngr.getString("CommandG5"), SoundSys.GetEventIDByPath(RUS_LANG ? "Sounds/VoiceOver_New/RUS/M/1st/Comm_G5_M_RUS" : "Sounds/VoiceOver_New/ENG/M/1st/Comm_G5_M_ENG"),
																				   SoundSys.GetEventIDByPath(RUS_LANG ? "Sounds/VoiceOver_New/RUS/M/3st/Comm_G5_M_RUS" : "Sounds/VoiceOver_New/ENG/M/3st/Comm_G5_M_ENG"), 
																				   SoundSys.GetEventIDByPath(RUS_LANG ? "Sounds/VoiceOver_New/RUS/F/1st/Comm_G5_F_RUS" : "Sounds/VoiceOver_New/ENG/F/1st/Comm_G5_F_ENG"), 
																				   SoundSys.GetEventIDByPath(RUS_LANG ? "Sounds/VoiceOver_New/RUS/F/3st/Comm_G5_F_RUS" : "Sounds/VoiceOver_New/ENG/F/3st/Comm_G5_F_ENG"));

		m_CommandsG[6] = CallCommand(Command_G6, gLangMngr.getString("CommandG6"), SoundSys.GetEventIDByPath(RUS_LANG ? "Sounds/VoiceOver_New/RUS/M/1st/Comm_G6_M_RUS" : "Sounds/VoiceOver_New/ENG/M/1st/Comm_G6_M_ENG"),
																				   SoundSys.GetEventIDByPath(RUS_LANG ? "Sounds/VoiceOver_New/RUS/M/3st/Comm_G6_M_RUS" : "Sounds/VoiceOver_New/ENG/M/3st/Comm_G6_M_ENG"), 
																				   SoundSys.GetEventIDByPath(RUS_LANG ? "Sounds/VoiceOver_New/RUS/F/1st/Comm_G6_F_RUS" : "Sounds/VoiceOver_New/ENG/F/1st/Comm_G6_F_ENG"),
																				   SoundSys.GetEventIDByPath(RUS_LANG ? "Sounds/VoiceOver_New/RUS/F/3st/Comm_G6_F_RUS" : "Sounds/VoiceOver_New/ENG/F/3st/Comm_G6_F_ENG"));

		m_CommandsG[7] = CallCommand(Command_G7, gLangMngr.getString("CommandG7"), SoundSys.GetEventIDByPath(RUS_LANG ? "Sounds/VoiceOver_New/RUS/M/1st/Comm_G7_M_RUS" : "Sounds/VoiceOver_New/ENG/M/1st/Comm_G7_M_ENG"),
																				   SoundSys.GetEventIDByPath(RUS_LANG ? "Sounds/VoiceOver_New/RUS/M/3st/Comm_G7_M_RUS" : "Sounds/VoiceOver_New/ENG/M/3st/Comm_G7_M_ENG"), 
																				   SoundSys.GetEventIDByPath(RUS_LANG ? "Sounds/VoiceOver_New/RUS/F/1st/Comm_G7_F_RUS" : "Sounds/VoiceOver_New/ENG/F/1st/Comm_G7_F_ENG"), 
																				   SoundSys.GetEventIDByPath(RUS_LANG ? "Sounds/VoiceOver_New/RUS/F/3st/Comm_G7_F_RUS" : "Sounds/VoiceOver_New/ENG/F/3st/Comm_G7_F_ENG"));

		m_CommandsG[8] = CallCommand(Command_G8, gLangMngr.getString("CommandG8"), SoundSys.GetEventIDByPath(RUS_LANG ? "Sounds/VoiceOver_New/RUS/M/1st/Comm_G8_M_RUS" : "Sounds/VoiceOver_New/ENG/M/1st/Comm_G8_M_ENG"),
																				   SoundSys.GetEventIDByPath(RUS_LANG ? "Sounds/VoiceOver_New/RUS/M/3st/Comm_G8_M_RUS" : "Sounds/VoiceOver_New/ENG/M/3st/Comm_G8_M_ENG"), 
																				   SoundSys.GetEventIDByPath(RUS_LANG ? "Sounds/VoiceOver_New/RUS/F/1st/Comm_G8_F_RUS" : "Sounds/VoiceOver_New/ENG/F/1st/Comm_G8_F_ENG"), 
																				   SoundSys.GetEventIDByPath(RUS_LANG ? "Sounds/VoiceOver_New/RUS/F/3st/Comm_G8_F_RUS" : "Sounds/VoiceOver_New/ENG/F/3st/Comm_G8_F_ENG"));

		m_CommandsG[9] = CallCommand(Command_G9, gLangMngr.getString("CommandG9"), SoundSys.GetEventIDByPath(RUS_LANG ? "Sounds/VoiceOver_New/RUS/M/1st/Comm_G9_M_RUS" : "Sounds/VoiceOver_New/ENG/M/1st/Comm_G9_M_ENG"),
																				   SoundSys.GetEventIDByPath(RUS_LANG ? "Sounds/VoiceOver_New/RUS/M/3st/Comm_G9_M_RUS" : "Sounds/VoiceOver_New/ENG/M/3st/Comm_G9_M_ENG"), 
																				   SoundSys.GetEventIDByPath(RUS_LANG ? "Sounds/VoiceOver_New/RUS/F/1st/Comm_G9_F_RUS" : "Sounds/VoiceOver_New/ENG/F/1st/Comm_G9_F_ENG"), 
																				   SoundSys.GetEventIDByPath(RUS_LANG ? "Sounds/VoiceOver_New/RUS/F/3st/Comm_G9_F_RUS" : "Sounds/VoiceOver_New/ENG/F/3st/Comm_G9_F_ENG"));
	}

	if (gClientLogic().localPlayer_ && m_CommandsH[0].id == -1) // H - key commands
	{
		// fill in command list
		m_CommandsH[0] = CallCommand(Command_H1, gLangMngr.getString("CommandH1"), -1, -1, -1, -1);
		m_CommandsH[1] = CallCommand(Command_H2, gLangMngr.getString("CommandH2"), -1, -1, -1, -1);
		m_CommandsH[2] = CallCommand(Command_H3, gLangMngr.getString("CommandH3"), -1, -1, -1, -1);
		m_CommandsH[3] = CallCommand(Command_H4, gLangMngr.getString("CommandH4"), -1, -1, -1, -1);
	}

	if (gClientLogic().localPlayer_)
	{
		if (!gClientLogic().localPlayer_->bDead && !hudMain->isChatInputActive() && !hudAttm->isActive() && !hudVault->isActive() && !hudStore->isActive() && !hudSafelock->isActive() && !hudTrade->isActive() && !hudRepair->isActive() && !hudCraft->isActive() && !hudSell->isActive())
		{
			if (hudMain->isRangeFinderUIVisible())
			{				
				if (Keyboard->WasPressed(kbs1))
					addCommRoseCommand(Command_R1);
				else if (Keyboard->WasPressed(kbs2))
					addCommRoseCommand(Command_R2);
				else if (Keyboard->WasPressed(kbs3))
					addCommRoseCommand(Command_R3);
			}

			if (m_bVisible && m_CurrentMsgType == 0) // check for voice messages
			{
				if(Keyboard->WasPressed(kbs1)){
					PlayMessage(gClientLogic().localPlayer_, Command_G1);
					HideMessages();}
				else if(Keyboard->WasPressed(kbs2)){
					PlayMessage(gClientLogic().localPlayer_, Command_G2);
					HideMessages();}
				else if(Keyboard->WasPressed(kbs3)){
					PlayMessage(gClientLogic().localPlayer_, Command_G3);
					HideMessages();}
				else if(Keyboard->WasPressed(kbs4)){
					PlayMessage(gClientLogic().localPlayer_, Command_G4);
					HideMessages();}
				else if(Keyboard->WasPressed(kbs5)){
					PlayMessage(gClientLogic().localPlayer_, Command_G5);
					HideMessages();}
				else if (Keyboard->WasPressed(kbs6)){
					PlayMessage(gClientLogic().localPlayer_, Command_G6);
					HideMessages();}
				else if (Keyboard->WasPressed(kbs7)){
					PlayMessage(gClientLogic().localPlayer_, Command_G7);
					HideMessages();}
				else if(Keyboard->WasPressed(kbs8)){
					PlayMessage(gClientLogic().localPlayer_, Command_G8);
					HideMessages();}
				else if(Keyboard->WasPressed(kbs9)){
					PlayMessage(gClientLogic().localPlayer_, Command_G9);
					HideMessages();}				
				else if(Keyboard->WasPressed(kbsEsc))
					HideMessages();
			}			
			else if(m_bVisible && m_CurrentMsgType == 1) // check for commrose messages
			{
				if (Keyboard->WasPressed(kbs1)){
					addCommRoseCommand(Command_H1);
					HideMessages();}
				else if (Keyboard->WasPressed(kbs2)){
					addCommRoseCommand(Command_H2);
					HideMessages();}
				else if (Keyboard->WasPressed(kbs3)){
					addCommRoseCommand(Command_H3);
					HideMessages();}
				else if (Keyboard->WasPressed(kbs4)){
					addCommRoseCommand(Command_H4);
					HideMessages();}
				else if (Keyboard->WasPressed(kbsEsc))
					HideMessages();
			}

			if (InputMappingMngr->wasPressed(r3dInputMappingMngr::KS_COMMAND_G))
				ShowMessages(0);
			else if(InputMappingMngr->wasPressed(r3dInputMappingMngr::KS_COMMAND_H))
				ShowMessages(1);		
		}
		else
		{
			if(m_bVisible)
				HideMessages();
		}
	}
}

void HUDCommCalls::Draw()
{
	/*if (m_bVisible)
	{			
		gfxMovie.SetCurentRTViewport(Scaleform::GFx::Movie::SM_NoScale);
		gfxMovie.SetCurentRTViewport(Scaleform::GFx::Movie::SM_ExactFit);

		gfxMovie.UpdateAndDraw();
	}*/	
}

void HUDCommCalls::addCommRoseCommand(int id)
{
	if (gClientLogic().localPlayer_ == NULL)
		return;

	// anti spam check
	if (gClientLogic().localPlayer_)
	{		
		static float lastTimeMessage = 0;
		if (r3dGetTime() - lastTimeMessage < 5.0f)
		{
			if (hudMain) hudMain->showMessage(gLangMngr.getString("InfoMsg_CooldownActive"));
			return; // anti spam
		}
		lastTimeMessage = r3dGetTime();
	}

	if (id == Command_R1 || id == Command_R2 || id == Command_R3)
	{
		const float MAX_CASTING_DISTANCE = 20000.f;
		r3dPoint3D dir;
		if (g_camera_mode->GetInt() == 1)
			r3dScreenTo3D(r3dRenderer->ScreenW2, r3dRenderer->ScreenH*0.32f, &dir);
		else
			r3dScreenTo3D(r3dRenderer->ScreenW2, r3dRenderer->ScreenH2, &dir);

		PxRaycastHit hit;
		PxSceneQueryFilterData filter(PxFilterData(COLLIDABLE_STATIC_MASK | (1 << PHYSCOLL_NETWORKPLAYER), 0, 0, 0), PxSceneQueryFilterFlag::eSTATIC);
		if (g_pPhysicsWorld->raycastSingle(PxVec3(gCam.x, gCam.y, gCam.z), PxVec3(dir.x, dir.y, dir.z), MAX_CASTING_DISTANCE, PxSceneQueryFlag::eIMPACT, hit, filter))
		{
			r3dVector pos(hit.impact.x, hit.impact.y, hit.impact.z);
			showHUDIcon(gClientLogic().localPlayer_, id, pos);

			// send msg to server
			PKT_C2C_CommRoseCommand_s n;
			n.id = id;
			n.pos = pos;
			p2pSendToHost(gClientLogic().localPlayer_, &n, sizeof(n));
			//r3dOutToLog("### btnID %d posx %.2f posy %.2f posz %.2f\n", btnID, pos.x, pos.y, pos.z);
		}
	}
	else
	{
		showHUDIcon(gClientLogic().localPlayer_, id, r3dVector(0, 0, 0));

		// send msg to server
		PKT_C2C_CommRoseCommand_s n;
		n.id = id;
		n.pos = r3dVector(0, 0, 0);
		p2pSendToHost(gClientLogic().localPlayer_, &n, sizeof(n));
	}
}

void HUDCommCalls::showHUDIcon(class obj_Player* from, int id, const r3dVector& pos)
{
	// check for distance
	//float distance = (from->GetPosition() - gClientLogic().localPlayer_->GetPosition()).Length();
	//if (distance > 600.0f)
		//return;	
	
	if (id == Command_R1)
	{
		if (g_enable_voice_commands->GetBool())
		{
			if (from == gClientLogic().localPlayer_)
				SoundSys.PlayAndForget(SoundSys.GetEventIDByPath(from->m_isFemaleHero ? RUS_LANG ? "Sounds/VoiceOver_New/RUS/F/1st/Comm_R1_F_RUS" : "Sounds/VoiceOver_New/ENG/F/1st/Comm_R1_F_ENG" : 
																						RUS_LANG ? "Sounds/VoiceOver_New/RUS/M/1st/Comm_R1_M_RUS" : "Sounds/VoiceOver_New/ENG/M/1st/Comm_R1_M_ENG"), r3dPoint3D(0, 0, 0));
			else
				SoundSys.PlayAndForget(SoundSys.GetEventIDByPath(from->m_isFemaleHero ? RUS_LANG ? "Sounds/VoiceOver_New/RUS/F/3st/Comm_R1_F_RUS" : "Sounds/VoiceOver_New/ENG/F/3st/Comm_R1_F_ENG" : 
																						RUS_LANG ? "Sounds/VoiceOver_New/RUS/M/3st/Comm_R1_M_RUS" : "Sounds/VoiceOver_New/ENG/M/3st/Comm_R1_M_ENG"), r3dPoint3D(0, 0, 0));
		}
		if (hudMain)hudMain->addHUDIcon(HUDDisplay::HUDIcon_Attack, 10.0f, pos);		
	}
	else if (id == Command_R2)
	{
		if (g_enable_voice_commands->GetBool())
		{
			if (from == gClientLogic().localPlayer_)
				SoundSys.PlayAndForget(SoundSys.GetEventIDByPath(from->m_isFemaleHero ? RUS_LANG ? "Sounds/VoiceOver_New/RUS/F/1st/Comm_R2_F_RUS" : "Sounds/VoiceOver_New/ENG/F/1st/Comm_R2_F_ENG" : 
																						RUS_LANG ? "Sounds/VoiceOver_New/RUS/M/1st/Comm_R2_M_RUS" : "Sounds/VoiceOver_New/ENG/M/1st/Comm_R2_M_ENG"), r3dPoint3D(0, 0, 0));
			else
				SoundSys.PlayAndForget(SoundSys.GetEventIDByPath(from->m_isFemaleHero ? RUS_LANG ? "Sounds/VoiceOver_New/RUS/F/3st/Comm_R2_F_RUS" : "Sounds/VoiceOver_New/ENG/F/3st/Comm_R2_F_ENG" : 
																						RUS_LANG ? "Sounds/VoiceOver_New/RUS/M/3st/Comm_R2_M_RUS" : "Sounds/VoiceOver_New/ENG/M/3st/Comm_R2_M_ENG"), r3dPoint3D(0, 0, 0));
		}
		if (hudMain)hudMain->addHUDIcon(HUDDisplay::HUDIcon_Defend, 10.0f, pos);		
	}
	else if (id == Command_R3)
	{
		if (g_enable_voice_commands->GetBool())
		{
			if (from == gClientLogic().localPlayer_)
				SoundSys.PlayAndForget(SoundSys.GetEventIDByPath(from->m_isFemaleHero ? RUS_LANG ? "Sounds/VoiceOver_New/RUS/F/1st/Comm_R3_F_RUS" : "Sounds/VoiceOver_New/ENG/F/1st/Comm_R3_F_ENG" : 
																						RUS_LANG ? "Sounds/VoiceOver_New/RUS/M/1st/Comm_R3_M_RUS" : "Sounds/VoiceOver_New/ENG/M/1st/Comm_R3_M_ENG"), r3dPoint3D(0, 0, 0));
			else
				SoundSys.PlayAndForget(SoundSys.GetEventIDByPath(from->m_isFemaleHero ? RUS_LANG ? "Sounds/VoiceOver_New/RUS/F/3st/Comm_R3_F_RUS" : "Sounds/VoiceOver_New/ENG/F/3st/Comm_R3_F_ENG" : 
																						RUS_LANG ? "Sounds/VoiceOver_New/RUS/M/3st/Comm_R3_M_RUS" : "Sounds/VoiceOver_New/ENG/M/3st/Comm_R3_M_ENG"), r3dPoint3D(0, 0, 0));
		}
		if (hudMain)hudMain->addHUDIcon(HUDDisplay::HUDIcon_Spotted, 10.0f, pos);		
	}
	else if (id >= Command_H1 && id <= Command_H4)
	{
		switch(id)
		{
		case Command_H1:
			if (g_enable_voice_commands->GetBool())
				SoundSys.PlayAndForget(SoundSys.GetEventIDByPath(from == gClientLogic().localPlayer_?"Sounds/VoiceOver/ENG/Metal_Gear_Alert_2D":"Sounds/VoiceOver/ENG/Metal_Gear_Alert_3D"),
																 from == gClientLogic().localPlayer_?r3dPoint3D(0, 0, 0):from->GetPosition());
			break;
		case Command_H2:
			if (g_enable_voice_commands->GetBool())
				SoundSys.PlayAndForget(SoundSys.GetEventIDByPath("Sounds/VoiceOver/ENG/friendly units need medic"), r3dPoint3D(0, 0, 0));
			break;
		case Command_H3:
			if (g_enable_voice_commands->GetBool())
				SoundSys.PlayAndForget(SoundSys.GetEventIDByPath("Sounds/VoiceOver/ENG/friendly units need ammo"), r3dPoint3D(0, 0, 0));
			break;
		case Command_H4:
			if (g_enable_voice_commands->GetBool())
				SoundSys.PlayAndForget(SoundSys.GetEventIDByPath("Sounds/VoiceOver/ENG/friendly units need help"), r3dPoint3D(0, 0, 0));
			break;
		}

		if(from == gClientLogic().localPlayer_) // do not show icon above our own head
			return;

		if (from->m_commRoseTimer > 0 && !from->m_CommRoseIcon.IsUndefined())
		{
			if (hudMain)hudMain->removeUserIcon(from->m_CommRoseIcon);
		}
		from->m_commRoseTimer = 5.0f;
		switch(id)
		{
		case Command_H1:
			if (hudMain)hudMain->addScreenIcon(from->m_CommRoseIcon, "alert");
			break;
		case Command_H2:
			if (hudMain)hudMain->addScreenIcon(from->m_CommRoseIcon, "medic");
			break;
		case Command_H3:
			if (hudMain)hudMain->addScreenIcon(from->m_CommRoseIcon, "ammo");
			break;
		case Command_H4:
			if (hudMain)hudMain->addScreenIcon(from->m_CommRoseIcon, "help");
			break;
		}
		if (hudMain)hudMain->moveUserIcon(from->m_CommRoseIcon, pos, false, true); // hide until player update
	}
}

//extern char* s_ChatIgnoreList[256];
//extern int s_CharIgnoreListNum;

void HUDCommCalls::PlayMessage(obj_Player* from, int id)
{
	if(!from)
		return;
	
	char plrFromUserName[64];
	from->GetUserName(plrFromUserName);
	// anti spam check
	if(from == gClientLogic().localPlayer_)
	{
		static float lastTimeMessage = 0;
		if(r3dGetTime() - lastTimeMessage < 3.0f)
		{
			if (hudMain) hudMain->showMessage(gLangMngr.getString("InfoMsg_CooldownActive"));
			return; // anti spam
		}
		lastTimeMessage = r3dGetTime();
	}
	/*else
	{
		// check ignore list		
		for(int i=0; i<s_CharIgnoreListNum; ++i)
		{
			if(stricmp(s_ChatIgnoreList[i], plrFromUserName)==0)
				return; // ignore msg
		}
		//from->m_pingIconTimer = 1.0f;
	}*/

	int chatTabIndex = 0;
	if(gClientLogic().localPlayer_->GroupID != 0 && from->GroupID == gClientLogic().localPlayer_->GroupID)
		chatTabIndex = 3;
	else if(gClientLogic().localPlayer_->ClanID != 0 && from->ClanID == gClientLogic().localPlayer_->ClanID)
		chatTabIndex = 4;

	if (id >= Command_G1 && id <= Command_G9)
	{
		int i = id;
		if (m_CommandsG[i].id != -1)
		{
			if (hudMain)
				hudMain->addChatMessage(chatTabIndex, plrFromUserName, m_CommandsG[i].txtCommand, 0);

			if (g_enable_voice_commands->GetBool())
			{
				if (from == gClientLogic().localPlayer_)
					SoundSys.PlayAndForget(from->m_isFemaleHero?m_CommandsG[i].voiceCmdFamale:m_CommandsG[i].voiceCmdMale, r3dPoint3D(0, 0, 0));
				else
					SoundSys.PlayAndForget(from->m_isFemaleHero?m_CommandsG[i].voiceCmdFamaleRadio:m_CommandsG[i].voiceCmdMaleRadio, r3dPoint3D(0, 0, 0));
			}
		}
	}

	if(from == gClientLogic().localPlayer_)
	{
		HideMessages();
		// send msg to server
		PKT_C2C_VoiceCommand_s n;
		n.id = id;
		p2pSendToHost(from, &n, sizeof(n));
	}
}

void HUDCommCalls::ShowMessages(int msgType)
{
	if(m_bVisible && m_CurrentMsgType == msgType)
	{
		HideMessages();
		return;
	}
	m_CurrentMsgType = msgType;

	CallCommand* commandsArray = 0;
	if(msgType == 0)
		commandsArray = &m_CommandsG[0];
	else
		commandsArray = &m_CommandsH[0];
	
	for (int i = 0; i<10; ++i)
	{
		if(commandsArray[i].id!=-1)
		{
			if (hudMain)
				hudMain->addCommandsData(commandsArray[i].txtCommand);
		}
	}
	
	if (!m_bVisible)
	{
		if (hudMain)
			hudMain->toggleCommandsPopUp(true);
	}
	
	m_bVisible = true;
}

void HUDCommCalls::HideMessages()
{
	Deactivate();

	if (hudMain)
		hudMain->toggleCommandsPopUp(m_bVisible);
}

void HUDCommCalls::Deactivate()
{
	m_bVisible = false;
}