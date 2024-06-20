//=========================================================================
//	Module: obj_AirDropSpawn.h
//	Copyright (C) Online Warmongers Group Inc. 2012.
//=========================================================================

#pragma once

#include "GameCommon.h"
#include "gameobjects/GameObj.h"

//////////////////////////////////////////////////////////////////////////

class obj_AirDropSpawn: public GameObject
{
	DECLARE_CLASS(obj_AirDropSpawn, GameObject)

	/**	AirDrop spawn radius. */
	float spawnRadius;
	uint32_t	m_AirDropItemID; // used to select which item to spawn

	void CloneParameters(obj_AirDropSpawn *o);

public:
	obj_AirDropSpawn();
	~obj_AirDropSpawn();	

	virtual void AppendRenderables(RenderArray (& render_arrays  )[ rsCount ], const r3dCamera& Cam);
#ifndef FINAL_BUILD
	virtual float DrawPropertyEditor(float scrx, float scry, float scrw, float scrh, const AClass* startClass, const GameObjects& selected);
#endif
	virtual BOOL OnCreate();
	virtual BOOL Update();
	virtual BOOL OnDestroy();
	virtual	void ReadSerializedData(pugi::xml_node& node);
	virtual void WriteSerializedData(pugi::xml_node& node);
	virtual	GameObject * Clone();
};

