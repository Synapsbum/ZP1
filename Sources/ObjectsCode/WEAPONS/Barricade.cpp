#include "r3dPCH.h"
#include "r3d.h"

#include "Barricade.h"
#include "WeaponConfig.h"
#include "WeaponArmory.h"
#include "..\ai\AI_Player.H"
#include "..\..\multiplayer\ClientGameLogic.h"
#include "../../EclipseStudio/Sources/ObjectsCode/world/MaterialTypes.h"
#ifdef VEHICLES_ENABLED
#include "../../GameEngine/gameobjects/obj_Vehicle.h"
#endif
#include "..\..\ui\HUDPause.h"
#include "..\..\ui\HUDSafelock.h"

extern bool gDestroyingWorld;
extern HUDPause*	hudPause;
extern HUDSafelock*	hudSafelock;

IMPLEMENT_CLASS(obj_Barricade, "obj_Barricade", "Object");
AUTOREGISTER_CLASS(obj_Barricade);

obj_Barricade::obj_Barricade()
	: isRenderable(true) 
	, defferredPhysicsDisable(false)
{
	m_PrivateModel = NULL;
	m_ItemID = -1;
	m_RotX = 0;
	ObjTypeFlags = OBJTYPE_GameplayItem | OBJTYPE_Barricade;
	m_GeneratorSound = 0;
	m_BBoxSet = 0;
	m_Health = 0;//AlexRedd:: barricade health
}

obj_Barricade::~obj_Barricade()
{

}

BOOL obj_Barricade::OnCreate()
{
	bool trustServerPosition = false;
	r3d_assert(m_ItemID > 0);
	if(m_ItemID == WeaponConfig::ITEMID_BarbWireBarricade)
		m_PrivateModel = r3dGOBAddMesh("Data\\ObjectsDepot\\Weapons\\Item_Barricade_BarbWire_Built.sco", true, false, true, true );
	else if(m_ItemID == WeaponConfig::ITEMID_WoodShieldBarricade)
		m_PrivateModel = r3dGOBAddMesh("Data\\ObjectsDepot\\Weapons\\Item_Barricade_WoodShield_Built.sco", true, false, true, true );
	else if(m_ItemID == WeaponConfig::ITEMID_WoodShieldBarricadeZB)
		m_PrivateModel = r3dGOBAddMesh("Data\\ObjectsDepot\\Weapons\\Item_Barricade_WoodShield_Built_ZB.sco", true, false, true, true );
	else if(m_ItemID == WeaponConfig::ITEMID_RiotShieldBarricade)
		m_PrivateModel = r3dGOBAddMesh("Data\\ObjectsDepot\\Weapons\\Item_Riot_Shield_01.sco", true, false, true, true );
	else if(m_ItemID == WeaponConfig::ITEMID_RiotShieldBarricadeZB)
		m_PrivateModel = r3dGOBAddMesh("Data\\ObjectsDepot\\Weapons\\Item_Riot_Shield_ZB.sco", true, false, true, true );
	else if(m_ItemID == WeaponConfig::ITEMID_SandbagBarricade)
		m_PrivateModel = r3dGOBAddMesh("Data\\ObjectsDepot\\Weapons\\item_barricade_Sandbag_built.sco", true, false, true, true );
	else if(m_ItemID == WeaponConfig::ITEMID_WoodenDoorBlock)
		m_PrivateModel = r3dGOBAddMesh("Data\\ObjectsDepot\\Weapons\\Block_Door_Wood_2M_01.sco", true, false, true, true );
	else if(m_ItemID == WeaponConfig::ITEMID_MetalWallBlock)
	{
		m_PrivateModel = r3dGOBAddMesh("Data\\ObjectsDepot\\Weapons\\Block_Wall_Metal_2M_01.sco", true, false, true, true );
		trustServerPosition = true; // as they can be stacked on top of each other
	}
	else if(m_ItemID == WeaponConfig::ITEMID_TallBrickWallBlock)
	{
		m_PrivateModel = r3dGOBAddMesh("Data\\ObjectsDepot\\Weapons\\Block_Wall_Brick_Tall_01.sco", true, false, true, true );
		trustServerPosition = true; // as they can be stacked on top of each other
	}
	else if(m_ItemID == WeaponConfig::ITEMID_WoodenWallPiece)
	{
		m_PrivateModel = r3dGOBAddMesh("Data\\ObjectsDepot\\Weapons\\Block_Wall_Wood_2M_01.sco", true, false, true, true );
		trustServerPosition = true; // as they can be stacked on top of each other
	}
	else if(m_ItemID == WeaponConfig::ITEMID_ShortBrickWallPiece)
	{
		m_PrivateModel = r3dGOBAddMesh("Data\\ObjectsDepot\\Weapons\\Block_Wall_Brick_Short_01.sco", true, false, true, true );
		trustServerPosition = true; // as they can be stacked on top of each other
	}
	else if(m_ItemID == WeaponConfig::ITEMID_PlaceableLight)
		m_PrivateModel = r3dGOBAddMesh("Data\\ObjectsDepot\\Weapons\\Block_Light_01.sco", true, false, true, true );
	else if(m_ItemID == WeaponConfig::ITEMID_SmallPowerGenerator)
		m_PrivateModel = r3dGOBAddMesh("Data\\ObjectsDepot\\Weapons\\Block_PowerGen_01_Small.sco", true, false, true, true );
	else if(m_ItemID == WeaponConfig::ITEMID_BigPowerGenerator)
		m_PrivateModel = r3dGOBAddMesh("Data\\ObjectsDepot\\Weapons\\Block_PowerGen_01_Industrial.sco", true, false, true, true );
	else if(m_ItemID == WeaponConfig::ITEMID_SSBarricadeShield)
		m_PrivateModel = r3dGOBAddMesh("Data\\ObjectsDepot\\ZH_Items_Gameplay\\SS_Barricade_Shield.sco", true, false, true, true );

	if(m_PrivateModel==NULL)
		return FALSE;

	ReadPhysicsConfig();

	//PhysicsConfig.group = PHYSCOLL_NETWORKPLAYER;
	//PhysicsConfig.isDynamic = 0;

	// raycast and see where the ground is and place dropped box there
	if(!trustServerPosition)
	{
		PxRaycastHit hit;
		PxSceneQueryFilterData filter(PxFilterData(COLLIDABLE_STATIC_MASK, 0, 0, 0), PxSceneQueryFilterFlag::eSTATIC);
		if(g_pPhysicsWorld->raycastSingle(PxVec3(GetPosition().x, GetPosition().y+1, GetPosition().z), PxVec3(0, -1, 0), 50.0f, PxSceneQueryFlag::eIMPACT, hit, filter))
		{
			SetPosition(r3dPoint3D(hit.impact.x, hit.impact.y, hit.impact.z));
		}
	}
	SetRotationVector(r3dPoint3D(m_RotX, 0, 0));

	UpdateTransform();

	SoundSys.PlayAndForget(SoundSys.GetEventIDByPath("Sounds/Misc/Place_barricade"), GetPosition());

	if(m_ItemID == WeaponConfig::ITEMID_PlaceableLight)
	{
		const r3dPoint3D lightOffset(0.41f, 2.0f, -0.16f);

		m_Light.Assign(GetPosition()+lightOffset);
		m_Light.SetType(R3D_OMNI_LIGHT);
		m_Light.SetRadius(10.0f, 25.0f);
		m_Light.SetColor(255, 255, 182);
		m_Light.bCastShadows = false;//r_lighting_quality->GetInt() == 3 && r_shadows_quality->GetInt() >= 3;
		m_Light.Intensity = 2.0f;
		m_Light.SpotAngleFalloffPow = 1.6f;
		m_Light.bSSShadowBlur = 1;
		m_Light.bUseGlobalSSSBParams = 0;
		m_Light.SSSBParams.Bias = 0.1f;
		m_Light.SSSBParams.PhysRange = 43.12f;
		m_Light.SSSBParams.Radius = 10.0f;
		m_Light.SSSBParams.Sense = 635.18f;

		m_Light.TurnOff();
		WorldLightSystem.Add(&m_Light); 
		m_Light.TurnOn();
	}
	else if(m_ItemID == WeaponConfig::ITEMID_SmallPowerGenerator || m_ItemID == WeaponConfig::ITEMID_BigPowerGenerator)
	{
		m_GeneratorSound = SoundSys.Play(SoundSys.GetEventIDByPath("Sounds/Effects/Generator/Generator_Running"), GetPosition());
	}

	return parent::OnCreate();
}

BOOL obj_Barricade::OnDestroy()
{
	Destroy(true);

	m_PrivateModel = NULL;
	if(m_Light.pLightSystem)
		WorldLightSystem.Remove(&m_Light);
	if(m_GeneratorSound)
	{
		SoundSys.Stop(m_GeneratorSound);
		SoundSys.Release(m_GeneratorSound);
		m_GeneratorSound = NULL;
	}

	return parent::OnDestroy();
}

void obj_Barricade::OnContactModify(PhysicsCallbackObject *obj, PxContactSet& contacts)
{
	if (!obj)
		return;

	GameObject* gameObj = obj->isGameObject();
	if (!gameObj)
		return;

#ifdef VEHICLES_ENABLED
	if (gameObj->isObjType(OBJTYPE_Vehicle) && ((obj_Vehicle*)gameObj)->CanSmashBarricade())
	{
		obj_Vehicle* vehicle = (obj_Vehicle*)gameObj;
		vehicle->DoSlowDown();

		for(uint32_t i = 0; i < contacts.size(); ++i)
		{
			contacts.ignore(i);

			Destroy(false);
		}
	}
#endif
}

BOOL obj_Barricade::Update()
{
	// Can't destroy barricades while Physics simulation is running,
	// so we do it in a deffered manner.  (e.g. car runs into barricade)
	if( defferredPhysicsDisable )
	{
		m_bEnablePhysics = true;
		UpdatePhysicsEnabled(false);
		defferredPhysicsDisable = false;
	}

	if( !m_BBoxSet )
	{
		if( r3dMesh* m = GetObjectMesh() )
		{
			if( m->IsLoaded() )
			{
				m_BBoxSet = 1;
				SetBBoxLocal( GetObjectMesh()->localBBox ) ;
			}
		}
	}

	return parent::Update();
}

void obj_Barricade::Destroy(bool playParticles)
{
	if( PhysicsObject )
	{
		PxActor* actor = PhysicsObject->getPhysicsActor();
		if( actor )
		{
			m_bEnablePhysics = false;
			defferredPhysicsDisable = true;
		}
	}
	if( playParticles && !gDestroyingWorld )
	{
		switch( m_ItemID )
		{
		default:
			break;
		case WeaponConfig::ITEMID_WoodShieldBarricade:
		case WeaponConfig::ITEMID_WoodShieldBarricadeZB:
			SpawnImpactParticle(r3dHash::MakeHash(""), r3dHash::MakeHash("Barricade_Smash_Wood"), GetPosition(), r3dPoint3D(0,1,0));
			break;
		case WeaponConfig::ITEMID_BarbWireBarricade:
			SpawnImpactParticle(r3dHash::MakeHash(""), r3dHash::MakeHash("Barricade_Smash_BarbWire"), GetPosition(), r3dPoint3D(0,1,0));
			break;
		case WeaponConfig::ITEMID_RiotShieldBarricade:
		case WeaponConfig::ITEMID_RiotShieldBarricadeZB:
		case WeaponConfig::ITEMID_SSBarricadeShield:
			SpawnImpactParticle(r3dHash::MakeHash(""), r3dHash::MakeHash("Barricade_Smash_Riot"), GetPosition(), r3dPoint3D(0,1,0));
			break;
		case WeaponConfig::ITEMID_SandbagBarricade:
			SpawnImpactParticle(r3dHash::MakeHash(""), r3dHash::MakeHash("Barricade_Smash_Sandbag"), GetPosition(), r3dPoint3D(0,1,0));
			break;
		}
	}

	isRenderable = false;
}

void obj_Barricade::DrawDebugInfo()
{
//#ifndef FINAL_BUILD
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

	/*r3dPoint3D position;

	switch( r_show_barricade_debug->GetInt() )
	{
	default:
		break;

	case 1:
		{
			r3dPoint3D scrCoord;
			if(r3dProjectToScreen(GetPosition(), &scrCoord))
			{
				Font_Editor->PrintF(scrCoord.x, scrCoord.y, r3dColor24( 0, 255, 200 ), "NetID: %d", GetNetworkID() );
			}
		}
		break;
	}*/

	//AlexRedd:: barricade health
	if(gClientLogic().localPlayer_)
	{
		r3dVector dirFromPlayer = GetPosition() - gClientLogic().localPlayer_->GetPosition();
		float dist = dirFromPlayer.Length();
		if(dist < 1)
		{
			r3dPoint3D scrCoord;			
			if(r3dProjectToScreen(GetPosition() + r3dPoint3D(0, 1, 0), &scrCoord))
			{				
				if(m_Health > 0.0f && !gClientLogic().localPlayer_->bDead && (hudPause && !hudPause->isActive()) && (hudSafelock && !hudSafelock->isActive()))
				{
					const WeaponConfig* wc = g_pWeaponArmory->getWeaponConfig(m_ItemID);
					if(wc)
					{
						float maxHealth = wc->m_AmmoDamage;
						static float maxBarSize = 100.f;

						r3dColor24	clr;
						if (m_Health < maxHealth/4)
							clr = r3dColor::red;
						else if (m_Health < maxHealth/2)
							clr = r3dColor::orange;
						else
							clr = r3dColor::white;

						Font_Editor->PrintF(scrCoord.x - maxBarSize/2 + 20, scrCoord.y - 15, r3dColor24::white, "%d/%d", int(m_Health), int(maxHealth));

						r3dDrawBox2D( scrCoord.x - maxBarSize/2 - 1, scrCoord.y + 0, maxBarSize + 2, 7, r3dColor24::black );
						r3dDrawBox2D( scrCoord.x - maxBarSize/2 - 0, scrCoord.y + 1, maxBarSize + 0, 5, r3dColor24::grey );
						r3dDrawBox2D( scrCoord.x - maxBarSize/2 - 0, scrCoord.y + 1, m_Health*maxBarSize/maxHealth, 5, clr );
					}
				}
			}			
		}
	}
//#endif
}

#ifdef VEHICLES_ENABLED
bool obj_Barricade::CanStopVehicle()
{
	switch( m_ItemID )
	{
	default:
		return false;
	case WeaponConfig::ITEMID_BarbWireBarricade:
	case WeaponConfig::ITEMID_WoodShieldBarricade:
	case WeaponConfig::ITEMID_WoodShieldBarricadeZB:
		return false;

	case WeaponConfig::ITEMID_RiotShieldBarricade:
	case WeaponConfig::ITEMID_RiotShieldBarricadeZB:
	case WeaponConfig::ITEMID_SandbagBarricade:
	case WeaponConfig::ITEMID_SSBarricadeShield:
		return true;
	}
}
#endif

struct BarricadeDeferredRenderable : MeshDeferredRenderable
{
	void Init()
	{
		DrawFunc = Draw;
	}

	static void Draw( Renderable* RThis, const r3dCamera& Cam )
	{
		BarricadeDeferredRenderable* This = static_cast< BarricadeDeferredRenderable* >( RThis );
		r3dApplyPreparedMeshVSConsts(This->Parent->preparedVSConsts);

		if(This->DrawState != rsCreateSM)
			This->Parent->m_PrivateModel->DrawMeshDeferred(r3dColor::white, 0);
		else
			This->Parent->m_PrivateModel->DrawMeshShadows();
	}

	obj_Barricade* Parent;
	eRenderStageID DrawState;
};

//#ifndef FINAL_BUILD
struct BarricadeDebugRenderable : Renderable
{
	void Init()
	{
		DrawFunc = Draw;
	}

	static void Draw( Renderable* RThis, const r3dCamera& Cam )
	{
		BarricadeDebugRenderable* This = static_cast<BarricadeDebugRenderable*>( RThis );

		This->Parent->DrawDebugInfo();
	}

	obj_Barricade* Parent;
};
//#endif

void obj_Barricade::AppendShadowRenderables( RenderArray & rarr, int sliceIndex, const r3dCamera& Cam )
{
	if (!isRenderable)
		return;

	uint32_t prevCount = rarr.Count();
	m_PrivateModel->AppendShadowRenderables( rarr );
	for( uint32_t i = prevCount, e = rarr.Count(); i < e; i ++ )
	{
		BarricadeDeferredRenderable& rend = static_cast<BarricadeDeferredRenderable&>( rarr[ i ] );
		rend.Init();
		rend.Parent = this;
		rend.DrawState = rsCreateSM;
	}
}

void obj_Barricade::AppendRenderables( RenderArray ( & render_arrays  )[ rsCount ], const r3dCamera& Cam )
{
	if (!isRenderable)
		return;

	uint32_t prevCount = render_arrays[ rsFillGBuffer ].Count();
	m_PrivateModel->AppendRenderablesDeferred( render_arrays[ rsFillGBuffer ], r3dColor::white, GetSelfIllumMultiplier() );
	for( uint32_t i = prevCount, e = render_arrays[ rsFillGBuffer ].Count(); i < e; i ++ )
	{
		BarricadeDeferredRenderable& rend = static_cast<BarricadeDeferredRenderable&>( render_arrays[ rsFillGBuffer ][ i ] );
		rend.Init();
		rend.Parent = this;
		rend.DrawState = rsFillGBuffer;
	}
//#ifndef FINAL_BUILD
	//if(r_show_barricade_debug->GetBool())
	//{
		BarricadeDebugRenderable rend;
		rend.Init();
		rend.Parent		= this;
		rend.SortValue	= 2*RENDERABLE_USER_SORT_VALUE;
		render_arrays[ rsDrawFlashUI ].PushBack( rend );
	//}
//#endif
}

/*virtual*/
r3dMesh* obj_Barricade::GetObjectMesh() 
{
	return m_PrivateModel;
}

/*virtual*/
r3dMesh* obj_Barricade::GetObjectLodMesh()
{
	return m_PrivateModel;
}