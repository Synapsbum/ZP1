#include "r3dPCH.h"
#include "r3d.h"

#include "FrontEndShared.h"
#include "../ObjectsCode/weapons/Weapon.h"
#include "APIScaleformGfx.h"

#include "../ObjectsCode/weapons/WeaponArmory.h"
#include "../ObjectsCode/weapons/Weapon.h"
#include "../ObjectsCode/weapons/Ammo.h"
#include "../ObjectsCode/weapons/Gear.h"


HashTableDynamic<const char*, FixedString256, NameHashFunc_T, 1024> dictionaryHash_;

// disconnect from master server after this seconds of unactivity
float		_p2p_idleTime  = 10.0f; 

GraphicSettings::GraphicSettings()
{
	mesh_quality = r_mesh_quality->GetInt();
	texture_quality = r_texture_quality->GetInt();
	terrain_quality = r_terrain_quality->GetInt();
	shadows_quality = r_shadows_quality->GetInt();
	lighting_quality = r_lighting_quality->GetInt();
	environment_quality = r_environment_quality->GetInt();
	antialiasing_quality = r_antialiasing_quality->GetInt();
	anisotropy_quality = r_anisotropy_quality->GetInt();
	postprocess_quality = r_postprocess_quality->GetInt();
	ssao_quality = r_ssao_quality->GetInt();
}

DWORD GraphSettingsToVars( const GraphicSettings& settings )
{
	DWORD flags = 0;

	if( r_mesh_quality->GetInt() != settings.mesh_quality )
	{
		flags |= FrontEndShared::SC_MESH_QUALITY;
		r_mesh_quality->SetInt( settings.mesh_quality );
	}

	if( r_texture_quality->GetInt() != settings.texture_quality )
	{
		flags |= FrontEndShared::SC_TEXTURE_QUALITY;
		r_texture_quality->SetInt( settings.texture_quality );
	}

	if( r_terrain_quality->GetInt() != settings.terrain_quality )
	{
		flags |= FrontEndShared::SC_TERRAIN_QUALITY;
		r_terrain_quality->SetInt( settings.terrain_quality );
	}

	if( r_shadows_quality->GetInt() != settings.shadows_quality )
	{
		flags |= FrontEndShared::SC_SHADOWS_QUALITY;
		r_shadows_quality->SetInt( settings.shadows_quality );
	}

	if( r_lighting_quality->GetInt() != settings.lighting_quality )
	{
		flags |= FrontEndShared::SC_LIGHTING_QUALITY;
		r_lighting_quality->SetInt( settings.lighting_quality );
	}

	if( r_environment_quality->GetInt() != settings.environment_quality )
	{
		flags |= FrontEndShared::SC_ENVIRONMENT_QUALITY;
		r_environment_quality->SetInt( settings.environment_quality );
	}

	if( r_antialiasing_quality->GetInt() != settings.antialiasing_quality )
	{
		flags |= FrontEndShared::SC_ANTIALIASING_QUALITY;
		r_antialiasing_quality->SetInt( settings.antialiasing_quality );
	}

	if( r_anisotropy_quality->GetInt() != settings.anisotropy_quality )
	{
		flags |= FrontEndShared::SC_ANISOTROPY_QUALITY;
		r_anisotropy_quality->SetInt( settings.anisotropy_quality );
	}

	if( r_postprocess_quality->GetInt() != settings.postprocess_quality )
	{
		flags |= FrontEndShared::SC_POSTPROCESS_QUALITY;
		r_postprocess_quality->SetInt( settings.postprocess_quality );
	}

	if( r_ssao_quality->GetInt() != settings.ssao_quality )
	{
		flags |= FrontEndShared::SC_SSAO_QUALITY;
		r_ssao_quality->SetInt( settings.ssao_quality );
	}

	r_decals_proximity_multiplier->SetFloat(1 - float(settings.environment_quality) / (S_ULTRA + 1));

	return flags;
}

void FillDefaultSettings( GraphicSettings& settings, r3dDevStrength strength )
{
	switch( strength )
	{
	case S_WEAK:
		settings.mesh_quality			= 2;
		settings.texture_quality		= 1;
		settings.terrain_quality		= 1;
		settings.shadows_quality		= 0; // [TH] - no shadows on low
		settings.lighting_quality		= 3; //AlexRedd:: from 2
		settings.environment_quality    = 1;
		//settings.antialiasing_quality	= 1;
		settings.anisotropy_quality		= 2;
		settings.postprocess_quality	= 1;
		//settings.ssao_quality			= 1;
		break;

	case S_MEDIUM:
		settings.mesh_quality			= 2;
		settings.texture_quality		= 2;
		settings.terrain_quality		= 2;
		settings.shadows_quality		= 2;
		settings.lighting_quality		= 3; //AlexRedd:: from 2
		settings.environment_quality    = 2;
		//settings.antialiasing_quality	= 1;
		settings.anisotropy_quality		= 2;
		settings.postprocess_quality	= 1;
		//settings.ssao_quality			= 2;
		break;

	case S_STRONG:
		settings.mesh_quality			= 3;
		settings.texture_quality		= 3;
		settings.terrain_quality		= 3;
		settings.shadows_quality		= 3;
		settings.lighting_quality		= 3;
		settings.environment_quality	= 3;
		//settings.antialiasing_quality	= 1;
		settings.anisotropy_quality		= 3;
		settings.postprocess_quality	= 2;
		//settings.ssao_quality			= 3;
		break;

	case S_ULTRA:
		settings.mesh_quality			= 3;
		settings.texture_quality		= 3;
		settings.terrain_quality		= 3;
		settings.shadows_quality		= 4;
		settings.lighting_quality		= 3;
		settings.environment_quality	= 3;
		//settings.antialiasing_quality	= 1;
		settings.anisotropy_quality		= 4;
		settings.postprocess_quality	= 3;
		//settings.ssao_quality			= 4;
		break;

	default:
		r3dError( "SetDefaultSettings: unknown strength..." );

	};
}

DWORD SetDefaultSettings( r3dDevStrength strength )
{
	GraphicSettings settings;
	FillDefaultSettings( settings, strength );

	r3d_assert( strength <=  S_ULTRA );

	r_overall_quality->SetInt( strength + 1 );
	r_apex_enabled->SetBool(r_overall_quality->GetInt() == 4);

	return GraphSettingsToVars( settings );
}

DWORD SetCustomSettings( const GraphicSettings& settings )
{
	r_apex_enabled->SetBool( false );

	r_overall_quality->SetInt( 5 );

	return GraphSettingsToVars( settings );
}

bool CreateFullIniPath( char* dest, bool old );

#define CUSTOM_INI_FILE "CustomSettings.ini"

void FillIniPath( char* target )
{
	bool useLocal = true;

	if( CreateConfigPath( target ) )
	{
		strcat( target, CUSTOM_INI_FILE );
		useLocal = false;
	}

	if( useLocal )
	{
		strcpy( target, CUSTOM_INI_FILE );
	}
}

void SaveCustomSettings( const GraphicSettings& settings )
{
	char fullPath[ MAX_PATH * 2 ];

	FillIniPath( fullPath );

	r3dOutToLog( "SaveCustomSettings: using file %s\n", fullPath );

	FILE* fout = fopen_for_write( fullPath, "wt");

	fprintf( fout, "%s %d\n", r_mesh_quality->GetName(), settings.mesh_quality );
	fprintf( fout, "%s %d\n", r_texture_quality->GetName(), settings.texture_quality );
	fprintf( fout, "%s %d\n", r_terrain_quality->GetName(), settings.terrain_quality );
	fprintf( fout, "%s %d\n", r_shadows_quality->GetName(), settings.shadows_quality );
	fprintf( fout, "%s %d\n", r_lighting_quality->GetName(), settings.lighting_quality );
	fprintf( fout, "%s %d\n", r_environment_quality->GetName(), settings.environment_quality );
	fprintf( fout, "%s %d\n", r_antialiasing_quality->GetName(), settings.antialiasing_quality );
	fprintf( fout, "%s %d\n", r_anisotropy_quality->GetName(), settings.anisotropy_quality );
	fprintf( fout, "%s %d\n", r_postprocess_quality->GetName(), settings.postprocess_quality );
	fprintf( fout, "%s %d\n", r_ssao_quality->GetName(), settings.ssao_quality );

	fclose( fout );
}

static void CheckOption( const char* line, const CmdVar* var, int* target )
{
	int val;
	char scanfline[ 512 ];

	sprintf( scanfline, "%s %%d", var->GetName() );
	
	if( sscanf( line, scanfline, &val ) == 1 ) 
		*target = val;
}

GraphicSettings GetCustomSettings()
{
	GraphicSettings settings;

	char fullPath[ MAX_PATH * 2 ];

	FillIniPath( fullPath );

	r3dOutToLog( "GetCustomSettings: using file %s\n", fullPath );

	if( FILE* fin = fopen( fullPath, "rt") )
	{
		for( ; !feof( fin ) ; )
		{
			char line[ 1024 ] = { 0 };

			fgets( line, sizeof line - 1, fin );

			if( strlen( line ) )
			{
				CheckOption( line, r_mesh_quality, &settings.mesh_quality );
				CheckOption( line, r_texture_quality, &settings.texture_quality );
				CheckOption( line, r_terrain_quality, &settings.terrain_quality );
				CheckOption( line, r_shadows_quality, &settings.shadows_quality );
				CheckOption( line, r_lighting_quality, &settings.lighting_quality );
				CheckOption( line, r_environment_quality, &settings.environment_quality );
				CheckOption( line, r_antialiasing_quality, &settings.antialiasing_quality );
				CheckOption( line, r_anisotropy_quality, &settings.anisotropy_quality );
				CheckOption( line, r_postprocess_quality, &settings.postprocess_quality );
				CheckOption( line, r_ssao_quality, &settings.ssao_quality );
			}
		}

		fclose( fin );
	}

	return settings;
}

void FillSettingsFromVars ( GraphicSettings& settings )
{
	settings.mesh_quality			= r_mesh_quality		->GetInt();
	settings.texture_quality		= r_texture_quality		->GetInt();
	settings.terrain_quality		= r_terrain_quality		->GetInt();
	settings.shadows_quality		= r_shadows_quality		->GetInt();
	settings.lighting_quality		= r_lighting_quality	->GetInt();
	settings.environment_quality	= r_environment_quality	->GetInt();
	settings.antialiasing_quality	= r_antialiasing_quality->GetInt();
	settings.anisotropy_quality		= r_anisotropy_quality	->GetInt();
	settings.postprocess_quality	= r_postprocess_quality	->GetInt();
	settings.ssao_quality			= r_ssao_quality		->GetInt();
}


void GetInterfaceSize(int& width, int& height, int& y_shift, const r3dScaleformMovie &m)
{
	int x, y;
	m.GetViewport(&x, &y, &width, &height);
	y_shift = (r_height->GetInt() - height) / 2;
}

float GetOptimalDist(const r3dPoint3D& boxSize, float halfFovInDegrees)
{
	float halfFOV = R3D_DEG2RAD(halfFovInDegrees);
	float halfH = boxSize.y * 0.5f;
	float treeR = R3D_MAX(boxSize.x, boxSize.z);

	float t = tanf( halfFOV );
	float distance = (halfH / t) + treeR;

	return distance;
}

void getWeaponStats(const WeaponConfig* wc, float* damagePerc, int* damage, float* spreadPerc, int* spread, float* recoilPerc, int* recoil, float* decayPerc, int* decay)
{	
		const float maxScaleValue = 0.61f;
		const int maxDamageValue = 100;		
		const int maxSpreadValue = 15;
		const int maxRecoilValue = 13;
		const int maxDecayValue = 1000;
		
		*damage = (int)wc->m_AmmoDamage;
		*damagePerc = (float)((*damage/maxDamageValue > maxScaleValue) ? maxScaleValue : *damage*maxScaleValue/maxDamageValue);		
		
		*spread = (int)wc->m_spread;
		*spreadPerc = (float)((*spread/maxSpreadValue > maxScaleValue) ? maxScaleValue : *spread*maxScaleValue/maxSpreadValue);	
		
		*recoil = (int)wc->m_recoil;
		*recoilPerc = (float)((*recoil/maxRecoilValue > maxScaleValue) ? maxScaleValue : *recoil*maxScaleValue/maxRecoilValue);

		*decay = (int)wc->m_AmmoDecay;
		*decayPerc = (float)((*decay/maxDecayValue > maxScaleValue) ? maxScaleValue : *decay*maxScaleValue/maxDecayValue);
}

void getGearStats(const GearConfig* gc, float* damagePerc, int* damage)
{	
		const float maxScaleValue = 0.61f;
		const int maxDamageValue = 25;
		
		*damage = (int)(gc->m_damagePerc*100.0f)+1;
		*damagePerc = (float)((*damage/maxDamageValue > maxScaleValue) ? (*damage-maxDamageValue)*maxScaleValue/maxDamageValue : *damage*maxScaleValue/maxDamageValue);		
}

void getAttachmentStats(const WeaponAttachmentConfig* attc, float* damagePerc, int* damage, float* spreadPerc, int* spread, float* recoilPerc, int* recoil)
{	
		const float maxScaleValue = 0.61f;
		const int maxDamageValue = 100;		
		const int maxSpreadValue = 15;
		const int maxRecoilValue = 13;
		
		*damage = (int)attc->m_Damage;
		if(*damage>0)
			*damagePerc = (float)((*damage/maxDamageValue > maxScaleValue) ? maxScaleValue : *damage*maxScaleValue/maxDamageValue);
		else
			*damagePerc = 0.0f;
		
		*spread = (int)attc->m_Spread;
		if(*spread>0)
			*spreadPerc = (float)((*spread/maxSpreadValue > maxScaleValue) ? maxScaleValue : *spread*maxScaleValue/maxSpreadValue);
		else
			*spreadPerc = 0.0f;
		
		*recoil = (int)attc->m_Recoil;
		if(*recoil>0)
			*recoilPerc = (float)((*recoil/maxRecoilValue > maxScaleValue) ? maxScaleValue : *recoil*maxScaleValue/maxRecoilValue);
		else
			*recoilPerc = 0.0f;
}

void getAmmoStats(const WeaponAttachmentConfig* ammoc, float* damagePerc, int* damage, float* spreadPerc, int* spread, float* recoilPerc, int* recoil, float* clipSizePerc, int* clipsize)
{	
		const float maxScaleValue = 0.61f;
		const int maxDamageValue = 100;
		const int maxClipSizeValue = 100;
		const int maxSpreadValue = 15;
		const int maxRecoilValue = 13;

		*clipsize = (int)ammoc->m_Clipsize;
		*clipSizePerc = (float)((*clipsize/maxClipSizeValue > maxScaleValue) ? maxScaleValue : *clipsize*maxScaleValue/maxClipSizeValue);
		
		*damage = (int)ammoc->m_Damage;
		if(*damage>0)
			*damagePerc = (float)((*damage/maxDamageValue > maxScaleValue) ? maxScaleValue : *damage*maxScaleValue/maxDamageValue);
		else
			*damagePerc = 0.0f;
		
		*spread = (int)ammoc->m_Spread;
		if(*spread>0)
			*spreadPerc = (float)((*spread/maxSpreadValue > maxScaleValue) ? maxScaleValue : *spread*maxScaleValue/maxSpreadValue);
		else
			*spreadPerc = 0.0f;
		
		*recoil = (int)ammoc->m_Recoil;
		if(*recoil>0)
			*recoilPerc = (float)((*recoil/maxRecoilValue > maxScaleValue) ? maxScaleValue : *recoil*maxScaleValue/maxRecoilValue);
		else
			*recoilPerc = 0.0f;
}

void getBackpackStats(const BackpackConfig* bc,  float* maxSlotsPerc, int* maxSlots, float* maxWeightPerc, int* maxWeight)
{	
		const float maxScaleValue = 0.61f;
		const int maxSlotsValue = 100;		
		const int maxWeightValue = 100;				
		
		*maxSlots = (int)bc->m_maxSlots;
		*maxSlotsPerc = (float)((*maxSlots/maxSlotsValue > maxScaleValue) ? maxScaleValue : *maxSlots*maxScaleValue/maxSlotsValue);	
		
		*maxWeight = (int)bc->m_maxWeight;
		*maxWeightPerc = (float)((*maxWeight/maxWeightValue > maxScaleValue) ? maxScaleValue : *maxWeight*maxScaleValue/maxWeightValue);
}

void getFoodStats(const FoodConfig* fc, float* healthPerc, int* health, float* toxicityPerc, int* toxicity, float* waterPerc, int* water, float* foodPerc, int* food, float* staminaPerc, int* stamina)
{	
		const float maxScaleValue = 0.61f;
		const int maxValue = 100;		
		
		*health = (int)fc->Health;
		if(*health>0)
			*healthPerc = (float)((*health/maxValue > maxScaleValue) ? maxScaleValue : *health*maxScaleValue/maxValue);
		else
			*healthPerc = 0.0f;
		
		*toxicity = (int)fc->Toxicity;
		if(*toxicity>0)
			*toxicityPerc = (float)((*toxicity/maxValue > maxScaleValue) ? maxScaleValue : *toxicity*maxScaleValue/maxValue);
		else
			*toxicityPerc = 0.0f;
		
		*water = (int)fc->Water;
		if(*water>0)
			*waterPerc = (float)((*water/maxValue > maxScaleValue) ? maxScaleValue : *water*maxScaleValue/maxValue);
		else
			*waterPerc = 0.0f;

		*food = (int)fc->Food;
		if(*food>0)
			*foodPerc = (float)((*food/maxValue > maxScaleValue) ? maxScaleValue : *food*maxScaleValue/maxValue);
		else
			*foodPerc = 0.0f;

		*stamina = (int)(fc->Stamina*100);
		if(*stamina>0)
			*staminaPerc = (float)((*stamina/maxValue > maxScaleValue) ? maxScaleValue : *stamina*maxScaleValue/maxValue);
		else
			*staminaPerc = 0.0f;
}

void reloadInventoryInfo(class r3dScaleformMovie& gfxMovie)
{
	Scaleform::GFx::Value var[7];
	// clear inventory DB
	gfxMovie.Invoke("_root.api.clearInventory", NULL, 0);

	// add all items
	for(uint32_t i=0; i<gUserProfile.ProfileData.NumItems; ++i)
	{
		if(gUserProfile.ProfileData.Inventory[i].itemID == 0)
			continue;

		var[0].SetUInt(uint32_t(gUserProfile.ProfileData.Inventory[i].InventoryID));
		var[1].SetUInt(gUserProfile.ProfileData.Inventory[i].itemID);
		var[2].SetNumber(gUserProfile.ProfileData.Inventory[i].quantity);
		var[3].SetNumber(gUserProfile.ProfileData.Inventory[i].Var1);
		var[4].SetNumber(gUserProfile.ProfileData.Inventory[i].Var2);
		bool isConsumable = false;
		{
			const WeaponConfig* wc = g_pWeaponArmory->getWeaponConfig(gUserProfile.ProfileData.Inventory[i].itemID);
			if(wc && wc->category == storecat_UsableItem && wc->m_isConsumable)
				isConsumable = true;
		}
		var[5].SetBoolean(isConsumable);
		var[6].SetString(getAdditionalDescForItem(gUserProfile.ProfileData.Inventory[i].itemID, gUserProfile.ProfileData.Inventory[i].Var1, gUserProfile.ProfileData.Inventory[i].Var2, gUserProfile.ProfileData.Inventory[i].Var3));
		gfxMovie.Invoke("_root.api.addInventoryItem", var, 7);
	}
}

void addItemCategoriesToUI(class r3dScaleformMovie& gfxMovie)
{
	// add categories
	Scaleform::GFx::Value var[12];
	var[2].SetNumber(0); 
	var[3].SetNumber(0);
	var[4].SetNumber(0);
	var[5].SetNumber(0);
	var[6].SetNumber(0);
	var[7].SetNumber(0);
	var[8].SetNumber(0);
	var[9].SetNumber(0);	
	var[10].SetNumber(0);//Skin system
	var[11].SetNumber(0);

	// store & inventory tabs
	var[0].SetNumber(0);
	var[1].SetString("weapon");
	var[2].SetBoolean(true);
	var[3].SetBoolean(true);
	gfxMovie.Invoke("_root.api.addTabType", var, 4);

	var[0].SetNumber(1);
	var[1].SetString("ammo");
	var[2].SetBoolean(true);
	var[3].SetBoolean(true);
	gfxMovie.Invoke("_root.api.addTabType", var, 4);

	var[0].SetNumber(2);
	var[1].SetString("equipment");
	var[2].SetBoolean(true);
	var[3].SetBoolean(true);
	gfxMovie.Invoke("_root.api.addTabType", var, 4);

	var[0].SetNumber(3);
	var[1].SetString("grenade");
	var[2].SetBoolean(true);
	var[3].SetBoolean(true);
	gfxMovie.Invoke("_root.api.addTabType", var, 4);

	var[0].SetNumber(4);
	var[1].SetString("melee");
	var[2].SetBoolean(true);
	var[3].SetBoolean(true);
	gfxMovie.Invoke("_root.api.addTabType", var, 4);

	var[0].SetNumber(5);
	var[1].SetString("gear");
	var[2].SetBoolean(true);
	var[3].SetBoolean(true);
	gfxMovie.Invoke("_root.api.addTabType", var, 4);

	var[0].SetNumber(6);
	var[1].SetString("helmet");
	var[2].SetBoolean(true);
	var[3].SetBoolean(true);
	gfxMovie.Invoke("_root.api.addTabType", var, 4);

	var[0].SetNumber(7);
	var[1].SetString("backpack");
	var[2].SetBoolean(true);
	var[3].SetBoolean(true);
	gfxMovie.Invoke("_root.api.addTabType", var, 4);

	var[0].SetNumber(8);
	var[1].SetString("food");
	var[2].SetBoolean(true);
	var[3].SetBoolean(true);
	gfxMovie.Invoke("_root.api.addTabType", var, 4);

	var[0].SetNumber(9);
	var[1].SetString("survival");
	var[2].SetBoolean(true);
	var[3].SetBoolean(true);
	gfxMovie.Invoke("_root.api.addTabType", var, 4);
	
	//Skin system
	var[0].SetNumber(10);
	var[1].SetString("skins");
	var[2].SetBoolean(true);
	var[3].SetBoolean(true);
	gfxMovie.Invoke("_root.api.addTabType", var, 4);

	var[0].SetNumber(11);
	var[1].SetString("case");
	var[2].SetBoolean(true);
	var[3].SetBoolean(false);
	gfxMovie.Invoke("_root.api.addTabType", var, 4);		
}

int	 getStoreBuyIdxUI(int priceGD, int priceGC)
{
	// 4 for GamePoints (CASH)
	// 8 for GameDollars (in-game)

	int buyIdx = 0;
	if(priceGC > 0)
	{
		buyIdx = 4;
	}
	else if(priceGD > 0)
	{
		buyIdx = 8;
	}

	return buyIdx;
}

static bool isStoreFilteredItem(uint32_t itemId)
{
	// clan items
	if(itemId >= 301151 && itemId <= 301157) // clan items
		return true;
	// char_revive
	if(itemId == 301159)
		return true;
	// premium acc
	if(itemId == 301257)
		return true;
	// char rename
	if(itemId == 301399)
		return true;
	//gd reward br
	if(itemId == 301400)//AlexRedd:: BR mode
		return true;

	return false;
}

// Blackmarket System Begin
void clearBlackmarketSettingUI(class r3dScaleformMovie& gfxMovie)
{
	// clear current UI
	gfxMovie.Invoke("_root.api.clearBlackmarketSettingDB", NULL, 0);
}

void addBlackmarketSettingToUI(class r3dScaleformMovie& gfxMovie)
{
	Scaleform::GFx::Value var[2];
	for (auto& setting : vBlackmarketSetting)
	{
		var[0].SetString(setting.opt_name.c_str());
		var[1].SetNumber(setting.opt_value);
		gfxMovie.Invoke("_root.api.addBlackmarketSetting", var, 2);
	}
}

void clearBlackmarketUI(class r3dScaleformMovie& gfxMovie)
{
	// clear current UI
	gfxMovie.Invoke("_root.api.clearBlackmarketDB", NULL, 0);
}

void addBlackmarketToUI(class r3dScaleformMovie& gfxMovie)
{
	Scaleform::GFx::Value var[7];
	for (auto& itm : vBlackmarketItems)
	{
		if (isStoreFilteredItem(itm.ItemID))
			continue;

		var[0].SetUInt(itm.PK_BlackmarketID);
		var[1].SetUInt(itm.FK_CustomerID);
		var[2].SetUInt(itm.ItemID);
		var[3].SetUInt(itm.SinglePrice);
		var[4].SetUInt(itm.Quantity);
		var[5].SetUInt(itm.CurrencyType);
		var[6].SetBoolean(itm.IsOwnItem(gUserProfile.CustomerID));
		gfxMovie.Invoke("_root.api.addBlackmarketItem", var, 7);
	}
}
// Blackmarket System End

void addStoreToUI(class r3dScaleformMovie& gfxMovie)
{
	Scaleform::GFx::Value var[10];
	for(uint32_t i=0; i<g_NumStoreItems; ++i)
	{
		if(isStoreFilteredItem(g_StoreItems[i].itemID))
			continue;

		int quantity = storecat_GetItemBuyStackSize(g_StoreItems[i].itemID);
		var[0].SetUInt(g_StoreItems[i].itemID);
		var[1].SetNumber(g_StoreItems[i].pricePerm);
		var[2].SetNumber(g_StoreItems[i].gd_pricePerm);
		var[3].SetNumber(quantity);		// quantity
		var[4].SetBoolean(g_StoreItems[i].isNew);

		gfxMovie.Invoke("_root.api.addStoreItem", var, 5);
	}
}

void addItemsAndCategoryToUI(r3dScaleformMovie& gfxMovie)
{
	Scaleform::GFx::Value var[20];
	r3dgameVector(const WeaponConfig*) allWpns;
	r3dgameVector(const GearConfig*) allGear;
	r3dgameVector(const ModelItemConfig*) allItem;
	r3dgameVector(const HeroConfig*) allHeroes;
	r3dgameVector(const FoodConfig*) allFood;
	r3dgameVector(const CraftComponentConfig*) allCraftComp;
	r3dgameVector(const CraftRecipeConfig*) allCraftRecipe;
	r3dgameVector(const BackpackConfig*) allBackpack;
	r3dgameVector(const WeaponAttachmentConfig*) allAmmo;
	r3dgameVector(const WeaponAttachmentConfig*) allAttachments;
	r3dgameVector(const SkinRecipeConfig*) allSkins;//new add skin
	r3dgameVector(const GearSkinRecipeConfig*) allGearSkins;//new add skin
	r3dgameVector(const GearSkinComponentConfig*) allGearSkinComp;//new add skin

	g_pWeaponArmory->startItemSearch();
	while(g_pWeaponArmory->searchNextItem())
	{
		uint32_t itemID = g_pWeaponArmory->getCurrentSearchItemID();
		const WeaponConfig* config = g_pWeaponArmory->getWeaponConfig(itemID);
		if(config)
		{
			allWpns.push_back(config);
		}
		const GearConfig* gearConfig = g_pWeaponArmory->getGearConfig(itemID);
		if(gearConfig)
		{
			allGear.push_back(gearConfig);
		}
		const ModelItemConfig* itemConfig = g_pWeaponArmory->getAccountItemConfig(itemID);
		if(itemConfig)
		{
			allItem.push_back(itemConfig);
		}
		const HeroConfig* heroConfig = g_pWeaponArmory->getHeroConfig(itemID);
		if(heroConfig)
		{
			allHeroes.push_back(heroConfig);
		}			
		const FoodConfig* foodConfig = g_pWeaponArmory->getFoodConfig(itemID);
		if(foodConfig)
		{
			allFood.push_back(foodConfig);
		}		
		const CraftComponentConfig* craftCompConfig = g_pWeaponArmory->getCraftComponentConfig(itemID);
		if(craftCompConfig)
		{
			allCraftComp.push_back(craftCompConfig);
		}
		const CraftRecipeConfig* craftRecConfig = g_pWeaponArmory->getCraftRecipeConfig(itemID);
		if(craftRecConfig)
		{
			allCraftRecipe.push_back(craftRecConfig);
		}//new add skin
		const GearSkinComponentConfig* GearSkinCompConfig = g_pWeaponArmory->getGearSkinComponentConfig(itemID);
		if (GearSkinCompConfig)
		{
			allGearSkinComp.push_back(GearSkinCompConfig);
		}

		const GearSkinRecipeConfig* GearSkinRecConfig = g_pWeaponArmory->getGearSkinRecipeConfig(itemID);
		if (GearSkinRecConfig)
		{
			allGearSkins.push_back(GearSkinRecConfig);
		}
		const SkinRecipeConfig* SkinRecipientConfig = g_pWeaponArmory->getSkinRecipeConfig(itemID);//getAttachmentConfig(itemID);
		if (SkinRecipientConfig)
		{
			allSkins.push_back(SkinRecipientConfig);
		}//new add skin
		const BackpackConfig* backpackConfig = g_pWeaponArmory->getBackpackConfig(itemID);
		if(backpackConfig)
		{
			allBackpack.push_back(backpackConfig);
		}
		const WeaponAttachmentConfig* wpnAttmConfig = g_pWeaponArmory->getAttachmentConfig(itemID);
		if(wpnAttmConfig)
		{
			if(wpnAttmConfig->m_type == WPN_ATTM_CLIP)
				allAmmo.push_back(wpnAttmConfig);
			else
				allAttachments.push_back(wpnAttmConfig);
		}			
	}

	const size_t backpackSize = allBackpack.size();
	for(size_t i = 0; i < backpackSize; ++i)
	{
		const BackpackConfig* backpack = allBackpack[i];

		var[0].SetUInt(backpack->m_itemID);
		var[1].SetNumber(backpack->category);
		var[2].SetString(backpack->m_StoreName);
		var[3].SetString(backpack->m_Description);
		var[4].SetString(backpack->m_StoreIcon);
		var[5].SetBoolean(false);					// is Stackable
		var[6].SetNumber(backpack->m_Weight);						// weight
		var[7].SetInt(backpack->m_maxSlots);
		var[8].SetNumber(backpack->m_maxWeight);
		gfxMovie.Invoke("_root.api.addItem", var, 9);

		/////////////////////////////////////////////////////
		{	//AlexRedd:: Backpack for UI
			Scaleform::GFx::Value var[4];
			var[0].SetUInt(backpack->m_itemID);

			int slots=0, weight=0;
			float maxSlotsPerc=0, maxWeightPerc=0;
			getBackpackStats(backpack, &maxSlotsPerc, &slots, &maxWeightPerc, &weight);

			var[1].SetString(gLangMngr.getString("StatSlots"));
			var[2].SetNumber(slots);
			var[3].SetNumber(maxSlotsPerc);
			gfxMovie.Invoke("_root.api.addItemStat", var, 4);

			var[1].SetString(gLangMngr.getString("StatWeight"));
			var[2].SetNumber(weight);
			var[3].SetNumber(maxWeightPerc);
			gfxMovie.Invoke("_root.api.addItemStat", var, 4);					
		}
		/////////////////////////////////////////////////////
	}


	const size_t gearSize = allGear.size();
	for(size_t i = 0; i < gearSize; ++i)
	{
		const GearConfig* gear = allGear[i];

		var[0].SetUInt(gear->m_itemID);
		var[1].SetNumber(gear->category);
		var[2].SetString(gear->m_StoreName);
		var[3].SetString(gear->m_Description);
		var[4].SetString(gear->m_StoreIcon);
		var[5].SetBoolean(false); // is stackable
		var[6].SetNumber(gear->m_Weight);
		gfxMovie.Invoke("_root.api.addItem", var, 7);
		
		/////////////////////////////////////////////////////
		{	//AlexRedd:: Gear stats for UI
			Scaleform::GFx::Value var[4];
			var[0].SetUInt(gear->m_itemID);
			
			int damage=0;
			float damagPerc=0;
			getGearStats(gear, &damagPerc, &damage);			
							
			var[1].SetString(gLangMngr.getString("StatProtection"));
			var[2].SetNumber(damage);
			var[3].SetNumber(damagPerc);
			gfxMovie.Invoke("_root.api.addItemStat", var, 4);			
		}
		/////////////////////////////////////////////////////
	}

	const size_t itemSize = allItem.size();
	for(size_t i = 0; i < itemSize; ++i)
	{
		const ModelItemConfig* gear = allItem[i];

		var[0].SetUInt(gear->m_itemID);
		var[1].SetNumber(gear->category);
		var[2].SetString(gear->m_StoreName);
		var[3].SetString(gear->m_Description);
		var[4].SetString(gear->m_StoreIcon);
		var[5].SetBoolean(false); // is stackable
		var[6].SetNumber(gear->m_Weight);
		gfxMovie.Invoke("_root.api.addItem", var, 7);
	}

	const size_t foodSize = allFood.size();
	for(size_t i = 0; i < foodSize; ++i)
	{
		const FoodConfig* food = allFood[i];

		var[0].SetUInt(food->m_itemID);
		var[1].SetNumber(food->category);
		var[2].SetString(food->m_StoreName);
		var[3].SetString(food->m_Description);
		var[4].SetString(food->m_StoreIcon);
		var[5].SetBoolean(false); // is stackable
		var[6].SetNumber(food->m_Weight);
		gfxMovie.Invoke("_root.api.addItem", var, 7);

		/////////////////////////////////////////////////////
		{	//AlexRedd:: Food stats for UI
			Scaleform::GFx::Value var[4];
			var[0].SetUInt(food->m_itemID);

			int health=0, toxicity=0, water=0, Food=0, stamina=0;
			float healthPerc=0, toxicityPerc=0, waterPerc=0, foodPerc=0, staminaPerc=0;
			getFoodStats(food, &healthPerc, &health, &toxicityPerc, &toxicity, &waterPerc, &water, &foodPerc, &Food, &staminaPerc, &stamina);

			var[1].SetString(gLangMngr.getString("StatHealth"));
			var[2].SetNumber(health);
			var[3].SetNumber(healthPerc);
			gfxMovie.Invoke("_root.api.addItemStat", var, 4);

			var[1].SetString(gLangMngr.getString("StatToxity"));
			var[2].SetNumber(toxicity);
			var[3].SetNumber(toxicityPerc);
			gfxMovie.Invoke("_root.api.addItemStat", var, 4);

			var[1].SetString(gLangMngr.getString("StatWater"));
			var[2].SetNumber(water);
			var[3].SetNumber(waterPerc);
			gfxMovie.Invoke("_root.api.addItemStat", var, 4);

			var[1].SetString(gLangMngr.getString("StatFood"));
			var[2].SetNumber(Food);
			var[3].SetNumber(foodPerc);
			gfxMovie.Invoke("_root.api.addItemStat", var, 4);

			var[1].SetString(gLangMngr.getString("StatStamina"));
			var[2].SetNumber(stamina);
			var[3].SetNumber(staminaPerc);
			gfxMovie.Invoke("_root.api.addItemStat", var, 4);
		}
		/////////////////////////////////////////////////////
	}

	const size_t craftCompSize = allCraftComp.size();
	for(size_t i = 0; i < craftCompSize; ++i)
	{
		const CraftComponentConfig* comp = allCraftComp[i];

		var[0].SetUInt(comp->m_itemID);
		var[1].SetNumber(comp->category);
		var[2].SetString(comp->m_StoreName);
		var[3].SetString(comp->m_Description);
		var[4].SetString(comp->m_StoreIcon);
		var[5].SetBoolean(false); // is stackable
		var[6].SetNumber(comp->m_Weight);
		gfxMovie.Invoke("_root.api.addItem", var, 7);
	}

	const size_t craftRecSize = allCraftRecipe.size();
	for(size_t i = 0; i < craftRecSize; ++i)
	{
		const CraftRecipeConfig* rec = allCraftRecipe[i];

		var[0].SetUInt(rec->m_itemID);
		var[1].SetNumber(rec->category);
		var[2].SetString(rec->m_StoreName);
		var[3].SetString(rec->m_Description);
		var[4].SetString(rec->m_StoreIcon);
		var[5].SetBoolean(false); // is stackable
		var[6].SetNumber(rec->m_Weight);
		gfxMovie.Invoke("_root.api.addItem", var, 7);
	}
//new add skin
	const size_t GearSkinRecSize = allGearSkins.size();
	for (size_t i = 0; i < GearSkinRecSize; ++i)
	{
		const GearSkinRecipeConfig* rec = allGearSkins[i];

		var[0].SetUInt(rec->m_itemID);
		var[1].SetNumber(rec->category);
		var[2].SetString(rec->m_StoreName);
		var[3].SetString(rec->m_Description);
		var[4].SetString(rec->m_StoreIcon);
		var[5].SetBoolean(false); // is stackable
		var[6].SetNumber(rec->m_Weight);
		gfxMovie.Invoke("_root.api.addItem", var, 7);
	}
//new add skin
	const size_t heroSize = allHeroes.size();
	for(size_t i = 0; i < heroSize; ++i)
	{
		const HeroConfig* hero = allHeroes[i];
		if(hero->m_Weight < 0) // not available to players
			continue;

		var[0].SetUInt(hero->m_itemID);
		var[1].SetNumber(hero->category);
		var[2].SetString(hero->m_StoreName);
		var[3].SetString(hero->m_Description);
		var[4].SetString(hero->m_StoreIcon);
		char tmpStr[256];
		r3dscpy(tmpStr, hero->m_StoreIcon);
		r3dscpy(&tmpStr[strlen(tmpStr)-4], "2.dds");
		var[5].SetString(tmpStr);

		char tmpStr2[256];
		r3dscpy(tmpStr2, hero->m_StoreIcon);
		r3dscpy(&tmpStr2[strlen(tmpStr2)-4], "3.dds");
		var[6].SetString(tmpStr2);

		var[7].SetInt(hero->getNumHeads());
		var[8].SetInt(hero->getNumBodys());
		var[9].SetInt(hero->getNumLegs());
		gfxMovie.Invoke("_root.api.addHero", var, 10);

		var[0].SetUInt(hero->m_itemID);
		var[1].SetNumber(hero->category);
		var[2].SetString(hero->m_StoreName);
		var[3].SetString(hero->m_Description);
		var[4].SetString(hero->m_StoreIcon);
		gfxMovie.Invoke("_root.api.addItem", var, 5);
	}

	const size_t weaponSize = allWpns.size();
	for(size_t i = 0; i < weaponSize; ++i)
	{
		const WeaponConfig* weapon = allWpns[i];

		var[0].SetUInt(weapon->m_itemID);
		var[1].SetNumber(weapon->category);
		var[2].SetString(weapon->m_StoreName);
		var[3].SetString(weapon->m_Description);
		var[4].SetString(weapon->m_StoreIcon);
		var[5].SetBoolean(weapon->category == storecat_GRENADE); // multi Purchase Item
		var[6].SetNumber(weapon->m_Weight); // weight
		gfxMovie.Invoke("_root.api.addItem", var, 7);

		/////////////////////////////////////////////////////
		{	//AlexRedd:: Weapon stats for UI
			Scaleform::GFx::Value var[4];
			var[0].SetUInt(weapon->m_itemID);
			
			int damage=0, spread=0, recoil=0, decay=0;
			float damagPerc=0, spreadPerc=0, recoilPerc=0, decayPerc=0;
			getWeaponStats(weapon, &damagPerc, &damage, &spreadPerc, &spread, &recoilPerc, &recoil, &decayPerc, &decay);
			
			if(weapon->category!=storecat_UsableItem)
			{
				var[1].SetString(gLangMngr.getString("StatDamage"));
				var[2].SetNumber(damage);
				var[3].SetNumber(damagPerc);
				gfxMovie.Invoke("_root.api.addItemStat", var, 4);

				var[1].SetString(gLangMngr.getString("StatAccuracy"));
				var[2].SetNumber(spread);
				var[3].SetNumber(spreadPerc);
				gfxMovie.Invoke("_root.api.addItemStat", var, 4);

				var[1].SetString(gLangMngr.getString("StatRecoil"));
				var[2].SetNumber(recoil);
				var[3].SetNumber(recoilPerc);
				gfxMovie.Invoke("_root.api.addItemStat", var, 4);

				var[1].SetString(gLangMngr.getString("StatDecay"));
				var[2].SetNumber(decay);
				var[3].SetNumber(decayPerc);
				gfxMovie.Invoke("_root.api.addItemStat", var, 4);				
			}

			if(weapon->m_itemID == WeaponConfig::ITEMID_Antibiotics || weapon->m_itemID == WeaponConfig::ITEMID_Bandages ||
			   weapon->m_itemID == WeaponConfig::ITEMID_Bandages2 || weapon->m_itemID == WeaponConfig::ITEMID_Medkit ||
			   weapon->m_itemID == WeaponConfig::ITEMID_Painkillers)
			{				
				var[1].SetString(gLangMngr.getString("StatHealth"));
				var[2].SetNumber(damage);
				var[3].SetNumber(damagPerc);
				gfxMovie.Invoke("_root.api.addItemStat", var, 4);								
			}
			if(weapon->m_itemID == WeaponConfig::ITEMID_C01Vaccine || weapon->m_itemID == WeaponConfig::ITEMID_C04Vaccine)
			{				
				var[1].SetString(gLangMngr.getString("StatToxity"));
				var[2].SetNumber(damage);
				var[3].SetNumber(damagPerc);
				gfxMovie.Invoke("_root.api.addItemStat", var, 4);								
			}
		}
		/////////////////////////////////////////////////////
	}

	const size_t ammoSize = allAmmo.size();
	for(size_t i = 0; i < ammoSize; ++i)
	{
		const WeaponAttachmentConfig* attm = allAmmo[i];

		var[0].SetUInt(attm->m_itemID);
		var[1].SetNumber(419);
		var[2].SetString(attm->m_StoreName);
		var[3].SetString(attm->m_Description);
		var[4].SetString(attm->m_StoreIcon);
		var[5].SetBoolean(false);					// multi Purchase Item
		var[6].SetNumber(attm->m_Weight); // weight
		gfxMovie.Invoke("_root.api.addItem", var, 7);

		/////////////////////////////////////////////////////
		{	//AlexRedd:: Ammo stats for UI
			Scaleform::GFx::Value var[4];
			var[0].SetUInt(attm->m_itemID);

			int damage=0, spread=0, recoil=0, clipsize=0;
			float damagPerc=0, spreadPerc=0, recoilPerc=0, clipSizePerc=0;
			getAmmoStats(attm, &damagPerc, &damage, &spreadPerc, &spread, &recoilPerc, &recoil, &clipSizePerc, &clipsize);

			var[1].SetString(gLangMngr.getString("StatDamage"));
			var[2].SetNumber(damage);
			var[3].SetNumber(damagPerc);
			gfxMovie.Invoke("_root.api.addItemStat", var, 4);

			var[1].SetString(gLangMngr.getString("StatAccuracy"));
			var[2].SetNumber(spread);
			var[3].SetNumber(spreadPerc);
			gfxMovie.Invoke("_root.api.addItemStat", var, 4);

			var[1].SetString(gLangMngr.getString("StatRecoil"));
			var[2].SetNumber(recoil);
			var[3].SetNumber(recoilPerc);
			gfxMovie.Invoke("_root.api.addItemStat", var, 4);

			var[1].SetString(gLangMngr.getString("StatClip"));
			var[2].SetNumber(clipsize);
			var[3].SetNumber(clipSizePerc);
			gfxMovie.Invoke("_root.api.addItemStat", var, 4);			
		}
		/////////////////////////////////////////////////////
	}

	const size_t attmSize = allAttachments.size();
	for(size_t i = 0; i < attmSize; ++i)
	{
		const WeaponAttachmentConfig* attm = allAttachments[i];

		var[0].SetUInt(attm->m_itemID);
		var[1].SetNumber(attm->category);
		var[2].SetString(attm->m_StoreName);
		var[3].SetString(attm->m_Description);
		var[4].SetString(attm->m_StoreIcon);
		var[5].SetBoolean(false);					// multi Purchase Item
		var[6].SetNumber(attm->m_Weight); // weight
		gfxMovie.Invoke("_root.api.addItem", var, 7);

		/////////////////////////////////////////////////////
		{	//AlexRedd:: Attachment stats for UI
			Scaleform::GFx::Value var[4];
			var[0].SetUInt(attm->m_itemID);

			int damage=0, spread=0, recoil=0;
			float damagPerc=0, spreadPerc=0, recoilPerc=0, decayPerc=0;
			getAttachmentStats(attm, &damagPerc, &damage, &spreadPerc, &spread, &recoilPerc, &recoil);

			if(attm->category!=419)
			{
				var[1].SetString(gLangMngr.getString("StatDamage"));
				var[2].SetNumber(damage);
				var[3].SetNumber(damagPerc);
				gfxMovie.Invoke("_root.api.addItemStat", var, 4);

				var[1].SetString(gLangMngr.getString("StatAccuracy"));
				var[2].SetNumber(spread);
				var[3].SetNumber(spreadPerc);
				gfxMovie.Invoke("_root.api.addItemStat", var, 4);

				var[1].SetString(gLangMngr.getString("StatRecoil"));
				var[2].SetNumber(recoil);
				var[3].SetNumber(recoilPerc);
				gfxMovie.Invoke("_root.api.addItemStat", var, 4);
			}
		}
		/////////////////////////////////////////////////////
	}
//new add skin
	const size_t ItemSkinSize = allSkins.size(); // Skin System
	for (size_t i = 0; i < ItemSkinSize; ++i)
	{
		const SkinRecipeConfig* SkinConf = allSkins[i];

		var[0].SetUInt(SkinConf->m_itemID);
		var[1].SetNumber(SkinConf->category);
		var[2].SetString(SkinConf->m_StoreName);
		var[3].SetString(SkinConf->m_Description);
		var[4].SetString(SkinConf->m_StoreIcon);
		var[5].SetBoolean(false); // is stackable
		var[6].SetNumber(SkinConf->m_Weight);
		gfxMovie.Invoke("_root.api.addItem", var, 7);
	}
//new add skin
	// categories
	var[0].SetNumber(storecat_HeroPackage);
	var[1].SetString("storecat_HeroPackage");
	var[2].SetNumber(-1);
	var[3].SetNumber(-1);
	gfxMovie.Invoke("_root.api.addCategory", var, 4);

	var[0].SetNumber(storecat_Backpack);
	var[1].SetString("storecat_Backpack");
	var[2].SetNumber(7);
	var[3].SetNumber(-1);
	gfxMovie.Invoke("_root.api.addCategory", var, 4);
	
	var[0].SetNumber(storecat_Armor);
	var[1].SetString("storecat_Armor");
	var[2].SetNumber(5);
	var[3].SetNumber(2);
	gfxMovie.Invoke("_root.api.addCategory", var, 4);

	var[0].SetNumber(storecat_Helmet);
	var[1].SetString("storecat_Helmet");
	var[2].SetNumber(6);
	var[3].SetNumber(3);
	gfxMovie.Invoke("_root.api.addCategory", var, 4);

	var[0].SetNumber(storecat_ASR);
	var[1].SetString("storecat_ASR");
	var[2].SetNumber(0);
	var[3].SetNumber(0);
	gfxMovie.Invoke("_root.api.addCategory", var, 4);

	var[0].SetNumber(storecat_SNP);
	var[1].SetString("storecat_SNP");
	var[2].SetNumber(0);
	var[3].SetNumber(0);
	gfxMovie.Invoke("_root.api.addCategory", var, 4);

	var[0].SetNumber(storecat_SHTG);
	var[1].SetString("storecat_SHTG");
	var[2].SetNumber(0);
	var[3].SetNumber(0);
	gfxMovie.Invoke("_root.api.addCategory", var, 4);

	var[0].SetNumber(storecat_MG);
	var[1].SetString("storecat_MG");
	var[2].SetNumber(0);
	var[3].SetNumber(0);
	gfxMovie.Invoke("_root.api.addCategory", var, 4);

	var[0].SetNumber(storecat_HG);
	var[1].SetString("storecat_HG");
	var[2].SetNumber(0);
	var[3].SetNumber(1);
	var[4].SetNumber(0); // allow HG into primary slot
	gfxMovie.Invoke("_root.api.addCategory", var, 5);

	var[0].SetNumber(storecat_SMG);
	var[1].SetString("storecat_SMG");
	var[2].SetNumber(0);
	var[3].SetNumber(0);
	gfxMovie.Invoke("_root.api.addCategory", var, 4);

	var[0].SetNumber(storecat_SUPPORT);
	var[1].SetString("storecat_SUPPORT");
	var[2].SetNumber(0);
	var[3].SetNumber(0);
	gfxMovie.Invoke("_root.api.addCategory", var, 4);

	var[0].SetNumber(storecat_UsableItem);
	var[1].SetString("storecat_UsableItem");
	var[2].SetNumber(9);
	var[3].SetNumber(4);
	gfxMovie.Invoke("_root.api.addCategory", var, 4);

	var[0].SetNumber(storecat_GRENADE);
	var[1].SetString("storecat_GRENADE");
	var[2].SetNumber(3);
	var[3].SetNumber(4);
	gfxMovie.Invoke("_root.api.addCategory", var, 4);

	var[0].SetNumber(storecat_MELEE);
	var[1].SetString("storecat_MELEE");
	var[2].SetNumber(4);
	var[3].SetNumber(1);
	gfxMovie.Invoke("_root.api.addCategory", var, 4);

	var[0].SetNumber(storecat_Food);
	var[1].SetString("storecat_Food");
	var[2].SetNumber(8);
	var[3].SetNumber(-1);
	gfxMovie.Invoke("_root.api.addCategory", var, 4);

	var[0].SetNumber(storecat_Water);
	var[1].SetString("storecat_Water");
	var[2].SetNumber(8);
	var[3].SetNumber(-1);
	gfxMovie.Invoke("_root.api.addCategory", var, 4);

	var[0].SetNumber(storecat_Components);
	var[1].SetString("storecat_Components");
	var[2].SetNumber(9);
	var[3].SetNumber(-1);
	gfxMovie.Invoke("_root.api.addCategory", var, 4);

	var[0].SetNumber(storecat_CraftRecipe);
	var[1].SetString("storecat_CraftRecipe");
	var[2].SetNumber(9);
	var[3].SetNumber(-1);
	gfxMovie.Invoke("_root.api.addCategory", var, 4);
//new add skin
	var[0].SetNumber(storecat_WeaponsSkinsRecipe);
	var[1].SetString("storecat_WeaponsSkinsRecipe");
	var[2].SetNumber(9);
	var[3].SetNumber(-1);
	gfxMovie.Invoke("_root.api.addCategory", var, 4);

	var[0].SetNumber(storecat_GearSkinRecipe);
	var[1].SetString("storecat_GearSkinRecipe");
	var[2].SetNumber(9);
	var[3].SetNumber(-1);
	gfxMovie.Invoke("_root.api.addCategory", var, 4);
//new add skin
	// special category for ammo
	var[0].SetNumber(419);
	var[1].SetString("ammo");
	var[2].SetNumber(1); 
	var[3].SetNumber(-1);
	gfxMovie.Invoke("_root.api.addCategory", var, 4);

	var[0].SetNumber(storecat_FPSAttachment);
	var[1].SetString("storecat_FPSAttachment");
	var[2].SetNumber(2); 
	var[3].SetNumber(-1);
	gfxMovie.Invoke("_root.api.addCategory", var, 4);

	var[0].SetNumber(storecat_LootBox);
	var[1].SetString("storecat_LootBox");
	var[2].SetNumber(11);
	var[3].SetNumber(-1);
	gfxMovie.Invoke("_root.api.addCategory", var, 4);	
}

const char* getAdditionalDescForItem(uint32_t itemID, int Var1, int Var2, int Var3)
{
	static char final_string[256] = {0};
	char first_string[128] = {0};
	char second_string[128] = {0};

	const WeaponConfig* wc = g_pWeaponArmory->getWeaponConfig(itemID);
	const WeaponAttachmentConfig* wac = g_pWeaponArmory->getAttachmentConfig(itemID);
	if(wc && wc->category != storecat_UsableItem && wc->category != storecat_GRENADE && wc->category!=storecat_MELEE) // weapon
	{
		const WeaponAttachmentConfig* clip = g_pWeaponArmory->getAttachmentConfig(Var2<0?(wc->FPSDefaultID[WPN_ATTM_CLIP]):(Var2));
		if(clip)
		{
			//=================?��?�� ���է��駹?��?
			//Oosmar02:: Recupere les dommages
			int Damage;
			Damage = int(wc->m_AmmoDamage);

			//Oosmar02:: Test
			/*char Spread[16];
			sprintf(Spread, "%.1f", (int)wc->m_spread);
			char Recoil[16];
			sprintf(Recoil, "%.1f", int(m_pConfig->m_recoil));*/

			//Oosmar02:: Recupere le spread et le recul
			int Spread;
			Spread = int(wc->m_spread);
			int Recoil;
			Recoil = int(wc->m_recoil);
			//=================?��?�� ���է��駹?��?

			int ammoLeft = Var1<0?clip->m_Clipsize:Var1;
			if(Var1<0)
				sprintf(first_string, gLangMngr.getString("WeaponInfoString_ammo"), Damage, Spread, Recoil); //Oosmar02:: ICI
			else if(ammoLeft > 0)
			{
				if(ammoLeft == 1)
					sprintf(first_string, gLangMngr.getString("WeaponInfoString_ammo"), /*clip->m_StoreName,*/ ammoLeft, Damage, Spread, Recoil);
				else
					sprintf(first_string, gLangMngr.getString("WeaponInfoString_ammo"), /*clip->m_StoreName,*/ ammoLeft, Damage, Spread, Recoil);
			}
			else
				sprintf(first_string, gLangMngr.getString("WeaponInfoString_NoAmmo"), Damage, Spread, Recoil);
		}
	}
	else if(wac)
	{
		int ammoLeft = Var1<0?wac->m_Clipsize:Var1;
		if(ammoLeft > 0)
		{
			if(ammoLeft == 1)
				sprintf(first_string, gLangMngr.getString("AmmoInfoString_Singular"), ammoLeft);
			else
				sprintf(first_string, gLangMngr.getString("AmmoInfoString_Regular"), ammoLeft);
		}
	}

	float itemDurability = float(Var3)/100.0f;
	int numDigits = 0;
	if(Var3%100!=0) numDigits=1;
	if(Var3%10!=0) numDigits=2;
	if(Var3==-1)
	{
		itemDurability=100.0f;
		numDigits = 0;
	}	
	if (wc && (wc->m_itemID >= WeaponConfig::ITEMID_VehicleSpawner_Buggy && wc->m_itemID <= WeaponConfig::ITEMID_VehicleSpawner_Convertable))
	{
		sprintf(first_string, gLangMngr.getString("ItemFuel"), 0, 100.0f);
		sprintf(second_string, gLangMngr.getString("ItemCondition"), numDigits, itemDurability);
	}
	//else
		//sprintf(second_string, gLangMngr.getString("ItemCondition"), numDigits, itemDurability);//AlexRedd:: durability disabled for now

	sprintf(final_string, "%s\n%s", first_string, second_string);
	return final_string;
}