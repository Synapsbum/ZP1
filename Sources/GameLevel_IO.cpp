#include "r3dPCH.h"
#include "r3d.h"

#include "GameObjects/ObjManag.h"
#include "ObjectsCode/WORLD/obj_Group.h"

#include "TrueNature2/Terrain3.h"

#include "GameLevel.h"

#ifdef WO_SERVER
#include "ServerGameLogic.h"

// temporary server object
class obj_ServerDummyObject : public GameObject
{
public:
	DECLARE_CLASS(obj_ServerDummyObject, GameObject)
	// keep alive, do not destroy
	// virtual BOOL Update() { return FALSE; }
};
IMPLEMENT_CLASS(obj_ServerDummyObject, "obj_ServerDummyObject", "Object");
AUTOREGISTER_CLASS(obj_ServerDummyObject);

#endif

#include "..\..\EclipseStudio\Sources\multiplayer\ClientGameLogic.h"


GameObject * LoadLevelObject(pugi::xml_node & curNode, bool JustForBuildings = false)
{
	pugi::xml_node posNode = curNode.child("position");
	const char* class_name = curNode.attribute("className").value();
	const char* load_name = curNode.attribute("fileName").value();
	r3dPoint3D pos(0, 0, 0);
	pos.x = posNode.attribute("x").as_float();
	pos.y = posNode.attribute("y").as_float();
	pos.z = posNode.attribute("z").as_float();

	GameObject* obj = NULL;

	//#ifndef WO_SERVER
	//
	//	if (gClientLogic().m_gameInfo.mapId == GBGameInfo::MAPID_ZH_Warehouse)
	//		g_FastLoad->SetBool(false);
	//
	//	if (gClientLogic().m_gameInfo.mapId == GBGameInfo::MAPID_WZ_AircraftCarrier)
	//		g_FastLoad->SetBool(false);
	//
	//#endif

#ifdef WO_SERVER
	int class_id = AObjectTable_GetClassID(class_name, "Object");
	if (class_id == -1)
	{
		r3dOutToLog("skipped not defined server object %s\n", class_name);
		class_name = "obj_ServerDummyObject";
	}
#endif
	// aLca :: Add Toxic Zone
	if (JustForBuildings)
	{
		if (!strcmp(class_name, "obj_Building"))
		{
			obj = srv_CreateGameObject(class_name, load_name, pos);
			if (!obj)
			{
				r3dOutToLog("!!!Failed to create object! class: %s, name: %s\n", class_name, load_name);
				return NULL;
			}
			r3d_assert(obj);
			obj->ReadSerializedData(curNode);
			obj->LastShadowExtrusion = r3dGetTime();
			obj->ObjFlags |= OBJFLAG_ForceSceneBoxBBox | OBJFLAG_AlwaysDraw;
			return obj;
		}
		return NULL;
	}

#ifdef FINAL_BUILD
	if (strcmp(r3dGameLevel::GetHomeDir(), "Levels\\WZ_FrontEndLighting") && g_FastLoad->GetBool())
	{
		if (!strcmp(class_name, "obj_StoreNPC") ||
			!strcmp(class_name, "obj_Gravestone") ||
			!strcmp(class_name, "obj_RepairBench") ||
			!strcmp(class_name, "obj_VaultNPC") ||
			!strcmp(class_name, "obj_SellNPC") ||
			!strcmp(class_name, "obj_Terrain") ||
			!strcmp(class_name, "obj_PermanentNote") ||
			!strcmp(class_name, "obj_WaterPlane") ||
			!strcmp(class_name, "obj_Lake") ||
			!strcmp(class_name, "obj_Road") ||
			!strcmp(class_name, "obj_ParticleSystem") ||
			!strcmp(class_name, "obj_LightMesh") ||
			!strcmp(class_name, "obj_LightHelper") ||
			!strcmp(class_name, "obj_MissionStateObject") ||
			!strcmp(class_name, "obj_LocalColorCorrection") ||
			!strcmp(class_name, "obj_DamageArea") ||
			!strcmp(class_name, "obj_GasArea") ||
			!strcmp(class_name, "obj_AmbientSound"))
		{
			obj = srv_CreateGameObject(class_name, load_name, pos);
			if (!obj)
			{
				r3dOutToLog("!!!Failed to create object! class: %s, name: %s\n", class_name, load_name);
				return NULL;
			}
			r3d_assert(obj);
			obj->ReadSerializedData(curNode);
			return obj;
		}
	}
	else
	{
		obj = srv_CreateGameObject(class_name, load_name, pos);
		if (!obj)
		{
			r3dOutToLog("!!!Failed to create object! class: %s, name: %s\n", class_name, load_name);
			return NULL;
		}
		r3d_assert(obj);
		obj->ReadSerializedData(curNode);
		return obj;
	}
#else
	extern bool g_bEditMode;
	if (strcmp(r3dGameLevel::GetHomeDir(), "Levels\\WZ_FrontEndLighting") && g_FastLoad->GetBool() && !g_bEditMode)
	{
		if (!strcmp(class_name, "obj_StoreNPC") ||
			!strcmp(class_name, "obj_Gravestone") ||
			!strcmp(class_name, "obj_RepairBench") ||
			!strcmp(class_name, "obj_VaultNPC") ||
			!strcmp(class_name, "obj_SellNPC") ||
			!strcmp(class_name, "obj_Terrain") ||
			!strcmp(class_name, "obj_PermanentNote") ||
			!strcmp(class_name, "obj_WaterPlane") ||
			!strcmp(class_name, "obj_Lake") ||
			!strcmp(class_name, "obj_Road") ||
			!strcmp(class_name, "obj_ParticleSystem") ||
			!strcmp(class_name, "obj_LightMesh") ||
			!strcmp(class_name, "obj_LightHelper") ||
			!strcmp(class_name, "obj_MissionStateObject") ||
			!strcmp(class_name, "obj_LocalColorCorrection") ||
			!strcmp(class_name, "obj_DamageArea") ||
			!strcmp(class_name, "obj_GasArea") ||
			!strcmp(class_name, "obj_AmbientSound"))
		{
			obj = srv_CreateGameObject(class_name, load_name, pos);
			if (!obj)
			{
				r3dOutToLog("!!!Failed to create object! class: %s, name: %s\n", class_name, load_name);
				return NULL;
			}
			r3d_assert(obj);
			obj->ReadSerializedData(curNode);
			return obj;
		}
	}
	else
	{
		obj = srv_CreateGameObject(class_name, load_name, pos);
		if (!obj)
		{
			r3dOutToLog("!!!Failed to create object! class: %s, name: %s\n", class_name, load_name);
			return NULL;
		}
		r3d_assert(obj);
		obj->ReadSerializedData(curNode);
		return obj;
	}
#endif

#ifdef WO_SERVER
	obj = srv_CreateGameObject(class_name, load_name, pos);
	if (!obj)
	{
		r3dOutToLog("!!!Failed to create object! class: %s, name: %s\n", class_name, load_name);
		return NULL;
	}
	r3d_assert(obj);
	obj->ReadSerializedData(curNode);
	return obj;
#endif

	return NULL;
}


void LoadLevelObjectsGroups(pugi::xml_node & curNode, r3dTL::TArray < GameObject * > & dObjects)
{
	dObjects.Clear();
	pugi::xml_node xmlObject = curNode.child("object");
	while (!xmlObject.empty())
	{
		GameObject* obj = LoadLevelObject(xmlObject);

		dObjects.PushBack(obj);

		xmlObject = xmlObject.next_sibling();
	}
}


void LoadLevelObjects(pugi::xml_node & curNode, float range, ESerializeFile sfType, bool isFastLoadBuilding = false)
{
	int count = 0;

	pugi::xml_node xmlObject = curNode.child("object");

	int hasTerrain = 0;

	while (!xmlObject.empty())
	{
		count++;

		if (!hasTerrain)
		{
			const char *className = xmlObject.attribute("className").value();
			if (!stricmp(className, "obj_Terrain"))
			{
				hasTerrain = 1;
			}
		}

		xmlObject = xmlObject.next_sibling();
	}

	if (hasTerrain)
	{
		range -= 0.1f;
	}

	float delta = 0.f;

	if (hasTerrain)
	{
		r3dITerrain::LoadingProgress = 0.25f * range;
		delta = count ? (range * 0.75f) / (count - 1) : 0.f;
	}
	else
	{
		r3dITerrain::LoadingProgress = 0.0f;
		delta = count ? range / count : 0.f;
	}

	xmlObject = curNode.child("object");
	while (!xmlObject.empty())
	{
		GameObject* obj = LoadLevelObject(xmlObject);

		void AdvanceLoadingProgress(float);

		if (obj && obj->ObjTypeFlags & OBJTYPE_Terrain)
		{
			AdvanceLoadingProgress(r3dITerrain::LoadingProgress);
			r3dITerrain::LoadingProgress = 0.f;
		}
		else
		{
			AdvanceLoadingProgress(delta);
		}

		xmlObject = xmlObject.next_sibling();
	}
}

bool ParseXMLFile(const char *xmlFileName, pugi::xml_document &outXml, char *& outBuf)
{
	r3dFile* f = r3d_open(xmlFileName, "rb");
	if (!f)
		return false;

	outBuf = game_new char[f->size + 1];
	fread(outBuf, f->size, 1, f);
	outBuf[f->size] = 0;
	pugi::xml_parse_result parseResult = outXml.load_buffer_inplace(outBuf, f->size);
	fclose(f);
	if (!parseResult)
		r3dError("Failed to parse XML, error: %s", parseResult.description());
	return true;
}

int LoadLevel_Objects(float BarRange, bool JustForBuilding_FastLoading)
{
	char fname[MAX_PATH];
	sprintf(fname, "%s\\LevelData.xml", r3dGameLevel::GetHomeDir());

	pugi::xml_document xmlLevelFile;
	char *levelData = 0;
	ParseXMLFile(fname, xmlLevelFile, levelData);

	pugi::xml_node xmlLevel = xmlLevelFile.child("level");

	g_leveldata_xml_ver->SetInt(0);
	if (!xmlLevel.attribute("version").empty())
	{
		g_leveldata_xml_ver->SetInt(xmlLevel.attribute("version").as_int());
	}


	if (g_level_settings_ver->GetInt() < 2 || g_level_settings_ver->GetInt() >= 3)
	{
		GameWorld().m_MinimapOrigin.x = xmlLevel.attribute("minimapOrigin.x").as_float();
		GameWorld().m_MinimapOrigin.z = xmlLevel.attribute("minimapOrigin.z").as_float();
		GameWorld().m_MinimapSize.x = xmlLevel.attribute("minimapSize.x").as_float();
		GameWorld().m_MinimapSize.z = xmlLevel.attribute("minimapSize.z").as_float();

		if (g_level_settings_ver->GetInt() < 2)
		{
			r_shadow_extrusion_limit->SetFloat(xmlLevel.attribute("shadowLimitHeight").as_float());
			if (!xmlLevel.attribute("near_plane").empty())
			{
				r_near_plane->SetFloat(xmlLevel.attribute("near_plane").as_float());
				r_far_plane->SetFloat(xmlLevel.attribute("far_plane").as_float());
			}
		}
	}

	if (GameWorld().m_MinimapSize.x == 0 || GameWorld().m_MinimapSize.z == 0)
	{
		GameWorld().m_MinimapSize.x = 100;
		GameWorld().m_MinimapSize.z = 100;
	}

	const float SOUND_BAR_RATIO = 0.25f;

	LoadLevelObjects(xmlLevel, BarRange * (1.f - SOUND_BAR_RATIO), SF_LevelData, JustForBuilding_FastLoading);

	//	Sound data (do not load on server)
#ifndef WO_SERVER
	sprintf(fname, "%s\\SoundData.xml", r3dGameLevel::GetHomeDir());

	pugi::xml_document xmlSoundFile;
	char *soundData = 0;
	ParseXMLFile(fname, xmlSoundFile, soundData);

	xmlLevel = xmlSoundFile.child("sounds");
	LoadLevelObjects(xmlLevel, BarRange * SOUND_BAR_RATIO, SF_SoundData);
#endif

	//	Don't load server data in final build (will load on server automatically)
	//AlexRedd:: Why?? This will desync with our studio.. we can't see/edit/save - items spawns/player spawns/vehicles spawns etc..
#ifndef FINAL_BUILD
	{
		//AlexRedd:: SHOULD LOAD ONLY FOR EDIT MODE.
		extern bool g_bEditMode;
		if (g_bEditMode)
		{
			//	Server data
			sprintf(fname, "%s\\ServerData.xml", r3dGameLevel::GetHomeDir());

			pugi::xml_document xmlServerFile;
			char *serverData = 0;
			ParseXMLFile(fname, xmlServerFile, serverData);

			xmlLevel = xmlServerFile.child("server_objects");
			LoadLevelObjects(xmlLevel, 0.f, SF_ServerData);

			delete[] serverData;
		}
	}
#endif

	{
#ifdef WO_SERVER
		//	Server data
		//AlexRedd:: improved!
		if (gServerLogic.ginfo_.channel == 3 || gServerLogic.ginfo_.channel == 4)
			sprintf(fname, "%s\\ServerDataPremium.xml", r3dGameLevel::GetHomeDir());
		else if (gServerLogic.ginfo_.IsDevEvent() && (gServerLogic.ginfo_.flags & GBGameInfo::SFLAGS_DisableSNP))
			sprintf(fname, "%s\\ServerDataDevNoSnipers.xml", r3dGameLevel::GetHomeDir());
		else if (gServerLogic.ginfo_.IsDevEvent() && (gServerLogic.ginfo_.flags & GBGameInfo::SFLAGS_DisableASR))
			sprintf(fname, "%s\\ServerDataDevSnipers.xml", r3dGameLevel::GetHomeDir());
		else
			sprintf(fname, "%s\\ServerData.xml", r3dGameLevel::GetHomeDir());

		pugi::xml_document xmlServerFile;
		char *serverData = 0;
		ParseXMLFile(fname, xmlServerFile, serverData);

		xmlLevel = xmlServerFile.child("server_objects");
		LoadLevelObjects(xmlLevel, 0.f, SF_ServerData);

		delete[] serverData;
#endif
	}

	// delete only after we are done parsing xml!
	delete[] levelData;
#ifndef WO_SERVER 
	delete[] soundData;
#endif

	return 1;
}

#ifndef FINAL_BUILD
#ifndef WO_SERVER
int LoadLevel_Groups()
{
	char fname[MAX_PATH];
	sprintf(fname, "Data\\ObjectsDepot\\LevelGroups.xml", r3dGameLevel::GetHomeDir());
	obj_Group::LoadFromFile(fname);
	return 1;
}
#endif	
#endif

int LoadLevel_MatLibs()
{
	if (r3dMaterialLibrary::IsDynamic) {
		// skip loading level materials if we're in editing mode
		return 1;
	}

	char fname[MAX_PATH];
	sprintf(fname, "%s\\room.mat", r3dGameLevel::GetHomeDir());

	r3dFile* f = r3d_open(fname, "rt");
	if (!f) {
		r3dArtBug("LoadLevel: can't find %s - switching to dynamic matlib\n", fname);
		r3dMaterialLibrary::IsDynamic = true;
		return 1;
	}

	char Str2[256], Str3[256];
	sprintf(Str2, "%s\\room.mat", r3dGameLevel::GetHomeDir());
	sprintf(Str3, "%s\\Textures\\", r3dGameLevel::GetHomeDir());

	r3dMaterialLibrary::LoadLibrary(Str2, Str3);
	return 1;
}