#pragma once

#include "GameCommon.h"
#include "gameobjects/GameObj.h"

class obj_LobbyArea : public MeshGameObject
{
	DECLARE_CLASS(obj_LobbyArea, MeshGameObject)
	
public:
	float		useRadius;
		
	static r3dgameVector(obj_LobbyArea*) LoadedLobbyAreas;
public:
	obj_LobbyArea();
	virtual ~obj_LobbyArea();

	virtual	BOOL		Load(const char *name);

	virtual	BOOL		OnCreate();
	virtual	BOOL		OnDestroy();

	virtual	BOOL		Update();
 #ifndef FINAL_BUILD
 	virtual	float		DrawPropertyEditor(float scrx, float scry, float scrw, float scrh, const AClass* startClass, const GameObjects& selected) OVERRIDE;
 #endif
	virtual	void		AppendRenderables(RenderArray (& render_arrays  )[ rsCount ], const r3dCamera& Cam);
	virtual void		WriteSerializedData(pugi::xml_node& node);
	virtual	void		ReadSerializedData(pugi::xml_node& node);

};
