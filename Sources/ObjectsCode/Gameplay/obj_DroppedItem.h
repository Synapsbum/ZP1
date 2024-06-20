#pragma once

#include "GameCommon.h"
#include "GameCode/UserProfile.h"
#include "SharedUsableItem.h"
#include "../../EclipseStudio/Sources/ObjectsCode/world/Lamp.h"
#include "APIScaleformGfx.h"

class obj_DroppedItem : public SharedUsableItem
{
	DECLARE_CLASS(obj_DroppedItem, SharedUsableItem)
protected:
	Scaleform::GFx::Value m_CaseIcon;
public:
	wiInventoryItem	m_Item;

	int NeedInitPhysics;	

	float	m_AirHigthPos;
	bool	m_IsOnTerrain;
	r3dPoint3D AirDropPos;
	r3dPoint3D CasePos;
	float	AirCraftDistance;
	bool	NetworLocal;
	bool	startdown;

	class obj_LightHelper* Light1;
	class obj_LightHelper* Light2;
	
public:
	obj_DroppedItem();
	virtual ~obj_DroppedItem();

	void				SetHighlight( bool highlight );
	bool				GetHighlight() const;

	void				StartLights();
	void				UpdateObjectPositionAfterCreation();

	virtual	BOOL		Load(const char *name);

	virtual	BOOL		OnCreate();
	virtual	BOOL		OnDestroy();

	void* sndAircraft;
	BYTE m_FirstTime;	

	virtual	BOOL		Update();
	void		ServerPost(float posY);
	obj_Building*	AirCraft;
	obj_Building*	SupplyDrop;

	r3dColor GetColor(int idx);
	r3dColor GetOutLineColor();
	r3dColor GetOutLineColorNew();
	r3dColor GetCatOutLineColor();

	virtual void		AppendRenderables( RenderArray ( & render_arrays  )[ rsCount ], const r3dCamera& Cam ) OVERRIDE;
};
