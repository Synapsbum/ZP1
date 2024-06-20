#pragma  once

#include "GameCommon.h"
#include "../Gameplay/SharedUsableItem.h"
#include "APIScaleformGfx.h"

class obj_AirDrop : public SharedUsableItem
{
	DECLARE_CLASS(obj_AirDrop, SharedUsableItem)
protected:
	Scaleform::GFx::Value m_AirdropIcon;
public:
	obj_AirDrop();
	virtual ~obj_AirDrop();

	virtual	BOOL		Load(const char *name);

	virtual	BOOL		OnCreate();
	virtual BOOL		OnDestroy();

	virtual BOOL		Update();

	virtual void		AppendRenderables( RenderArray ( & render_arrays  )[ rsCount ], const r3dCamera& Cam ) OVERRIDE;

	uint32_t			m_ItemID;
	float				m_RotX;
	//uint32_t			m_OwnerCustomerID; // to show it to devs
	class obj_ParticleSystem* m_FlareParticle;
	void*	m_sndPlayThrow;	
};
