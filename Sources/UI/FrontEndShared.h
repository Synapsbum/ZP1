#pragma once

struct NameHashFunc_T
{
	inline int operator () ( const char * szKey )
	{
		return r3dHash::MakeHash( szKey );
	}
};
extern HashTableDynamic<const char*, FixedString256, NameHashFunc_T, 1024> dictionaryHash_;

// temp file to make it easier to port to new frontend UI
enum EGameResult {
	GRESULT_Unknown,
	GRESULT_Playing,	// game in progress
	GRESULT_Finished,	// finished successfully
	GRESULT_Exit,		// player exited
	GRESULT_Disconnect,	// game server disconnected by itself
	GRESULT_DoubleLogin,
	GRESULT_Failed_To_Join_Game,
	GRESULT_ShownWelcomePackage,
	GRESULT_Timeout,
	GRESULT_Unsync,
	GRESULT_StillInGame,
};

struct GraphicSettings
{
	int mesh_quality;
	int texture_quality;
	int terrain_quality;
	int shadows_quality;
	int lighting_quality;
	int environment_quality;
	int antialiasing_quality;
	int anisotropy_quality;
	int postprocess_quality;
	int ssao_quality;

	GraphicSettings();
};

void addItemsAndCategoryToUI(class r3dScaleformMovie& gfxMovie);
void addItemCategoriesToUI(class r3dScaleformMovie& gfxMovie);
void reloadInventoryInfo(class r3dScaleformMovie& gfxMovie);
// Blackmarket System Begin
void clearBlackmarketSettingUI(class r3dScaleformMovie& gfxMovie);
void addBlackmarketSettingToUI(class r3dScaleformMovie& gfxMovie);
void clearBlackmarketUI(class r3dScaleformMovie& gfxMovie);
void addBlackmarketToUI(class r3dScaleformMovie& gfxMovie);
// Blackmarket System End
void addStoreToUI(class r3dScaleformMovie& gfxMovie);
int	 getStoreBuyIdxUI(int priceGD, int priceGC);

void FillDefaultSettings( GraphicSettings& settings, r3dDevStrength strength );
DWORD	SetDefaultSettings( r3dDevStrength strength );
DWORD	SetCustomSettings( const GraphicSettings& settings );
void			SaveCustomSettings( const GraphicSettings& settings );
GraphicSettings GetCustomSettings();
DWORD	GraphSettingsToVars( const GraphicSettings& settings );
void	FillSettingsFromVars ( GraphicSettings& settings );

class r3dScaleformMovie;

void GetInterfaceSize(int& width, int& height, int& y_shift, const r3dScaleformMovie &m);
float GetOptimalDist(const r3dPoint3D& boxSize, float halfFovInDegrees);
void getWeaponStats(const class WeaponConfig* wc, float* damagePerc, int* damage, float* spreadPerc, int* spread, float* recoilPerc, int* recoil, float* decayPerc, int* decay); //AlexRedd:: Weapon stats for UI
void getGearStats(const class GearConfig* gc, float* damagePerc, int* damage); //AlexRedd:: Gear stats for UI
void getAttachmentStats(const class WeaponAttachmentConfig* attc, float* damagePerc, int* damage, float* spreadPerc, int* spread, float* recoilPerc, int* recoil); //AlexRedd:: Attachment stats for UI
void getAmmoStats(const class WeaponAttachmentConfig* ammoc, float* damagePerc, int* damage, float* spreadPerc, int* spread, float* recoilPerc, int* recoil, float* clipSizePerc, int* clipsize); //AlexRedd:: Ammo stats for UI
void getBackpackStats(const class BackpackConfig* bc, float* maxSlotsPerc, int* maxSlots, float* maxWeightPerc, int* maxWeight); //AlexRedd:: Backpack stats for UI
void getFoodStats(const class FoodConfig* fc, float* healthPerc, int* health, float* toxicityPerc, int* toxicity, float* waterPerc, int* water, float* foodPerc, int* food, float* staminaPerc, int* stamina); //AlexRedd:: Food stats for UI

const char* getAdditionalDescForItem(uint32_t itemID, int Var1, int Var2, int Var3); // returns pointer to LOCAL static variable!

extern float		_p2p_idleTime;

enum EBuyType
{
	EBT_UNKNOWN=0,
	EBT_STORE,
	EBT_ABILITY,
	EBT_ARMORY
};

struct FrontEndShared
{
	enum SettingsChangeFlags
	{
		SC_MESH_QUALITY			= 1	<< 0,
		SC_TEXTURE_QUALITY		= 1	<< 1,
		SC_TERRAIN_QUALITY		= 1	<< 2,
//		SC_WATER_QUALITY		= 1 << 3,
		SC_SHADOWS_QUALITY		= 1 << 4,		
		SC_LIGHTING_QUALITY		= 1 << 5,
//		SC_PARTICLES_QUALITY	= 1 << 6,
		SC_ENVIRONMENT_QUALITY	= 1 << 7,
		SC_ANTIALIASING_QUALITY	= 1 << 8,
		SC_ANISOTROPY_QUALITY	= 1 << 9,
		SC_POSTPROCESS_QUALITY	= 1 << 10,
		SC_SSAO_QUALITY			= 1 << 11,
		SC_ALL					= 0x7fffffff
	};

	enum EResults
	{
		RET_Exit = 100,
		RET_JoinGame,
		RET_Diconnected,
		RET_DoubleLogin,
		RET_Banned,
		RET_LoggedIn,
	};
};
