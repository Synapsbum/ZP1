#include "r3dPCH.h"
#include "r3d.h"

#include "Safelock.h"
#include "WeaponConfig.h"
#include "WeaponArmory.h"
#include "..\ai\AI_Player.H"
#include "..\..\multiplayer\ClientGameLogic.h"
#include "..\..\ui\HUDDisplay.h"
extern HUDDisplay*	hudMain;


IMPLEMENT_CLASS(obj_Safelock, "obj_Safelock", "Object");
AUTOREGISTER_CLASS(obj_Safelock);

obj_Safelock::obj_Safelock()
{
	m_ItemID = -1;
	m_RotX = 0;
	m_AllowAsyncLoading = 1;
	m_OwnerCustomerID = 0;
}

obj_Safelock::~obj_Safelock()
{

}

BOOL obj_Safelock::Load(const char *fname)
{
	return TRUE;
}

BOOL obj_Safelock::OnCreate()
{
	r3d_assert(m_ItemID > 0);
	const char* cpMeshName = "";
	if(m_ItemID == WeaponConfig::ITEMID_Lockbox)
		cpMeshName = "Data\\ObjectsDepot\\Weapons\\Item_Lockbox_01.sco";
	else if(m_ItemID == WeaponConfig::ITEMID_LockboxBig)
		cpMeshName = "Data\\ObjectsDepot\\Weapons\\Item_Lockbox_01_Big.sco";
	else if(m_ItemID == WeaponConfig::ITEMID_LockboxSmall)
		cpMeshName = "Data\\ObjectsDepot\\Weapons\\Item_Lockbox_01_Small.sco";

	if(!parent::Load(cpMeshName)) 
		return FALSE;

	m_ActionUI_Title = gLangMngr.getString("Lockbox");
	m_ActionUI_Msg = gLangMngr.getString("HoldEToOpenLockbox");

	ReadPhysicsConfig();

	//PhysicsConfig.group = PHYSCOLL_NETWORKPLAYER;
	//PhysicsConfig.isDynamic = 0;

	SetBBoxLocal( GetObjectMesh()->localBBox ) ;

	if (hudMain)
		hudMain->addScreenIcon(m_LockboxIcon, "lockbox");

	// raycast and see where the ground is and place dropped box there
	// ptumik: do not do ray cast, as all the checks were done when placing lockbox, and what can happen now is that lockbox will be too far away from where server think it is and it will cause issues
	/*PxRaycastHit hit;
	PxSceneQueryFilterData filter(PxFilterData(COLLIDABLE_STATIC_MASK, 0, 0, 0), PxSceneQueryFilterFlag::eSTATIC);
	if(g_pPhysicsWorld->raycastSingle(PxVec3(GetPosition().x, GetPosition().y+1, GetPosition().z), PxVec3(0, -1, 0), 50.0f, PxSceneQueryFlag::eIMPACT, hit, filter))
	{
		SetPosition(r3dPoint3D(hit.impact.x, hit.impact.y, hit.impact.z));
		SetRotationVector(r3dPoint3D(m_RotX, 0, 0));
	}*/

	m_spawnPos = GetPosition();

	UpdateTransform();

	return parent::OnCreate();
}

BOOL obj_Safelock::OnDestroy()
{
	if (hudMain && !m_LockboxIcon.IsUndefined())
		hudMain->removeUserIcon(m_LockboxIcon);

	return parent::OnDestroy();
}

BOOL obj_Safelock::Update()
{
	// show lockbox icon
	if (m_OwnerCustomerID == gUserProfile.CustomerID)
	{
		float offset = 1.0f;
		if (m_ItemID == WeaponConfig::ITEMID_Lockbox)
			offset = 1.3f;
		else if (m_ItemID == WeaponConfig::ITEMID_LockboxBig)
			offset = 1.5f;
		else if (m_ItemID == WeaponConfig::ITEMID_LockboxSmall)
			offset = 1.0f;

		float alpha = 0.0f;
		if (gClientLogic().localPlayer_) // show lockbox icon only within 100 meters of player
			alpha = 1.0f - R3D_CLAMP((GetPosition() - gClientLogic().localPlayer_->GetPosition()).Length() / 100.0f, 0.0f, 1.0f);
		if (hudMain && !m_LockboxIcon.IsUndefined())
		{
			hudMain->moveUserIcon(m_LockboxIcon, GetPosition() + r3dPoint3D(0, offset, 0), true);
			hudMain->setScreenIconAlpha(m_LockboxIcon, alpha);
			hudMain->setScreenIconScale(m_LockboxIcon, R3D_CLAMP(alpha, 0.8f, 1.0f));
		}
	}
	return parent::Update();
}

struct LockboxDebugRenderable : Renderable
{
	void Init()
	{
		DrawFunc = Draw;
	}

	static void Draw( Renderable* RThis, const r3dCamera& Cam )
	{
		LockboxDebugRenderable* This = static_cast<LockboxDebugRenderable*>( RThis );

		r3dRenderer->SetMaterial(NULL);

		struct PushRestoreStates
		{
			PushRestoreStates()
			{
				r3dRenderer->SetRenderingMode( R3D_BLEND_ALPHA | R3D_BLEND_NZ | R3D_BLEND_PUSH );
			}

			~PushRestoreStates()
			{
				r3dRenderer->SetRenderingMode( R3D_BLEND_POP );
			}
		} pushRestoreStates; (void)pushRestoreStates;

		float dist = (gCam - This->Parent->GetPosition()).Length();
		int a    = int(R3D_MAX(1.0f - (dist / 100.0f), 0.1f)*255.0f);
		r3dPoint3D scrCoord;
		if(r3dProjectToScreen(This->Parent->GetPosition() + r3dPoint3D(0, 2.0f, 0), &scrCoord))
		{
			Font_Editor->PrintF(scrCoord.x, scrCoord.y+12, r3dColor(255,255,255,a), "OwnerID:%d", This->Parent->m_OwnerCustomerID);
		}
	}

	obj_Safelock* Parent;
};


void obj_Safelock::AppendRenderables( RenderArray ( & render_arrays  )[ rsCount ], const r3dCamera& Cam ) 
{
	parent::AppendRenderables(render_arrays, Cam);

#ifndef FINAL_BUILD
	if(d_debug_show_lockbox_owner->GetBool())
	{
		LockboxDebugRenderable rend;

		rend.Init();
		rend.Parent		= this;
		rend.SortValue	= 2*RENDERABLE_USER_SORT_VALUE;

		render_arrays[ rsDrawFlashUI ].PushBack( rend );
	}
#endif
}
