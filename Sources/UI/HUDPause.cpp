#include "r3dPCH.h"
#include "r3d.h"

#include "../../../Eternity/sf/Console/config.h"
#include "HUDPause.h"
#include "HUDDisplay.h"
#include "HUDAttachments.h"
#include "HUDCrafting.h"
#include "LangMngr.h"

#include "FrontendShared.h"
#include "TeamSpeakClient.h"

#include "../multiplayer/clientgamelogic.h"
#include "../ObjectsCode/AI/AI_Player.H"
#include "../ObjectsCode/weapons/Weapon.h"
#include "../ObjectsCode/weapons/WeaponArmory.h"
#include "../GameLevel.h"

#ifdef VEHICLES_ENABLED
#include "../GameEngine/gameobjects/obj_Vehicle.h"
#endif

void writeGameOptionsFile();

extern HUDDisplay* hudMain;
extern HUDCrafting* hudCraft;

#ifdef MISSIONS
// These must match the ActionIconNames in MissionEnums.h on the server.
static const char* ActionIconNames[] = {
	 "Goto"
	,"Kill"
	,"Collect"
	,"Drop"
	,"Use"
	,"On"
	,"Off"
	,"Rad"//AlexRedd:: radioactive icon
};
#endif

HUDPause::HUDPause()
	: isActive_( false )
	, isInit( false )
	, prevKeyboardCaptureMovie( NULL )
	, isLockedUI( false )
	, wpnAttachIdx( 0 )	
{
	needUpdateMode_ = false;
}

HUDPause::~HUDPause()
{	
	r_hud_filter_mode->SetInt(HUDFilter_Default);//Cynthia: 257, force to turn off nightvision when disconnecting from game
}

void HUDPause::eventBackpackGridSwap(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount == 2);
	int gridFrom = args[0].GetInt();
	int gridTo = args[1].GetInt();

	obj_Player* plr = gClientLogic().localPlayer_;
	r3d_assert(plr);
	plr->BackpackGridSwap(gridFrom, gridTo);

	updateSurvivorTotalWeight();

	gfxMovie.Invoke("_root.api.backpackGridSwapSuccess", "");
}

void HUDPause::ChangePlayerBackpackFast(int ItemID)
{
	obj_Player* plr = gClientLogic().localPlayer_;
	r3d_assert(plr);
	wiCharDataFull& slot = plr->CurLoadout;

	// find inventory slot
	int slotFrom = -1;
	for(int i=0; i<slot.BackpackSize; ++i)
	{
		if(slot.Items[i].itemID == ItemID)
		{
			slotFrom = i;
			break;
		}
	}
	if(slotFrom == -1)
		return;

	if(plr->ChangeBackpack(slotFrom))
	{
		{
			Scaleform::GFx::Value var[10];
			char tmpGamertag[128];
			if(plr->ClanID != 0)
				sprintf(tmpGamertag, "[%s] %s", plr->ClanTag, slot.Gamertag);
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
			gfxMovie.Invoke("_root.api.updateClientSurvivor", var, 10);

			if (gClientLogic().localPlayer_)
				SoundSys.PlayAndForget(SoundSys.GetEventIDByPath("Sounds/UI Events/User_Action/User_Action_Swap_Backpack"), gClientLogic().localPlayer_->GetPosition());

		}
		reloadBackpackInfo();
		updateSurvivorTotalWeight();
		gfxMovie.Invoke("_root.api.changeBackpackSuccess", "");
	}
	else if (gClientLogic().localPlayer_->CurLoadout.BackpackID == WeaponConfig::ITEMID_NoDrop_Backpack)//AlexRedd:: no drop backpack
	{
		UnlockUI();
		Scaleform::GFx::Value var[3];
		var[0].SetString(gLangMngr.getString("FailedToChangeBackpack2"));
		var[1].SetBoolean(true);
		var[2].SetString("$ERROR");
		gfxMovie.Invoke("_root.api.showInfoMsg", var, 3);
	}
	else
	{
		UnlockUI();
		Scaleform::GFx::Value var[3];
		var[0].SetString(gLangMngr.getString("FailedToChangeBackpack"));
		var[1].SetBoolean(true);
		var[2].SetString("$ERROR");
		gfxMovie.Invoke("_root.api.showInfoMsg", var, 3);
	}
}

void HUDPause::eventShowContextMenuCallback(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount == 2);

	uint32_t itemID = args[0].GetUInt();
	int slotID = args[1].GetInt();

	const BaseItemConfig* itm = g_pWeaponArmory->getConfig(itemID);
	r3d_assert(itm);
	if(!itm)
		return;

	obj_Player* plr = gClientLogic().localPlayer_;
	wiInventoryItem& wi = plr->CurLoadout.Items[slotID];
	r3d_assert(plr);

	Scaleform::GFx::Value var[2];
	if(itm->category == storecat_Food)
	{
		var[0].SetString("$FR_PAUSE_EAT");
		var[1].SetInt(HPA_USE_ITEM);
		gfxMovie.Invoke("_root.api.Main.Inventory.addContextMenuOption", var, 2);
	}
	else if(itm->category == storecat_Water)
	{
		var[0].SetString("$FR_PAUSE_DRINK");
		var[1].SetInt(HPA_USE_ITEM);
		gfxMovie.Invoke("_root.api.Main.Inventory.addContextMenuOption", var, 2);
	}
	else if(itm->category == storecat_Armor || itm->category == storecat_Helmet || (itm->category>=storecat_ASR && itm->category<=storecat_GRENADE)||itm->category==storecat_MELEE)
	{
		if(itm->category==storecat_GRENADE && slotID>=2 && slotID<=5) { /*do nothing*/ }
		else if(slotID!=0 && slotID!=1 && slotID!=6 && slotID!=7)
		{
			var[0].SetString("$FR_PAUSE_EQUIP");
			var[1].SetInt(HPA_USE_ITEM);
			gfxMovie.Invoke("_root.api.Main.Inventory.addContextMenuOption", var, 2);
		}
	}
	else if(itm->category == storecat_FPSAttachment)
	{
		const WeaponAttachmentConfig* wac = g_pWeaponArmory->getAttachmentConfig(itm->m_itemID);
		// see if we can attach this to any of our 2 weapons
		for(int i=wiCharDataFull::CHAR_LOADOUT_WEAPON1; i<=wiCharDataFull::CHAR_LOADOUT_WEAPON2; i++)
		{
			Weapon* wpn = plr->m_Weapons[i];
			if(wpn && wac)
			{
				const WeaponConfig* wc = wpn->getConfig();
				r3d_assert(wc);

				if(wc->isAttachmentValid(wac))
				{
					var[0].SetString("$FR_PAUSE_ATTACH");
					var[1].SetInt(HPA_USE_ITEM);
					gfxMovie.Invoke("_root.api.Main.Inventory.addContextMenuOption", var, 2);
					wpnAttachIdx = i;
					break;
				}
			}
		}
	}
	else if (itm->category == storecat_Backpack)
	{		
		if (!gClientLogic().m_gameInfo.IsDevEvent() //AlexxRedd:: do not allowed for dev event servers
#ifdef ENABLE_BATTLE_ROYALE
			//AlexRedd:: BR mode
			&& !gClientLogic().m_gameInfo.IsGameBR()
#endif //ENABLE_BATTLE_ROYALE
			 &&	itm->m_itemID != WeaponConfig::ITEMID_NoDrop_Backpack)//AlexRedd:: no drop backpack)
		{			
			var[0].SetString("$FR_PAUSE_CHANGEBP");
			var[1].SetInt(HPA_USE_ITEM);
			gfxMovie.Invoke("_root.api.Main.Inventory.addContextMenuOption", var, 2);			
		}
#ifdef ENABLE_BATTLE_ROYALE
		//AlexRedd:: BR mode
		else if(gClientLogic().m_gameInfo.IsGameBR() && gClientLogic().m_gameHasStarted)
		{
			var[0].SetString("$FR_PAUSE_CHANGEBP");
			var[1].SetInt(HPA_USE_ITEM);
			gfxMovie.Invoke("_root.api.Main.Inventory.addContextMenuOption", var, 2);
		}
#endif //ENABLE_BATTLE_ROYALE
	}
	else if(itm->category == storecat_CraftRecipe)
	{
		if (!gClientLogic().m_gameInfo.IsDevEvent()//AlexxRedd:: not for dev event servers
#ifdef ENABLE_BATTLE_ROYALE
			&& !gClientLogic().m_gameInfo.IsGameBR()//AlexxRedd:: not for br servers
#endif //ENABLE_BATTLE_ROYALE
			)
		{
			var[0].SetString("$FR_PAUSE_LEARN_RECIPE");
			var[1].SetInt(HPA_LEARN_RECIPE);
			gfxMovie.Invoke("_root.api.Main.Inventory.addContextMenuOption", var, 2);			
		}
	}
	else if (itm->category == storecat_WeaponsSkinsRecipe)
	{
		const SkinRecipeConfig* skcfg = g_pWeaponArmory->getSkinRecipeConfig(itemID);

		if (skcfg)
		{
			ItemID = skcfg->ItemSKN.itemSknID;
			SkinID = skcfg->ItemSKN.SkinItemID;
			OriginalID = skcfg->ItemSKN.OriginalItemID;
			LearnslotID = slotID;
		}

		var[0].SetString("$FR_PAUSE_LEARN_SKIN");
		var[1].SetInt(HPA_LEARN_SKIN);
		gfxMovie.Invoke("_root.api.Main.Inventory.addContextMenuOption", var, 2);
	}
	else if (itm->category == storecat_GearSkinRecipe)
	{
		var[0].SetString("$FR_PAUSE_LEARN_RECIPE");
		var[1].SetInt(HPA_LEARN_GREARSKIN_RECIPE);
		gfxMovie.Invoke("_root.api.Main.Inventory.addContextMenuOption", var, 2);
	}
#ifdef VEHICLES_ENABLED
	else if (itm->m_itemID == WeaponConfig::ITEMID_RepairKit || itm->m_itemID == WeaponConfig::ITEMID_PremRepairKit)
	{
		if (gClientLogic().localPlayer_->IsNearVehicle())
		{
			var[0].SetString("$FR_PAUSE_VEHICLE_REPAIR");
			var[1].SetInt(HPA_USE_ITEM);
			gfxMovie.Invoke("_root.api.Main.Inventory.addContextMenuOption", var, 2);
		}
	}
	else if (itm->m_itemID == WeaponConfig::ITEMID_Gas)
	{
		if (gClientLogic().localPlayer_->IsNearVehicle())
		{
			var[0].SetString("$FR_PAUSE_VEHICLE_REFUEL");
			var[1].SetInt(HPA_USE_ITEM);
			gfxMovie.Invoke("_root.api.Main.Inventory.addContextMenuOption", var, 2);
		}
	}
#endif
	else
	{	if(itm->m_itemID!=WeaponConfig::ITEMID_Binoculars && itm->m_itemID!=WeaponConfig::ITEMID_RangeFinder
			&& itm->m_itemID!=WeaponConfig::ITEMID_RepairKit && itm->m_itemID!=WeaponConfig::ITEMID_PremRepairKit
			&& itm->m_itemID!=WeaponConfig::ITEMID_AssaultCase && itm->m_itemID!=WeaponConfig::ITEMID_SniperCase
			&& itm->m_itemID!=WeaponConfig::ITEMID_GearCase && itm->m_itemID!=WeaponConfig::ITEMID_MedicCase)
		{
			var[0].SetString("$FR_PAUSE_USE_ITEM");
			var[1].SetInt(HPA_USE_ITEM);
			gfxMovie.Invoke("_root.api.Main.Inventory.addContextMenuOption", var, 2);
		}
	}

	if(itm->category >=storecat_ASR && itm->category<=storecat_SMG)
	{
		var[0].SetString("$FR_PAUSE_UNLOAD_WEAPON");
		var[1].SetInt(HPA_UNLOAD_WEAPON_CLIP);
		gfxMovie.Invoke("_root.api.Main.Inventory.addContextMenuOption", var, 2);
	}

	if(itm->category == storecat_FPSAttachment)
	{
		const WeaponAttachmentConfig* wac = g_pWeaponArmory->getAttachmentConfig(itemID);
		r3d_assert(wac);
		if(wac && wac->m_type == WPN_ATTM_CLIP)
		{
			var[0].SetString("$FR_PAUSE_COMBINE_CLIP");
			var[1].SetInt(HPA_STACK_CLIPS);
			gfxMovie.Invoke("_root.api.Main.Inventory.addContextMenuOption", var, 2);
		}
	}

	if(!gClientLogic().m_gameInfo.IsDevEvent() && 
#ifdef ENABLE_BATTLE_ROYALE
		 !gClientLogic().m_gameInfo.IsGameBR() && 
#endif //ENABLE_BATTLE_ROYALE
			(itm->m_itemID==WeaponConfig::ITEMID_AssaultCase || itm->m_itemID==WeaponConfig::ITEMID_SniperCase
		|| itm->m_itemID==WeaponConfig::ITEMID_GearCase || itm->m_itemID==WeaponConfig::ITEMID_MedicCase))
	{
		var[0].SetString("$FR_OPEN_CASE");
		var[1].SetInt(HPA_OPEN_CASE);
		gfxMovie.Invoke("_root.api.Main.Inventory.addContextMenuOption", var, 2);
	}

	// check if player has repair kits, if yes, show repair context menu
	bool hasRepairKits = false;
	for(int i=0; i<plr->CurLoadout.BackpackSize; ++i)
	{
		if(plr->CurLoadout.Items[i].itemID == WeaponConfig::ITEMID_RepairKit || plr->CurLoadout.Items[i].itemID == WeaponConfig::ITEMID_PremRepairKit)
		{
			hasRepairKits = true;
			break;
		}
	}
	if(hasRepairKits && ((g_pWeaponArmory->getWeaponConfig(itemID) && g_pWeaponArmory->getWeaponConfig(itemID)->m_RepairAmount>0) || (g_pWeaponArmory->getGearConfig(itemID) && g_pWeaponArmory->getGearConfig(itemID)->m_RepairAmount>0) || (g_pWeaponArmory->getAttachmentConfig(itemID) && g_pWeaponArmory->getAttachmentConfig(itemID)->m_RepairAmount>0)))
	{
		int itemCondition = plr->CurLoadout.Items[slotID].Var3/100;
		if(itemCondition < 100 && plr->CurLoadout.Items[slotID].Var3 >= 0)
		{
			var[0].SetString("$FR_PAUSE_REPAIR");
			var[1].SetInt(HPA_REPAIR_ITEM);
			gfxMovie.Invoke("_root.api.Main.Inventory.addContextMenuOption", var, 2);
		}
	}

	if( !plr->IsSwimming())
	{
		bool vipItem = plr->GetVipItem(slotID);
		if(!vipItem)
		{
			var[0].SetString("$FR_PAUSE_INVENTORY_DROP_ITEM");
			var[1].SetInt(HPA_DROP_ITEM);
			gfxMovie.Invoke("_root.api.Main.Inventory.addContextMenuOption", var, 2);
		}
	}

	//AlexRedd:: drop all
	if (!plr->IsSwimming() && wi.quantity > 1)//AlexRedd:: no drop backpack
	{
		bool vipItem = plr->GetVipItem(slotID);
		if(!vipItem)
		{
			var[0].SetString("$FR_PAUSE_INVENTORY_DROP_ALL_ITEM");
			var[1].SetInt(HPA_DROP_ALL_ITEM);
			gfxMovie.Invoke("_root.api.Main.Inventory.addContextMenuOption", var, 2);
		}
	}	

//#ifndef FINAL_BUILD
	if( !plr->IsSwimming() && plr->CurLoadout.Skills[CUserSkills::SKILL_Repair3] &&//AlexRedd:: Destroy item with skill
#ifdef ENABLE_BATTLE_ROYALE
		//AlexRedd:: BR mode
		!gClientLogic().m_gameInfo.IsGameBR() &&
#endif //ENABLE_BATTLE_ROYALE
		!gClientLogic().m_gameInfo.IsDevEvent())
	{
		const BaseItemConfig* cfg = g_pWeaponArmory->getConfig(itemID);
		if(cfg->m_ResWood > 0 || cfg->m_ResStone > 0 || cfg->m_ResMetal > 0)
		{
			//var[0].SetString("$FR_PAUSE_DISASSEMBLE_ITEM");
			var[0].SetString("$FR_PAUSE_INVENTORY_DESTROY_ITEM");
			var[1].SetInt(HPA_DISASSEMBLE_ITEM);
			gfxMovie.Invoke("_root.api.Main.Inventory.addContextMenuOption", var, 2);
		}
	}
//#endif
}

void HUDPause::eventContextMenu_Action(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount==2);
	int slotID = args[0].GetInt();
	int actionID = args[1].GetInt();

#ifdef ENABLE_BATTLE_ROYALE
	//if (gClientLogic().m_gameInfo.IsGameBR() && gUserProfile.ProfileData.isDevAccount==0)//AlexRedd:: BR mode
		//return;
#endif //ENABLE_BATTLE_ROYALE


	if(r3dGetTime() < AllowItemDropAfterThisTime)
		return; // do not allow to interact with items first X seconds after entering game world (to prevent dropping items and dupes)

	obj_Player* plr = gClientLogic().localPlayer_;
	r3d_assert(plr);	

	if(actionID == HPA_DROP_ITEM)
	{
		bool vipItem = plr->GetVipItem(slotID);
		if(vipItem)
		{
			Scaleform::GFx::Value var[3];
			var[0].SetString(gLangMngr.getString("InGameUI_ErrorMsg_Forbidden"));
			var[1].SetBoolean(true);
			var[2].SetString("");
			gfxMovie.Invoke("_root.api.showInfoMsg", var, 3);
			return;
		}
		if (plr->IsInVehicle())//AlexRedd:: not allowed in vehicle
		{
			Scaleform::GFx::Value var[3];
			var[0].SetString(gLangMngr.getString("InGameUI_ErrorMsg_NotAllowed"));
			var[1].SetBoolean(true);
			var[2].SetString("");
			gfxMovie.Invoke("_root.api.showInfoMsg", var, 3);
			return;
		}
		plr->DropItem(slotID);
		updateSurvivorTotalWeight();
		gfxMovie.Invoke("_root.api.backpackGridSwapSuccess", "");
	}
	else if (actionID == HPA_DROP_ALL_ITEM)//AlexRedd:: drop all
	{
		bool vipItem = plr->GetVipItem(slotID);
		if(vipItem)
		{
			Scaleform::GFx::Value var[3];
			var[0].SetString(gLangMngr.getString("InGameUI_ErrorMsg_Forbidden"));
			var[1].SetBoolean(true);
			var[2].SetString("");
			gfxMovie.Invoke("_root.api.showInfoMsg", var, 3);
			return;
		}
		if (plr->IsInVehicle())//AlexRedd:: not allowed in vehicle
		{
			Scaleform::GFx::Value var[3];
			var[0].SetString(gLangMngr.getString("InGameUI_ErrorMsg_NotAllowed"));
			var[1].SetBoolean(true);
			var[2].SetString("");
			gfxMovie.Invoke("_root.api.showInfoMsg", var, 3);
			return;
		}
		plr->DropAllItem(slotID);
		updateSurvivorTotalWeight();
		gfxMovie.Invoke("_root.api.backpackGridSwapSuccess", "");
	}	
	else if(actionID == HPA_UNLOAD_WEAPON_CLIP)
	{
		plr->UnloadWeaponClip(slotID);
		updateSurvivorTotalWeight();
		// no need to call it, actual item isn't changed. plr->OnBackpackChanged(slotID);
		gfxMovie.Invoke("_root.api.backpackGridSwapSuccess", "");
	}
	else if(actionID == HPA_OPEN_CASE)
	{
		wiInventoryItem& wi = plr->CurLoadout.Items[slotID];
		plr->OpenCase(slotID);
		updateSurvivorTotalWeight();
		gfxMovie.Invoke("_root.api.backpackGridSwapSuccess", "");		
	}
	else if(actionID == HPA_REPAIR_ITEM)
	{
		uint32_t numRegularRepairKits = 0;
		uint32_t numPremiumRepairKits = 0;
		for(int i=0; i<plr->CurLoadout.BackpackSize; ++i)
		{
			if(plr->CurLoadout.Items[i].itemID == WeaponConfig::ITEMID_RepairKit)
				numRegularRepairKits+=plr->CurLoadout.Items[i].quantity;
			else if(plr->CurLoadout.Items[i].itemID == WeaponConfig::ITEMID_PremRepairKit)
				numPremiumRepairKits+=plr->CurLoadout.Items[i].quantity;
		}

		float repairAmount = 0;
		float premRepairAmount = 0;
		{
			const GearConfig* gc = g_pWeaponArmory->getGearConfig(plr->CurLoadout.Items[slotID].itemID);
			if(gc)
			{
				repairAmount = gc->m_RepairAmount;
				premRepairAmount = gc->m_PremRepairAmount;
			}
			const WeaponConfig* wc = g_pWeaponArmory->getWeaponConfig(plr->CurLoadout.Items[slotID].itemID);
			if(wc)
			{
				repairAmount = wc->m_RepairAmount;
				premRepairAmount = wc->m_PremRepairAmount;
			}
			const WeaponAttachmentConfig* wac = g_pWeaponArmory->getAttachmentConfig(plr->CurLoadout.Items[slotID].itemID);
			if(wac)
			{
				repairAmount = wac->m_RepairAmount;
				premRepairAmount = wac->m_PremRepairAmount;
			}
		}

		//public function showRepairMenu(slotID:uint, numRepairKits:uint, numPremium:uint, itemName:String, curDur:uint, newDur:uint, newPDur:uint)
		Scaleform::GFx::Value var[7];
		var[0].SetUInt(slotID);
		var[1].SetUInt(numRegularRepairKits);
		var[2].SetUInt(numPremiumRepairKits);
		var[3].SetString(g_pWeaponArmory->getConfig(plr->CurLoadout.Items[slotID].itemID)->m_StoreName);
		var[4].SetUInt(plr->CurLoadout.Items[slotID].Var3/100);
		var[5].SetUInt(R3D_MIN(plr->CurLoadout.Items[slotID].Var3/100 + int(repairAmount), 100));
		var[6].SetUInt(R3D_MIN(plr->CurLoadout.Items[slotID].Var3/100 + int(premRepairAmount), 100));
		gfxMovie.Invoke("_root.api.Main.Inventory.showRepairMenu", var, 7);
	}
	else if(actionID == HPA_STACK_CLIPS)
	{
		PKT_C2S_PlayerCombineClip_s n3;
		n3.SlotFrom = slotID;
		p2pSendToHost(gClientLogic().localPlayer_, &n3, sizeof(n3));

		if (gClientLogic().localPlayer_)
			SoundSys.PlayAndForget(SoundSys.GetEventIDByPath("Sounds/UI Events/User_Action/User_Action_Stack_Clips"),gClientLogic().localPlayer_->GetPosition());

		LockUI();
		return;
	}
	else if(actionID == HPA_LEARN_RECIPE)
	{
		wiInventoryItem& wi = plr->CurLoadout.Items[slotID];
		r3d_assert(wi.itemID && wi.quantity > 0);

		if(plr->CurLoadout.hasRecipe(wi.itemID))
		{
			Scaleform::GFx::Value var[2];
			var[0].SetString(gLangMngr.getString("AlreadyLearnedThatRecipe"));
			var[1].SetBoolean(true);
			gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);
		}
		else
		{
			PKT_C2S_LearnRecipe_s n2;
			n2.slotFrom = slotID;
			p2pSendToHost(gClientLogic().localPlayer_, &n2, sizeof(n2));
			
			plr->CurLoadout.RecipesLearned[plr->CurLoadout.NumRecipes++]=wi.itemID; // it should not fail on server

			if (gClientLogic().localPlayer_)
				SoundSys.PlayAndForget(SoundSys.GetEventIDByPath("Sounds/UI Events/User_Action/User_Action_Learn_Recipe"),gClientLogic().localPlayer_->GetPosition());

			LockUI();
		}
		return;
	}
	else if (actionID == HPA_LEARN_SKIN)
	{
		if (!plr->CheckSkinBought(ItemID, SkinID))
		{

			PKT_C2S_LearnSkinItem_s n3;
			n3.ItemID = ItemID;
			n3.SkinID = SkinID;
			n3.OriginalID = OriginalID;
			n3.SlotID = LearnslotID;
			p2pSendToHost(gClientLogic().localPlayer_, &n3, sizeof(n3));

			ItemID = 0;
			SkinID = 0;
			OriginalID = 0;
		}
		else {
			Scaleform::GFx::Value var[2];
			var[0].SetString(gLangMngr.getString("AlreadyLearnedThatRecipe"));
			var[1].SetBoolean(true);
			gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);
		}
		return;
	}
	else if (actionID == HPA_LEARN_GREARSKIN_RECIPE)
	{
		wiInventoryItem& wi = plr->CurLoadout.Items[slotID];
		r3d_assert(wi.itemID && wi.quantity > 0);

		if (plr->CurLoadout.hasGearSkinRecipe(wi.itemID))
		{
			Scaleform::GFx::Value var[2];
			var[0].SetString(gLangMngr.getString("AlreadyLearnedThatRecipe"));
			var[1].SetBoolean(true);
			gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);
		}
		else
		{
			// EDITAR AKI
			PKT_C2S_LearnGearSkinRecipe_s n2;
			n2.slotFrom = slotID;
			p2pSendToHost(gClientLogic().localPlayer_, &n2, sizeof(n2));

			plr->CurLoadout.GearSkinRecipesLearned[plr->CurLoadout.NumGearSkinRecipes++] = wi.itemID; // it should not fail on server

			LockUI();
		}
		return;
	}
	else if(actionID == HPA_USE_ITEM)
	{
		wiInventoryItem& wi = plr->CurLoadout.Items[slotID];
		r3d_assert(wi.itemID && wi.quantity > 0);

		{
			const WeaponConfig* wc = g_pWeaponArmory->getWeaponConfig(wi.itemID);
			if (wc)
			{
				if (wc->category != storecat_UsableItem) // if consumable, it will send use packet
				{
					Scaleform::GFx::Value newargs[2];
					newargs[0].SetInt(slotID);
					int slotTo = 0;
					if (wc->category == storecat_MELEE || wc->category == storecat_HG)
						slotTo = 1;
					else if (wc->category == storecat_GRENADE)
					{
						// find empty quick slot
						if (plr->CurLoadout.Items[2].itemID == 0)
							slotTo = 2;
						else if (plr->CurLoadout.Items[3].itemID == 0)
							slotTo = 3;
						else if (plr->CurLoadout.Items[4].itemID == 0)
							slotTo = 4;
						else if (plr->CurLoadout.Items[5].itemID == 0)
							slotTo = 5;
						else
							slotTo = 5;
					}
					newargs[1].SetInt(slotTo);
					eventBackpackGridSwap(pMovie, newargs, 2);
					return;
				}
			}
			const GearConfig* gc = g_pWeaponArmory->getGearConfig(wi.itemID);
			if (gc)
			{
				Scaleform::GFx::Value newargs[2];
				newargs[0].SetInt(slotID);
				int slotTo = 6;
				if (gc->category == storecat_Helmet)
					slotTo = 7;
				newargs[1].SetInt(slotTo);
				eventBackpackGridSwap(pMovie, newargs, 2);
				return;
			}
			const BackpackConfig* bc = g_pWeaponArmory->getBackpackConfig(wi.itemID);
			if (bc)
			{
				//AlexRedd:: Simple logic for change backpack without window
				//For prevent any kind of bugs/dupe, lock the HUD
				LockUI();
				ChangePlayerBackpackFast(wi.itemID);
				//gfxMovie.Invoke("_root.api.Main.showChangeBackpack", "");
				return;
			}
			const WeaponAttachmentConfig* wac = g_pWeaponArmory->getAttachmentConfig(wi.itemID);
			if (wac)
			{
				plr->ChangeWeaponByIndex(wpnAttachIdx);
				plr->EquipWpnAttm(wpnAttachIdx, slotID);

				reloadBackpackInfo();
				return;
			}
			const CraftComponentConfig* comp = g_pWeaponArmory->getCraftComponentConfig(wi.itemID);
			if (comp
#ifdef VEHICLES_ENABLED	
				&& wi.itemID != WeaponConfig::ITEMID_Gas
#endif
				)
			{
				Deactivate();
				hudCraft->Activate();
				return;
			}
		}

#ifdef VEHICLES_ENABLED
		if (wi.itemID == WeaponConfig::ITEMID_RepairKit || wi.itemID == WeaponConfig::ITEMID_PremRepairKit)
		{
			obj_Vehicle* vehicle = gClientLogic().localPlayer_->IsNearVehicle();
			if (!vehicle)
				return;

			PKT_C2S_VehicleRepair_s n;
			n.slotId = slotID;
			n.vehicleId = toP2pNetId(vehicle->GetNetworkID());
			p2pSendToHost(plr, &n, sizeof(n));

			if (gClientLogic().localPlayer_)
				SoundSys.PlayAndForget(SoundSys.GetEventIDByPath("Sounds/UI Events/User_Action/User_Action_Repair_Car"),gClientLogic().localPlayer_->GetPosition());

			LockUI();
			return;
		}
		else if (wi.itemID == WeaponConfig::ITEMID_Gas)
		{
			obj_Vehicle* vehicle = gClientLogic().localPlayer_->IsNearVehicle();
			if (!vehicle)
				return;

			PKT_C2S_VehicleRefuel_s n;
			n.slotId = slotID;
			n.vehicleId = toP2pNetId(vehicle->GetNetworkID());
			p2pSendToHost(plr, &n, sizeof(n));

			if (gClientLogic().localPlayer_)
				SoundSys.PlayAndForget(SoundSys.GetEventIDByPath("Sounds/UI Events/User_Action/User_Action_Refuel_Car"),gClientLogic().localPlayer_->GetPosition());

			LockUI();
			return;
		}
		else if (wi.itemID == WeaponConfig::ITEMID_EmptyJerryCan)
		{
			// todo : add a method to reload jerry can from a source
		}
		else
#endif
		if(wi.itemID == WeaponConfig::ITEMID_PieceOfPaper) // world note
		{
			if(hudMain->canShowWriteNote())
			{
				Deactivate();
				hudMain->showWriteNote(slotID);
			}
			return;
		}
		else if(wi.itemID == WeaponConfig::ITEMID_Binoculars || wi.itemID == WeaponConfig::ITEMID_RangeFinder)
		{
			return;
		}
		else if(wi.itemID == WeaponConfig::ITEMID_AirHorn)
		{
			// todo
			return;
		}
		else if (wi.itemID == WeaponConfig::ITEMID_AerialDelivery)
		{
			// use raycast to fix bug when player able to walk on water by jumping
			bool bWasSwimming = plr->IsSwimming();
			{							
				float waterDepth;
				float waterLevel;
				if(plr->IsInWater( waterDepth, waterLevel ))
				{
					Scaleform::GFx::Value var[3];
					var[0].SetString(gLangMngr.getString("InfoMsg_NotAllowed"));
					var[1].SetBoolean(true);
					var[2].SetString("");
					gfxMovie.Invoke("_root.api.showInfoMsg", var, 3);				
					SoundSys.PlayAndForget(SoundSys.GetEventIDByPath("Sounds/UI Events/Misc/UI_Error_MsgSound"), gClientLogic().localPlayer_->GetPosition());
					return;
				}
			}
			if (plr->CurLoadout.GameFlags & wiCharDataFull::GAMEFLAG_NearPostBox)
			{
				Scaleform::GFx::Value var[3];
				var[0].SetString(gLangMngr.getString("InfoMsg_NotAllowed"));
				var[1].SetBoolean(true);
				var[2].SetString("");
				gfxMovie.Invoke("_root.api.showInfoMsg", var, 3);				
				SoundSys.PlayAndForget(SoundSys.GetEventIDByPath("Sounds/UI Events/Misc/UI_Error_MsgSound"), gClientLogic().localPlayer_->GetPosition());
				return;
			}			
			else if ((r3dGetTime() - plr->lastTimeUsedConsumable) < plr->currentConsumableCooldownTime || (r3dGetTime() - plr->lastTimeUsedAirdrop) < plr->currentAirdropCooldownTime)
			{
				Scaleform::GFx::Value var[3];
				var[0].SetString(gLangMngr.getString("InfoMsg_CooldownActive"));
				var[1].SetBoolean(true);
				var[2].SetString("");
				gfxMovie.Invoke("_root.api.showInfoMsg", var, 3);
				return;
			}
			else if (gClientLogic().localPlayer_->m_NumbeAirDrop <= 0)
			{
				Scaleform::GFx::Value var[3];
				var[0].SetString(gLangMngr.getString("InfoMsg_NotAirDropOnMap"));
				var[1].SetBoolean(true);
				var[2].SetString("");
				gfxMovie.Invoke("_root.api.showInfoMsg", var, 3);
				return;
			}
			else
			{
				plr->localPlayer_UseItem(slotID, wi.itemID, plr->GetPosition());
				hudMain->showMessage2(gLangMngr.getString("InfoMsg_AirdropOnMap"));
			}
		}
		else if(wi.itemID == WeaponConfig::ITEMID_BarbWireBarricade ||
			wi.itemID == WeaponConfig::ITEMID_WoodShieldBarricade ||
			wi.itemID == WeaponConfig::ITEMID_WoodShieldBarricadeZB ||
			wi.itemID == WeaponConfig::ITEMID_RiotShieldBarricade ||
			wi.itemID == WeaponConfig::ITEMID_RiotShieldBarricadeZB ||
			wi.itemID == WeaponConfig::ITEMID_SandbagBarricade ||
			wi.itemID == WeaponConfig::ITEMID_Lockbox ||
			wi.itemID == WeaponConfig::ITEMID_LockboxBig ||
			wi.itemID == WeaponConfig::ITEMID_LockboxSmall ||
			wi.itemID == WeaponConfig::ITEMID_WoodenDoorBlock ||
			wi.itemID == WeaponConfig::ITEMID_MetalWallBlock ||
			wi.itemID == WeaponConfig::ITEMID_TallBrickWallBlock ||
			wi.itemID == WeaponConfig::ITEMID_WoodenWallPiece ||
			wi.itemID == WeaponConfig::ITEMID_ShortBrickWallPiece ||
			wi.itemID == WeaponConfig::ITEMID_FarmBlock ||
			wi.itemID == WeaponConfig::ITEMID_SolarWaterPurifier ||
			wi.itemID == WeaponConfig::ITEMID_PlaceableLight ||
			wi.itemID == WeaponConfig::ITEMID_SmallPowerGenerator ||
			wi.itemID == WeaponConfig::ITEMID_BigPowerGenerator || 
			//AlexRedd:: car spawner
			wi.itemID == WeaponConfig::ITEMID_VehicleSpawner_Buggy ||
			wi.itemID == WeaponConfig::ITEMID_VehicleSpawner_Stryker ||
			wi.itemID == WeaponConfig::ITEMID_VehicleSpawner_ZombieKiller ||
			wi.itemID == WeaponConfig::ITEMID_VehicleSpawner_ScoutMilitary ||
			wi.itemID == WeaponConfig::ITEMID_VehicleSpawner_BuggyGun ||
			wi.itemID == WeaponConfig::ITEMID_VehicleSpawner_ScoutSurvivor ||
			wi.itemID == WeaponConfig::ITEMID_VehicleSpawner_BoneCrusher ||
			wi.itemID == WeaponConfig::ITEMID_VehicleSpawner_ScavengersScout ||
			wi.itemID == WeaponConfig::ITEMID_VehicleSpawner_Cubus ||
			wi.itemID == WeaponConfig::ITEMID_VehicleSpawner_Hippy ||
			wi.itemID == WeaponConfig::ITEMID_VehicleSpawner_Convertable ||
			//AlexRedd:: new barricade
			wi.itemID == WeaponConfig::ITEMID_SSBarricadeShield)
		{
			Scaleform::GFx::Value var[3];
			var[0].SetString(gLangMngr.getString("HUD_BarricadeUse_PlaceInQuickSlot"));
			var[1].SetBoolean(true);
			var[2].SetString("");
			gfxMovie.Invoke("_root.api.showInfoMsg", var, 3);
			return;
		}
		else
		{
			if((wi.itemID == WeaponConfig::ITEMID_Bandages || wi.itemID == WeaponConfig::ITEMID_Bandages2 || wi.itemID == WeaponConfig::ITEMID_Antibiotics ||
				wi.itemID == WeaponConfig::ITEMID_Painkillers || wi.itemID == WeaponConfig::ITEMID_Medkit || wi.itemID == WeaponConfig::ITEMID_CraftedBandages) 
				)
			{
				if((r3dGetTime() - plr->lastTimeUsedConsumable) < plr->currentConsumableCooldownTime)
				{
					Scaleform::GFx::Value var[3];
					var[0].SetString(gLangMngr.getString("InfoMsg_CooldownActive"));
					var[1].SetBoolean(true);
					var[2].SetString("");
					gfxMovie.Invoke("_root.api.showInfoMsg", var, 3);
					return;
				}
				if(plr->CurLoadout.Health > 99)
				{
					Scaleform::GFx::Value var[3];
					var[0].SetString(gLangMngr.getString("InfoMsg_MaxHealthAlready"));
					var[1].SetBoolean(true);
					var[2].SetString("");
					gfxMovie.Invoke("_root.api.showInfoMsg", var, 3);
					return;
				}
			}
			else if(wi.itemID == WeaponConfig::ITEMID_AerialDelivery)
			{
				if((r3dGetTime() - plr->lastTimeUsedAirdrop) < plr->currentAirdropCooldownTime)
				{
					Scaleform::GFx::Value var[3];
					var[0].SetString(gLangMngr.getString("InfoMsg_CooldownActive"));
					var[1].SetBoolean(true);
					var[2].SetString("");
					gfxMovie.Invoke("_root.api.showInfoMsg", var, 3);
					return;
				}				
			}
			if((wi.itemID == WeaponConfig::ITEMID_C01Vaccine || wi.itemID == WeaponConfig::ITEMID_C04Vaccine) 
				&& plr->CurLoadout.Toxic < 1.0f)
			{
				Scaleform::GFx::Value var[3];
				var[0].SetString(gLangMngr.getString("InfoMsg_NoTocixInBlood"));
				var[1].SetBoolean(true);
				var[2].SetString("");
				gfxMovie.Invoke("_root.api.showInfoMsg", var, 3);
				return;
			}
			if(g_pWeaponArmory->getFoodConfig(wi.itemID))
			{
				if((r3dGetTime() - plr->lastTimeUsedConsumable) < plr->currentConsumableCooldownTime)
				{
					Scaleform::GFx::Value var[3];
					var[0].SetString(gLangMngr.getString("InfoMsg_CooldownActive"));
					var[1].SetBoolean(true);
					var[2].SetString("");
					gfxMovie.Invoke("_root.api.showInfoMsg", var, 3);
					return;
				}
			}

			plr->localPlayer_UseItem(slotID, wi.itemID, plr->GetPosition());
		}

		//local logic
		wi.quantity--;
		if(wi.quantity <= 0) {
			wi.Reset();
		}

		plr->OnBackpackChanged(slotID);
		updateSurvivorTotalWeight();
		gfxMovie.Invoke("_root.api.backpackGridSwapSuccess", "");
	}
	else if(actionID == HPA_DISASSEMBLE_ITEM)
	{
		wiInventoryItem& wi = plr->CurLoadout.Items[slotID];
		r3d_assert(wi.itemID && wi.quantity > 0);

		PKT_C2S_BackpackDisassembleItem_s n2;
		n2.SlotFrom = slotID;
		p2pSendToHost(gClientLogic().localPlayer_, &n2, sizeof(n2));

		if (gClientLogic().localPlayer_)
			SoundSys.PlayAndForget(SoundSys.GetEventIDByPath("Sounds/UI Events/User_Action/User_Action_Injection"), gClientLogic().localPlayer_->GetPosition());	
			
		LockUI();
		return;
	}
}

void HUDPause::LockUI()
{
	r3d_assert(!isLockedUI);

	isLockedUI = true;
	Scaleform::GFx::Value var[2];
	var[0].SetString("");
	var[1].SetBoolean(false);
	gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);
}

void HUDPause::UnlockUI()
{
	if(isLockedUI)
	{
		isLockedUI = false;
		gfxMovie.Invoke("_root.api.hideInfoMsg", "");
	}
}

#ifdef MISSIONS
HUDPause::HUDMissionButton& HUDPause::HUDMissionButton::operator=( const HUDMissionButton& button )
{
	m_missionID		= button.m_missionID;
	m_accepted		= button.m_accepted;
	r3dscpy( m_missionNameStringID, button.m_missionNameStringID );
	r3dscpy( m_missionDescStringID, button.m_missionDescStringID );
	r3dscpy( m_missionIconPath, button.m_missionIconPath );

	return *this;
}

int HUDPause::FindMissionButtonIndex( uint32_t missionID )
{
	for(uint32_t i = 0; i < missionButtons.size(); ++i)
	{
		if( missionButtons[ i ].m_missionID == missionID )
			return i;
	}
	return -1;
}

int HUDPause::FindFirstAvailableMissionButtonIndex()
{
	return (int)missionButtons.size();
}

bool HUDPause::CopyMissionButton( uint32_t index, HUDMissionButton& button )
{
	if( index >= missionButtons.size() )
		return false;

	button.m_missionID		= missionButtons[ index ].m_missionID;
	button.m_accepted		= missionButtons[ index ].m_accepted;
	button.m_declined		= missionButtons[ index ].m_declined;
	r3dscpy( button.m_missionNameStringID, missionButtons[ index ].m_missionNameStringID );
	r3dscpy( button.m_missionDescStringID, missionButtons[ index ].m_missionDescStringID );
	r3dscpy( button.m_missionIconPath, missionButtons[ index ].m_missionIconPath );

	return true;
}

bool HUDPause::SetMissionButton( uint32_t index, bool active, const HUDMissionButton& button )
{
	if( active )
	{
		if( index >= missionButtons.size() )
			missionButtons.push_back( button );
		else
			missionButtons[ index ] = button;
	}
	else
	{
		if( index >= missionButtons.size() )
			return false;

		missionButtons.erase( missionButtons.begin() + index );
	}
	return true;
}

bool HUDPause::SetMissionIcon( uint32_t actionID, const HUDMissionIcon& icon )
{
	missionIcons[ actionID ] = icon;
	//r3dOutToLog("SetMissionIcon - Active:%s, Icon:%s, Location<%.2f, %.2f, %.2f | actionID %d>\n", (icon.m_active) ? "true" : "false", ActionIconNames[ icon.m_mapIcon - 1 ], icon.m_location.x, icon.m_location.y, icon.m_location.z, actionID);
	return true;
}
#endif

bool HUDPause::SetAirDropIcon(uint32_t AirDroID,  const HUDAirDropIcon& icon )
{	
	AirDropsIcons[ AirDroID ] = icon;	
	return true;
}

bool HUDPause::SetCarIcon(uint32_t carID, const HUDCarIcon& icon)
{
	CarIcons[carID] = icon;
	return true;
}

//AlexRedd:: rare box
bool HUDPause::SetLootBoxIcon( uint32_t boxID, const HUDLootBoxIcon& icon )
{
	LootBoxIcons[ boxID ] = icon;
	return true;
}

//AlexRedd:: BR mode
bool HUDPause::SetGasIcon( const HUDGasIcon& Gas )
{
	GasIcon = Gas;	
	return true;
}

void HUDPause::showMessage(const char* msg)
{
	Scaleform::GFx::Value var[3];
	var[0].SetString(msg);
	var[1].SetBoolean(true);
	var[2].SetString("");
	gfxMovie.Invoke("_root.api.showInfoMsg", var, 3);
}

void HUDPause::showMsgCase(DWORD caseItemID, DWORD winItemID, int Quantity)
{	
	char msgText[512] = {0};	

	const BaseItemConfig* caseItemConfig = g_pWeaponArmory->getConfig(caseItemID);	
	const BaseItemConfig* winItemConfig = g_pWeaponArmory->getConfig(winItemID);	
	
	if (caseItemConfig && winItemConfig)
	{		
		sprintf(msgText, gLangMngr.getString("YouOpenedCase"), caseItemConfig->m_StoreName);

		Scaleform::GFx::Value var[8];
		var[0].SetString(caseItemConfig->m_StoreIcon);
		var[1].SetString(winItemConfig->m_StoreIcon);
		var[2].SetString(msgText);
		var[3].SetBoolean(true);
		var[4].SetString(caseItemConfig->m_StoreName);
		var[5].SetInt(Quantity);
		var[6].SetString(winItemConfig->m_StoreName);
		var[7].SetString(winItemConfig->m_Description);
		gfxMovie.Invoke("_root.api.showInfoMsgCase", var, 8);		

		if (gClientLogic().localPlayer_)
			SoundSys.PlayAndForget(SoundSys.GetEventIDByPath("Sounds/UI Events/Misc/UI_Item_Move_Equip"), gClientLogic().localPlayer_->GetPosition());
	}
	else{
		Scaleform::GFx::Value var[8];
		if(caseItemConfig)
			var[0].SetString(caseItemConfig->m_StoreIcon);
		else
			var[0].SetString("");
		var[1].SetString("$Data/Weapons/StoreIcons/Oops.dds");
		var[2].SetString("$FR_CaseIsEmpty");
		var[3].SetBoolean(true);
		if(caseItemConfig)
			var[4].SetString(caseItemConfig->m_StoreName);
		else
			var[4].SetString("");
		var[5].SetInt(0);
		var[6].SetString("");
		var[7].SetString("");
		gfxMovie.Invoke("_root.api.showInfoMsgCase", var, 8);

		if (gClientLogic().localPlayer_)
			SoundSys.PlayAndForget(SoundSys.GetEventIDByPath("Sounds/UI Events/Misc/UI_Error_MsgSound"), gClientLogic().localPlayer_->GetPosition());
	}	
}

void HUDPause::eventChangeBackpack(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount==2);
	int bpslotID = args[0].GetInt();
	uint32_t itemID = args[1].GetUInt();

	obj_Player* plr = gClientLogic().localPlayer_;
	r3d_assert(plr);
	wiCharDataFull& slot = plr->CurLoadout;

	// find inventory slot
	int slotFrom = -1;
	for(int i=0; i<slot.BackpackSize; ++i)
	{
		if(slot.Items[i].itemID == itemID)
		{
			slotFrom = i;
			break;
		}
	}
	if(slotFrom == -1)
		return;

	if(plr->ChangeBackpack(slotFrom))
	{
		{
			Scaleform::GFx::Value var[10];
			char tmpGamertag[128];
			if(plr->ClanID != 0)
				sprintf(tmpGamertag, "[%s] %s", plr->ClanTag, slot.Gamertag);
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
			gfxMovie.Invoke("_root.api.updateClientSurvivor", var, 10);

			if (gClientLogic().localPlayer_)
				SoundSys.PlayAndForget(SoundSys.GetEventIDByPath("Sounds/UI Events/User_Action/User_Action_Swap_Backpack"), gClientLogic().localPlayer_->GetPosition());

		}
		reloadBackpackInfo();
		updateSurvivorTotalWeight();
		gfxMovie.Invoke("_root.api.changeBackpackSuccess", "");
	}
	else if (gClientLogic().localPlayer_->CurLoadout.BackpackID == WeaponConfig::ITEMID_NoDrop_Backpack)//AlexRedd:: no drop backpack
	{
		UnlockUI();
		Scaleform::GFx::Value var[3];
		var[0].SetString(gLangMngr.getString("FailedToChangeBackpack2"));
		var[1].SetBoolean(true);
		var[2].SetString("$ERROR");
		gfxMovie.Invoke("_root.api.showInfoMsg", var, 3);
	}
	else
	{
		UnlockUI(); //AlexRedd:: Disable UI, some shit happens, server will not send to us to disable it because we dont have success on change backpack
		Scaleform::GFx::Value var[3];
		var[0].SetString(gLangMngr.getString("FailedToChangeBackpack"));
		var[1].SetBoolean(true);
		var[2].SetString("$ERROR");
		gfxMovie.Invoke("_root.api.showInfoMsg", var, 3);
	}
}

void HUDPause::eventMsgBoxCallback(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	// FOR DENIS: send cancel disconnect request to server
	/*
	if(DisconnectAt > 0)
	{
		//PKT_C2S_DisconnectReq_s n;
		//p2pSendToHost(gClientLogic().localPlayer_, &n, sizeof(n), true);

		DisconnectAt = -1; 
	}*/
}

void HUDPause::eventBackToGame(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	Deactivate();
}

void HUDPause::eventQuitGame(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
#ifndef FINAL_BUILD	
	d_navmesh_debug->SetBool(false);
#endif // FINAL_BUILD

	// send disconnect request to server
	PKT_C2S_DisconnectReq_s n;
	p2pSendToHost(gClientLogic().localPlayer_, &n, sizeof(n), true);

	if(!gClientLogic().localPlayer_->bDead && !gUserProfile.ProfileData.isDevAccount && !(gClientLogic().localPlayer_->CurLoadout.GameFlags & wiCharDataFull::GAMEFLAG_NearPostBox))
		DisconnectAt = r3dGetTime() + 10.0f; 
	else
		DisconnectAt = r3dGetTime();

	isDisconnecting_ = true;
}

void HUDPause::eventSendCallForHelp(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount==2);
	const char* distress = args[0].GetString();
	const char* reward = args[1].GetString();

	// local time out check to prevent spamming server
	static float nextTimeOfSend = 0.0f;
	if(r3dGetTime() < nextTimeOfSend)
	{
		Scaleform::GFx::Value var[3];
		var[0].SetString(gLangMngr.getString("HUD_CallForHelp_TooFrequent"));
		var[1].SetBoolean(true);
		var[2].SetString("$ERROR");
		gfxMovie.Invoke("_root.api.showInfoMsg", var, 3);
		return;
	}

	nextTimeOfSend = r3dGetTime() + 60.0f;

	PKT_C2S_CallForHelpReq_s n;
	r3dscpy(n.distress, distress);
	r3dscpy(n.reward, reward);
	p2pSendToHost(gClientLogic().localPlayer_, &n, sizeof(n), true);

	{
		Scaleform::GFx::Value var[3];
		var[0].SetString(gLangMngr.getString("HUD_CallForHelp_RequestSent"));
		var[1].SetBoolean(true);
		var[2].SetString("");
		gfxMovie.Invoke("_root.api.showInfoMsg", var, 3);
	}
}

r3dPoint2D getMinimapPosWithExternalSize(const r3dPoint3D& pos, const r3dPoint3D& worldOrigin, const r3dPoint3D& worldSize)
{
	float left_corner_x = worldOrigin.x;
	float bottom_corner_y = worldOrigin.z; 
	float x_size = worldSize.x;
	float y_size = worldSize.z;

	float x = R3D_CLAMP((pos.x-left_corner_x)/x_size, 0.0f, 1.0f);
	float y = 1.0f-R3D_CLAMP((pos.z-bottom_corner_y)/y_size, 0.0f, 1.0f);

	return r3dPoint2D(x, y);
}

r3dPoint2D getMinimapPos(const r3dPoint3D& pos)
{
	r3dPoint3D worldOrigin = GameWorld().m_MinimapOrigin;
	r3dPoint3D worldSize = GameWorld().m_MinimapSize;
	return getMinimapPosWithExternalSize(pos, worldOrigin, worldSize);
}

void HUDPause::eventDisableHotKeys(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount == 1);
	disableHotKeys = args[0].GetBool();
}

void HUDPause::eventShowMap(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	showMinimapFunc();
}

void HUDPause::eventRepairItem(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount==2);
	int slotID = args[0].GetInt();
	int isPremiumKit = args[1].GetInt();

	PKT_C2S_RepairItemReq_s n3;
	n3.SlotFrom   = slotID;
	n3.RepairMode = isPremiumKit ? 3 : 2;
	p2pSendToHost(gClientLogic().localPlayer_, &n3, sizeof(n3));

	if (gClientLogic().localPlayer_)
		SoundSys.PlayAndForget(SoundSys.GetEventIDByPath("Sounds/UI Events/User_Action/User_Action_RepairWeapon"), gClientLogic().localPlayer_->GetPosition());

	LockUI();
}

#ifdef MISSIONS
bool HUDPause::removeMissionButton( uint32_t missionID, uint32_t reason )
{
	int index = FindMissionButtonIndex( missionID );
	if( index == -1 )
		return false;

	if( reason != 1 /*Mission::RMV_MissionCompleted*/ ||
		( reason == 1 /*Mission::RMV_MissionCompleted*/ && !missionButtons[ index ].m_canRepeat ) )
	{
		//r3dOutToLog("!!! Mission(%d): Removing Mission Button(%d): %s!\n", missionID, index, gLangMngr.getString(missionButtons[index].m_missionNameStringID));
		missionButtons.erase( missionButtons.begin() + index );
	}
	return true;
}

bool HUDPause::setMissionButton( uint32_t missionID, bool accepted, bool declined )
{
	if( accepted && declined )
	{
		r3dOutToLog("!!! ERROR: Attempted to set the mission button to both accepted and declined!\nDefaulting to accepted.\n");
		declined = false;
	}

	int index = FindMissionButtonIndex( missionID );
	if( index == -1 )
		return false;

	missionButtons[ index ].m_accepted = accepted;
	missionButtons[ index ].m_declined = declined;

	return true;
}

void HUDPause::clearMissionButtonDeclines()
{
	for( std::vector<HUDMissionButton>::iterator iter = missionButtons.begin();
		 iter != missionButtons.end(); ++iter )
	{
		iter->m_declined = false;
	}
}

#endif

void HUDPause::showMinimapFunc()
{
	const ClientGameLogic& CGL = gClientLogic();
	obj_Player* plr = CGL.localPlayer_;
	r3d_assert(plr);
	char tmpPlrName[128];
	plr->GetUserName(tmpPlrName);
	setMinimapPosition(plr->GetPosition(), plr->GetvForw());

	Scaleform::GFx::Value var[6];
	for(int i=0; i<R3D_ARRAYSIZE(CGL.playerNames); ++i)
	{
		if(CGL.playerNames[i].Gamertag[0])
		{
			if(CGL.playerNames[i].groupFlags & 2)
			{
				r3dPoint2D mapPos = getMinimapPos(r3dPoint3D(CGL.playerNames[i].lastPosX, 0, CGL.playerNames[i].lastPosZ));

				var[0].SetNumber(mapPos.x);
				var[1].SetNumber(mapPos.y);
				gfxMovie.Invoke("_root.api.Main.Map.addGroupPlayer", var, 2);
			}
			if(plr->ClanID != 0
#ifdef ENABLE_BATTLE_ROYALE
				//AlexRedd:: BR mode
				&& !gClientLogic().m_gameInfo.IsGameBR()
#endif //ENABLE_BATTLE_ROYALE
				)
			{
				if(CGL.playerNames[i].clanID == plr->ClanID && stricmp(CGL.playerNames[i].Gamertag, tmpPlrName)!=0 )
				{
					r3dPoint2D mapPos = getMinimapPos(r3dPoint3D(CGL.playerNames[i].lastPosX, 0, CGL.playerNames[i].lastPosZ));

					var[0].SetNumber(mapPos.x);
					var[1].SetNumber(mapPos.y);
					gfxMovie.Invoke("_root.api.Main.Map.addClanPlayer", var, 2);
				}
			}
			if(CGL.playerNames[i].c4h_activeUntil > r3dGetTime())
			{
				//public function addCallForHelpEvent(posx:Number, posy:Number, distressText:String, rewardText:String, username:String)
				r3dPoint2D mapPos = getMinimapPos(r3dPoint3D(CGL.playerNames[i].c4h_locX, 0, CGL.playerNames[i].c4h_locZ));
				var[0].SetNumber(mapPos.x);
				var[1].SetNumber(mapPos.y);
				var[2].SetString(CGL.playerNames[i].c4h_distress);
				var[3].SetString(CGL.playerNames[i].c4h_reward);
				var[4].SetString(CGL.playerNames[i].Gamertag);
				var[5].SetString("help");
				gfxMovie.Invoke("_root.api.Main.Map.addCallForHelpEvent", var, 6);
			}
		}
	}

#ifdef MISSIONS
	for( std::map< uint32_t, HUDMissionIcon >::iterator mapIter = missionIcons.begin();
		 mapIter != missionIcons.end(); ++mapIter )
 	{
		const HUDMissionIcon& icon = mapIter->second;
		if( icon.m_active )
		{
 			// public function addMissionIcon(posx:Number, posy:Number, iconType:String)
 			r3dPoint2D mapPos = getMinimapPos( r3dPoint3D( icon.m_location.x, 0, icon.m_location.z ) );
 			var[0].SetNumber(mapPos.x);
 			var[1].SetNumber(mapPos.y);
			var[2].SetString(ActionIconNames[ icon.m_mapIcon - 1 ] ); // Decrement by 1 here because 0 - "NotSet" is not valid or represented.
			gfxMovie.Invoke("_root.api.Main.Map.addMissionIcon", var, 3);
		}
 	}
#endif
	
	for( std::map< uint32_t, HUDAirDropIcon >::iterator mapAirDrop = AirDropsIcons.begin();
		 mapAirDrop != AirDropsIcons.end(); ++mapAirDrop )
 	{
		const HUDAirDropIcon& AirDrop = mapAirDrop->second;		

		if( AirDrop.m_active )
		{
 			r3dPoint2D mapPos = getMinimapPos( r3dPoint3D( AirDrop.m_location.x, 0, AirDrop.m_location.z ) );
 			var[0].SetNumber(mapPos.x);
 			var[1].SetNumber(mapPos.y);
			gfxMovie.Invoke("_root.api.Main.Map.addAirDropPos", var, 2);
		}
 	}

	for (std::map< uint32_t, HUDCarIcon >::iterator mapCar = CarIcons.begin();
		mapCar != CarIcons.end(); ++mapCar)
	{
		const HUDCarIcon& Car = mapCar->second;

		if (Car.m_active)
		{
			r3dPoint2D mapPos = getMinimapPos(r3dPoint3D(Car.m_location.x, 0, Car.m_location.z));
			var[0].SetNumber(mapPos.x);
			var[1].SetNumber(mapPos.y);
			gfxMovie.Invoke("_root.api.Main.Map.addCarPos", var, 2);
		}
	}

	//AlexRedd:: rare box
	for( std::map< uint32_t, HUDLootBoxIcon >::iterator mapLootBox = LootBoxIcons.begin();
		 mapLootBox != LootBoxIcons.end(); ++mapLootBox )
 	{
		const HUDLootBoxIcon& icon = mapLootBox->second;		

		if( icon.m_active )
		{
			char typeStr[32]={0};
			char distressStr[32]={0};
			char rewardStr[32]={0};
			char nameStr[32]={0};

			switch(icon.type)
			{			
			case 1:
				r3dscpy(typeStr, "rare1");//SNIPER
				r3dscpy(distressStr, (gLangMngr.getString("$HUD_distressRAREbox1")));
				r3dscpy(rewardStr, (gLangMngr.getString("$HUD_rewardRAREbox1")));
				r3dscpy(nameStr, (gLangMngr.getString("$HUD_nameRAREbox1")));
				break;
			case 2:
				r3dscpy(typeStr, "rare2");//RIFLLE
				r3dscpy(distressStr, (gLangMngr.getString("$HUD_distressRAREbox2")));
				r3dscpy(rewardStr, (gLangMngr.getString("$HUD_rewardRAREbox2")));
				r3dscpy(nameStr, (gLangMngr.getString("$HUD_nameRAREbox2")));
				break;
			case 3:
				r3dscpy(typeStr, "rare3");//ARMOR
				r3dscpy(distressStr, (gLangMngr.getString("$HUD_distressRAREbox3")));
				r3dscpy(rewardStr, (gLangMngr.getString("$HUD_rewardRAREbox3")));
				r3dscpy(nameStr, (gLangMngr.getString("$HUD_nameRAREbox3")));
				break;			
			default:
				r3dscpy(typeStr, "help");
				r3dscpy(distressStr, "COMMON ULTRA RAR IS HERE!");
				r3dscpy(rewardStr, "ULTRA RAR ITEM");
				r3dscpy(nameStr, "ULTRA RAR BOX!");
			}

			r3dPoint2D mapPos = getMinimapPos( r3dPoint3D( icon.m_location.x, 0, icon.m_location.z ) );
			var[0].SetNumber(mapPos.x);
			var[1].SetNumber(mapPos.y);
			var[2].SetString(distressStr);
			var[3].SetString(rewardStr);
			var[4].SetString(nameStr);
			var[5].SetString(typeStr);
			gfxMovie.Invoke("_root.api.Main.Map.addCallForHelpEvent", var, 6);
		}
 	}

#ifdef ENABLE_BATTLE_ROYALE
	//AlexRedd:: BR mode
	if(gClientLogic().m_gameInfo.IsGameBR())
	{
		if(gClientLogic().m_gameAboutToStart || gClientLogic().m_gameHasStarted)
		{
			char sFullPath[512];
			char sFullPathImg[512];		
			sprintf(sFullPath, "%s\\%s", r3dGameLevel::GetHomeDir(), "minimap.dds");		
			sprintf(sFullPathImg, "$%s", sFullPath); // use '$' char to indicate absolute path

			if(r3dFileExists(sFullPath))
				gfxMovie.Invoke("_root.api.setMapIcon", sFullPathImg);

			if(gClientLogic().m_gameHasStarted)
				showGasRadius();
		}		
	}	
#endif //ENABLE_BATTLE_ROYALE
	
	gfxMovie.Invoke("_root.api.Main.Map.submitPlayerLocations", "");
}

void HUDPause::showGasRadius()//AlexRedd:: BR mode
{
	if( GasIcon.m_active )
	{
		Scaleform::GFx::Value var[3];
		r3dPoint2D mapPos = getMinimapPos( r3dPoint3D( GasIcon.m_location.x, 0, GasIcon.m_location.z ) );
		var[0].SetNumber(mapPos.x);
		var[1].SetNumber(mapPos.y);
		// shit here
		//var[2].SetNumber(int(gClientLogic().m_gameGasRadius/2.25f));// 2048x2048 map
		var[2].SetNumber(int(gClientLogic().m_gameGasRadius));//512x512 map

		gfxMovie.Invoke("_root.api.Main.Map.addGasPos", var, 3);			
		//r3dOutToLog("!!!addGasPos: mapPos.x %.1f | mapPos.y %.1f | GasRadius %.1f\n", mapPos.x, mapPos.y, gClientLogic().m_gameGasRadius);
	} 
}

void HUDPause::setMinimapPosition(const r3dPoint3D& pos, const r3dPoint3D& dir)
{
	r3dPoint2D mapPos = getMinimapPos(pos);

	// calculate rotation around Y axis
	r3dPoint3D d = dir;
	d.y = 0;
	d.Normalize();
	float dot1 = d.Dot(r3dPoint3D(0,0,1)); // north
	float dot2 = d.Dot(r3dPoint3D(1,0,0));
	float deg = acosf(dot1);
	deg = R3D_RAD2DEG(deg);
	if(dot2<0) 
		deg = 360 - deg;
	deg = R3D_CLAMP(deg, 0.0f, 360.0f);	

	Scaleform::GFx::Value var[3];
	var[0].SetNumber(mapPos.x);
	var[1].SetNumber(mapPos.y);
	var[2].SetNumber(deg);
	gfxMovie.Invoke("_root.api.setPlayerPosition", var, 3);
}

void HUDPause::eventMissionAccept(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount==1);
	uint32_t missionID = args[0].GetUInt();

#ifdef MISSIONS
	const uint32_t MaxAllowedMissions = 3;
	obj_Player* plr = gClientLogic().localPlayer_;
	if( plr && !plr->bDead && plr->acceptedMissions.size() < MaxAllowedMissions )
	{
		if( setMissionButton( missionID, true, false ) )
		{
			gfxMovie.Invoke("_root.api.Main.Missions.clearMissionList", "");
			eventMissionRequestList( pMovie, NULL, 0 );

			PKT_C2S_AcceptMission_s n;
			n.missionID = missionID;
			p2pSendToHost(plr, &n, sizeof(n));
		}
	}
	else if( plr->bDead )
	{
		showMessage( gLangMngr.getString("$FR_PLAY_GAME_SURVIVOR_DEAD") );
	}
	else
	{
		showMessage( gLangMngr.getString("HUD_Msg_MissionMaxReached") );
	}
#endif
}

void HUDPause::eventMissionDecline(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	obj_Player* plr = gClientLogic().localPlayer_;
	if( plr && plr->bDead )
	{
		showMessage( gLangMngr.getString("$FR_PLAY_GAME_SURVIVOR_DEAD") );
		return;
	}

	r3d_assert(argCount==1);
	uint32_t missionID = args[0].GetUInt();

#ifdef MISSIONS
	setMissionButton( missionID, false, true );
	gfxMovie.Invoke("_root.api.Main.Missions.clearMissionList", "");
	eventMissionRequestList( pMovie, NULL, 0 );
#endif
}

void HUDPause::eventMissionAbandon(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	obj_Player* plr = gClientLogic().localPlayer_;
	if( plr && plr->bDead )
	{
		showMessage( gLangMngr.getString("$FR_PLAY_GAME_SURVIVOR_DEAD") );
		return;
	}

	r3d_assert(argCount==1);
	uint32_t missionID = args[0].GetUInt();

#ifdef MISSIONS
	if( setMissionButton( missionID, false, false ) )
	{
		PKT_C2S_AbandonMission_s n;
		n.missionID = missionID;
		p2pSendToHost(gClientLogic().localPlayer_, &n, sizeof(n));
	}

	gfxMovie.Invoke("_root.api.Main.Missions.clearMissionList", "");
	eventMissionRequestList( pMovie, NULL, 0 );
#endif
}

void HUDPause::eventMissionRequestList(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount == 0);

	// populate list with setMissionInfo

#ifdef MISSIONS
	uint32_t index = 0;
	//r3dOutToLog("!!! Mission HUD Populate\n");
	for( std::vector<HUDMissionButton>::const_iterator iter = missionButtons.begin();
		 iter != missionButtons.end(); ++iter )
	{
		if( iter->m_declined )
			continue;

		setMissionInfo( index, iter->m_declined, iter->m_accepted,
			gLangMngr.getString( iter->m_missionNameStringID ),
			gLangMngr.getString( iter->m_missionDescStringID ),
			iter->m_missionIconPath, iter->m_missionID );
		++index;
	}
#endif

	gfxMovie.Invoke("_root.api.Main.Missions.showMissionList", "");
}

void HUDPause::setMissionInfo(uint32_t BtnIdx, bool isDeclined, bool isAccepted, const char* MissionName, const char* MissionDesc, const char* MissionIcon, uint32_t missionID)
{
	if(!isInit)
		return;

	//r3dOutToLog("!!! Mission(%d): BtnIdx(%d), %s, %d\n", missionID, BtnIdx, MissionName, missionID);
	Scaleform::GFx::Value var[7];
	var[0].SetUInt(BtnIdx);
	var[1].SetBoolean(isDeclined);
	var[2].SetBoolean(isAccepted);
	var[3].SetString(MissionName);
	var[4].SetString(MissionDesc);
	var[5].SetString(MissionIcon);
	var[6].SetUInt(missionID);
	gfxMovie.Invoke("_root.api.Main.Missions.SetMissionInfo", var, 7);
}

bool HUDPause::Init()
{
 	if(!gfxMovie.Load("Data\\Menu\\WarZ_HUD_menu.swf", false)) 
 		return false;
 
#define MAKE_CALLBACK(FUNC) game_new r3dScaleformMovie::TGFxEICallback<HUDPause>(this, &HUDPause::FUNC)
 	gfxMovie.RegisterEventHandler("eventBackpackGridSwap", MAKE_CALLBACK(eventBackpackGridSwap));
	gfxMovie.RegisterEventHandler("eventShowContextMenuCallback", MAKE_CALLBACK(eventShowContextMenuCallback));
	gfxMovie.RegisterEventHandler("eventContextMenu_Action", MAKE_CALLBACK(eventContextMenu_Action));
	gfxMovie.RegisterEventHandler("eventChangeBackpack", MAKE_CALLBACK(eventChangeBackpack));
	gfxMovie.RegisterEventHandler("eventMsgBoxCallback", MAKE_CALLBACK(eventMsgBoxCallback));
	gfxMovie.RegisterEventHandler("eventBackToGame", MAKE_CALLBACK(eventBackToGame));
	gfxMovie.RegisterEventHandler("eventQuitGame", MAKE_CALLBACK(eventQuitGame));
	gfxMovie.RegisterEventHandler("eventShowMap", MAKE_CALLBACK(eventShowMap));
	gfxMovie.RegisterEventHandler("eventOptionsControlsRequestKeyRemap", MAKE_CALLBACK(eventOptionsControlsRequestKeyRemap));
	gfxMovie.RegisterEventHandler("eventOptionsControlsReset", MAKE_CALLBACK(eventOptionsControlsReset));
	gfxMovie.RegisterEventHandler("eventOptionsApply", MAKE_CALLBACK(eventOptionsApply));	
	gfxMovie.RegisterEventHandler("eventSendCallForHelp", MAKE_CALLBACK(eventSendCallForHelp));	
	gfxMovie.RegisterEventHandler("eventDisableHotKeys", MAKE_CALLBACK(eventDisableHotKeys));	
	gfxMovie.RegisterEventHandler("eventRepairItem", MAKE_CALLBACK(eventRepairItem));	
	gfxMovie.RegisterEventHandler("eventMissionAccept", MAKE_CALLBACK(eventMissionAccept));	
	gfxMovie.RegisterEventHandler("eventMissionDecline", MAKE_CALLBACK(eventMissionDecline));	
	gfxMovie.RegisterEventHandler("eventMissionAbandon", MAKE_CALLBACK(eventMissionAbandon));	
	gfxMovie.RegisterEventHandler("eventMissionRequestList", MAKE_CALLBACK(eventMissionRequestList));	

	gfxMovie.SetCurentRTViewport( Scaleform::GFx::Movie::SM_ExactFit );

	AllowItemDropAfterThisTime = 999999999999999.0f;
	disableHotKeys = false;
	isDisconnecting_ = false;	

	// add items
	{
		addItemsAndCategoryToUI(gfxMovie);
	}

	{
		char sFullPath[512];
		char sFullPathImg[512];
		if(gUserProfile.ProfileData.isDevAccount>0 && gClientLogic().m_gameInfo.mapId==GBGameInfo::MAPID_ZP_Test)		
			sprintf(sFullPath, "%s\\%s", r3dGameLevel::GetHomeDir(), "minimap_dev.dds");
		else if(gClientLogic().m_gameInfo.mapId==GBGameInfo::MAPID_WZ_Quarantine/*MAPID_WZ_BRmap*/ && !gClientLogic().m_gameHasStarted)
			sprintf(sFullPath, "%s\\%s", r3dGameLevel::GetHomeDir(), "minimap_lobby.dds");
		else		
			sprintf(sFullPath, "%s\\%s", r3dGameLevel::GetHomeDir(), "minimap.dds");		
		sprintf(sFullPathImg, "$%s", sFullPath); // use '$' char to indicate absolute path

		if(r3dFileExists(sFullPath))
			gfxMovie.Invoke("_root.api.setMapIcon", sFullPathImg);
	}

	for(int i=0; i<r3dInputMappingMngr::KS_NUM; ++i)
	{
		Scaleform::GFx::Value args[2];
		args[0].SetString(gLangMngr.getString(InputMappingMngr->getMapName((r3dInputMappingMngr::KeybordShortcuts)i)));
		args[1].SetString(InputMappingMngr->getKeyName((r3dInputMappingMngr::KeybordShortcuts)i));
		gfxMovie.Invoke("_root.api.addKeyboardMapping", args, 2);
	}

	m_waitingForKeyRemap = -1;

	isActive_ = false;
	isInit = true;
	return true;
}

bool HUDPause::Unload()
{
	r_gameplay_blur_strength->SetFloat(0.0f);

 	gfxMovie.Unload();
	isActive_ = false;
	isInit = false;
	return true;
}

void HUDPause::Update()
{
	//AlexRedd:: change backpack button disabled now for dev event servers and BR game
	if (gClientLogic().m_gameInfo.IsDevEvent()
#ifdef ENABLE_BATTLE_ROYALE
		 || gClientLogic().m_gameInfo.IsGameBR()
#endif //ENABLE_BATTLE_ROYALE
		)
		gfxMovie.SetVariable("_root.api.Main.Inventory.BtnChange.visible", false);

#ifdef ENABLE_BATTLE_ROYALE
	if(gClientLogic().m_gameInfo.IsGameBR())//AlexRedd:: BR mode
		gfxMovie.SetVariable("_root.api.Main.Map.MapZoom.visible", false);// disabled now. TODO: fix me
#endif //ENABLE_BATTLE_ROYALE

	if(m_waitingForKeyRemap != -1)
	{
		// query input manager for any input
		bool conflictRemapping = false;
		if(InputMappingMngr->attemptRemapKey((r3dInputMappingMngr::KeybordShortcuts)m_waitingForKeyRemap, conflictRemapping))
		{
			Scaleform::GFx::Value var[2];
			var[0].SetNumber(m_waitingForKeyRemap);
			var[1].SetString(InputMappingMngr->getKeyName((r3dInputMappingMngr::KeybordShortcuts)m_waitingForKeyRemap));
			gfxMovie.Invoke("_root.api.updateKeyboardMapping", var, 2);
			m_waitingForKeyRemap = -1;

			void writeInputMap();
			writeInputMap();

			if(conflictRemapping)
			{
				Scaleform::GFx::Value var[2];
				var[0].SetString(gLangMngr.getString("ConflictRemappingKeys"));
				var[1].SetBoolean(true);
				gfxMovie.Invoke("_root.api.showInfoMsg", var, 2);		
			}
		}
	}

	if(DisconnectAt > 0)
	{
		int timeLeft = int(ceilf(DisconnectAt - r3dGetTime()));
		Scaleform::GFx::Value var[3];
		char tmpMsg[128];
		if(timeLeft > 0)
			sprintf(tmpMsg, gLangMngr.getString("HUD_DisconnectingIn"), timeLeft);
		else
			r3dscpy(tmpMsg, gLangMngr.getString("HUD_DisconnectingSoon"));
		var[0].SetString(tmpMsg);
		var[1].SetBoolean(false);
		var[2].SetString(gLangMngr.getString("HUD_DisconnectingTitle"));
		gfxMovie.Invoke("_root.api.showInfoMsg", var, 3);
		// FOR DENIS: enable this to show CANCEL button
// 		var[1].SetString("CANCEL");
// 		var[2].SetString("DISCONNECTING");
// 		gfxMovie.Invoke("_root.api.showInfoMsg2", var, 3);
	}
}

void HUDPause::Draw()
{
	gfxMovie.SetCurentRTViewport(Scaleform::GFx::Movie::SM_NoScale);
	gfxMovie.SetCurentRTViewport(Scaleform::GFx::Movie::SM_ExactFit);

 	gfxMovie.UpdateAndDraw();
}

void HUDPause::Deactivate()
{
#ifdef MISSIONS
	clearMissionButtonDeclines();
#endif

	if(DisconnectAt > 0.0) // do not allow to hide this menu if player is disconneting
		return;

	if(isLockedUI)
		return;

	{
		Scaleform::GFx::Value var[1];
		var[0].SetString("menu_close");
		gfxMovie.OnCommandCallback("eventSoundPlay", var, 1);
	}

	if(prevKeyboardCaptureMovie)
	{
		prevKeyboardCaptureMovie->SetKeyboardCapture();
		prevKeyboardCaptureMovie = NULL;
	}

	if( !g_cursor_mode->GetInt() )
	{
		r3dMouse::Hide();
	}

	isActive_ = false;
	
	disableHotKeys = false;

	//r_hud_filter_mode->SetInt(0);
	r_gameplay_blur_strength->SetFloat(0.0f);
	
}

extern const char* getReputationString(int reputation);
void HUDPause::Activate()
{
	r3d_assert(isLockedUI==false);
	DisconnectAt = 0.0f;

	prevKeyboardCaptureMovie = gfxMovie.SetKeyboardCapture(); // for mouse scroll events

	//r_hud_filter_mode->SetInt(HUDFilter_Menu);
	r_gameplay_blur_strength->SetFloat(0.8f);

	r3d_assert(!isActive_);
	r3dMouse::Show();
	isActive_ = true;

	Scaleform::GFx::Value var[38];
	var[0].SetNumber(0);
	var[1].SetNumber( r_overall_quality->GetInt());
	var[2].SetNumber( ((r_gamma_pow->GetFloat()-2.2f)+1.0f)/2.0f);
	var[3].SetNumber( 0.0f );//r_contrast->GetFloat());
	var[4].SetNumber( s_sound_volume->GetFloat());
	var[5].SetNumber( s_music_volume->GetFloat());
	var[6].SetNumber( g_voip_volume->GetFloat());
	var[7].SetNumber( g_tps_camera_mode->GetInt());
	var[8].SetNumber( g_enable_voice_commands->GetInt() + 1);
	var[9].SetNumber( r_antialiasing_quality->GetInt());
	var[10].SetNumber( r_ssao_quality->GetInt());
	var[11].SetNumber( r_terrain_quality->GetInt());
	var[12].SetNumber( r_environment_quality->GetInt() ); 
	var[13].SetNumber( 0 );
	var[14].SetNumber( r_shadows_quality->GetInt()+1);
	var[15].SetNumber( r_lighting_quality->GetInt());
	var[16].SetNumber( 0 );
	var[17].SetNumber( r_mesh_quality->GetInt());
	var[18].SetNumber( r_anisotropy_quality->GetInt());
	var[19].SetNumber( r_postprocess_quality->GetInt());
	var[20].SetNumber( r_texture_quality->GetInt());
	var[21].SetNumber( g_vertical_look->GetBool());
	var[22].SetNumber( 0 ); // not used
	var[23].SetNumber( g_mouse_wheel->GetBool());
	var[24].SetNumber( g_mouse_sensitivity->GetFloat());
	var[25].SetNumber( g_mouse_acceleration->GetBool());
	var[26].SetNumber( g_toggle_aim->GetBool());
	var[27].SetNumber( g_toggle_crouch->GetBool());
	var[28].SetNumber( r_ScreenMode->GetInt());
	var[29].SetNumber( r_vsync_enabled->GetInt()+1);
	//new opt
	var[30].SetNumber( g_crosshaire_mode->GetInt() + 1);
	var[31].SetNumber( g_highlight_mode->GetInt() + 1);
	var[32].SetNumber( g_chat_message_sound->GetInt() + 1);
	var[33].SetNumber( g_disable_jump_sound->GetInt() + 1);
	var[34].SetNumber( g_enable_killstreak_sounds->GetInt() + 1);
	var[35].SetNumber( g_killstreak_sounds->GetInt() + 1);
	var[36].SetNumber( g_crosshaire_color->GetInt() + 1);
	var[37].SetNumber( g_enable_alpha_sounds->GetInt() + 1);

	gfxMovie.Invoke("_root.api.setOptions", var, 38);

	// add player info
	{
		Scaleform::GFx::Value var[22];

		obj_Player* plr = gClientLogic().localPlayer_;
		r3d_assert(plr);

		plr->showSlotsInfoTimer = r3dGetTime();

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
		var[21].SetBoolean(false); // globalInventory. For now false. Separate UI for it

		gfxMovie.Invoke("_root.api.addClientSurvivor", var, 22);
	}

	{
		char tmpStr[256];
		sprintf(tmpStr, gLangMngr.getString("HUD_ServerNameTitle"), gClientLogic().m_gameInfo.name);
		gfxMovie.Invoke("_root.api.setServerName", tmpStr);
	}

	{
		Scaleform::GFx::Value var[3];
		var[0].SetUInt(gUserProfile.ProfileData.ResWood);
		var[1].SetUInt(gUserProfile.ProfileData.ResStone);
		var[2].SetUInt(gUserProfile.ProfileData.ResMetal);
		gfxMovie.Invoke("_root.api.setResourcesNum", var, 3);
		//public function setResourcesNum(numWood:uint, numStone:uint, numMetal:uint)
	}

	reloadBackpackInfo();

	updateSurvivorTotalWeight();

	// hide by default
	gfxMovie.Invoke("_root.api.showInventory", 0);
	gfxMovie.Invoke("_root.api.showMap", 0);
	gfxMovie.Invoke("_root.api.showMissions", 0);
	gfxMovie.Invoke("_root.api.showOptions", 0);

	{
		Scaleform::GFx::Value var[1];
		var[0].SetString("menu_open");
		gfxMovie.OnCommandCallback("eventSoundPlay", var, 1);
	}
}

void HUDPause::updateSurvivorTotalWeight()
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

void HUDPause::setTime(__int64 utcTime)
{
	const static char* months[12] = {
		"January",
		"February",
		"March",
		"April",
		"May",
		"June",
		"July",
		"August",
		"September",
		"October",
		"November",
		"December"
	};
	struct tm* tm = _gmtime64(&utcTime);

	char date[128];
	char time[128];
	sprintf(date, "%s %d, %d", gLangMngr.getString(months[tm->tm_mon]), tm->tm_mday, 1900 + tm->tm_year);
	sprintf(time, "%02d:%02d", tm->tm_hour, tm->tm_min);

	Scaleform::GFx::Value var[2];
	var[0].SetString(date);
	var[1].SetString(time);
	gfxMovie.Invoke("_root.api.setTime", var, 2);
}

void HUDPause::showInventory()
{
	gfxMovie.Invoke("_root.api.showInventory", 1);
}

void HUDPause::showMap()
{
	gfxMovie.Invoke("_root.api.showMap", 1);
	showMinimapFunc();
}

void HUDPause::UpdateSkinIcons()
{
	obj_Player* plr = gClientLogic().localPlayer_;
	r3d_assert(plr);
	wiCharDataFull& slot = plr->CurLoadout;

	for (std::map< uint32_t, obj_Player::WeaponSkins >::iterator Skinids = plr->WpnSKN.begin(); Skinids != plr->WpnSKN.end(); ++Skinids)
	{
		const obj_Player::WeaponSkins& SkinWpn = Skinids->second;

		const GearConfig* gc = g_pWeaponArmory->getGearConfig(SkinWpn.itemID);
		const BackpackConfig* bpk = g_pWeaponArmory->getBackpackConfig(SkinWpn.itemID);
		const BaseItemConfig* itm = g_pWeaponArmory->getConfig(SkinWpn.itemID);
		int SkinID = -1;

		if (gc)
		{
			SkinID = plr->GetItemIDSkin("Gear", SkinWpn.itemID, plr->GetSkinIDHud(SkinWpn.itemID));
		}
		if (itm)
		{

			if (itm->category >= storecat_ASR && itm->category <= storecat_MELEE) //Показать иконки ху?скин
			{
				SkinID = plr->GetItemIDSkin("Weapon", SkinWpn.itemID, plr->GetSkinIDHud(SkinWpn.itemID));
			}
		}
		if (bpk)
		{
			SkinID = plr->GetItemIDSkin("BackPack", SkinWpn.itemID, plr->GetSkinIDHud(SkinWpn.itemID));
		}

		if (SkinID<0) return;


		const BaseItemConfig* bic = g_pWeaponArmory->getConfig(SkinID);
		if (bic) {
			Scaleform::GFx::Value var[2];
			var[0].SetUInt(SkinWpn.itemID);
			var[1].SetString(bic->m_StoreIcon);
			gfxMovie.Invoke("_root.api.SetIconItem", var, 2);
		}
	}

	gfxMovie.Invoke("_root.api.Main.Inventory.showBackpack", "");
}

void HUDPause::reloadBackpackInfo()
{
	updateSurvivorTotalWeight();

	// reset backpack
	{
		gfxMovie.Invoke("_root.api.clearBackpack", "");
		gfxMovie.Invoke("_root.api.clearBackpacks", "");
		gfxMovie.Invoke("_root.api.setBackpackName", "");
		gfxMovie.Invoke("_root.api.setMaxWeight", "");
		gfxMovie.Invoke("_root.api.Main.Inventory.showDialBox", 0);
	}

	r3dgameVector(uint32_t) uniqueBackpacks; // to filter identical backpack
	int backpackSlotIDInc = 0;
	// add backpack content info
	{
		obj_Player* plr = gClientLogic().localPlayer_;
		r3d_assert(plr);
		wiCharDataFull& slot = plr->CurLoadout;

		const BackpackConfig* bc = g_pWeaponArmory->getBackpackConfig(slot.BackpackID);
		r3d_assert(bc);
		float BPMaxWeight = bc->m_maxWeight;
		float potentialWeight = 0;
		float itemsWeight = 0;

		Scaleform::GFx::Value var[7];
		for (int a = 0; a < slot.BackpackSize; a++)
		{	
			if (slot.Items[a].itemID != 0)
			{
				const BaseItemConfig* bic = g_pWeaponArmory->getConfig(slot.Items[a].itemID);
				if(bic){
					itemsWeight = bic->m_Weight * slot.Items[a].quantity;
					if(slot.Skills[CUserSkills::SKILL_Physical3])
						itemsWeight *= 0.95f;
					if(slot.Skills[CUserSkills::SKILL_Physical7])
						itemsWeight *= 0.9f;

					if (a != 0 && a != 1 && a != 6 && a == 7)
						potentialWeight += itemsWeight; 
				}

				if(potentialWeight <= BPMaxWeight || potentialWeight - BPMaxWeight < 0.001f)//cynthia: 1679
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
				else{
					potentialWeight -= itemsWeight; 
					plr->DropItem(a);
				}
			}
		}
	}

	gfxMovie.Invoke("_root.api.Main.Inventory.showBackpack", "");
}

void HUDPause::eventOptionsControlsRequestKeyRemap(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(args);
	r3d_assert(argCount == 1);

	int remapIndex = (int)args[0].GetNumber();
	r3d_assert(m_waitingForKeyRemap == -1);

	m_waitingForKeyRemap = remapIndex;
}

void HUDPause::eventOptionsControlsReset(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(args);
	r3d_assert(argCount == 0);

	for(int i=0; i<r3dInputMappingMngr::KS_NUM; ++i)
	{
		Scaleform::GFx::Value args[2];
		args[0].SetString(gLangMngr.getString(InputMappingMngr->getMapName((r3dInputMappingMngr::KeybordShortcuts)i)));
		args[1].SetString(InputMappingMngr->getKeyName((r3dInputMappingMngr::KeybordShortcuts)i));
		gfxMovie.Invoke("_root.api.setKeyboardMapping", args, 2);
	}
	void writeInputMap();
	writeInputMap();

	// update those to match defaults in Vars.h
	g_vertical_look			->SetBool(false);
	g_mouse_wheel			->SetBool(true);
	g_mouse_sensitivity		->SetFloat(1.0f);
	g_mouse_acceleration	->SetBool(false);
	g_toggle_aim			->SetBool(false);
	g_toggle_crouch			->SetBool(false);

	g_crosshaire_mode->SetInt(0);
	g_highlight_mode->SetInt(0);
	g_chat_message_sound->SetInt(0);
	g_disable_jump_sound->SetBool(true);
	g_enable_killstreak_sounds->SetBool(true);
	g_killstreak_sounds->SetBool(false);
	g_crosshaire_color->SetInt(0);
	g_enable_alpha_sounds->SetBool(false);
	g_enable_voice_commands->SetInt(1);

	// write to ini file
	writeGameOptionsFile();

	Scaleform::GFx::Value var[38];
	var[0].SetNumber(0);
	var[1].SetNumber( r_overall_quality->GetInt());
	var[2].SetNumber( ((r_gamma_pow->GetFloat()-2.2f)+1.0f)/2.0f);
	var[3].SetNumber( 0.0f);//r_contrast->GetFloat());
	var[4].SetNumber( s_sound_volume->GetFloat());
	var[5].SetNumber( s_music_volume->GetFloat());
	var[6].SetNumber( g_voip_volume->GetFloat());
	var[7].SetNumber( g_tps_camera_mode->GetInt());
	var[8].SetNumber( g_enable_voice_commands->GetInt() + 1);
	var[9].SetNumber( r_antialiasing_quality->GetInt());
	var[10].SetNumber( r_ssao_quality->GetInt());
	var[11].SetNumber( r_terrain_quality->GetInt());
	var[12].SetNumber( r_environment_quality->GetInt() ); 
	var[13].SetNumber( 0 );
	var[14].SetNumber( r_shadows_quality->GetInt()+1);
	var[15].SetNumber( r_lighting_quality->GetInt());
	var[16].SetNumber( 0 );
	var[17].SetNumber( r_mesh_quality->GetInt());
	var[18].SetNumber( r_anisotropy_quality->GetInt());
	var[19].SetNumber( r_postprocess_quality->GetInt());
	var[20].SetNumber( r_texture_quality->GetInt());
	var[21].SetNumber( g_vertical_look->GetBool());
	var[22].SetNumber( 0 ); // not used
	var[23].SetNumber( g_mouse_wheel->GetBool());
	var[24].SetNumber( g_mouse_sensitivity->GetFloat());
	var[25].SetNumber( g_mouse_acceleration->GetBool());
	var[26].SetNumber( g_toggle_aim->GetBool());
	var[27].SetNumber( g_toggle_crouch->GetBool());
	var[28].SetNumber(r_ScreenMode->GetInt());
	var[29].SetNumber( r_vsync_enabled->GetInt()+1);
	//new opt
	var[30].SetNumber( g_crosshaire_mode->GetInt() + 1);
	var[31].SetNumber( g_highlight_mode->GetInt() + 1);
	var[32].SetNumber( g_chat_message_sound->GetInt() + 1);
	var[33].SetNumber( g_disable_jump_sound->GetInt() + 1);
	var[34].SetNumber( g_enable_killstreak_sounds->GetInt() + 1);
	var[35].SetNumber( g_killstreak_sounds->GetInt() + 1);
	var[36].SetNumber( g_crosshaire_color->GetInt() + 1);
	var[37].SetNumber( g_enable_alpha_sounds->GetInt() + 1);

	gfxMovie.Invoke("_root.api.setOptions", var, 38);
 
	gfxMovie.Invoke("_root.api.reloadOptions", "");
}

void HUDPause::eventOptionsApply(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	int old_width = r_width->GetInt();
	//r_width->SetInt(width);
	int old_height = r_height->GetInt();
	//r_height->SetInt(height);

	int old_screenmode = r_ScreenMode->GetInt();
	//r_ScreenMode->SetInt((int)args[21].GetNumber());

	int old_vsync = r_vsync_enabled->GetInt();
	r_vsync_enabled->SetInt((int)args[22].GetNumber() - 1);

	r_overall_quality		->SetInt( (int)args[6].GetNumber());

	DWORD settingsChangedFlags = 0;
	GraphicSettings settings;

	switch( r_overall_quality->GetInt() )
	{
		case 1:
			FillDefaultSettings( settings, S_WEAK );
			settingsChangedFlags = SetDefaultSettings( S_WEAK );
			break;
		case 2:
			FillDefaultSettings( settings, S_MEDIUM );
			settingsChangedFlags = SetDefaultSettings( S_MEDIUM );
			break;
		case 3:
			FillDefaultSettings( settings, S_STRONG );
			settingsChangedFlags = SetDefaultSettings( S_STRONG );
			break;
		case 4:
			FillDefaultSettings( settings, S_ULTRA );
			settingsChangedFlags = SetDefaultSettings( S_ULTRA );
			break;
		case 5:
			settings = GetCustomSettings();
			settingsChangedFlags = SetCustomSettings( settings );
			break;
		default:
			r3d_assert( false );
	}

// clamp brightness and contrast, otherwise if user set it to 0 the screen will be white
//	r_brightness			->SetFloat( R3D_CLAMP((float)args[1].GetNumber(), 0.25f, 0.75f) );
//	r_contrast				->SetFloat( R3D_CLAMP((float)args[2].GetNumber(), 0.25f, 0.75f) );
	r_gamma_pow->SetFloat(2.2f + (float(args[1].GetNumber())*2.0f-1.0f));

	s_sound_volume			->SetFloat( R3D_CLAMP((float)args[3].GetNumber(), 0.0f, 1.0f) );
	s_music_volume			->SetFloat( R3D_CLAMP((float)args[4].GetNumber(), 0.0f, 1.0f) );
	g_voip_volume			->SetFloat( R3D_CLAMP((float)args[5].GetNumber(), 0.0f, 1.0f) );

	g_tps_camera_mode->SetInt((int)args[7].GetNumber());	

	g_vertical_look			->SetBool( !!(int)args[8].GetNumber() );
	g_mouse_wheel			->SetBool( !!(int)args[9].GetNumber() );
	g_mouse_sensitivity		->SetFloat( (float)args[0].GetNumber() );
	g_mouse_acceleration	->SetBool( !!(int)args[10].GetNumber() );

	g_enable_voice_commands->SetInt((int)args[11].GetNumber() - 1);

	g_crosshaire_mode->SetInt((int)args[12].GetNumber() - 1);
	g_highlight_mode->SetInt((int)args[13].GetNumber() - 1);
	g_chat_message_sound->SetInt((int)args[14].GetNumber() - 1);
	g_disable_jump_sound->SetInt((int)args[15].GetNumber() - 1);
	g_enable_killstreak_sounds->SetInt((int)args[16].GetNumber() - 1);
	g_killstreak_sounds->SetInt((int)args[17].GetNumber() - 1);
	g_crosshaire_color->SetInt((int)args[18].GetNumber() - 1);
	g_enable_alpha_sounds->SetInt((int)args[19].GetNumber() - 1);

	settingsChangedFlags |= GraphSettingsToVars( settings );

	void applyGraphicsOptions( uint32_t settingsFlags );
	applyGraphicsOptions( settingsChangedFlags );
	
	if(gTeamSpeakClient.m_playbackOpened)
	{
		gTeamSpeakClient.SetVolume(g_voip_volume->GetFloat());
	}

	// write to ini file
	writeGameOptionsFile();

	//reset reticle
	if (hudMain)
	{
		hudMain->refreshReticle();
		hudMain->setTPSReticleVisibility(g_camera_mode->GetInt() == 0 ? 1 : 0);
		hudMain->setReticleScale(1.0f);
	}

	if (old_screenmode != r_ScreenMode->GetInt() || old_width != r_width->GetInt() || old_height != r_height->GetInt() || old_vsync != r_vsync_enabled->GetInt())
		SetNeedUpdateMode();
}

void HUDPause::SetNeedUpdateMode()
{
	needUpdateMode_ = true;
}

