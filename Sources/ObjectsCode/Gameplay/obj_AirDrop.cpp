#include "r3dPCH.h"
#include "r3d.h"

#include "obj_AirDrop.h"
#include "ObjectsCode/weapons/WeaponConfig.h"
#include "ObjectsCode/weapons/WeaponArmory.h"
#include "..\ai\AI_Player.H"
#include "..\..\multiplayer\ClientGameLogic.h"
#include "../EFFECTS/obj_ParticleSystem.h"
#include "../../EclipseStudio/Sources/ObjectsCode/world/MaterialTypes.h"
#include "..\..\ui\HUDDisplay.h"
extern HUDDisplay*	hudMain;


IMPLEMENT_CLASS(obj_AirDrop, "obj_AirDrop", "Object");
AUTOREGISTER_CLASS(obj_AirDrop);

obj_AirDrop::obj_AirDrop()
{
	m_ItemID = -1;
	m_RotX = 0;
	m_AllowAsyncLoading = 1;	
	m_FlareParticle = NULL;
	m_sndPlayThrow = NULL;	
}

obj_AirDrop::~obj_AirDrop()
{

}

BOOL obj_AirDrop::Load(const char *fname)
{
	return TRUE;
}

BOOL obj_AirDrop::OnCreate()
{
	r3d_assert(m_ItemID > 0);
	const char* cpMeshName = "";
	const AirdropConfig* cfg = g_pWeaponArmory->getAirdropConfig(m_ItemID);
	if(m_ItemID == WeaponConfig::ITEMID_AirDrop || m_ItemID == WeaponConfig::ITEMID_AirDrop2)
	{
		if(cfg)
		   cpMeshName = cfg->m_ModelPath;
		else
			cpMeshName = "Data\\ObjectsDepot\\ZH_Objects_Props_Military\\AirDrop_01_Static.sco";
	}
	else
		r3dError("!!! unknown airdrop item %d\n", m_ItemID);

	if(!parent::Load(cpMeshName)) 
		return FALSE;

	m_ActionUI_Title = gLangMngr.getString("AirDrop");
	m_ActionUI_Msg = gLangMngr.getString("HoldEToPickUpARBX");	

	if(cfg)
	{
		SpawnImpactParticle(r3dHash::MakeHash(""), r3dHash::MakeHash("Airdrop_EXP"), GetPosition(), r3dPoint3D(0,1,0));
		m_FlareParticle = (obj_ParticleSystem*)srv_CreateGameObject("obj_ParticleSystem", cfg->m_ParticleFile, GetPosition() + r3dPoint3D(2,0,0));
		m_sndPlayThrow = SoundSys.Play(SoundSys.GetEventIDByPath(cfg->m_ParticleSound), GetPosition() + r3dPoint3D(2,0,0));	
	}

	if (hudMain)
		hudMain->addScreenIcon(m_AirdropIcon, "airdrop");

	ReadPhysicsConfig();

	//PhysicsConfig.group = PHYSCOLL_NETWORKPLAYER;
	//PhysicsConfig.isDynamic = 0;

	SetBBoxLocal( GetObjectMesh()->localBBox ) ;

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

BOOL obj_AirDrop::OnDestroy()
{
	if(m_FlareParticle!=NULL)
		m_FlareParticle=NULL;

	if(m_sndPlayThrow)
	{
		SoundSys.Release(m_sndPlayThrow); 
		m_sndPlayThrow = NULL;
	}	

	if (hudMain && !m_AirdropIcon.IsUndefined())
		hudMain->removeUserIcon(m_AirdropIcon);

	return parent::OnDestroy();
}

BOOL obj_AirDrop::Update()
{
	if(m_FlareParticle!=NULL && m_FlareParticle->Torch==0)
	{
		m_FlareParticle=NULL;
		m_sndPlayThrow=NULL;
		m_sndPlayThrow = SoundSys.Play(SoundSys.GetEventIDByPath("Sounds/WarZ/PlayerSounds/PLAYER_THROWFLARE"), GetPosition() + r3dPoint3D(2,0,0));
		m_FlareParticle = (obj_ParticleSystem*)srv_CreateGameObject("obj_ParticleSystem", "fx_airdrop", GetPosition() + r3dPoint3D(2,0,0));
	}

	float alpha = 0.0f;
	if (gClientLogic().localPlayer_) // show airdrop icon only within 300 meters of player
		alpha = 1.0f - R3D_CLAMP((GetPosition() - gClientLogic().localPlayer_->GetPosition()).Length() / 300.0f, 0.0f, 1.0f);
	if (hudMain && !m_AirdropIcon.IsUndefined())
	{
		hudMain->moveUserIcon(m_AirdropIcon, GetPosition() + r3dPoint3D(0, 2.0f, 0), true);
		hudMain->setScreenIconAlpha(m_AirdropIcon, alpha);
		hudMain->setScreenIconScale(m_AirdropIcon, R3D_CLAMP(alpha, 0.5f, 1.0f));
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

		/*float dist = (gCam - This->Parent->GetPosition()).Length();
		int a    = int(R3D_MAX(1.0f - (dist / 100.0f), 0.1f)*255.0f);
		r3dPoint3D scrCoord;
		if(r3dProjectToScreen(This->Parent->GetPosition() + r3dPoint3D(0, 2.0f, 0), &scrCoord))
		{
			Font_Editor->PrintF(scrCoord.x, scrCoord.y+12, r3dColor(255,255,255,a), "OwnerID:%d", This->Parent->m_OwnerCustomerID);
		}*/
	}

	obj_AirDrop* Parent;
};


void obj_AirDrop::AppendRenderables( RenderArray ( & render_arrays  )[ rsCount ], const r3dCamera& Cam ) 
{
	parent::AppendRenderables(render_arrays, Cam);

/*#ifndef FINAL_BUILD
	if(d_debug_show_lockbox_owner->GetBool())
	{
		LockboxDebugRenderable rend;

		rend.Init();
		rend.Parent		= this;
		rend.SortValue	= 2*RENDERABLE_USER_SORT_VALUE;

		render_arrays[ rsDrawFlashUI ].PushBack( rend );
	}
#endif*/
}
