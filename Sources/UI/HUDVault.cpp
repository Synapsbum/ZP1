#include "r3dPCH.h"
#include "r3d.h"

#include "../../../Eternity/sf/Console/config.h"
#include "HUDVault.h"
#include "HUDDisplay.h"
#include "HUDAttachments.h"
#include "HUDStore.h"
#include "HUDTrade.h"
#include "HUDRepair.h"
#include "HUDPause.h"
#include "HUDSafelock.h"
#include "HUDSell.h"
#include "HUDCommCalls.h"
#include "LangMngr.h"

#include "FrontendShared.h"

#include "../multiplayer/clientgamelogic.h"
#include "../ObjectsCode/AI/AI_Player.H"
#include "../ObjectsCode/weapons/Weapon.h"
#include "../ObjectsCode/weapons/WeaponArmory.h"
#include "../GameLevel.h"

extern HUDAttachments*	hudAttm;
extern HUDDisplay* hudMain;
extern HUDStore*	hudStore;
extern HUDPause*	hudPause;
extern HUDTrade* hudTrade;
extern HUDSafelock* hudSafelock;
extern HUDSell*	    hudSell;
extern HUDRepair* hudRepair;
extern HUDCommCalls* hudCommCalls;

HUDVault::HUDVault()
{
	isActive_ = false;
	isInit = false;
	prevKeyboardCaptureMovie = NULL;
	m_DeadTimer_ = 0;
	m_DeadExitTimer_ = 0;
}

HUDVault::~HUDVault()
{
}

void HUDVault::eventBackpackGridSwap(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount == 2);
	int gridFrom = args[0].GetInt();
	int gridTo = args[1].GetInt();

	obj_Player* plr = gClientLogic().localPlayer_;
	r3d_assert(plr);

	if(plr->bDead)//AlexRedd:: not allowed!
	{
		Scaleform::GFx::Value var[3];
		var[0].SetString(gLangMngr.getString("InGameUI_ErrorMsg_NotAllowed"));
		var[1].SetBoolean(true);
		var[2].SetString("");
		gfxMovie.Invoke("_root.api.showInfoMsg", var, 3);
		return;
	}

	plr->BackpackGridSwap(gridFrom, gridTo);

	updateSurvivorTotalWeight();

	gfxMovie.Invoke("_root.api.backpackGridSwapSuccess", "");
}

void HUDVault::eventBackpackFromInventory(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	//inventoryID:uint, gridTo:int, amount:int
	r3d_assert(argCount == 3);
	uint32_t inventoryID = args[0].GetUInt();
	int gridTo = args[1].GetInt();
	int amount = args[2].GetInt();

	obj_Player* plr = gClientLogic().localPlayer_;
	r3d_assert(plr);

	r3d_assert(isLockedUI==false);
	isLockedUI = true;
	lockedOp = 1;

	Scaleform::GFx::Value var[2];
	var[0].SetString(gLangMngr.getString("OneMomentPlease"));
	var[1].SetBoolean(false);
	gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);

	PKT_C2S_FromInventoryReq_s n;
	n.InventoryID     = inventoryID;
	n.SlotTo          = gridTo == -1 ? 0xFF : gridTo;		// or 0xFF if server should decide it
	n.Amount          = amount;
	if(n.SlotTo != 0xFF)
	{
		n.dbg_ItemID      = plr->CurLoadout.Items[n.SlotTo].itemID;
		n.dbg_Quantity    = plr->CurLoadout.Items[n.SlotTo].quantity;
	}
	else
	{
		n.dbg_ItemID      = 0;
		n.dbg_Quantity    = 0;
	}

	n.dbg_InvItemID = 0;
	n.dbg_InvQuantity = 0;
	for(uint32_t i=0; i<gUserProfile.ProfileData.NumItems; ++i)
	{
		if(gUserProfile.ProfileData.Inventory[i].InventoryID == inventoryID)
		{
			n.dbg_InvItemID   = gUserProfile.ProfileData.Inventory[i].itemID;
			n.dbg_InvQuantity = gUserProfile.ProfileData.Inventory[i].quantity;
			break;
		}
	}
	// we must have that item in inventory
	r3d_assert(n.dbg_InvItemID);

	// move non-stackable items by one
	if(n.Amount > 1 && !storecat_IsItemStackable(n.dbg_InvItemID))
		n.Amount = 1;

	p2pSendToHost(gClientLogic().localPlayer_, &n, sizeof(n));
}

void HUDVault::eventBackpackToInventory(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	//gridFrom:int, amount:int
	r3d_assert(argCount == 2);
	int gridFrom = args[0].GetInt();
	int amount = args[1].GetInt();

	obj_Player* plr = gClientLogic().localPlayer_;
	r3d_assert(plr);

	if(plr->bDead)//AlexRedd:: not allowed!
	{
		Scaleform::GFx::Value var[3];
		var[0].SetString(gLangMngr.getString("InGameUI_ErrorMsg_NotAllowed"));
		var[1].SetBoolean(true);
		var[2].SetString("");
		gfxMovie.Invoke("_root.api.showInfoMsg", var, 3);
		return;
	}

	r3d_assert(isLockedUI==false);
	isLockedUI = true;
	lockedOp = 2;

	Scaleform::GFx::Value var[2];
	var[0].SetString(gLangMngr.getString("OneMomentPlease"));
	var[1].SetBoolean(false);
	gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);

	PKT_C2S_ToInventoryReq_s n;
	n.SlotFrom     = gridFrom;
	n.Amount       = amount; 
	n.dbg_ItemID   = plr->CurLoadout.Items[n.SlotFrom].itemID;
	n.dbg_Quantity = plr->CurLoadout.Items[n.SlotFrom].quantity;
	p2pSendToHost(gClientLogic().localPlayer_, &n, sizeof(n));
}

void HUDVault::eventReturnToGame(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	Deactivate();

	if(hudMain && hudMain->isShowingYouAreDead && gClientLogic().localPlayer_->bDead)
		r3dMouse::Show();
}

void HUDVault::eventOpenStore(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	Deactivate();

	if(hudMain && hudMain->isShowingYouAreDead && gClientLogic().localPlayer_->bDead && gUserProfile.ProfileData.PremiumAcc>0)
	{
		r3dMouse::Show();
#ifdef ENABLE_INGAME_STORE		
		if (!hudAttm->isActive() && !hudSafelock->isActive() && !hudTrade->isActive() && !hudRepair->isActive() && !hudSell->isActive() && !hudCommCalls->isVisible() &&
#ifdef ENABLE_BATTLE_ROYALE
			//AlexRedd:: BR mode
			!gClientLogic().m_gameInfo.IsGameBR() &&
#endif //ENABLE_BATTLE_ROYALE
			 !gClientLogic().m_gameInfo.IsDevEvent() )
		{
			if(!hudStore->isActive())
			{
				if(hudPause->isActive())
					hudPause->Deactivate();

				hudStore->Activate();
			}
			else
				hudStore->Deactivate();
		}
#endif
	}
}

void HUDVault::setErrorMsg(const char* errMsg)
{
	if(isLockedUI)
	{
		isLockedUI = false;
		lockedOp = 0;
		gfxMovie.Invoke("_root.api.hideInfoMsg", "");
		Scaleform::GFx::Value var[3];
		var[0].SetString(errMsg);
		var[1].SetBoolean(true);
		var[2].SetString("ERROR");
		gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);
	}
}

void HUDVault::unlockUI()
{
	r3d_assert(isLockedUI);
	isLockedUI = false;
	gfxMovie.Invoke("_root.api.hideInfoMsg", "");
	if(lockedOp == 0)
	{
	}
	else if(lockedOp==1)
	{
		gfxMovie.Invoke("_root.api.backpackFromInventorySuccess", "");
	}
	else if(lockedOp==2)
	{
		gfxMovie.Invoke("_root.api.backpackToInventorySuccess", "");
	}
	else if(lockedOp == 3)
	{
		gfxMovie.Invoke("_root.api.backpackToInventorySuccess", "");
		gfxMovie.Invoke("_root.api.backpackFromInventorySuccess", "");

		reloadBackpackInfo();
		updateSurvivorTotalWeight();
		gfxMovie.Invoke("_root.api.changeBackpackSuccess", "");
		gfxMovie.Invoke("_root.api.showInventoryScreen", "");
	}
	else
		r3d_assert(false);
	
	lockedOp = 0;
}

bool HUDVault::Init()
{
 	if(!gfxMovie.Load("Data\\Menu\\Vault.swf", false)) 
 		return false;
 
#define MAKE_CALLBACK(FUNC) game_new r3dScaleformMovie::TGFxEICallback<HUDVault>(this, &HUDVault::FUNC)
 	gfxMovie.RegisterEventHandler("eventBackpackFromInventory", MAKE_CALLBACK(eventBackpackFromInventory));
 	gfxMovie.RegisterEventHandler("eventBackpackToInventory", MAKE_CALLBACK(eventBackpackToInventory));
	gfxMovie.RegisterEventHandler("eventBackpackGridSwap", MAKE_CALLBACK(eventBackpackGridSwap));
	gfxMovie.RegisterEventHandler("eventReturnToGame", MAKE_CALLBACK(eventReturnToGame));
	gfxMovie.RegisterEventHandler("eventOpenStore", MAKE_CALLBACK(eventOpenStore));
	gfxMovie.RegisterEventHandler("eventMoveAllItems", MAKE_CALLBACK(eventMoveAllItems));
	gfxMovie.RegisterEventHandler("eventLoad_Loadout", MAKE_CALLBACK(eventLoad_Loadout));
	gfxMovie.RegisterEventHandler("eventSave_Loadout", MAKE_CALLBACK(eventSave_Loadout));
	gfxMovie.RegisterEventHandler("eventDelete_Loadout", MAKE_CALLBACK(eventDelete_Loadout));

	gfxMovie.SetCurentRTViewport( Scaleform::GFx::Movie::SM_ExactFit );

	// add items
	{
		addItemCategoriesToUI(gfxMovie);
		addItemsAndCategoryToUI(gfxMovie);
	}

	isActive_ = false;
	isInit = true;
	isLockedUI = false;
	lockedOp = 0;
	scan_loadout();
	return true;
}

bool HUDVault::Unload()
{
 	gfxMovie.Unload();
	isActive_ = false;
	isInit = false;
	return true;
}

void HUDVault::Update()
{
	if(m_DeadTimer_ > 0)
	{
		int timeLeft = m_DeadTimer_;
		
		int minutes = timeLeft/60;
		timeLeft -= minutes*60;
		int seconds = timeLeft;
		char tempStr[32];
		sprintf(tempStr, "%02d:%02d", minutes, seconds);

		char TimeLeft[128] = { 0 };
		sprintf(TimeLeft, "%s %s", gLangMngr.getString("$FR_RESPAWN_IN"), tempStr);
		gfxMovie.SetVariable("_root.Main.RespawnTime.text", TimeLeft);
		gfxMovie.SetVariable("_root.Main.RespawnTime.visible", true);
	}
	else if(m_DeadExitTimer_ > 0)
	{
		int timeLeft = m_DeadExitTimer_;
		
		int minutes = timeLeft/60;
		timeLeft -= minutes*60;
		int seconds = timeLeft;
		char tempStr[32];
		sprintf(tempStr, "%02d:%02d", minutes, seconds);

		char TimeLeft[128] = { 0 };
		sprintf(TimeLeft, "%s %s", gLangMngr.getString("$FR_EXIT_IN"), tempStr);
		gfxMovie.SetVariable("_root.Main.RespawnTime.text", TimeLeft);
		gfxMovie.SetVariable("_root.Main.RespawnTime.visible", true);
	}
	else
	{
		gfxMovie.SetVariable("_root.Main.RespawnTime.visible", false);
	}
}

void HUDVault::Draw()
{
	gfxMovie.SetCurentRTViewport(Scaleform::GFx::Movie::SM_NoScale);
	gfxMovie.SetCurentRTViewport(Scaleform::GFx::Movie::SM_ExactFit);

 	gfxMovie.UpdateAndDraw();
}

void HUDVault::Deactivate()
{
	if(isLockedUI)
		return;

	if(prevKeyboardCaptureMovie)
	{
		prevKeyboardCaptureMovie->SetKeyboardCapture();
		prevKeyboardCaptureMovie = NULL;
	}

	gfxMovie.Invoke("_root.api.hideInventoryScreen", "");

	{
		Scaleform::GFx::Value var[1];
		var[0].SetString("menu_close");
		gfxMovie.OnCommandCallback("eventSoundPlay", var, 1);
	}

	if( !g_cursor_mode->GetInt() )
	{
		r3dMouse::Hide();
	}

	isActive_ = false;
	gfxMovie.SetVariable("_root.api.Main.BtnStore.visible", false);
}

extern const char* getReputationString(int reputation);
void HUDVault::Activate()
{
	r3d_assert(!isActive_);
	r3dMouse::Show();
	isActive_ = true;

	prevKeyboardCaptureMovie = gfxMovie.SetKeyboardCapture(); // for mouse scroll events

	if(hudMain && hudMain->isShowingYouAreDead && gClientLogic().localPlayer_->bDead && gUserProfile.ProfileData.PremiumAcc>0)
	{
		gfxMovie.SetVariable("_root.api.Main.BtnStore.visible", true);
	}
	else{
		gfxMovie.SetVariable("_root.api.Main.BtnStore.visible", false);
	}

	// add player info
	{
		Scaleform::GFx::Value var[22];

		obj_Player* plr = gClientLogic().localPlayer_;
		r3d_assert(plr);

		// reset barricade placement if going into UI
		plr->m_needToDrawBarricadePlacement = 0; 
		plr->m_BarricadeMeshPlacement = NULL;

		wiCharDataFull& slot = plr->CurLoadout;
		char tmpGamertag[128];
		if(plr->ClanID != 0)
			sprintf(tmpGamertag, "[%s] %s", plr->ClanTag, slot.Gamertag);
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
		if(slot.Skills[CUserSkills::SKILL_Physical3])
			totalWeight *= 0.95f;
		if(slot.Skills[CUserSkills::SKILL_Physical7])
			totalWeight *= 0.9f;

		var[15].SetNumber(totalWeight);		// weight
		var[16].SetNumber(0);		// zombies Killed
		var[17].SetNumber(0);		// bandits killed
		var[18].SetNumber(0);		// civilians killed
		var[19].SetString(getReputationString(slot.Stats.Reputation));	// alignment
		var[20].SetString("");	// last Map
		var[21].SetBoolean(true); // global inventory

		gfxMovie.Invoke("_root.api.addClientSurvivor", var, 22);
	}

	reloadInventoryInfo(gfxMovie);

	reloadBackpackInfo();

	updateSurvivorTotalWeight();

	{
		Scaleform::GFx::Value var[1];
		var[0].SetString("menu_open");
		gfxMovie.OnCommandCallback("eventSoundPlay", var, 1);
	}

	gfxMovie.Invoke("_root.api.showInventoryScreen", "");
}

void HUDVault::updateInventory()
{
	reloadInventoryInfo(gfxMovie);
}

void HUDVault::updateSurvivorTotalWeight()
{
	obj_Player* plr = gClientLogic().localPlayer_;
	r3d_assert(plr);

	float totalWeight = plr->CurLoadout.getTotalWeight();
	if(plr->CurLoadout.Skills[CUserSkills::SKILL_Physical3])
		totalWeight *= 0.95f;
	if(plr->CurLoadout.Skills[CUserSkills::SKILL_Physical7])
		totalWeight *= 0.9f;

	Scaleform::GFx::Value var[2];
	wiCharDataFull& slot = plr->CurLoadout;
	char tmpGamertag[128];
	if(plr->ClanID != 0)
		sprintf(tmpGamertag, "[%s] %s", plr->ClanTag, slot.Gamertag);
	else
		r3dscpy(tmpGamertag, slot.Gamertag);

	var[0].SetString(tmpGamertag);
	var[1].SetNumber(totalWeight);
	gfxMovie.Invoke("_root.api.updateClientSurvivorWeight", var, 2);
}

void HUDVault::reloadBackpackInfo()
{
	updateSurvivorTotalWeight();
	// reset backpack
	{
		gfxMovie.Invoke("_root.api.clearBackpack", "");
		gfxMovie.Invoke("_root.api.clearBackpacks", "");
	}

	r3dgameVector(uint32_t) uniqueBackpacks; // to filter identical backpack
	int backpackSlotIDInc = 0;
	// add backpack content info
	{
		obj_Player* plr = gClientLogic().localPlayer_;
		r3d_assert(plr);
		wiCharDataFull& slot = plr->CurLoadout;

		Scaleform::GFx::Value var[7];
		for (int a = 0; a < slot.BackpackSize; a++)
		{
			if (slot.Items[a].itemID != 0)
			{
				var[0].SetInt(a);
				var[1].SetUInt(0); // not used for game
				var[2].SetUInt(slot.Items[a].itemID);
				var[3].SetInt(slot.Items[a].quantity);
				var[4].SetInt(slot.Items[a].Var1);
				var[5].SetInt(slot.Items[a].Var2);
				var[6].SetString(getAdditionalDescForItem(slot.Items[a].itemID, slot.Items[a].Var1, slot.Items[a].Var2, slot.Items[a].Var3));
				gfxMovie.Invoke("_root.api.addBackpackItem", var, 7);

				const BackpackConfig* bpc = g_pWeaponArmory->getBackpackConfig(slot.Items[a].itemID);
				if(bpc)
				{
					if(std::find<r3dgameVector(uint32_t)::iterator, uint32_t>(uniqueBackpacks.begin(), uniqueBackpacks.end(), slot.Items[a].itemID) != uniqueBackpacks.end())
						continue;

					// add backpack info
					var[0].SetInt(backpackSlotIDInc++);
					var[1].SetUInt(slot.Items[a].itemID);
					gfxMovie.Invoke("_root.api.addBackpack", var, 2);

					uniqueBackpacks.push_back(slot.Items[a].itemID);
				}
			}
		}
	}

	gfxMovie.Invoke("_root.api.Main.showBackpack", "");
}

void HUDVault::eventMoveAllItems(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount == 0);

	r3d_assert(isLockedUI == false);
	isLockedUI = true;
	lockedOp = 2;

	Scaleform::GFx::Value var[2];
	var[0].SetString(gLangMngr.getString("OneMomentPlease"));
	var[1].SetBoolean(false);
	gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);

	PKT_C2S_MoveAllItemsToGIReq_s n;
	p2pSendToHost(gClientLogic().localPlayer_, &n, sizeof(n));
}

void HUDVault::scan_loadout()
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

void HUDVault::clear_loadout()
{
	gfxMovie.Invoke("_root.api.clear_loadout", "");
}

void HUDVault::add_loadout(const char* filename)
{
	const wiCharDataFull& slot = gUserProfile.ProfileData.ArmorySlots[gUserProfile.SelectedCharID];

	loadout l;
	l.load_loadout(slot, filename);

	Scaleform::GFx::Value args[2];
	args[0].SetString(l.name);
	args[1].SetString(filename);
	gfxMovie.Invoke("_root.api.add_loadout", args, 2);

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

void HUDVault::refresh_loadout()
{
	gfxMovie.Invoke("_root.api.refresh_loadout", "");
}

void HUDVault::eventLoad_Loadout(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount == 1);

	const char* name = args[0].GetString();
	load_loadout(name);
}

void HUDVault::load_loadout(const char* name)
{
	const wiCharDataFull& slot = gUserProfile.ProfileData.ArmorySlots[gUserProfile.SelectedCharID];

	loadout l;
	l.load_loadout(slot, name);

	r3d_assert(isLockedUI == false);
	isLockedUI = true;
	lockedOp = 3;

	Scaleform::GFx::Value var[2];
	var[0].SetString(gLangMngr.getString("OneMomentPlease"));
	var[1].SetBoolean(false);
	gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);

	PKT_C2S_ChangeLoadoutReq_s n;
	n.loadout_ = l;
	p2pSendToHost(gClientLogic().localPlayer_, &n, sizeof(n));
}

bool HUDVault::find_free_loadout_name(const char* fname)
{
	std::ifstream file(fname);
	return file.is_open();
}

void HUDVault::eventSave_Loadout(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
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


void HUDVault::save_loadout(const char* fname, const char* name)
{
	const wiCharDataFull& slot = gClientLogic().localPlayer_->CurLoadout;
	loadout l;
	l.save_loadout(slot, fname, name);
}

void HUDVault::clear_loadout_pictures()
{
	gfxMovie.Invoke("_root.api.clear_loadout_picture", "");
}

void HUDVault::add_loadout_picture(const char* filename, const char* picture)
{
	Scaleform::GFx::Value args[2];
	args[0].SetString(filename);
	args[1].SetString(picture);
	gfxMovie.Invoke("_root.api.add_loadout_picture", args, 2);
}

void HUDVault::eventDelete_Loadout(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount == 1);

	char* fname = (char*)args[0].GetString();
	DeleteFile(fname);
	scan_loadout();
}
