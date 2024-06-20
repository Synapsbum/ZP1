#include "r3dPCH.h"
#include "r3d.h"

#include "../../../Eternity/sf/Console/config.h"
#include "HUDPause.h"
#include "HUDVault.h"
#include "HUDSafelock.h"
#include "HUDTrade.h"
#include "HUDStore.h"
#include "HUDSkins.h"
#include "LangMngr.h"
#include "HUDSell.h"
#include "HUDRepair.h"
#include "HUDCrafting.h"

#include "FrontendShared.h"
#include "../ObjectsCode/weapons/Gear.h"
#include "../ObjectsCode/weapons/BackPack.h"

#include "../multiplayer/clientgamelogic.h"
#include "../ObjectsCode/AI/AI_Player.H"
#include "../ObjectsCode/weapons/Weapon.h"
#include "../ObjectsCode/weapons/WeaponArmory.h"
#include "../GameLevel.h"

const int SKINS_MENU_MAX = 3;

HUDSkins::HUDSkins()
{
	isActive_ = false;
	isInit = false;
}

HUDSkins::~HUDSkins()
{
}

void HUDSkins::eventSelectAttachment(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount == 4);
	int id = args[0].GetInt();
	int slotID = args[1].GetInt();
	int itemID = args[2].GetInt();
	int SkinID = args[3].GetInt();

	obj_Player* plr = gClientLogic().localPlayer_;
	r3d_assert(plr);
	wiCharDataFull& slot = plr->CurLoadout;

	PKT_C2S_GearSkinItem_s n3;
 	n3.recipeID= itemID;
	n3.SkinID = SkinID;
 	p2pSendToHost(gClientLogic().localPlayer_, &n3, sizeof(n3));

	{
		Scaleform::GFx::Value var[1];
		var[0].SetString("menu_click");
		gfxMovie.OnCommandCallback("eventSoundPlay", var, 1);
	}
}

bool HUDSkins::Init()
{
	if(!gfxMovie.Load("Data\\Menu\\WarZ_HUD_SkinsUI.swf", false)) 
		return false;

#define MAKE_CALLBACK(FUNC) game_new r3dScaleformMovie::TGFxEICallback<HUDSkins>(this, &HUDSkins::FUNC)
	gfxMovie.RegisterEventHandler("eventSelectAttachment", MAKE_CALLBACK(eventSelectAttachment));

	gfxMovie.SetCurentRTViewport( Scaleform::GFx::Movie::SM_ExactFit );

	isActive_ = false;
	isInit = true;
	return true;
}

bool HUDSkins::Unload()
{
	gfxMovie.Unload();
	isActive_ = false;
	isInit = false;
	return true;
}

void HUDSkins::Update()
{
}

void HUDSkins::Draw()
{
	gfxMovie.UpdateAndDraw();
}

void HUDSkins::Deactivate()
{
	if( !g_cursor_mode->GetInt() )
	{
		r3dMouse::Hide();
	}

	{
		Scaleform::GFx::Value var[1];
		var[0].SetString("menu_close");
		gfxMovie.OnCommandCallback("eventSoundPlay", var, 1);
	}


	isActive_ = false;
	obj_Player* plr = gClientLogic().localPlayer_;
	plr->updateWeaponUI();
}

void HUDSkins::Activate()
{
	r3d_assert(!isActive_);

	// check if user has weapon in his current slot
	obj_Player* plr = gClientLogic().localPlayer_;
	r3d_assert(plr);

	wiCharDataFull& slot = plr->CurLoadout;

	//if(plr->m_SelectedWeapon !=0 && plr->m_SelectedWeapon!=1) // attm only for primary and secondary slots
	//	return;

	extern HUDPause*	hudPause;
	if(hudPause->isActive()) return;
	extern HUDVault*	hudVault;
	if(hudVault->isActive()) return;
	extern HUDSell* hudSell;
	if(hudSell->isActive()) return;
	extern HUDRepair* hudRepair;
	if(hudRepair->isActive()) return;
	extern HUDCrafting* hudCraft;
	if(hudCraft->isActive()) return;
	extern HUDStore*	hudStore;
	if(hudStore->isActive()) return;
	extern HUDSafelock* hudSafelock;
	if(hudSafelock->isActive()) return;
	extern HUDTrade* hudTrade;
	if(hudTrade->isActive()) return;

	if(g_camera_mode->GetInt() == 2) // switch player to FPS mode
	{
		plr->switchFPS_TPS();
	}

	r3dMouse::Show();
	isActive_ = true;

	{
		Scaleform::GFx::Value var[1];
		var[0].SetString("menu_open");
		gfxMovie.OnCommandCallback("eventSoundPlay", var, 1);
	}

	gfxMovie.Invoke("_root.api.Main.clearskins", "");

	int GEAR_POS = 0;
	int	ARMOR_POS = 1;
	int BACKPACK_POS = 2;

	float screenW = 1920.0f;//r3dRenderer->ScreenW;
	float screenH = 1080.0f;//r3dRenderer->ScreenH;
	const r3dPoint2D SkinsSlotPos[SKINS_MENU_MAX] = {
		r3dPoint2D(screenW*0.240f, screenH*0.05f), // GEAR
		r3dPoint2D(screenW*0.240f, screenH*0.35f), // ARMOR
		r3dPoint2D(screenW*0.240f, screenH*0.67f) // BACKPACK
	};

	Gear* gear = plr->uberEquip_->slots_[SLOT_Helmet].gear;
	Gear* armor = plr->uberEquip_->slots_[SLOT_Armor].gear;
	BackPack* bkp = plr->uberEquip_->slots_[SLOT_Backpack].bpk;

	Scaleform::GFx::Value var[6];

	int CountGear = 1;
	int CountArmor = 1;
	int CountBackpack = 1;

	SkinsData DataA[999];
	SkinsData DataG[999];
	SkinsData DataB[999];

	for (int x=0; x<999; ++x)
	{
		DataA[x].Reset();
		DataG[x].Reset();
		DataB[x].Reset();
	}

	for(int i=0; i<plr->CurLoadout.NumGearSkinRecipes; ++i)
	{
		const GearSkinRecipeConfig* cfg = g_pWeaponArmory->getGearSkinRecipeConfig(plr->CurLoadout.GearSkinRecipesLearned[i]);
		//r3d_assert(cfg);

		if (cfg == NULL)
		{
			r3dOutToLog("######### THE RECIPE SKINS ID %i is old, remove\n",plr->CurLoadout.GearSkinRecipesLearned[i]);
			continue;
		}

		if (gear && gear->getItemID() == cfg->DefaultItemID)
		{
			DataG[cfg->SkinID].SkinID = cfg->SkinID;
			DataG[cfg->SkinID].craftedItemID = cfg->craftedItemID;
			DataG[cfg->SkinID].RecipeID = plr->CurLoadout.GearSkinRecipesLearned[i];
			DataG[cfg->SkinID].DefaultItemID = cfg->DefaultItemID;
			CountGear++;
		}
		if (armor && armor->getItemID() == cfg->DefaultItemID)
		{
			DataA[cfg->SkinID].SkinID = cfg->SkinID;
			DataA[cfg->SkinID].craftedItemID = cfg->craftedItemID;
			DataA[cfg->SkinID].RecipeID = plr->CurLoadout.GearSkinRecipesLearned[i];
			DataA[cfg->SkinID].DefaultItemID = cfg->DefaultItemID;
			CountArmor++;
		}
		if (bkp && bkp->getItemID() == cfg->DefaultItemID)
		{
			DataB[cfg->SkinID].SkinID = cfg->SkinID;
			DataB[cfg->SkinID].craftedItemID = cfg->craftedItemID;
			DataB[cfg->SkinID].RecipeID = plr->CurLoadout.GearSkinRecipesLearned[i];
			DataB[cfg->SkinID].DefaultItemID = cfg->DefaultItemID;
			CountBackpack++;
		}
	}

	if (gear)
	{
		const BaseItemConfig* cfg = g_pWeaponArmory->getConfig(gear->getItemID());

		var[0].SetInt(GEAR_POS);
		var[1].SetInt(int(SkinsSlotPos[GEAR_POS].x));
		var[2].SetInt(int(SkinsSlotPos[GEAR_POS].y));
		var[3].SetInt(GEAR_POS);
		var[4].SetString(gLangMngr.getString("$HUD_GearSkins"));
		gfxMovie.Invoke("_root.api.Main.addSkin", var, 5);

		var[1].SetInt(0);//posicion en el boton
		var[2].SetString(gLangMngr.getString(cfg->m_StoreName));
		var[3].SetInt(gear->getItemID());
		var[4].SetInt(0);//Numero del skin default es 0 SkinID
		var[5].SetString(gear->getStoreIcon());
		gfxMovie.Invoke("_root.api.Main.addSlotSkin", var, 6);

		int SlotActive = 0;

		int Count = 1;
		for (int i=1;i<30;i++)
		{
			if (DataG[i].SkinID != 0)
			{
				const BaseItemConfig* cfg = g_pWeaponArmory->getConfig(DataG[i].craftedItemID);

				var[1].SetInt(Count);//posicion en el boton
				var[2].SetString(gLangMngr.getString(cfg->m_StoreName));
				var[3].SetInt(DataG[i].RecipeID);
				var[4].SetInt(DataG[i].SkinID);//Numero del skin default es 0
				var[5].SetString(cfg->m_StoreIcon);
				gfxMovie.Invoke("_root.api.Main.addSlotSkin", var, 6);

				if (gear->ActualSkin == DataG[i].SkinID)
					SlotActive=Count;

				Count++;
			}
		}

		var[1].SetInt(SlotActive);
		gfxMovie.Invoke("_root.api.Main.setSlotActiveSkin", var, 2);
	}
	if (armor)
	{
		const BaseItemConfig* cfg = g_pWeaponArmory->getConfig(armor->getItemID());

		var[0].SetInt(ARMOR_POS);
		var[1].SetInt(int(SkinsSlotPos[ARMOR_POS].x));
		var[2].SetInt(int(SkinsSlotPos[ARMOR_POS].y));
		var[3].SetInt(ARMOR_POS);
		var[4].SetString(gLangMngr.getString("$HUD_ArmorSkins"));
		gfxMovie.Invoke("_root.api.Main.addSkin", var, 5);

		var[1].SetInt(0);//posicion en el boton
		var[2].SetString(gLangMngr.getString(cfg->m_StoreName));
		var[3].SetInt(armor->getItemID());
		var[4].SetInt(0);//Numero del skin default es 0
		var[5].SetString(armor->getStoreIcon());
		gfxMovie.Invoke("_root.api.Main.addSlotSkin", var, 6);

		int SlotActive = 0;

		int Count = 1;

		for (int i=1;i<30;i++)
		{
			if (DataA[i].SkinID != 0)
			{
				const BaseItemConfig* cfg = g_pWeaponArmory->getConfig(DataA[i].craftedItemID);

				var[1].SetInt(Count);//posicion en el boton
				var[2].SetString(gLangMngr.getString(cfg->m_StoreName));
				var[3].SetInt(DataA[i].RecipeID);
				var[4].SetInt(DataA[i].SkinID);//Numero del skin default es 0
				var[5].SetString(cfg->m_StoreIcon);
				gfxMovie.Invoke("_root.api.Main.addSlotSkin", var, 6);

				if (armor->ActualSkin == DataA[i].SkinID)
					SlotActive=Count;

				Count++;
			}
		}

		var[1].SetInt(SlotActive);
		gfxMovie.Invoke("_root.api.Main.setSlotActiveSkin", var, 2);
	}
	if (bkp)
	{
		const BaseItemConfig* cfg = g_pWeaponArmory->getConfig(bkp->getItemID());

		var[0].SetInt(BACKPACK_POS);
		var[1].SetInt(int(SkinsSlotPos[BACKPACK_POS].x));
		var[2].SetInt(int(SkinsSlotPos[BACKPACK_POS].y));
		var[3].SetInt(BACKPACK_POS);
		var[4].SetString(gLangMngr.getString("$HUD_BackpackSkins"));
		gfxMovie.Invoke("_root.api.Main.addSkin", var, 5);

		var[1].SetInt(0);//posicion en el boton
		var[2].SetString(gLangMngr.getString(cfg->m_StoreName));
		var[3].SetInt(bkp->getItemID());
		var[4].SetInt(0);//Numero del skin default es 0
		var[5].SetString(cfg->m_StoreIcon);
		gfxMovie.Invoke("_root.api.Main.addSlotSkin", var, 6);


		int SlotActive = 0;

		int Count = 1;

		for (int i=1;i<30;i++)
		{
			if (DataB[i].SkinID != 0)
			{
				const BaseItemConfig* cfg = g_pWeaponArmory->getConfig(DataB[i].craftedItemID);

				var[1].SetInt(Count);//posicion en el boton
				var[2].SetString(gLangMngr.getString(cfg->m_StoreName));
				var[3].SetInt(DataB[i].RecipeID);
				var[4].SetInt(DataB[i].SkinID);//Numero del skin default es 0
				var[5].SetString(cfg->m_StoreIcon);
				gfxMovie.Invoke("_root.api.Main.addSlotSkin", var, 6);

				if (bkp->ActualSkin == DataB[i].SkinID)
					SlotActive=Count;

				Count++;
			}
		}

		var[1].SetInt(SlotActive);
		gfxMovie.Invoke("_root.api.Main.setSlotActiveSkin", var, 2);
	}
}

