#include "r3dPCH.h"
#include "r3d.h"

#include "../../../Eternity/sf/Console/config.h"
#include "HUDDisplay.h"
#include "HUDPause.h"
#include "HUDVault.h"
#include "HUDSafelock.h"
#include "HUDTrade.h"
#include "HUDStore.h"
#include "HUDAttachments.h"
#include "LangMngr.h"
#include "HUDRepair.h"
#include "HUDSell.h"
#include "HUDCommCalls.h"

#include "FrontendShared.h"

#include "../multiplayer/clientgamelogic.h"
#include "../ObjectsCode/AI/AI_Player.H"
#include "../ObjectsCode/weapons/Weapon.h"
#include "../ObjectsCode/weapons/WeaponArmory.h"
#include "../GameLevel.h"

extern HUDPause*	hudPause;

HUDAttachments::HUDAttachments()
{
	isActive_ = false;
	isInit = false;
}

HUDAttachments::~HUDAttachments()
{
}

void HUDAttachments::eventSelectAttachment(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount)
{
	r3d_assert(argCount == 4);
	int id = args[0].GetInt();
	int slotID = args[1].GetInt();
	int attmID = args[2].GetInt();
	int SkinID = args[3].GetInt();

	obj_Player* plr = gClientLogic().localPlayer_;
	r3d_assert(plr);
	wiCharDataFull& slot = plr->CurLoadout;

	const WeaponConfig* wc = plr->m_Weapons[plr->m_SelectedWeapon]->getConfig();

	// check if attm is default attm
	{
		if ((wc->FPSDefaultID[id] == attmID || attmID == 0) && id != WPN_ATTM_CLIP)
		{
			plr->RemoveWpnAttm(plr->m_SelectedWeapon, id);

			Scaleform::GFx::Value var[2];
			var[0].SetInt(id);
			var[1].SetInt(slotID);
			gfxMovie.Invoke("_root.api.setSlotActive", var, 2);
			return;
		}
	}

	{
		if (id == 6)
		{
			if (plr->GetSkinsBought()>1)
				plr->ChangeWeaponSKIN(SkinID, 0);
			return;
		}
	}

	// find this attm in backpack
	for (int i = 0; i<slot.BackpackSize; ++i)
	{
		if (slot.Items[i].itemID == attmID)
		{
			const WeaponAttachmentConfig* wac = g_pWeaponArmory->getAttachmentConfig(attmID);

			if (slot.Items[i].Var3 > 0)
			{
				plr->EquipWpnAttm(plr->m_SelectedWeapon, i);

				Scaleform::GFx::Value var[2];
				var[0].SetInt(wac->m_type);
				var[1].SetInt(slotID);
				gfxMovie.Invoke("_root.api.setSlotActive", var, 2);
			}
			else {


				Scaleform::GFx::Value var[2];
				var[0].SetInt(wac->m_type);
				var[1].SetInt(Last_SlotID);
				gfxMovie.Invoke("_root.api.setSlotActive", var, 2);
			}

			// need to close attachment screen, otherwise reload animation don't work by some reasons
			if (wac->m_type == WPN_ATTM_CLIP)
				Deactivate();

			hudPause->reloadBackpackInfo();

			return;
		}
	}

	r3dOutToLog("!!! eventSelectAttachment: unknown attm??\n");
}

bool HUDAttachments::Init()
{
	if (!gfxMovie.Load("Data\\Menu\\WarZ_HUD_AttachUI.swf", false))
		return false;

#define MAKE_CALLBACK(FUNC) game_new r3dScaleformMovie::TGFxEICallback<HUDAttachments>(this, &HUDAttachments::FUNC)
	gfxMovie.RegisterEventHandler("eventSelectAttachment", MAKE_CALLBACK(eventSelectAttachment));

	gfxMovie.SetCurentRTViewport(Scaleform::GFx::Movie::SM_ExactFit);
	isActive_ = false;
	isInit = true;
	return true;
}

bool HUDAttachments::Unload()
{
	gfxMovie.Unload();
	isActive_ = false;
	isInit = false;
	return true;
}

void HUDAttachments::Update()
{
}

void HUDAttachments::Draw()
{
	gfxMovie.SetCurentRTViewport(Scaleform::GFx::Movie::SM_NoScale);


	gfxMovie.UpdateAndDraw();
}

void HUDAttachments::Deactivate()
{
	if (!g_cursor_mode->GetInt())
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

void HUDAttachments::Activate()
{
	r3d_assert(!isActive_);

	// check if user has weapon in his current slot
	obj_Player* plr = gClientLogic().localPlayer_;
	r3d_assert(plr);

	wiCharDataFull& slot = plr->CurLoadout;

	if (plr->m_SelectedWeapon != 0 && plr->m_SelectedWeapon != 1) // attm only for primary and secondary slots
		return;

	extern HUDPause*	hudPause;
	if (hudPause->isActive()) return;
	extern HUDVault*	hudVault;
	if (hudVault->isActive()) return;
	extern HUDRepair* hudRepair;
	if (hudRepair->isActive()) return;
	extern HUDStore*	hudStore;
	if (hudStore->isActive()) return;
	extern HUDSafelock* hudSafelock;
	if (hudSafelock->isActive()) return;
	extern HUDTrade* hudTrade;
	if (hudTrade->isActive()) return;
	extern HUDSell* hudSell;
	if (hudSell->isActive()) return;
	extern HUDCommCalls* hudCommCalls;
	extern HUDDisplay*	hudMain;
	if (hudCommCalls->isVisible()) 
		if (hudMain)hudMain->toggleCommandsPopUp(false);

	wiInventoryItem& inv = slot.Items[plr->m_SelectedWeapon];
	int var2_clip = inv.Var2;
	const WeaponConfig* wc = g_pWeaponArmory->getWeaponConfig(inv.itemID);
	if (!wc)
	{
		return; // no weapon - do not activate this screen
	}
	/*if(wc->category == storecat_MELEE)
		return;*/

	Last_SlotID = 0;

	if (g_camera_mode->GetInt() == 0) // switch player to FPS mode
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

	gfxMovie.Invoke("_root.api.clearAttachments", "");

	float screenW = 1920.0f;//r3dRenderer->ScreenW;
	float screenH = 1080.0f;//r3dRenderer->ScreenH;
	const r3dPoint2D attmSlotPos[WPN_ATTM_MAX + 1] = {
		r3dPoint2D(screenW*0.05f, screenH*0.05f), // WPN_ATTM_MUZZLE
		r3dPoint2D(screenW*0.6f, screenH*0.3f), // WPN_ATTM_UPPER_RAIL
		r3dPoint2D(screenW*0.1f, screenH*0.3f), // WPN_ATTM_LEFT_RAIL
		r3dPoint2D(screenW*0.2f, screenH*0.5f), // WPN_ATTM_BOTTOM_RAIL
		r3dPoint2D(screenW*0.3f, screenH*0.7f), // WPN_ATTM_CLIP
		r3dPoint2D(screenW*0.0f, screenH*0.0f), // WPN_ATTM_RECEIVER
		r3dPoint2D(screenW*0.0f, screenH*0.0f), // WPN_ATTM_STOCK
		r3dPoint2D(screenW*0.0f, screenH*0.0f),  // WPN_ATTM_BARREL
		r3dPoint2D(screenW*0.7f, screenH*0.7f) // WPN_SKIN
	};

	// Skins Attachment
	Scaleform::GFx::Value var1[6];
	var1[0].SetInt(6);
	var1[1].SetInt(int(attmSlotPos[8].x));
	var1[2].SetInt(int(attmSlotPos[8].y));
	var1[3].SetInt(6);
	gfxMovie.Invoke("_root.api.addAttachment", var1, 4);

	int CountSKIN = 0;
	
	for (int i = 0; i<MAX_SKINS_BY_WEAPON; i++)
	{
		int SkinID = plr->GetSkinsBought(inv.itemID, i); //Getting id Bought
		
		char Pic[256] = { 0 };
		sprintf(Pic, "$Data/Weapons/StoreIcons/noSkin.dds");
		const char* attmIcon = Pic;

		if (SkinID != 99)
		{
			char name[64];
			sprintf(name, "%d_SkinName", plr->m_Weapons[plr->m_SelectedWeapon]->Skins[SkinID].itemID);
			var1[1].SetInt(CountSKIN);
			if (plr->GetSkinsBought()>1)
			{
				var1[2].SetString(strdup(gLangMngr.getString(name)));
				var1[5].SetString(plr->m_Weapons[plr->m_SelectedWeapon]->Skins[SkinID].StoreIcon);
			}
			else
			{
				var1[2].SetString(strdup(gLangMngr.getString("$DefaultSkin")));
				var1[5].SetString(attmIcon);
			}
			var1[3].SetInt(plr->m_Weapons[plr->m_SelectedWeapon]->Skins[SkinID].itemID);
			var1[4].SetInt(SkinID);
			gfxMovie.Invoke("_root.api.addSlot", var1, 6);

			if (SkinID == plr->GetSkinID(0,plr->m_SelectedWeapon))
			{
				var1[1].SetInt(CountSKIN);
				gfxMovie.Invoke("_root.api.setSlotActive", var1, 2);
			}

			CountSKIN++;
		}

		if (plr->GetSkinsBought() <= 1)
			break;
	}

	///////

	if (wc->category == storecat_MELEE)
		return;

	// add attachment slots
	bool processed_current_clip_in_weapon = false;

	Scaleform::GFx::Value var[6];
	for (int attmType = 0; attmType<WPN_ATTM_RECEIVER; ++attmType)
	{
		int slotInc = 0;
		var[0].SetInt(attmType);
		var[1].SetInt(int(attmSlotPos[attmType].x));
		var[2].SetInt(int(attmSlotPos[attmType].y));
		var[3].SetInt(attmType);
		gfxMovie.Invoke("_root.api.addAttachment", var, 4);
		
		char Pic[256] = { 0 };
		sprintf(Pic, "$Data/Weapons/StoreIcons/noItem.png");
		const char* attmIcon = Pic;

		const WeaponAttachmentConfig* wac = NULL;
		// check for default attachment, if any
		const char* attmName = "NONE";
		int attmID = 0;
		if (wc->FPSDefaultID[attmType] != 0)
		{
			wac = g_pWeaponArmory->getAttachmentConfig(wc->FPSDefaultID[attmType]);
			// we need to check that we have default clip in backpack, as otherwise it will show it in menu, but you cannot equip it, as it is not in your backpack
			bool hasIt = true;
			if (attmType == WPN_ATTM_CLIP)
			{
				bool found = false;
				for (int a = 0; a < slot.BackpackSize; a++)
				{
					if (slot.Items[a].itemID == wc->FPSDefaultID[attmType])
					{
						found = true;
						break;
					}
				}
				if (!found)
					hasIt = false;
			}

			if (wac && hasIt)
			{
				
				attmID = wc->FPSDefaultID[attmType];
				attmName = wac->m_StoreName;
				attmIcon = wac->m_StoreIcon;
				if (attmType == WPN_ATTM_CLIP && !processed_current_clip_in_weapon && var2_clip == attmID)
					processed_current_clip_in_weapon = true;
			}
		}

		std::vector<uint32_t> attmAddedToList; // to keep track of what attm are added, so that not to add them again

		if ((attmID && attmType == WPN_ATTM_CLIP) || attmType != WPN_ATTM_CLIP)
		{
			var[1].SetInt(slotInc++);
			var[2].SetString(attmName);
			var[3].SetInt(attmID);
			var[4].SetInt(0);
			var[5].SetString(attmIcon);
			gfxMovie.Invoke("_root.api.addSlot", var, 6);
			attmAddedToList.push_back(attmID);
		}

		bool defaultSlotActive = true;
		if (attmType == WPN_ATTM_CLIP && attmID == 0)
			defaultSlotActive = false;

		// add current clip in gun, in case if it is last clip and no longer available in backpack
		if (attmType == WPN_ATTM_CLIP && !processed_current_clip_in_weapon && var2_clip > 0)
		{
			processed_current_clip_in_weapon = true;
			wac = g_pWeaponArmory->getAttachmentConfig(var2_clip);
			if (wac && std::find(attmAddedToList.begin(), attmAddedToList.end(), wac->m_itemID) == attmAddedToList.end())
			{
				var[1].SetInt(slotInc++);
				var[2].SetString(wac->m_StoreName);
				var[3].SetInt(wac->m_itemID);
				var[4].SetInt(0);
				var[5].SetString(wac->m_StoreIcon);
				gfxMovie.Invoke("_root.api.addSlot", var, 6);
				attmAddedToList.push_back(wac->m_itemID);

				if (slot.Attachment[plr->m_SelectedWeapon].attachments[attmType] == wac->m_itemID)
				{
					var[1].SetInt(slotInc - 1);
					gfxMovie.Invoke("_root.api.setSlotActive", var, 2);
					defaultSlotActive = false;
				}
			}
		}

		// go through backpack and look for attachments that fit this weapon
		for (int a = 0; a < slot.BackpackSize; a++)
		{
			if (slot.Items[a].itemID != 0 && (wac = g_pWeaponArmory->getAttachmentConfig(slot.Items[a].itemID)) != NULL)
			{
				if (wac->m_itemID != wc->FPSDefaultID[attmType] && wac->m_type == attmType && !(attmType == WPN_ATTM_CLIP && var2_clip == wac->m_itemID))
				{
					if (wac->m_specID == wc->FPSSpecID[attmType] && wc->FPSSpecID[attmType] > 0) // specID equal zero means nothing goes in it except for default attm
					{
						if (std::find(attmAddedToList.begin(), attmAddedToList.end(), wac->m_itemID) == attmAddedToList.end())
						{
							if (slot.Items[a].Var3>0)
							{
								var[1].SetInt(slotInc++);
								var[2].SetString(wac->m_StoreName);
								var[3].SetInt(wac->m_itemID);
								var[4].SetInt(0);
								var[5].SetString(wac->m_StoreIcon);
								gfxMovie.Invoke("_root.api.addSlot", var, 6);
								attmAddedToList.push_back(wac->m_itemID);
							}
						}
					}

					for (int x = 0; x<(int)attmAddedToList.size(); x++)
					{
						if (slot.Attachment[plr->m_SelectedWeapon].attachments[attmType] == attmAddedToList[x])
						{
							///	r3dOutToLog("######## enter 1 slotInc-1 %i slotInc %i\n",slotInc-1,slotInc);
							var[1].SetInt(x);
							gfxMovie.Invoke("_root.api.setSlotActive", var, 2);
							defaultSlotActive = false;
							break;
						}
					}
				}
			}
		}
		if (defaultSlotActive)
		{
			var[1].SetInt(0);
			gfxMovie.Invoke("_root.api.setSlotActive", var, 2);
		}
	}
}