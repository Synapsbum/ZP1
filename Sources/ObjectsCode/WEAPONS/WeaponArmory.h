#pragma once

class Weapon;
class Ammo;
class Gear;
class BackPack;//new add skin
class GameObject;

#include "GameCode\UserProfile.h"
#include "BaseItemConfig.h"
#include "WeaponConfig.h"
#include "GearConfig.h"
#include "HeroConfig.h"

// base class, loads only DATA, no mesh\textures. Client is using ClientWeaponArmory class that loads all textures,etc.
class WeaponArmory
{
public:
	WeaponArmory();
	virtual ~WeaponArmory();

	// loads weapon library
	virtual bool Init();
	
	// releases all weapons.
	virtual void Destroy();

	virtual void UnloadMeshes();
	
	void updateLootBoxContent(pugi::xml_node& xmlLootBox, int srvLootType = 0);

	// returns pointer to a weapon, or null if not found
	// you are responsible for deleting pointer after you are done
	Gear* createGear(uint32_t itemID);
	BackPack* createBackPack(uint32_t itemID);//new add skin
	
	const WeaponConfig* getWeaponConfig(uint32_t itemID);
	const WeaponAttachmentConfig* getAttachmentConfig(uint32_t itemID);
	const GearConfig* getGearConfig(uint32_t itemID);
	const BackpackConfig* getBackpackConfig(uint32_t itemID);
	const HeroConfig* getHeroConfig(uint32_t itemID);
	const ModelItemConfig* getAccountItemConfig(uint32_t itemID); // ptumik: returns only if item is account, lootbox. Why do we even have this fn?! 
	const BaseItemConfig* getConfig(uint32_t itemID);
	const LootBoxConfig* getLootBoxConfig(uint32_t itemID);
	const FoodConfig*	getFoodConfig(uint32_t itemID);
	const VehicleInfoConfig*	getVehicleConfig(uint32_t itemID);
	const CraftComponentConfig* getCraftComponentConfig(uint32_t itemID);
	const CraftRecipeConfig* getCraftRecipeConfig(uint32_t itemID);
	const GearSkinComponentConfig* getGearSkinComponentConfig(uint32_t itemID);//new add skin
	const GearSkinRecipeConfig* getGearSkinRecipeConfig(uint32_t itemID);//new add skin

	const SkinRecipeConfig* getSkinRecipeConfig(uint32_t itemID);//new add skin
	const ScopeConfig* getScopeConfig(const char* name);
	const AchievementConfig* getAchievementConfig(const char* name);
	const AirdropConfig* getAirdropConfig(uint32_t itemID);

	int getNumWeapons() const { return m_NumWeaponsLoaded; }
	int getNumAttachments() const { return m_NumWeaponAttmLoaded; }
	int getNumGears() const { return m_NumGearLoaded; }
	int getNumBackpacks() const { return m_NumBackpackLoaded; }
	int getNumHeroes() const { return m_NumHeroLoaded; }
	int getNumItems() const { return m_NumItemLoaded; }
	int getNumLootBoxes() const { return m_NumLootBoxLoaded; }
	int getNumFoodItems() const { return m_NumFoodItemsLoaded; }
	int getNumCraftComponentItems() const { return m_NumCraftComponentsLoaded; }
	int getNumAchievements() const { return m_NumAchievementLoaded; }

	void startItemSearch();
	uint32_t getCurrentSearchItemID();
	bool searchNextItem();
	uint32_t getNumItemsInHash();

	const AchievementConfig* getAchievementByIndex(uint32_t index);
	const AchievementConfig* getAchievementByID(uint32_t ID);

	//void dumpStats() ;

protected:
	virtual Ammo* loadAmmo(pugi::xml_node& xmlAmmo);
	virtual WeaponConfig* loadWeapon(pugi::xml_node& xmlWeapon);
	virtual WeaponAttachmentConfig* loadWeaponAttachment(pugi::xml_node& xmlWeaponAttachment);
	virtual GearConfig* loadGear(pugi::xml_node& xmlGear);
	virtual BackpackConfig* loadBackback(pugi::xml_node& xmlItem);
	virtual HeroConfig* loadHero(pugi::xml_node& xmlHero);
	virtual ModelItemConfig* loadItem(pugi::xml_node& xmlItem);
	virtual LootBoxConfig* loadLootBox(pugi::xml_node& xmlItem);
	virtual FoodConfig* loadFoodItem(pugi::xml_node& xmlItem);
	virtual VehicleInfoConfig* loadVehicleItem(pugi::xml_node& xmlItem);
	virtual CraftComponentConfig* loadCraftComponentItem(pugi::xml_node& xmlItem);
	virtual CraftRecipeConfig* loadCraftRecipeItem(pugi::xml_node& xmlItem);
	virtual GearSkinRecipeConfig* loadGearSkinRecipeItem(pugi::xml_node& xmlItem);//new add skin
	virtual GearSkinComponentConfig* loadGearSkinComponentItem(pugi::xml_node& xmlItem);//new add skin
	virtual SkinRecipeConfig* loadSkinRecipeItem(pugi::xml_node& xmlItem);////// SKIN//new add skin
	virtual ScopeConfig* loadScope(pugi::xml_node& xmlScope);
	virtual AchievementConfig* loadAchievement(pugi::xml_node& xmlAchievement);
	virtual AirdropConfig* loadAirdropItem(pugi::xml_node& xmlItem);
	
	Ammo* getAmmo(const char* ammoName);

	static const int MAX_NUMBER_AMMO = 128;
	Ammo* m_AmmoArray[MAX_NUMBER_AMMO]; // todo: rewrite into hash later
	uint32_t	m_NumAmmoLoaded;

	struct ItemIDHashFunc_T { inline int operator () (const uint32_t key) { return key; } };
	HashTableDynamic<BaseItemConfig*, uint32_t, ItemIDHashFunc_T, 2048> m_itemsHash;

	uint32_t	m_NumWeaponsLoaded;
	uint32_t	m_NumWeaponAttmLoaded;
	uint32_t	m_NumGearLoaded;
	uint32_t	m_NumBackpackLoaded;
	uint32_t	m_NumHeroLoaded;
	uint32_t	m_NumItemLoaded;
	uint32_t	m_NumLootBoxLoaded;
	uint32_t	m_NumFoodItemsLoaded;
	uint32_t	m_NumVehiclesLoaded;
	uint32_t	m_NumCraftComponentsLoaded;
	uint32_t	m_NumCraftRecipesLoaded;
	uint32_t	m_NumSkinsRecipesLoaded;//new add skin
	uint32_t	m_NumGearSkinRecipesLoaded;//new add skin
	uint32_t	m_NumGearSkinComponentsLoaded;//new add skin
	uint32_t	m_NumAirdropLoaded;

	static const int MAX_NUMBER_SCOPE = 128;
	ScopeConfig* m_ScopeArray[MAX_NUMBER_SCOPE]; // todo: rewrite into hash later
	uint32_t	m_NumScopeLoaded;

	static const int MAX_NUMBER_ACHIEVEMENT = 128;
	AchievementConfig* m_AchievementArray[MAX_NUMBER_ACHIEVEMENT]; // todo: rewrite into hash later
	uint32_t	m_NumAchievementLoaded;
};
extern WeaponArmory* g_pWeaponArmory;
