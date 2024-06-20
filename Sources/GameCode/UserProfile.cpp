#include "r3dPCH.h"
#include "r3d.h"

#include "CkHttpRequest.h"
#include "CkHttp.h"
#include "CkHttpResponse.h"
#include "CkByteData.h"

#include "UserProfile.h"
#include "UserFriends.h"
#include "UserClans.h"
#include "UserServerRental.h"
#include "UserSkills.h"
#include "backend/WOBackendAPI.h"

#include "ObjectsCode/WEAPONS/WeaponConfig.h"
#include "ObjectsCode/WEAPONS/WeaponArmory.h"

#if defined(WO_SERVER) || defined(MASTER_SERVER)
#else
#include "SteamHelper.h"
#endif

#if defined(MASTER_SERVER)
#else
#if defined(MISSIONS) && defined(WO_SERVER)
#include "..\..\..\server\src\WO_GameServer\Sources\ObjectsCode\Missions\MissionManager.h"
extern Mission::MissionManager* g_pMissionMgr;
#endif
#endif

///////////////////////////////////////////////////////////////////////////////////////////////
#ifndef WO_SERVER
CClientUserProfile	gUserProfile;
#endif

bool storecat_IsItemStackable(uint32_t ItemID)
{
#if defined(MASTER_SERVER)
	return false;
#else
	const BaseItemConfig* itm = g_pWeaponArmory->getConfig(ItemID);
	if(!itm) 
		return true;
		
	// NOTE: MAKE SURE to sync this with WarZ.dbo.FN_IsItemStackable
	switch(itm->category)
	{
		case storecat_FPSAttachment:
		{
			// only clips can be stacked, all other attachments have durability now.
			// this is for preventing durability corruption in stacked attachments
			const WeaponAttachmentConfig* wac = (WeaponAttachmentConfig*)itm;
			if(wac->m_type == WPN_ATTM_CLIP)
				return true;
			return false;
		}

		case storecat_UsableItem:
		case storecat_Food:
		case storecat_Water:
		case storecat_GRENADE:
		case storecat_Components:
		case storecat_CraftRecipe:
			return true;
	}
	
	return false;
#endif
}

bool storecat_CanPlaceItemToSlot(const BaseItemConfig* itemCfg, int idx)
{
	bool canPlace = true;
	int cat = itemCfg->category;
	switch(idx)
	{
		case wiCharDataFull::CHAR_LOADOUT_WEAPON1:
			if(cat != storecat_ASR && cat != storecat_SNP && cat != storecat_SHTG && cat != storecat_MG && cat != storecat_SMG && cat != storecat_HG && cat != storecat_HG && cat !=storecat_SUPPORT)
				canPlace = false;
			break;
		case wiCharDataFull::CHAR_LOADOUT_WEAPON2:
			if(cat != storecat_MELEE && cat != storecat_HG)
				canPlace = false;
			break;
		case wiCharDataFull::CHAR_LOADOUT_ARMOR:
			if(cat != storecat_Armor)
				canPlace = false;
			break;
		case wiCharDataFull::CHAR_LOADOUT_HEADGEAR:
			if(cat != storecat_Helmet)
				canPlace = false;
			break;
		case wiCharDataFull::CHAR_LOADOUT_ITEM1:
		case wiCharDataFull::CHAR_LOADOUT_ITEM2:
		case wiCharDataFull::CHAR_LOADOUT_ITEM3:
		case wiCharDataFull::CHAR_LOADOUT_ITEM4:
			//if(cat != storecat_UsableItem)
			//	canPlace = false;
			canPlace = true;
			break;
	}
	
	return canPlace;
}

int storecat_GetItemBuyStackSize(uint32_t ItemID)
{
#if defined(MASTER_SERVER)
	return 0;
#else
	int quantityToAdd = 1;
	const WeaponConfig* wc = g_pWeaponArmory->getWeaponConfig(ItemID);
	const FoodConfig* fc = g_pWeaponArmory->getFoodConfig(ItemID);
	if(wc)
		quantityToAdd = wc->m_ShopStackSize;
	if(fc)
		quantityToAdd = fc->m_ShopStackSize;
		
	return quantityToAdd;
#endif
}

#if defined(MASTER_SERVER)
#else
#ifdef WO_SERVER	
void wiInventoryItem::ResetClipIfFull()
{
	const WeaponAttachmentConfig* atmCfg = g_pWeaponArmory->getAttachmentConfig(itemID);
	if(atmCfg && atmCfg->m_type == WPN_ATTM_CLIP && Var1 == atmCfg->m_Clipsize)
	{
		Var1 = -1;
	}
}
#endif
#endif

bool wiInventoryItem::CanStackWith(const wiInventoryItem& wi2, bool movingToGI/* = false*/) const
{
#if defined(MASTER_SERVER)
	return false;
#else
	// NOTE: MAKE SURE to sync this with WarZ.dbo.FN_IsItemStackable

	// we now allow stacking of 'non stackable' items in our global inventory
	if(!movingToGI && !storecat_IsItemStackable(itemID))
		return false;
	const BaseItemConfig* itm = g_pWeaponArmory->getConfig(itemID);
	if(!itm) 
		return false;

	// do not allow to stack weapons in GI
	//if(movingToGI && (itm->category >= storecat_ASR && itm->category <= storecat_SMG))
	//	return false;

	return (itemID == wi2.itemID && Var1 == wi2.Var1 && Var2 == wi2.Var2 && Var3 == wi2.Var3);
#endif
}

float wiCharDataFull::getTotalWeight() const
{
#if defined(MASTER_SERVER)
	return 0;
#else
	float totalWeight = 0.0f;
	for(int i=0; i<BackpackSize; ++i)
	{
		if (i == 0 || i == 1 || i == 6 || i == 7) // ignore weapon & armor slots
			continue;

		if(Items[i].itemID != 0 && Items[i].quantity > 0)
		{
			const BaseItemConfig* bic = g_pWeaponArmory->getConfig(Items[i].itemID);
			if(bic)
				totalWeight += bic->m_Weight * Items[i].quantity;
		}
	}
	return totalWeight;
#endif
}

CUserProfile::CUserProfile()
{
	memset((void *)&ProfileData, 0, sizeof(ProfileData));

	CustomerID = 0;
	username[0] = 0;
	email[0] = 0;

	SessionID = 0;
	AccountStatus = 0;
	ProfileDataDirty = 0;

	ProfileData.NumSlots = 0;
#if defined(MASTER_SERVER)
#else
#if defined(MISSIONS) && defined(WO_SERVER)
	for(int i = 0; i < wiUserProfile::MAX_LOADOUT_SLOTS; ++i)
	{
		// The prior memset wipes the missionsProgress pointer, so it is constructed as NULL,
		// and the pointer must be initialized after the last memset.  Deletion of the memory
		// is done when the obj_Player is destoryed, because the MissionsProgress structure
		// is needed for the life of the player, and destroying it in the destructor allows
		// CJobProcessUserJoin to destroy the MissionsProgress structure when the asynchronous
		// job is cleaned up.  When it is deleted, any loaded data goes with it.
		ProfileData.ArmorySlots[ i ].missionsProgress = game_new Mission::MissionsProgress;
	}
#endif
#endif
}

CUserProfile::~CUserProfile()
{
}

wiInventoryItem* CUserProfile::getInventorySlot(__int64 InventoryID)
{
	if(InventoryID == 0)
		return NULL;

	// todo: make it faster!
	for(uint32_t i=0; i<ProfileData.NumItems; ++i)
	{
		if(ProfileData.Inventory[i].InventoryID == InventoryID)
			return &ProfileData.Inventory[i];
	}

	return NULL;
}

bool CUserProfile::haveFreeInventorySlot(bool useLogicalLimit)
{
	int actualItems = 0;
	for(uint32_t i=0; i<ProfileData.NumItems; i++)
	{
		if(ProfileData.Inventory[i].itemID > 0)
			actualItems++;
	}

	if(useLogicalLimit)
	{
		if(actualItems < wiUserProfile::INVENTORY_SIZE_LIMIT)
			return true;
	}
	else
	{
		if(actualItems < wiUserProfile::MAX_INVENTORY_SIZE)
			return true;
	}
		
	return false;
}

wiInventoryItem* CUserProfile::getFreeInventorySlot()
{
	if(ProfileData.NumItems < wiUserProfile::MAX_INVENTORY_SIZE)
		return &ProfileData.Inventory[ProfileData.NumItems++];
		
	for(uint32_t i=0; i<ProfileData.NumItems; i++)
	{
		if(ProfileData.Inventory[i].itemID == 0)
			return &ProfileData.Inventory[i];
	}
	
	return NULL;
}

int CUserProfile::GetProfile(int CharID)
{
	CWOBackendReq req(this, "api_GetProfile1.aspx");
	if(CharID)
		req.AddParam("CharID", CharID);
		
	if(!req.Issue())
	{
		r3dOutToLog("GetProfile FAILED, code: %d\n", req.resultCode_);
		return req.resultCode_;
	}
	
	pugi::xml_document xmlFile;
	req.ParseXML(xmlFile);
	pugi::xml_node xmlAccount = xmlFile.child("account");
	uint32_t custID = xmlAccount.attribute("CustomerID").as_uint();
	if(custID == 0) // bad request
	{
		r3dOutToLog("Bad request in GetProfile()\n");
		return 9;
	}
	
	r3dscpy(email, xmlAccount.attribute("email").value());
	r3dscpy(username, xmlAccount.attribute("username").value());
	ProfileDataDirty              = xmlAccount.attribute("DataDirty").as_int();
	AccountStatus                 = xmlAccount.attribute("AccountStatus").as_int();
	ProfileData.AccountType       = xmlAccount.attribute("AccountType").as_int();
	ProfileData.GamePoints        = xmlAccount.attribute("GamePoints").as_int();
	ProfileData.GameDollars       = xmlAccount.attribute("GameDollars").as_int();
	ProfileData.isDevAccount      = xmlAccount.attribute("IsDeveloper").as_uint();
	ProfileData.IsMod = xmlAccount.attribute("IsMod").as_uint();
	ProfileData.TimePlayed        = xmlAccount.attribute("TimePlayed").as_uint();
	ProfileData.PremiumAcc        = xmlAccount.attribute("PremiumLeft").as_int();	
	ProfileData.ResWood           = xmlAccount.attribute("ResWood").as_int();
	ProfileData.ResStone          = xmlAccount.attribute("ResStone").as_int();
	ProfileData.ResMetal          = xmlAccount.attribute("ResMetal").as_int();
	ProfileData.BanTimeChat       = xmlAccount.attribute("BanChatLeft").as_int();//AlexRedd:: Ban chat
	ProfileData.BanChatCount      = xmlAccount.attribute("BanChatCount").as_int();//AlexRedd:: Ban chat
	ProfileData.SwearingCount	  = xmlAccount.attribute("ChatSwearingCount").as_int();//AlexRedd:: Auto ban chat
	ProfileData.RewardItemID	  = xmlAccount.attribute("RewardItemID").as_int();//AlexRedd:: Item Rewards System
	ProfileData.XP50Boosts        = xmlAccount.attribute("XP50BoostsLeft").as_int();
	ProfileData.showRewardPopUp	  = xmlAccount.attribute("ShowDailyRewardMessage").as_int();

	const char* curTime = xmlAccount.attribute("time").value();
	memset(&ServerTime, 0, sizeof(ServerTime));
	sscanf(curTime, "%d %d %d %d %d", 
		&ServerTime.tm_year, &ServerTime.tm_mon, &ServerTime.tm_mday,
		&ServerTime.tm_hour, &ServerTime.tm_min);
	ServerTime.tm_year -= 1900;
	ServerTime.tm_mon -= 1;
	ServerTime.tm_isdst = 1; // day light saving time

	// fill things
	ParseLoadouts(xmlAccount.child("chars"));
	ParseInventory(xmlAccount.child("inventory"));
	ParseBackpacks(xmlAccount.child("backpacks"));
	ParseWeaponSkins(xmlAccount.child("weaponskins"));//new add skin
	ParseGearSkins(xmlAccount.child("GearSkinData"));//new add skin

	return 0;
}

static void parseCharAttachments(const char* slotData, wiWeaponAttachment& attm)
{
	r3d_assert(slotData);
	if(*slotData == 0) 
	{
		memset(&attm, 0, sizeof(attm));
		return;
	}

	// should match arguments of ApiCharModifyAttachments
	int nargs = sscanf(slotData, "%d %d %d %d %d %d %d %d", 
		&attm.attachments[0], 
		&attm.attachments[1], 
		&attm.attachments[2], 
		&attm.attachments[3], 
		&attm.attachments[4], 
		&attm.attachments[5], 
		&attm.attachments[6], 
		&attm.attachments[7]);
	if(nargs != 8)
	{
		r3dOutToLog("Incorrect number of args in attachments %d\n", nargs);
		memset(&attm, 0, sizeof(attm));
	}
	
	return;
}

int CUserProfile::ApiLootBoxBuy(int itemId, int buyIdx, int qty)
{
	r3d_assert(buyIdx > 0);

	CWOBackendReq req(this, "api_MysteryBox.aspx");
	req.AddParam("func", "roll");
	req.AddParam("ItemID", itemId);
	req.AddParam("BuyIdx", buyIdx);
	req.AddParam("Qty", qty);
	
	if (!req.Issue())
	{
		r3dOutToLog("ApiLootBoxBuy FAILED, code: %d\n", req.resultCode_);
		return req.resultCode_;
	}

	int nargs = sscanf(req.bodyStr_, "%d", &ProfileData.lastWinID);
	if (nargs != 1)
	{
		r3dError("wrong answer for ApiMysteryBoxBuy");
		return 9;
	}

	return 0;
}

static void parseInventoryItem(pugi::xml_node xmlItem, wiInventoryItem& itm)
{
	itm.Reset();

	itm.InventoryID = xmlItem.attribute("id").as_int64();
	itm.itemID      = xmlItem.attribute("itm").as_uint();
	itm.quantity    = xmlItem.attribute("qt").as_uint();
	// if Var2/Var2/Var3 isn't supplied - set them -1 by default
	if(xmlItem.attribute("v1").value()[0])
		itm.Var1 = xmlItem.attribute("v1").as_int();
	else
		itm.Var1 = -1;
	if(xmlItem.attribute("v2").value()[0])
		itm.Var2 = xmlItem.attribute("v2").as_int();
	else
		itm.Var2 = -1;
	if(xmlItem.attribute("v3").value()[0])
		itm.Var3 = xmlItem.attribute("v3").as_int();
	else
		itm.Var3 = wiInventoryItem::MAX_DURABILITY;

	r3d_assert(itm.InventoryID > 0);
	r3d_assert(itm.itemID > 0);
	r3d_assert(itm.quantity > 0);
}

static void parseCharBackpack(pugi::xml_node xmlItem, wiCharDataFull& w)
{
	// enter into items list
	xmlItem = xmlItem.first_child();
	while(!xmlItem.empty())
	{
		wiInventoryItem itm;
		parseInventoryItem(xmlItem, itm);

		int slot = xmlItem.attribute("s").as_int();

		if(slot >= 0 && slot < w.BackpackSize)
		{
			// if item outside of slot for some reason, then skip it
			r3d_assert(w.Items[slot].InventoryID == 0);
			w.Items[slot] = itm;
		}

		xmlItem = xmlItem.next_sibling();
	}
}

void CUserProfile::ParseLoadouts(pugi::xml_node& xmlItem)
{
	// reset current backpacks
	for(int i=0; i<ProfileData.NumSlots; i++) {
		for(int j=0; j<ProfileData.ArmorySlots[i].BackpackSize; j++) {
			ProfileData.ArmorySlots[i].Items[j].Reset();
		}
	}

	ProfileData.NumSlots = 0;
	
	// parse all slots
	xmlItem = xmlItem.first_child();
	while(!xmlItem.empty())
	{
		wiCharDataFull& w = ProfileData.ArmorySlots[ProfileData.NumSlots++];
		wiStats& st = w.Stats;
		if(ProfileData.NumSlots > wiUserProfile::MAX_LOADOUT_SLOTS)
			r3dError("more than 6 profiles!");

		w.LoadoutID   = xmlItem.attribute("CharID").as_uint();
		r3dscpy(w.Gamertag, xmlItem.attribute("Gamertag").value());
		w.Alive       = xmlItem.attribute("Alive").as_int();
		w.Hardcore    = xmlItem.attribute("Hardcore").as_int();
		st.XP         = xmlItem.attribute("XP").as_int();
		st.SpendXP    = xmlItem.attribute("SpendXP").as_int();
		st.TimePlayed = xmlItem.attribute("TimePlayed").as_int();
		w.Health      = xmlItem.attribute("Health").as_float();
		w.Hunger      = xmlItem.attribute("Hunger").as_float();
		w.Thirst      = xmlItem.attribute("Thirst").as_float();
		w.Toxic       = xmlItem.attribute("Toxic").as_float();
		st.Reputation = xmlItem.attribute("Reputation").as_int();
		w.DeathUtcTime= xmlItem.attribute("DeathTime").as_int64();
		w.SecToRevive = xmlItem.attribute("SecToRevive").as_int();
		// premium account have 5 min revive time comparing to 10 min regular accounts
		if(ProfileData.PremiumAcc > 0 && w.Alive == 0)
			w.SecToRevive -= 300;

		w.GameMapId   = xmlItem.attribute("GameMapId").as_int();
		w.GameServerId= xmlItem.attribute("GameServerId").as_int();
		w.GamePos = r3dPoint3D(0, 0, 0);
		sscanf(xmlItem.attribute("GamePos").value(), "%f %f %f %f", &w.GamePos.x, &w.GamePos.y, &w.GamePos.z, &w.GameDir);
		w.GameFlags   = xmlItem.attribute("GameFlags").as_int();
		w.SecFromLastGame = xmlItem.attribute("SecFromLastGame").as_int64();

		w.HeroItemID  = xmlItem.attribute("HeroItemID").as_int();
		w.HeadIdx     = xmlItem.attribute("HeadIdx").as_int();
		w.BodyIdx     = xmlItem.attribute("BodyIdx").as_int();
		w.LegsIdx     = xmlItem.attribute("LegsIdx").as_int();

		w.ClanID      = xmlItem.attribute("ClanID").as_int();
		w.ClanRank    = xmlItem.attribute("ClanRank").as_int();
		r3dscpy(w.ClanTag, xmlItem.attribute("ClanTag").value());
		w.ClanTagColor= xmlItem.attribute("ClanTagColor").as_int();
		w.FriendlyFire      = xmlItem.attribute("FriendlyFire").as_int();//AlexRedd:: FriendlyFire system

		COMPILE_ASSERT(sizeof(w.Skills)/sizeof(w.Skills[0]) > CUserSkills::SKILL_ID_END);
		const char* skills = xmlItem.attribute("Skills").value();
		int skLen = R3D_MIN((int)strlen(skills), (int)CUserSkills::SKILL_ID_END);
		for(int i=0; i<skLen; i++)
		{
			if(skills[i] >= '0' && skills[i] <= '9')
				w.Skills[i] = skills[i] - '0';
		}

#if defined(MASTER_SERVER)
#else
#if defined(MISSIONS) && defined(WO_SERVER)
		ParseMissionData(w, xmlItem);
#endif
#endif

		const char* attm1 = xmlItem.attribute("attm1").value();
		const char* attm2 = xmlItem.attribute("attm2").value();
		parseCharAttachments(attm1, w.Attachment[0]);
		parseCharAttachments(attm2, w.Attachment[1]);

		w.BackpackID   = xmlItem.attribute("BackpackID").as_uint();
		w.BackpackSize = xmlItem.attribute("BackpackSize").as_int();
		
		// trackable stats
		st.KilledZombies   = xmlItem.attribute("ts00").as_int();
		st.KilledSurvivors = xmlItem.attribute("ts01").as_int();
		st.KilledBandits   = xmlItem.attribute("ts02").as_int();
		st.VictorysBattleRoyale  = xmlItem.attribute("ts03").as_int();//AlexRedd:: BR mode

		ParseCharData(w, xmlItem.child("CharData"));

		xmlItem = xmlItem.next_sibling();
	}
}

bool g_warn_about_full_inventory = false;
void CUserProfile::ParseInventory(pugi::xml_node& xmlItem)
{
	ProfileData.NumItems = 0;

	// enter into items list
	xmlItem = xmlItem.first_child();
	while(!xmlItem.empty())
	{
		wiInventoryItem* itm = getFreeInventorySlot();
		if(!itm)
		{
			r3dOutToLog("--- we don't have any more inventory slots, skipping rest of inventory\n");
			g_warn_about_full_inventory = true;
			return;
		}
		
		parseInventoryItem(xmlItem, *itm);

		xmlItem = xmlItem.next_sibling();
	}
}

void CUserProfile::ParseBackpacks(pugi::xml_node& xmlItem)
{
	// enter into items list
	xmlItem = xmlItem.first_child();
	while(!xmlItem.empty())
	{
		uint32_t CharID  = xmlItem.attribute("CharID").as_uint();
		r3d_assert(CharID);
		
		bool found = true;
		for(int i=0; i<ProfileData.NumSlots; i++) 
		{
			if(ProfileData.ArmorySlots[i].LoadoutID == CharID) 
			{
				parseCharBackpack(xmlItem, ProfileData.ArmorySlots[i]);
				found = true;
				break;
			}
		}
		
		if(!found)
			r3dError("bad backpack data for charid %d", CharID);

		xmlItem = xmlItem.next_sibling();
	}
}
//new add skin
void CUserProfile::ParseGearSkins(pugi::xml_node& xmlItem)
{
	for (int i = 0; i<ProfileData.NumSlots; i++)
	{
		wiCharDataFull& w = ProfileData.ArmorySlots[i];
		ParseGearSkinData(w, xmlItem);
	}
}

void CUserProfile::ParseWeaponSkins(pugi::xml_node& xmlItem)
{
	for (int i = 0; i<MAX_WEAPONS_SKINEABLED; i++) {
		ProfileData.WpnSKIN[i].Reset();
	}

	xmlItem = xmlItem.first_child();

	int WpnCount = 0;

	while (!xmlItem.empty())
	{
		int itemID = xmlItem.attribute("itemID").as_uint();
		int Skin = xmlItem.attribute("Skins").as_uint();
		int Selected = xmlItem.attribute("Selected").as_uint();

		bool wpnExist = false;
		int Slot = 0;
		for (int i = 0; i<999; i++)
		{
			WeaponSkins& w = ProfileData.WpnSKIN[i];

			if (w.itemID == itemID)
			{
				wpnExist = true;
				Slot = i;
				w.Skins[Skin] = Skin;
				w.LastSKINused = Selected;
				break;
			}
		}
		if (!wpnExist)
		{
			WeaponSkins& w = ProfileData.WpnSKIN[WpnCount];
			w.itemID = itemID;
			w.LastSKINused = Selected;
			w.Skins[Skin] = Skin;
			WpnCount++;
		}

		xmlItem = xmlItem.next_sibling();
	}

}

void CUserProfile::ParseGearSkinData(wiCharDataFull& loadout, pugi::xml_node& xmlItem)
{

	// parse recipes
	pugi::xml_node xmlGearSkin = xmlItem.child("recipes");
	loadout.NumGearSkinRecipes = xmlGearSkin.attribute("n").as_int();
	if (loadout.NumGearSkinRecipes >= R3D_ARRAYSIZE(loadout.GearSkinRecipesLearned))
		r3dError("too many recipes");

	for (int i = 0; i<loadout.NumGearSkinRecipes; i++) // gear Skins DB
	{
		char buf[32];
		sprintf(buf, "r%d", i);
		loadout.GearSkinRecipesLearned[i] = xmlGearSkin.attribute(buf).as_uint();
	}
}
//new add skin
void CUserProfile::ParseCharData(wiCharDataFull& loadout, pugi::xml_node& xmlItem)
{
	// parse recipes
	pugi::xml_node xmlRecipes = xmlItem.child("recipes");
	loadout.NumRecipes = xmlRecipes.attribute("n").as_int();
	if(loadout.NumRecipes >= R3D_ARRAYSIZE(loadout.RecipesLearned)) 
		r3dError("too many recipes");

	for(int i=0; i<loadout.NumRecipes; i++)
	{
		char buf[32];
		sprintf(buf, "r%d", i);
		loadout.RecipesLearned[i] = xmlRecipes.attribute(buf).as_uint();
	}

	// parse stats
	pugi::xml_node xmlStats = xmlItem.child("stats");
	loadout.Stats.Kills = xmlStats.attribute("k").as_uint();
	loadout.Stats.Deaths = xmlStats.attribute("d").as_uint();
	loadout.Stats.Headshots = xmlStats.attribute("h").as_uint();
	loadout.Stats.ShotsFired = xmlStats.attribute("sf").as_uint();
	loadout.Stats.ShotsHits = xmlStats.attribute("sh").as_uint();
	loadout.Stats.ShotsHeadshots = xmlStats.attribute("shs").as_uint();

	// med system
	pugi::xml_node xmlMedSystem = xmlItem.child("medsys");
	loadout.MedBleeding = xmlMedSystem.attribute("bl").as_float();
	loadout.MedFeverCold = xmlMedSystem.attribute("fev").as_float();
	//loadout.MedFoodPoison = xmlMedSystem.attribute("fpoi").as_float();
	loadout.MedBloodInfection = xmlMedSystem.attribute("blinf").as_float();
}

#ifdef WO_SERVER
void CUserProfile::SaveCharData(const wiCharDataFull& loadout, pugi::xml_node& xmlItem)
{
	// save recipes
	pugi::xml_node xmlRecipes = xmlItem.append_child();
	xmlRecipes.set_name("recipes");
	xmlRecipes.append_attribute("n") = loadout.NumRecipes;

	for(int i=0; i<loadout.NumRecipes; i++)
	{
		char buf[32];
		sprintf(buf, "r%d", i);
		xmlRecipes.append_attribute(buf) = loadout.RecipesLearned[i];
	}

	// save additional stats
	pugi::xml_node xmlStats = xmlItem.append_child();
	xmlStats.set_name("stats");
	xmlStats.append_attribute("k") = loadout.Stats.Kills;
	xmlStats.append_attribute("d") = loadout.Stats.Deaths;
	xmlStats.append_attribute("h") = loadout.Stats.Headshots;
	xmlStats.append_attribute("sf") = loadout.Stats.ShotsFired;
	xmlStats.append_attribute("sh") = loadout.Stats.ShotsHits;
	xmlStats.append_attribute("shs") = loadout.Stats.ShotsHeadshots;

	// med system
	pugi::xml_node xmlMedSystem = xmlItem.append_child();
	xmlMedSystem.set_name("medsys");
	xmlMedSystem.append_attribute("bl") = loadout.MedBleeding;
	xmlMedSystem.append_attribute("fev") = loadout.MedFeverCold;
	//xmlMedSystem.append_attribute("fpoi") = loadout.MedFoodPoison;
	xmlMedSystem.append_attribute("blinf") = loadout.MedBloodInfection;
}
//new add skin
void CUserProfile::SaveGearSkinData(const wiCharDataFull& loadout, pugi::xml_node& xmlItem)
{
	// save recipes
	pugi::xml_node xmlRecipes = xmlItem.append_child();
	xmlRecipes.set_name("recipes");
	xmlRecipes.append_attribute("n") = loadout.NumGearSkinRecipes;

	for(int i=0; i<loadout.NumGearSkinRecipes; i++)
	{
		char buf[32];
		sprintf(buf, "r%d", i);
		xmlRecipes.append_attribute(buf) = loadout.GearSkinRecipesLearned[i];
	}
}
//new add skin
#endif
#if defined(MASTER_SERVER)
#else
#if defined(MISSIONS) && defined(WO_SERVER)
void CUserProfile::ParseMissionData(wiCharDataFull& loadout, pugi::xml_node& xmlItem)
{
	if( Mission::g_pMissionMgr )
	{
		if( loadout.missionsProgress )
			loadout.missionsProgress->ParseMissionProgress( xmlItem );
	}
}

void CUserProfile::SaveMissionData(const wiCharDataFull& loadout, pugi::xml_node& xmlItem)
{
	if( Mission::g_pMissionMgr )
	{
		if( loadout.missionsProgress )
			loadout.missionsProgress->SaveMissionProgress( xmlItem );
	}
}
#endif
#endif


wiStoreItem g_StoreItems[MAX_NUM_STORE_ITEMS] = {0}; 
uint32_t g_NumStoreItems = 0;

int CUserProfile::ApiGetShopData()
{
	CWOBackendReq req(this, "api_GetShop2.aspx");
	if(!req.Issue())
	{
		r3dOutToLog("GetShopData FAILED, code: %d\n", req.resultCode_);
		return req.resultCode_;
	}

	g_NumStoreItems = 0;

	const char* d = req.bodyStr_;
	const char* p = d;
	if(p[0] != 'S' || p[1] != 'H' || p[2] != 'O' || p[3] != '1') {
		r3dOutToLog("GetShopData: bad answer #1\n");
		return 9;
	}
	p += 4;

	// skills. served as skillID/int[5]
	while(true)
	{
		if((p - d) >= req.bodyLen_) {
			r3dOutToLog("GetShopData: bad answer #3\n");
			return 9;
		}

		WORD skillID = *(WORD*)p; p += 2;
		if(skillID == 0xFFFF)
			break;
		r3d_assert(skillID < CUserSkills::SKILL_ID_END);

		COMPILE_ASSERT(sizeof(ShopSkillCosts)/5/sizeof(int) > CUserSkills::SKILL_ID_END);
		ShopSkillCosts[skillID][0] = *(int*)p;  p += 4;
		ShopSkillCosts[skillID][1] = *(int*)p;  p += 4;
		ShopSkillCosts[skillID][2] = *(int*)p;  p += 4;
		ShopSkillCosts[skillID][3] = *(int*)p;  p += 4;
		ShopSkillCosts[skillID][4] = *(int*)p;  p += 4;
	}

	// shop items
	while(1) 
	{
		if((p - d) >= req.bodyLen_) {
			r3dOutToLog("GetShopData: bad answer #2\n");
			return 9;
		}

		// end tag
		if(p[0] == 'S' && p[1] == 'H' && p[2] == 'O' && p[3] == '1')
			break;

		// item
		DWORD itemId       = *(DWORD*)p; p += 4;
		DWORD flags        = *(BYTE*)p;  p += 1;
		DWORD pricePerm    = *(DWORD*)p; p += 4;
		DWORD gd_pricePerm = *(DWORD*)p; p += 4;

		wiStoreItem& itm = g_StoreItems[g_NumStoreItems++];
		r3d_assert(g_NumStoreItems < MAX_NUM_STORE_ITEMS);
		itm.itemID       = itemId;
		itm.pricePerm    = pricePerm;
		itm.gd_pricePerm = gd_pricePerm;
		itm.isNew        = (flags & 0x1) ? true : false;
	}

	DeriveGamePricesFromItems();
	
/* exported shop for xsolla future use
#ifndef FINAL_BUILD
#ifndef WO_SERVER
	if(0)
	{
		FILE* f = fopen_for_write("warz_xsolla_item_shop.xml", "wt");
		fprintf(f, "<?xml version=\"1.0\"?>\n");
		fprintf(f, "<WarzItemShop>\n");
		for(uint32_t i=0; i<g_NumStoreItems; i++)
		{
			wiStoreItem& itm = g_StoreItems[i];
			if(itm.pricePerm == 0) continue;

			const BaseItemConfig* cfg = g_pWeaponArmory->getConfig(itm.itemID);
			if(!cfg) continue;
			
			fprintf(f, "\t<item>\n");
			fprintf(f, "\t\t<id>%d</id>\n", itm.itemID);
			fprintf(f, "\t\t<name>%s</name>\n", cfg->m_StoreName);
			fprintf(f, "\t\t<price>%.1f</price>\n", R3D_MAX(0.1f, (float)itm.pricePerm / 140.0f));
			fprintf(f, "\t</item>\n");
		}
		fprintf(f, "</WarzItemShop>\n");
		fclose(f);
	}
#endif	
#endif
*/

	return 0;
}

void CUserProfile::DeriveGamePricesFromItems()
{
	for(uint32_t i = 0; i<g_NumStoreItems; i++) 
	{
		const wiStoreItem& itm = g_StoreItems[i];

		switch(itm.itemID) 
		{
			case 301151: ShopClanCreate = itm.pricePerm; break;
			case 301159:				
				{
					bool hasPrice = false;
					hasPrice = g_StoreItems[i].hasAnyPrice();
					if(hasPrice)
					{
						if(itm.gd_pricePerm>0){
							ShopCharReviveGD = itm.gd_pricePerm;
							break;
						}
						else if(itm.pricePerm>0){
							ShopCharReviveGC = itm.pricePerm;
							break;
						}
					}
				}
			case 301257: ShopPremiumAcc = itm.pricePerm; break;
			case 301399: ShopCharRename = itm.pricePerm; break;
			case 301400: GD_REWARD_BR	= itm.gd_pricePerm; break;//AlexRedd:: BR mode
			case 301401:
				{
					bool hasPrice = false;
					hasPrice = g_StoreItems[i].hasAnyPrice();
					if(hasPrice)
					{
						if(itm.gd_pricePerm>0){
							ShopCharTeleportGD = itm.gd_pricePerm;
							break;
						}
						else if(itm.pricePerm>0){
							ShopCharTeleportGC = itm.pricePerm;
							break;
						}
					}
				}
				break;
			case 301402: ShopLegendAcc = itm.pricePerm; break;
			case 301403: ShopPioneerAcc = itm.pricePerm; break;
			case 301433: ShopRespawnSafeZone = itm.gd_pricePerm; break;
			case 301434: ShopRespawnWorld = itm.gd_pricePerm; break;
			case 301435: ShopRespawnNear = itm.gd_pricePerm; break;
			case 301436: ShopRespawnGroup = itm.gd_pricePerm; break;
		}
		
		// clan add members items
		if(itm.itemID >= 301152 && itm.itemID <= 301157)
		{
			ShopClanAddMembers_GP[itm.itemID - 301152]  = itm.pricePerm;
			ShopClanAddMembers_Num[itm.itemID - 301152] = itm.gd_pricePerm;
		}
	}
}

#ifndef WO_SERVER

class GameObject;
#include "ObjectsCode/Weapons/WeaponArmory.h"

CClientUserProfile::CClientUserProfile()
{
	steamCallbacks = NULL;
	friends = game_new CUserFriends();

	for(int i=0; i<wiUserProfile::MAX_LOADOUT_SLOTS; i++)
		clans[i] = game_new CUserClans();
		
	serverRent = game_new CUserServerRental();
	
	SelectedCharID = 0;

	ShopClanCreate = 0;
	ShopCharReviveGC = 0;
	ShopCharReviveGD = 0;
	ShopPremiumAcc = 0;
	ShopCharRename = 0;
	GD_REWARD_BR = 0;//AlexRedd:: BR mode
	ShopCharTeleportGD = 0;
	ShopCharTeleportGC = 0;
	ShopLegendAcc = 0;
	ShopPioneerAcc = 0;
	ShopRespawnSafeZone = 0;
	ShopRespawnWorld = 0;
	ShopRespawnNear = 0;
	ShopRespawnGroup = 0;
	memset(&ShopClanAddMembers_GP, 0, sizeof(ShopClanAddMembers_GP));
	memset(&ShopClanAddMembers_Num, 0, sizeof(ShopClanAddMembers_Num));
	memset(&ShopSkillCosts, 0, sizeof(ShopSkillCosts));
	
	m_GCPriceTable[0] = 0;
	m_GCPriceTable[1] = 0;
}

CClientUserProfile::~CClientUserProfile()
{
	SAFE_DELETE(friends);

	for(int i=0; i<wiUserProfile::MAX_LOADOUT_SLOTS; i++)
		SAFE_DELETE(clans[i]);
		
	SAFE_DELETE(serverRent);
}

void CClientUserProfile::GenerateSessionKey(char* outKey)
{
	char sessionInfo[128];
	sprintf(sessionInfo, "%d:%d", CustomerID, SessionID);
	for(size_t i=0; i<strlen(sessionInfo); ++i)
		sessionInfo[i] = sessionInfo[i]^0x64;
	
	CkString str;
	str = sessionInfo;
	str.base64Encode("utf-8");
	
	strcpy(outKey, str.getUtf8());
	return;
}

// special code that'll replace name/description/icon for specified item
template <class T>
static void replaceItemNameParams(T* itm, pugi::xml_node& xmlNode)
{
	const char* name = xmlNode.attribute("name").value();
	const char* desc = xmlNode.attribute("desc").value();
	const char* fname = xmlNode.attribute("fname").value();
	
	// replace description
	if(strcmp(desc, itm->m_Description) != 0)
	{
		free(itm->m_Description);

		itm->m_Description = strdup(desc);
	}
	
	// replace name
	if(strcmp(name, itm->m_StoreName) != 0)
	{
		free(itm->m_StoreName);

		itm->m_StoreName = strdup(name);
	}
	
	// replace store icon (FNAME)
	char storeIcon[256];
	sprintf(storeIcon, "$Data/Weapons/StoreIcons/%s.dds", fname);
	if(strcmp(storeIcon, itm->m_StoreIcon) != 0)
	{
		free(itm->m_StoreIcon);
		itm->m_StoreIcon = strdup(storeIcon);
	}
}

int CClientUserProfile::ApiGetItemsInfo()
{
	CWOBackendReq req(this, "api_GetItemsInfo.aspx");
	if(!req.Issue())
	{
		r3dOutToLog("GetItemsInfo FAILED, code: %d\n", req.resultCode_);
		return req.resultCode_;
	}

	pugi::xml_document xmlFile;
	req.ParseXML(xmlFile);
	
	pugi::xml_node xmlItems = xmlFile.child("items");

	// read gears (in <gears><g>...)
	pugi::xml_node xmlNode = xmlItems.child("gears").first_child();
	while(!xmlNode.empty())
	{
		uint32_t itemId = xmlNode.attribute("ID").as_uint();
		GearConfig* gc = const_cast<GearConfig*>(g_pWeaponArmory->getGearConfig(itemId));
		if(gc)
		{
			gc->m_Weight        = xmlNode.attribute("wg").as_float() / 1000.0f; 
			gc->m_damagePerc    = xmlNode.attribute("dp").as_float() / 100.0f;
			gc->m_damageMax     = xmlNode.attribute("dm").as_float();
		}

		xmlNode = xmlNode.next_sibling();
	}

	// read weapons (in <weapons><w>...)
	xmlNode = xmlItems.child("weapons").first_child();
	while(!xmlNode.empty())
	{
		uint32_t itemId = xmlNode.attribute("ID").as_uint();
		WeaponConfig* wc = const_cast<WeaponConfig*>(g_pWeaponArmory->getWeaponConfig(itemId));
		if(wc)
		{
			wc->m_AmmoDamage    = xmlNode.attribute("d1").as_float();
			wc->m_AmmoDecay     = xmlNode.attribute("d2").as_float();
			wc->m_fireDelay     = 60.0f / (xmlNode.attribute("rf").as_float());
			wc->m_spread        = xmlNode.attribute("sp").as_float();
			wc->m_recoil        = xmlNode.attribute("rc").as_float();
		}

		xmlNode = xmlNode.next_sibling();
	}

	// read packages(in <packages><p>...)
	/*xmlNode = xmlItems.child("packages").first_child();
	while(!xmlNode.empty())
	{
		uint32_t itemId = xmlNode.attribute("ID").as_uint();
		PackageConfig* pc = const_cast<PackageConfig*>(g_pWeaponArmory->getPackageConfig(itemId));
		if(pc)
		{
			replaceItemNameParams<PackageConfig>(pc, xmlNode);

			pc->m_addGD = xmlNode.attribute("gd").as_int();
			pc->m_addSP = xmlNode.attribute("sp").as_int();
			pc->m_itemID1    = xmlNode.attribute("i1i").as_int();
			pc->m_itemID1Exp = xmlNode.attribute("i1e").as_int();
			pc->m_itemID2    = xmlNode.attribute("i2i").as_int();
			pc->m_itemID2Exp = xmlNode.attribute("i2e").as_int();
			pc->m_itemID3    = xmlNode.attribute("i3i").as_int();
			pc->m_itemID3Exp = xmlNode.attribute("i3e").as_int();
			pc->m_itemID4    = xmlNode.attribute("i4i").as_int();
			pc->m_itemID4Exp = xmlNode.attribute("i4e").as_int();
			pc->m_itemID5    = xmlNode.attribute("i5i").as_int();
			pc->m_itemID5Exp = xmlNode.attribute("i5e").as_int();
			pc->m_itemID6    = xmlNode.attribute("i6i").as_int();
			pc->m_itemID6Exp = xmlNode.attribute("i6e").as_int();
		}

		xmlNode = xmlNode.next_sibling();
	}*/

	// read mystery crates/loot boxes names
	xmlNode = xmlItems.child("generics").first_child();
	while(!xmlNode.empty())
	{
		uint32_t itemId = xmlNode.attribute("ID").as_uint();
		ModelItemConfig* itm = const_cast<ModelItemConfig*>(g_pWeaponArmory->getAccountItemConfig(itemId));
		if(itm)
		{
			replaceItemNameParams<ModelItemConfig>(itm, xmlNode);
		}

		xmlNode = xmlNode.next_sibling();
	}

	return 0;
}

int CClientUserProfile::ApiCharCreate(const char* Gamertag, int Hardcore, int HeroItemID, int HeadIdx, int BodyIdx, int LegsIdx)
{
	CWOBackendReq req(this, "api_CharSlots.aspx");
	req.AddParam("func",       "create");
	req.AddParam("Gamertag",   Gamertag);
	req.AddParam("Hardcore",   Hardcore);
	req.AddParam("HeroItemID", HeroItemID);
	req.AddParam("HeadIdx",    HeadIdx);
	req.AddParam("BodyIdx",    BodyIdx);
	req.AddParam("LegsIdx",    LegsIdx);
	if(!req.Issue())
	{
		r3dOutToLog("ApiCharCreate failed: %d", req.resultCode_);
		return req.resultCode_;
	}

	// reread profile
	GetProfile();

	return 0;
}

int CClientUserProfile::ApiCharChangeSkin(int HeadIdx, int BodyIdx, int LegsIdx, int HeroItemID)
{
	r3d_assert(SelectedCharID >= 0 && SelectedCharID < wiUserProfile::MAX_LOADOUT_SLOTS);
	wiCharDataFull& w = ProfileData.ArmorySlots[SelectedCharID];
	r3d_assert(w.LoadoutID > 0);

	CWOBackendReq req(this, "api_CharSlots.aspx");
	req.AddParam("func", "skin");
	req.AddParam("CharID", w.LoadoutID);
	req.AddParam("HeadIdx", HeadIdx);
	req.AddParam("BodyIdx", BodyIdx);
	req.AddParam("LegsIdx", LegsIdx);
	req.AddParam("HeroItemID", HeroItemID);
	if (!req.Issue())
	{
		r3dOutToLog("ApiCharChangeSkin failed: %d", req.resultCode_);
		return req.resultCode_;
	}

	// if all is good, update local data
	w.HeadIdx = HeadIdx;
	w.BodyIdx = BodyIdx;
	w.LegsIdx = LegsIdx;
	w.HeroItemID = HeroItemID;

	return 0;
}

int CClientUserProfile::ApiCharRename(const char* Gamertag, int* out_MinutesLeft)
{
	r3d_assert(SelectedCharID >= 0 && SelectedCharID < wiUserProfile::MAX_LOADOUT_SLOTS);
	wiCharDataFull& w = ProfileData.ArmorySlots[SelectedCharID];
	r3d_assert(w.LoadoutID > 0);

	CWOBackendReq req(this, "api_CharSlots.aspx");
	req.AddParam("func",     "rename");
	req.AddParam("CharID",   w.LoadoutID);
	req.AddParam("Gamertag", Gamertag);
	if(!req.Issue())
	{
		if(req.resultCode_ == 4)
		{
			int nargs = sscanf(req.bodyStr_, "%d", out_MinutesLeft);
			r3d_assert(nargs == 1);
		}
		r3dOutToLog("ApiCharRename failed: %d", req.resultCode_);
		return req.resultCode_;
	}
	
	r3dscpy(w.Gamertag, Gamertag);
	ProfileData.GamePoints -= ShopCharRename;
	return 0;
}


int CClientUserProfile::ApiCharDelete()
{
	r3d_assert(SelectedCharID >= 0 && SelectedCharID < wiUserProfile::MAX_LOADOUT_SLOTS);
	wiCharDataFull& w = ProfileData.ArmorySlots[SelectedCharID];
	r3d_assert(w.LoadoutID > 0);

	CWOBackendReq req(this, "api_CharSlots.aspx");
	req.AddParam("func",   "delete");
	req.AddParam("CharID", w.LoadoutID);
	if(!req.Issue())
	{
		r3dOutToLog("ApiCharDelete failed: %d", req.resultCode_);
		return req.resultCode_;
	}

	w.LoadoutID = 0;

	if(GetProfile() != 0)
		return false;

	return 0;
}

int CClientUserProfile::ApiCharReviveCheck(int* out_needMoney, int* out_secToRevive)
{
	r3d_assert(SelectedCharID >= 0 && SelectedCharID < wiUserProfile::MAX_LOADOUT_SLOTS);
	wiCharDataFull& w = ProfileData.ArmorySlots[SelectedCharID];
	r3d_assert(w.LoadoutID > 0);

	CWOBackendReq req(this, "api_CharSlots.aspx");
	req.AddParam("func",   "revcheck");
	req.AddParam("CharID", w.LoadoutID);
	if(!req.Issue())
	{
		r3dOutToLog("ApiCharReviveCheck failed: %d", req.resultCode_);
		return req.resultCode_;
	}

	int nargs = sscanf(req.bodyStr_, "%d %d", out_needMoney, out_secToRevive);
	r3d_assert(nargs == 2);

	return 0;
}

int CClientUserProfile::ApiCharRevive(int NeedMoney)
{
	r3d_assert(SelectedCharID >= 0 && SelectedCharID < wiUserProfile::MAX_LOADOUT_SLOTS);
	wiCharDataFull& w = ProfileData.ArmorySlots[SelectedCharID];
	r3d_assert(w.LoadoutID > 0);

	CWOBackendReq req(this, "api_CharSlots.aspx");
	req.AddParam("func",   "revive");
	req.AddParam("CharID", w.LoadoutID);
	req.AddParam("NeedMoney", NeedMoney); // client must ack that it understand revive for cash. in case of theoretical missyncs
	req.AddParam("BuyIdx", gUserProfile.ShopCharReviveGC>0?4:5);
	if(!req.Issue())
	{
		r3dOutToLog("ApiCharRevive failed: %d", req.resultCode_);
		return req.resultCode_;
	}

	// reread profile
	GetProfile();
	
	return 0;
}

int CClientUserProfile::ApiSkillLearn(int SkillID, int SkillLevel)
{
	r3d_assert(SelectedCharID >= 0 && SelectedCharID < wiUserProfile::MAX_LOADOUT_SLOTS);
	wiCharDataFull& w = ProfileData.ArmorySlots[SelectedCharID];
	
	r3d_assert(SkillID >= 0 && SkillID < CUserSkills::SKILL_ID_END);
	r3d_assert(SkillLevel >= 1 && SkillLevel <= 5);
	r3d_assert(ShopSkillCosts[SkillID][SkillLevel - 1] != 0);

	CWOBackendReq req(this, "api_Skills.aspx");
	req.AddParam("func",       "learn");
	req.AddParam("CharID",     w.LoadoutID);
	req.AddParam("SkillID",    SkillID);
	req.AddParam("SkillLevel", SkillLevel);
	if(!req.Issue())
	{
		r3dOutToLog("ApiSkillLearn FAILED, code: %d\n", req.resultCode_);
		return req.resultCode_;
	}

	w.Stats.SpendXP  += ShopSkillCosts[SkillID][SkillLevel - 1];
	w.Skills[SkillID] = SkillLevel;
	return 0;
}

int CClientUserProfile::ApiUpgradeTrialAccount(const char* code)
{
	r3d_assert(code);
	CWOBackendReq req(this, "api_AccApplyKey.aspx");
	req.AddParam("serial", code);
	if(!req.Issue())
	{
		r3dOutToLog("ApiUpgradeTrialAccount FAILED, code: %d\n", req.resultCode_);
		return req.resultCode_;
	}

	return 0;
}

int CClientUserProfile::ApiGetGamePointsConvertsionRates()
{
	m_GPConvertRates.clear();

	CWOBackendReq req(this, "api_GPConvert.aspx");
	req.AddParam("func", "rates");
	if(gSteam.steamID > 0) req.AddParam("steam", "1");
	if(!req.Issue())
	{
		r3dOutToLog("ApiGetGamePointsConvertsionRates failed: %d", req.resultCode_);
		return req.resultCode_;
	}

	pugi::xml_document xmlFile;
	req.ParseXML(xmlFile);
	
	pugi::xml_node xmlItem = xmlFile.child("rates").first_child();
	while(!xmlItem.empty())
	{
		gp2gd_s g;
		g.GamePoints     = xmlItem.attribute("GP").as_int();
		g.ConversionRate = xmlItem.attribute("Rate").as_int();
		m_GPConvertRates.push_back(g);

		xmlItem = xmlItem.next_sibling();
	}

	pugi::xml_node xmlConvert = xmlFile.child("gc").first_child();
	for(int i=0; i<64; i++)
	{
		char buf[32];
		sprintf(buf, "r%d", i);
		m_GCPriceTable[i] = xmlConvert.attribute(buf).as_int();
		if(m_GCPriceTable[i] == 0) 
			break;
	}
	
	return 0;
}

int CClientUserProfile::ApiConvertGamePoints(int GamePoints)
{
	CWOBackendReq req(this, "api_GPConvert.aspx");
	req.AddParam("func", "convert");
	req.AddParam("GP",   GamePoints);
	if(!req.Issue())
	{
		r3dOutToLog("ApiConvertGamePoints failed: %d", req.resultCode_);
		return req.resultCode_;
	}

	int GameDollars = 0;
	int nargs = sscanf(req.bodyStr_, "%d", &GameDollars);
	r3d_assert(nargs == 1);
	
	ProfileData.GameDollars = GameDollars;
	ProfileData.GamePoints -= GamePoints;
	return 0;
}

int CClientUserProfile::GetConvertedGP2GD(int GamePoints, int& convRate)
{
	for(size_t i=0; i<m_GPConvertRates.size(); i++)
	{
		if(GamePoints < m_GPConvertRates[i].GamePoints)
		{
			convRate = m_GPConvertRates[i].ConversionRate;
			return GamePoints * m_GPConvertRates[i].ConversionRate;
		}
	}

	convRate = 0;
	return 0;
}

int CClientUserProfile::ApiBackpackToInventory(int GridFrom, int amount)
{
	r3d_assert(SelectedCharID >= 0 && SelectedCharID < wiUserProfile::MAX_LOADOUT_SLOTS);
	wiCharDataFull& w = ProfileData.ArmorySlots[SelectedCharID];
	r3d_assert(w.LoadoutID > 0);

	r3d_assert(GridFrom >= 0 && GridFrom < w.BackpackSize);
	wiInventoryItem* wi1 = &w.Items[GridFrom];
	r3d_assert(wi1->InventoryID > 0);
	r3d_assert(wi1->quantity >= amount);
	
	// scan for inventory and see if we can stack item there
	__int64 InvInventoryID = 0;
	for(uint32_t i=0; i<ProfileData.NumItems; i++)
	{
		const wiInventoryItem& wi2 = ProfileData.Inventory[i];
		if(wi1->CanStackWith(wi2, true)) // we allow to stack 'non stackable' items in GI
		{
			InvInventoryID = wi2.InventoryID;
			break;
		}
	}
	char strInventoryID[128];
	sprintf(strInventoryID, "%I64d", InvInventoryID);

	if(InvInventoryID == 0 && haveFreeInventorySlot() == false)
	{
		r3dOutToLog("there is no free inventory slots\n");
		return 9;
	}

	CWOBackendReq req(this, "api_CharBackpack.aspx");
	req.AddParam("CharID", w.LoadoutID);
	req.AddParam("op",     10);		// inventory operation code
	req.AddParam("v1",     strInventoryID);	// value 1
	req.AddParam("v2",     GridFrom);	// value 2
	req.AddParam("v3",     amount);		// value 3
	if(!req.Issue())
	{
		r3dOutToLog("ApiBackpackToInventory failed: %d\n", req.resultCode_);
		return req.resultCode_;
	}
	
	__int64 InventoryID = 0;
	int nargs = sscanf(req.bodyStr_, "%I64d", &InventoryID);
	r3d_assert(nargs == 1);
	r3d_assert(InventoryID > 0);

	// add one item to inventory
	wiInventoryItem* wi2 = getInventorySlot(InventoryID);
	if(wi2 == NULL) 
	{
		// add new inventory slot with same vars
		wi2 = getFreeInventorySlot();
		r3d_assert(wi2);
		
		*wi2 = *wi1;
		wi2->InventoryID = InventoryID;
		wi2->quantity    = amount;
	}
	else
	{
		wi2->quantity += amount;
	}
	
	// remove item
	wi1->quantity -= amount;
	if(wi1->quantity <= 0)
		wi1->Reset();
	
	return 0;
}
//new add skin
int CClientUserProfile::ApiUpdateAttachment()
{
	r3d_assert(SelectedCharID >= 0 && SelectedCharID < wiUserProfile::MAX_LOADOUT_SLOTS);
	wiCharDataFull& w = ProfileData.ArmorySlots[SelectedCharID];
	r3d_assert(w.LoadoutID > 0);

	wiWeaponAttachment& attm1 = w.Attachment[0];
	wiWeaponAttachment& attm2 = w.Attachment[1];

	char attm1Buf[256];
	char attm2Buf[256];

	sprintf(attm1Buf, "%d %d %d %d %d %d %d %d",
		attm1.attachments[0],
		attm1.attachments[1],
		attm1.attachments[2],
		attm1.attachments[3],
		attm1.attachments[4],
		attm1.attachments[5],
		attm1.attachments[6],
		attm1.attachments[7]);

	sprintf(attm2Buf, "%d %d %d %d %d %d %d %d",
		attm2.attachments[0],
		attm2.attachments[1],
		attm2.attachments[2],
		attm2.attachments[3],
		attm2.attachments[4],
		attm2.attachments[5],
		attm2.attachments[6],
		attm2.attachments[7]);

	CWOBackendReq req(this, "api_CharUpdateAttachment.aspx");
	req.AddParam("CharID", w.LoadoutID);
	req.AddParam("attm1", attm1Buf);
	req.AddParam("attm2", attm2Buf);
	if (!req.Issue())
	{
		r3dOutToLog("ApiUpdateAttachment failed: %d", req.resultCode_);
		return req.resultCode_;
	}

	// reread profile, as FNGear is changed
	GetProfile();
	//r3dOutToLog("############ Informacion guardada\n");
	return 0;
}
//new add skin
int CClientUserProfile::ApiBackpackFromInventory(__int64 InventoryID, int GridTo, int amount)
{
	r3d_assert(SelectedCharID >= 0 && SelectedCharID < wiUserProfile::MAX_LOADOUT_SLOTS);
	wiCharDataFull& w = ProfileData.ArmorySlots[SelectedCharID];
	r3d_assert(w.LoadoutID > 0);

	wiInventoryItem* wi1 = getInventorySlot(InventoryID);

	if(amount > 1 && !storecat_IsItemStackable(wi1->itemID))
		amount = 1;

	r3d_assert(wi1);
	r3d_assert(wi1->quantity >= amount);

	int idx_free   = -1;
	int idx_exists = -1;

	// search for free or existing slot
	if(GridTo >= 0)
	{
		// placing to free slot
		if(w.Items[GridTo].InventoryID == 0)
		{
			idx_free = GridTo;
		}
		else if(wi1->CanStackWith(w.Items[GridTo]))
		{
			idx_exists = GridTo;
		}
		else
		{
			// trying to stack not stackable item
			return 9;
		}
	}
	else
	{
		// search for same or free slot
		for(int i=wiCharDataFull::CHAR_LOADOUT_ITEM1; i<w.BackpackSize; i++)
		{
			if(wi1->CanStackWith(w.Items[i]))
			{
				idx_exists = i;
				break;
			}
		}
		// search for same or free slot
		for(int i=wiCharDataFull::CHAR_REAL_BACKPACK_IDX_START; i<w.BackpackSize; i++)
		{
			if(w.Items[i].itemID == 0 && idx_free == -1) 
			{
				//AlexRedd:: Auto-Equip items
				const GearConfig* gc = g_pWeaponArmory->getGearConfig(wi1->itemID);
				const WeaponConfig* wc = g_pWeaponArmory->getWeaponConfig(wi1->itemID);

				if(wc && w.Items[w.CHAR_LOADOUT_WEAPON1].quantity == 0 && (wc->category >= storecat_ASR && wc->category <= storecat_SMG))//Weapons
					idx_free = w.CHAR_LOADOUT_WEAPON1;
				else if(wc && w.Items[w.CHAR_LOADOUT_WEAPON2].quantity == 0 && (wc->category == storecat_HG || wc->category == storecat_MELEE))//Melee and pistols
					idx_free = w.CHAR_LOADOUT_WEAPON2;
				else if(wc && w.Items[w.CHAR_LOADOUT_ITEM1].quantity == 0 && (wc->category == storecat_UsableItem && wc->m_isConsumable || wc->category == storecat_GRENADE))//Usable items or grenades
					idx_free = w.CHAR_LOADOUT_ITEM1;
				else if(wc && w.Items[w.CHAR_LOADOUT_ITEM2].quantity == 0 && (wc->category == storecat_UsableItem && wc->m_isConsumable || wc->category == storecat_GRENADE))//Usable items or grenades
					idx_free = w.CHAR_LOADOUT_ITEM2;
				else if(wc && w.Items[w.CHAR_LOADOUT_ITEM3].quantity == 0 && (wc->category == storecat_UsableItem && wc->m_isConsumable || wc->category == storecat_GRENADE))//Usable items or grenades
					idx_free = w.CHAR_LOADOUT_ITEM3;
				else if(wc && w.Items[w.CHAR_LOADOUT_ITEM4].quantity == 0 && (wc->category == storecat_UsableItem && wc->m_isConsumable || wc->category == storecat_GRENADE))//Usable items or grenades
					idx_free = w.CHAR_LOADOUT_ITEM4;
				else if(gc && w.Items[w.CHAR_LOADOUT_ARMOR].quantity == 0 && gc->category == storecat_Armor)//Armor
					idx_free = w.CHAR_LOADOUT_ARMOR;
				else if(gc && w.Items[w.CHAR_LOADOUT_HEADGEAR].quantity == 0 && gc->category == storecat_Helmet)//Helmet
					idx_free = w.CHAR_LOADOUT_HEADGEAR;	
				else
					idx_free = i;
			}
		}
		if(idx_free == -1 && idx_exists == -1)
		{
			r3dOutToLog("ApiBackpackFromInventory - no free slots\n");
			return 6;
		}
	}
	GridTo = idx_exists != -1 ? idx_exists : idx_free;
	r3d_assert(GridTo != -1);
	
	char strInventoryID[128];
	sprintf(strInventoryID, "%I64d", InventoryID);

	CWOBackendReq req(this, "api_CharBackpack.aspx");
	req.AddParam("CharID", w.LoadoutID);
	req.AddParam("op",     11);		// inventory operation code
	req.AddParam("v1",     strInventoryID);	// value 1
	req.AddParam("v2",     GridTo);		// value 2
	req.AddParam("v3",     amount);		// value 3
	if(!req.Issue())
	{
		r3dOutToLog("ApiBackpackFromInventory failed: %d\n", req.resultCode_);
		return req.resultCode_;
	}

	// get new inventory id
	int nargs = sscanf(req.bodyStr_, "%I64d", &InventoryID);
	r3d_assert(nargs == 1);
	r3d_assert(InventoryID > 0);

	if(idx_exists != -1) 
	{
		r3d_assert(w.Items[idx_exists].InventoryID == InventoryID);
		w.Items[idx_exists].quantity += amount;
	} 
	else 
	{
		w.Items[idx_free] = *wi1;
		w.Items[idx_free].InventoryID = InventoryID;
		w.Items[idx_free].quantity    = amount;
	}
	
	wi1->quantity -= amount;
	if(wi1->quantity <= 0)
		wi1->Reset();
		
	return 0;
}

int CClientUserProfile::ApiBackpackGridSwap(int GridFrom, int GridTo)
{
	r3d_assert(SelectedCharID >= 0 && SelectedCharID < wiUserProfile::MAX_LOADOUT_SLOTS);
	wiCharDataFull& w = ProfileData.ArmorySlots[SelectedCharID];
	r3d_assert(w.LoadoutID > 0);

	// check if we can join both slots
	r3d_assert(GridFrom >= 0 && GridFrom < w.BackpackSize);
	r3d_assert(GridTo >= 0 && GridTo < w.BackpackSize);
	wiInventoryItem& wi1 = w.Items[GridFrom];
	wiInventoryItem& wi2 = w.Items[GridTo];
	if(wi1.CanStackWith(wi2))
	{
		return ApiBackpackGridJoin(GridFrom, GridTo);
	}
	
	CWOBackendReq req(this, "api_CharBackpack.aspx");
	req.AddParam("CharID", w.LoadoutID);
	req.AddParam("op",     12);		// inventory operation code
	req.AddParam("v1",     GridFrom);	// value 1
	req.AddParam("v2",     GridTo);		// value 2
	req.AddParam("v3",     0);		// value 3
	if(!req.Issue())
	{
		r3dOutToLog("ApiBackpackGridMove failed: %d", req.resultCode_);
		return req.resultCode_;
	}
	
	R3D_SWAP(wi1, wi2);
	
	return 0;
}

int CClientUserProfile::ApiBackpackGridJoin(int GridFrom, int GridTo)
{
	r3d_assert(SelectedCharID >= 0 && SelectedCharID < wiUserProfile::MAX_LOADOUT_SLOTS);
	wiCharDataFull& w = ProfileData.ArmorySlots[SelectedCharID];
	r3d_assert(w.LoadoutID > 0);

	r3d_assert(GridFrom >= 0 && GridFrom < w.BackpackSize);
	r3d_assert(GridTo >= 0 && GridTo < w.BackpackSize);
	wiInventoryItem& wi1 = w.Items[GridFrom];
	wiInventoryItem& wi2 = w.Items[GridTo];
	r3d_assert(wi1.itemID && wi1.itemID == wi2.itemID);
	
	CWOBackendReq req(this, "api_CharBackpack.aspx");
	req.AddParam("CharID", w.LoadoutID);
	req.AddParam("op",     13);		// inventory operation code
	req.AddParam("v1",     GridFrom);	// value 1
	req.AddParam("v2",     GridTo);		// value 2
	req.AddParam("v3",     0);		// value 3
	if(!req.Issue())
	{
		r3dOutToLog("ApiBackpackGridJoin failed: %d", req.resultCode_);
		return req.resultCode_;
	}
	
	wi2.quantity += wi1.quantity;
	wi1.Reset();
	
	return 0;
}


int CClientUserProfile::ApiChangeBackpack(__int64 InventoryID)
{
	r3d_assert(SelectedCharID >= 0 && SelectedCharID < wiUserProfile::MAX_LOADOUT_SLOTS);
	wiCharDataFull& w = ProfileData.ArmorySlots[SelectedCharID];
	r3d_assert(w.LoadoutID > 0);

	// no need to validate InventoryID - server will do that
	char strInventoryID[128];
	sprintf(strInventoryID, "%I64d", InventoryID);

	CWOBackendReq req(this, "api_CharBackpack.aspx");
	req.AddParam("CharID", w.LoadoutID);
	req.AddParam("op",     16);		// inventory operation code
	req.AddParam("v1",     strInventoryID);	// value 1
	req.AddParam("v2",     0);
	req.AddParam("v3",     0);
	if(!req.Issue())
	{
		r3dOutToLog("ApiChangeBackpack failed: %d", req.resultCode_);
		return req.resultCode_;
	}

	// reread profile, as inventory/backpack is changed
	GetProfile();
	
	return 0;
}

int CClientUserProfile::ApiBuyItem(int itemId, int buyIdx, __int64* out_InventoryID, int qty)
{
	r3d_assert(buyIdx > 0);

	// API SYNC POINT with server call CJobBuyItem::Exec
	CWOBackendReq req(this, "api_BuyItem3.aspx");
	req.AddParam("ItemID", itemId);
	req.AddParam("BuyIdx", buyIdx);
	req.AddParam("Qty", qty);
	if(!req.Issue())
	{
		r3dOutToLog("BuyItem FAILED, code: %d\n", req.resultCode_);
		return req.resultCode_;
	}

	int balance = 0;
	int nargs = sscanf(req.bodyStr_, "%d %I64d", &balance, out_InventoryID);
	if(nargs != 2)
	{
		r3dError("wrong answer for ApiBuyItem");
		return 9;
	}

	// update balance
	if(buyIdx >= 5 && buyIdx <= 8)
		ProfileData.GameDollars = balance;
	else
		ProfileData.GamePoints  = balance;

	return 0;
}

//
//
// Steam part of the code
//
//
#include "steam_api_dyn.h"

class CSteamClientCallbacks
{
  public:
	STEAM_CALLBACK( CSteamClientCallbacks, OnMicroTxnAuth, MicroTxnAuthorizationResponse_t, m_MicroTxnAuth);

	CSteamClientCallbacks() : 
		m_MicroTxnAuth(this, &CSteamClientCallbacks::OnMicroTxnAuth)
	{
	}
};

void CSteamClientCallbacks::OnMicroTxnAuth(MicroTxnAuthorizationResponse_t *pCallback)
{
	gUserProfile.steamAuthResp.gotResp     = true;
	gUserProfile.steamAuthResp.bAuthorized = pCallback->m_bAuthorized;
	gUserProfile.steamAuthResp.ulOrderID   = pCallback->m_ulOrderID;
}

void CClientUserProfile::RegisterSteamCallbacks()
{
	r3d_assert(steamCallbacks == NULL);
	steamCallbacks = game_new CSteamClientCallbacks();
}

void CClientUserProfile::DeregisterSteamCallbacks()
{
	SAFE_DELETE(steamCallbacks);
}

int CClientUserProfile::ApiSteamStartBuyGP(int priceInCents)
{
	r3d_assert(gSteam.steamID);
	r3d_assert(priceInCents >= 0.0f);
	steamAuthResp.gotResp = false;
	
	char strSteamId[1024];
	sprintf(strSteamId, "%I64d", gSteam.steamID);

	CWOBackendReq req(this, "api_SteamBuyGP.aspx");
	req.AddParam("func",     "auth");
	req.AddParam("steamId",  strSteamId);
	req.AddParam("country",  gSteam.country);
	req.AddParam("price",    priceInCents);

	if(!req.Issue())
	{
		r3dOutToLog("ApiSteamStartBuyGP FAILED, code: %d\n", req.resultCode_);
		return req.resultCode_;
	}

	return 0;
}

int CClientUserProfile::ApiSteamFinishBuyGP(__int64 orderId)
{
	char	strOrderId[1024];
	sprintf(strOrderId, "%I64d", orderId);

	CWOBackendReq req(this, "api_SteamBuyGP.aspx");
	req.AddParam("func",    "fin");
	req.AddParam("orderId", strOrderId);

	if(!req.Issue())
	{
		r3dOutToLog("ApiSteamFinishBuyGP FAILED, code: %d\n", req.resultCode_);
		return req.resultCode_;
	}

	// update balance
	int balance = 0;
	int nargs = sscanf(req.bodyStr_, "%d", &balance);
	r3d_assert(nargs == 1);
	
	ProfileData.GamePoints = balance;
	return 0;
}

//
// friends APIs
//
int CClientUserProfile::ApiFriendAddReq(const char* gamertag, int* outFriendStatus)
{
	CWOBackendReq req(this, "api_Friends.aspx");
	req.AddParam("func", "addReq");
	req.AddParam("name", gamertag);

	if(!req.Issue())
	{
		r3dOutToLog("ApiFriendAddReq FAILED, code: %d\n", req.resultCode_);
		return req.resultCode_;
	}

	int friendStatus;
	int nargs = sscanf(req.bodyStr_, "%d", &friendStatus);
	r3d_assert(nargs == 1);
	*outFriendStatus = friendStatus;
	
	return 0;
}

int CClientUserProfile::ApiFriendAddAns(DWORD friendId, bool allow)
{
	CWOBackendReq req(this, "api_Friends.aspx");
	req.AddParam("func", "addAns");
	req.AddParam("FriendID", friendId);
	req.AddParam("Allow", allow ? "1" : "0");

	if(!req.Issue())
	{
		r3dOutToLog("ApiFriendAddAns FAILED, code: %d\n", req.resultCode_);
		return req.resultCode_;
	}
	
	return 0;
}

int CClientUserProfile::ApiFriendRemove(DWORD friendId)
{
	CWOBackendReq req(this, "api_Friends.aspx");
	req.AddParam("func", "remove");
	req.AddParam("FriendID", friendId);

	if(!req.Issue())
	{
		r3dOutToLog("ApiFriendRemove FAILED, code: %d\n", req.resultCode_);
		return req.resultCode_;
	}
	
	return 0;
}

int CClientUserProfile::ApiFriendGetStats(DWORD friendId)
{
	CWOBackendReq req(this, "api_Friends.aspx");
	req.AddParam("func", "stats");
	req.AddParam("FriendID", friendId);

	if(!req.Issue())
	{
		r3dOutToLog("ApiFriendGetStats FAILED, code: %d\n", req.resultCode_);
		return req.resultCode_;
	}
	
	pugi::xml_document xmlFile;
	req.ParseXML(xmlFile);
	friends->SetCurrentStats(xmlFile);
	
	return 0;
}

int CClientUserProfile::ApiGetLeaderboard(int TableID, int StartPos, int* out_CurPos, int* lbSize)
{
	r3d_assert(SelectedCharID >= 0 && SelectedCharID < wiUserProfile::MAX_LOADOUT_SLOTS);
	wiCharDataFull& w = ProfileData.ArmorySlots[SelectedCharID];
	r3d_assert(w.LoadoutID > 0);
	r3d_assert(TableID >= 0 && TableID <= 100);
	
	CWOBackendReq req(this, "api_LeaderboardGet.aspx");
	req.AddParam("CharID", w.LoadoutID);
	req.AddParam("t",      TableID);
	req.AddParam("pos",    StartPos);

	if(!req.Issue())
	{
		r3dOutToLog("ApiGetLeaderboard FAILED, code: %d\n", req.resultCode_);
		return req.resultCode_;
	}
	
	pugi::xml_document xmlFile;
	req.ParseXML(xmlFile);
	
	pugi::xml_node xmlLeaderboard = xmlFile.child("leaderboard");
	*out_CurPos = xmlLeaderboard.attribute("pos").as_int();
	*lbSize = xmlLeaderboard.attribute("size").as_int(); 
	m_lbData[TableID].reserve(100);
	m_lbData[TableID].clear();

	pugi::xml_node xmlItem = xmlLeaderboard.first_child();
	while(!xmlItem.empty())
	{
		LBEntry_s lb;
		r3dscpy(lb.gamertag,       xmlItem.attribute("GT").value());
		lb.Alive                 = xmlItem.attribute("a").as_int();
		lb.stats.XP              = xmlItem.attribute("XP").as_uint();
		lb.stats.TimePlayed      = xmlItem.attribute("tp").as_uint();
		lb.stats.Reputation      = xmlItem.attribute("r").as_uint();
		lb.stats.KilledZombies   = xmlItem.attribute("ts00").as_uint();
		lb.stats.KilledSurvivors = xmlItem.attribute("ts01").as_uint();
		lb.stats.KilledBandits   = xmlItem.attribute("ts02").as_uint();
		lb.stats.VictorysBattleRoyale   = xmlItem.attribute("ts03").as_uint();//AlexRedd:: BR mode
		lb.ClanID                = xmlItem.attribute("ci").as_uint();
		r3dscpy(lb.ClanTag,        xmlItem.attribute("ct").value());
		lb.ClanTagColor          = xmlItem.attribute("cc").as_int();
		m_lbData[TableID].push_back(lb);

		xmlItem = xmlItem.next_sibling();
	}
	
	return 0;
}

int CClientUserProfile::ApiGetGPTransactions()
{
	CWOBackendReq req(this, "api_GetGPTransactions.aspx");
	if(!req.Issue())
	{
		r3dOutToLog("ApiGetGPTransactions FAILED, code: %d\n", req.resultCode_);
		return req.resultCode_;
	}
	
	pugi::xml_document xmlFile;
	req.ParseXML(xmlFile);
	
	pugi::xml_node xmlLeaderboard = xmlFile.child("gplog");
	ProfileData.GamePoints = xmlLeaderboard.attribute("balance").as_int();

	GPLog_.clear();
	GPLog_.reserve(1024);

	pugi::xml_node xmlItem = xmlLeaderboard.first_child();
	while(!xmlItem.empty())
	{
		GPLog_s gl;
		gl.TransactionID = xmlItem.attribute("i").as_int();
		gl.Time          = xmlItem.attribute("t").as_uint();
		gl.Amount        = xmlItem.attribute("a").as_int();
		gl.Previous      = xmlItem.attribute("p").as_int();
		gl.Description   = xmlItem.attribute("d").value();
		GPLog_.push_back(gl);

		xmlItem = xmlItem.next_sibling();
	}
	
	return 0;
}

//AlexRedd:: teleport system test
int CClientUserProfile::ApiGetTeleportPoints()
{
	CWOBackendReq req(this, "api_GetTeleportData.aspx");
	if (!req.Issue())
	{
		r3dOutToLog("GetTeleportPoints FAILED, code: %d\n", req.resultCode_);
		return req.resultCode_;
	}
	
	pugi::xml_document xmlFile;
	req.ParseXML(xmlFile);	
	pugi::xml_node xmlTeleport = xmlFile.child("getpoints").first_child();	

	TeleportLog_.clear();
	TeleportLog_.reserve(1024);
	
	while (!xmlTeleport.empty())
	{
		TeleportLog_s tl;		
		tl.MapID = xmlTeleport.attribute("MapID").as_int();
		tl.TeleportPos = r3dPoint3D(0, 0, 0);		
		sscanf(xmlTeleport.attribute("TeleportPos").value(), "%f %f %f", &tl.TeleportPos.x, &tl.TeleportPos.y, &tl.TeleportPos.z);
		TeleportLog_.push_back(tl);
		
		xmlTeleport = xmlTeleport.next_sibling();
	}
	
	return 0;
}

int CClientUserProfile::ApiTeleportChar(const char* Pos) 
{
	wiCharDataFull& w = ProfileData.ArmorySlots[SelectedCharID];
	r3d_assert(w.LoadoutID > 0);

	CWOBackendReq req(this, "api_UsersMgr.aspx");
	req.AddParam("func", "teleport");
	req.AddParam("charid", w.LoadoutID);
	req.AddParam("Pos", Pos);
	req.AddParam("MapID", w.GameMapId);
	if(!req.Issue())
	{
		r3dOutToLog("ApiTeleportChar failed: %d", req.resultCode_);
		return req.resultCode_;
	}

	// reread profile
	GetProfile();

	return 0;
}
/////////////////

int CClientUserProfile::ApiGetLottery()
{
	CWOBackendReq req(this, "api_GetLottery.aspx");
	if (!req.Issue())
	{
		r3dOutToLog("ApiGetLottery FAILED, code: %d\n", req.resultCode_);
		return req.resultCode_;
	}
	
	pugi::xml_document xmlFile;
	req.ParseXML(xmlFile);	
	pugi::xml_node xmlLottery = xmlFile.child("lottery").first_child();	

	LotLog_.clear();
	LotLog_.reserve(1024);
	
	while (!xmlLottery.empty())
	{
		LotLog_s gl;
		gl.ID = xmlLottery.attribute("i").as_int();
		gl.Time = xmlLottery.attribute("t").as_uint();
		gl.UserID = xmlLottery.attribute("u").value();
		gl.Bank = xmlLottery.attribute("b").as_int();
		gl.CountTickets = xmlLottery.attribute("ct").as_int();
		gl.Chance = xmlLottery.attribute("ch").value();
		LotLog_.push_back(gl);
		
		xmlLottery = xmlLottery.next_sibling();
	}

	pugi::xml_node xmlLotteryWinners = xmlFile.child("winners").first_child();
	WinLog_.clear();
	WinLog_.reserve(1024);

	while (!xmlLotteryWinners.empty())
	{
		WinLog_s wl;
		wl.Time = xmlLotteryWinners.attribute("t2").as_uint();
		wl.WUserID = xmlLotteryWinners.attribute("wu").value();
		wl.Jackpot = xmlLotteryWinners.attribute("j").as_int();
		WinLog_.push_back(wl);

		xmlLotteryWinners = xmlLotteryWinners.next_sibling();
	}
	return 0;
}

int CClientUserProfile::ApiRetBonusGetInfo()
{
	CWOBackendReq req(this, "api_RetBonus.aspx");
	req.AddParam("func", "info");
	if(!req.Issue())
	{
		r3dOutToLog("ApiRetBonusGetInfo FAILED, code: %d\n", req.resultCode_);
		return req.resultCode_;
	}

	pugi::xml_document xmlFile;
	req.ParseXML(xmlFile);

	curRetentionDays_ = 0;
	minutesToNextRetDay_ = 0;
	pugi::xml_node xmlBonus = xmlFile.child("retbonus");
	curRetentionDays_ = xmlBonus.attribute("d").as_int();
	minutesToNextRetDay_ = xmlBonus.attribute("m").as_int();

	retentionBonusByDays_.clear();
	retentionQuantity_.clear();
	pugi::xml_node xmlDays = xmlBonus.child("days").first_child();
	while(!xmlDays.empty())
	{
		int bonusID = xmlDays.attribute("b").as_int();
		int Quantity = xmlDays.attribute("q").as_int();
		retentionBonusByDays_.push_back(bonusID);
		retentionQuantity_.push_back(Quantity);		

		xmlDays = xmlDays.next_sibling();
	}
	
	if(curRetentionDays_ > (int)retentionBonusByDays_.size())
		curRetentionDays_ = (int)retentionBonusByDays_.size(); // equal size, as index is curRetentionDays_-1	

	// reread profile
	GetProfile();
	
	return 0;
}

int CClientUserProfile::ApiCloseRewardPopUp(int actionID)
{
	CWOBackendReq req(this, "api_RetBonus.aspx");
	req.AddParam("func", "close");
	req.AddParam("actionID", actionID);
	if(!req.Issue())
	{
		r3dOutToLog("ApiCloseRewardPopUp failed: %d", req.resultCode_);
		return req.resultCode_;
	}

	return 0;
}

int CClientUserProfile::ApiRetBonusClaim()// disabled in API
{
	CWOBackendReq req(this, "api_RetBonus.aspx");
	req.AddParam("func", "give");
	if(!req.Issue())
	{
		r3dOutToLog("ApiRetBonusClaim FAILED, code: %d\n", req.resultCode_);
		return req.resultCode_;
	}	
	
	return 0;
}

int CClientUserProfile::ApiGetUpgradeAccBonusLootData(int AccType)
{
	CWOBackendReq req(this, "api_GetUpgradeAccBonusLoots.aspx");
	req.AddParam("AccType", AccType);

	if(!req.Issue())
	{
		r3dOutToLog("GetUpgradeAccBonusLoots FAILED, code: %d\n", req.resultCode_);
		return req.resultCode_;
	}
	
	pugi::xml_document xmlFile;
	req.ParseXML(xmlFile);
	
	pugi::xml_node xmlBonusList = xmlFile.child("bonuslist");	

	BonusList_.clear();
	BonusList_.reserve(1024);

	pugi::xml_node xmlItem = xmlBonusList.first_child();
	while(!xmlItem.empty())
	{
		BonusList_s bl;
		bl.ItemID		 = xmlItem.attribute("i").as_int();		
		bl.Amount        = xmlItem.attribute("a").as_int();		
		BonusList_.push_back(bl);

		xmlItem = xmlItem.next_sibling();
	}
	
	return 0;
}

// Blackmarket System Begin
std::vector<wiBlackmarketSetting> vBlackmarketSetting;
std::vector<wiBlackmarketItem> vBlackmarketItems;

static std::vector<std::string> splitString(std::string s, std::string seperator)
{
	std::vector<std::string> output;

	size_t pos = 0;
	std::string token;
	while ((pos = s.find(seperator)) != std::string::npos)
	{
		token = s.substr(0, pos);
		output.push_back(token);
		s = s.erase(0, pos + seperator.length());
	}

	// last one
	output.push_back(s);

	return output;
}

int CClientUserProfile::ApiFetchBlackmarketSetting()
{
	vBlackmarketSetting.clear();

	CWOBackendReq req(this, "api_BlackmarketSetting.aspx");
	if (!req.Issue())
	{
		r3dOutToLog("api_BlackmarketSetting FAILED, code: %d\n", req.resultCode_);
		return req.resultCode_;
	}

	std::string str(req.bodyStr_);
	auto arr = splitString(str, "\n");
	for (auto entry : arr)
	{
		auto s = splitString(entry, " ");
		if (s.size() != 2)
			continue;

		auto opt_name = s[0];
		auto opt_value_str = s[1];

		if (opt_name.empty())
			continue;

		if (opt_value_str.empty())
			continue;

		// parse float
		float opt_value = std::stof(opt_value_str);

		wiBlackmarketSetting setting;
		setting.opt_name = opt_name;
		setting.opt_value = opt_value;
		vBlackmarketSetting.emplace_back(setting);
	}

	// success
	return 0;
}

wiBlackmarketSetting* GetBlackmarketSetting(std::string opt_name)
{
	for (auto& entry : vBlackmarketSetting)
	{
		if (entry.opt_name == opt_name)
			return &entry;
	}

	return nullptr;
}

int CClientUserProfile::ApiFetchBlackmarket(std::vector<uint32_t> cats)
{
	std::string catsStr("[");
	for (auto& cat : cats)
	{
		char tmp[128];
		sprintf_s(tmp, "%d", cat);
		catsStr.append(tmp);
		catsStr.append(",");
	}
	catsStr.append("]");

	auto it = catsStr.find_last_of(",");
	if (it != std::string::npos)
		catsStr.erase(it, 1);

	r3dOutToLog("ARR: %s\n", catsStr.c_str());

	CWOBackendReq req(this, "api_Blackmarket.aspx");
	req.AddParam("Category", catsStr.c_str());
	if (!req.Issue())
	{
		r3dOutToLog("api_Blackmarket FAILED, code: %d\n", req.resultCode_);
		return req.resultCode_;
	}

	// clear current cache
	vBlackmarketItems.clear();

	const char* d = req.bodyStr_;
	const char* p = d;

	const char* const header = "BLACKMARKET";
	for (size_t i = 0; i < strlen(header); ++i)
	{
		if (p[i] != header[i])
			return 0;
	}

	// offset - skip header
	p += strlen(header);

	// loop through each line - break by end tag
	while (true)
	{
		if ((p - d) >= req.bodyLen_) {
			r3dOutToLog("api_Blackmarket: bad answer #1\n");
			return 9;
		}

		// end tag
		bool not_end = false;
		for (size_t i = 0; i < strlen(header); ++i)
		{
			if (p[i] != header[i])
			{
				not_end = true;
				break;
			}
		}

		if (!not_end)
			break;

		// read linear from the response
		DWORD PK_BlackmarketID = *(DWORD*)p; p += 4;
		DWORD FK_CustomerID = *(DWORD*)p; p += 4;
		DWORD ItemID = *(DWORD*)p; p += 4;
		DWORD Quantity = *(DWORD*)p; p += 4;
		DWORD SinglePrice = *(DWORD*)p; p += 4;
		DWORD CurrencyType = *(DWORD*)p; p += 4;

		// emplace back item
		wiBlackmarketItem itm;
		itm.PK_BlackmarketID = PK_BlackmarketID;
		itm.FK_CustomerID = FK_CustomerID;
		itm.ItemID = ItemID;
		itm.Quantity = Quantity;
		itm.SinglePrice = SinglePrice;
		itm.CurrencyType = CurrencyType;
		vBlackmarketItems.emplace_back(itm);
	}

	// success
	return 0;
}

int CClientUserProfile::ApiBuyItemFromBlackmarket(uint32_t PK_BlackmarketID, uint32_t Quantity, __int64& OutInventoryID, uint32_t& OutItemID, uint32_t& OutQuantity, uint32_t& OutVar1, uint32_t& OutVar2, uint32_t& OutVar3)
{
	CWOBackendReq req(this, "api_BlackmarketBuyItem.aspx");
	req.AddParam("PK_BlackmarketID", PK_BlackmarketID);
	req.AddParam("Quantity", Quantity);
	if (!req.Issue())
	{
		r3dOutToLog("api_BlackmarketBuyItem FAILED, code: %d\n", req.resultCode_);
		return req.resultCode_;
	}

	__int64 InventoryID = 0;
	int ItemID = 0;
	int tmpOutQuantity = 0;
	int Var1 = 0;
	int Var2 = 0;
	int Var3 = 0;
	int CurrencyType = 0;
	int OutCurrency = 0;
	int nargs = sscanf(req.bodyStr_, "%I64d %d %d %d %d %d %d %d", &InventoryID, &ItemID, &tmpOutQuantity, &Var1, &Var2, &Var3, &CurrencyType, &OutCurrency);
	if (nargs != 8)
	{
		r3dError("wrong answer for api_BlackmarketBuyItem");
		return 9;
	}

	// update balance
	if (CurrencyType == 4) // GC
	{
		ProfileData.GamePoints = OutCurrency;
	}
	else if (CurrencyType == 8) // GD
	{
		ProfileData.GameDollars = OutCurrency;
	}

	OutInventoryID = InventoryID;
	OutItemID = ItemID;
	OutQuantity = tmpOutQuantity;
	OutVar1 = Var1;
	OutVar2 = Var2;
	OutVar3 = Var3;

	// success
	return 0;
}

int CClientUserProfile::ApiRemoveItemFromBlackmarket(uint32_t PK_BlackmarketID, uint32_t Quantity, __int64& OutInventoryID, uint32_t& OutItemID, uint32_t& OutQuantity, uint32_t& OutVar1, uint32_t& OutVar2, uint32_t& OutVar3)
{
	CWOBackendReq req(this, "api_BlackmarketRemoveItem.aspx");
	req.AddParam("PK_BlackmarketID", PK_BlackmarketID);
	req.AddParam("Quantity", Quantity);
	if (!req.Issue())
	{
		r3dOutToLog("api_BlackmarketRemoveItem FAILED, code: %d\n", req.resultCode_);
		return req.resultCode_;
	}

	__int64 InventoryID = 0;
	int ItemID = 0;
	int tmpOutQuantity = 0;
	int Var1 = 0;
	int Var2 = 0;
	int Var3 = 0;
	int nargs = sscanf(req.bodyStr_, "%I64d %d %d %d %d %d", &InventoryID, &ItemID, &tmpOutQuantity, &Var1, &Var2, &Var3);
	if (nargs != 6)
	{
		r3dError("wrong answer for api_BlackmarketRemoveItem");
		return 9;
	}

	OutInventoryID = InventoryID;
	OutItemID = ItemID;
	OutQuantity = tmpOutQuantity;
	OutVar1 = Var1;
	OutVar2 = Var2;
	OutVar3 = Var3;

	// success
	return 0;
}

int CClientUserProfile::ApiSellItemOnBlackmarket(__int64 InventoryID, uint32_t SinglePrice, uint32_t Quantity, uint32_t CurrencyType, __int64& OutInventoryID, uint32_t& OutQuantity)
{
	CWOBackendReq req(this, "api_BlackmarketPutItem.aspx");
	req.AddParam("InventoryID", (int)InventoryID);
	req.AddParam("SinglePrice", SinglePrice);
	req.AddParam("Quantity", Quantity);
	req.AddParam("CurrencyType", CurrencyType);
	if (!req.Issue())
	{
		r3dOutToLog("api_BlackmarketPutItem FAILED, code: %d\n", req.resultCode_);
		return req.resultCode_;
	}

	uint32_t OutCurrencyType = 0;
	uint32_t OutMoney = 0;

	int nargs = sscanf(req.bodyStr_, "%I64d %d %d %d", &OutInventoryID, &OutQuantity, &OutCurrencyType, &OutMoney);
	if (nargs != 4)
	{
		r3dError("wrong answer for api_BlackmarketPutItem");
		return 9;
	}

	// update balance
	if (OutCurrencyType == 4) // GC
	{
		ProfileData.GamePoints = OutMoney;
	}
	else if (OutCurrencyType == 8) // GD
	{
		ProfileData.GameDollars = OutMoney;
	}

	// success
	return 0;
}
// Blackmarket System End

#endif // ! WO_SERVER
