#include "r3dPCH.h"
#include "r3d.h"

#include "GameCommon.h"
#include "obj_DroppedItem.h"

#include "ObjectsCode/weapons/WeaponArmory.h"
#include "../EFFECTS/obj_ParticleSystem.h"
#include "../../multiplayer/ClientGameLogic.h"
#include "../../EclipseStudio/Sources/ObjectsCode/world/MaterialTypes.h"
#include "../ai/AI_Player.H"
#include "ui/HUDPause.h"
#include "..\..\ui\HUDDisplay.h"

extern bool g_bEditMode;
extern HUDPause*	hudPause;
extern HUDDisplay*	hudMain;

IMPLEMENT_CLASS(obj_DroppedItem, "obj_DroppedItem", "Object");
AUTOREGISTER_CLASS(obj_DroppedItem);

obj_DroppedItem::obj_DroppedItem()
{
	m_AllowAsyncLoading = 1;
	NeedInitPhysics = 0;
	m_AirHigthPos = 300.0f;
	m_IsOnTerrain = false;
	m_IsAirDrop = false;
	NetworLocal = false;
	AirDropPos = r3dPoint3D(0,0,0);
	CasePos = r3dPoint3D(0,0,0);
	AirCraftDistance = -700.0f;
	m_FirstTime = 0;
	AirCraft=NULL;
	Light1=NULL;
	Light2=NULL;	
}

obj_DroppedItem::~obj_DroppedItem()
{
}

void obj_DroppedItem::SetHighlight( bool highlight )
{
	m_FillGBufferTarget = highlight ? rsFillGBufferAfterEffects : rsFillGBuffer;
}

bool obj_DroppedItem::GetHighlight() const
{
	return m_FillGBufferTarget == rsFillGBufferAfterEffects;
}

BOOL obj_DroppedItem::Load(const char *fname)
{
	return TRUE;
}

BOOL obj_DroppedItem::OnCreate()
{
	R3DPROFILE_FUNCTION( "obj_DroppedItem::OnCreate" );

#ifndef FINAL_BUILD
	if( g_bEditMode )
	{
		m_Item.itemID = 'GOLD';
		SetHighlight( true );
	}
#endif

	r3d_assert(m_Item.itemID);
	
	const char* cpMeshName = "";
	if(m_Item.itemID == 'GOLD')
	{
		cpMeshName = "Data\\ObjectsDepot\\Weapons\\Item_Money_Stack_01.sco";
	}
	else if(m_Item.itemID == 'LOOT')
	{
		switch(u_random(2)) 
		{
		case 0:
			cpMeshName = "Data\\ObjectsDepot\\Weapons\\Item_LootCrate_01.sco";
			break;
		case 1:
			cpMeshName = "Data\\ObjectsDepot\\Weapons\\Item_LootCrate_02.sco";
			break;
		}				
	}
	else if(m_Item.itemID == 'ARDR')
	{		
		cpMeshName = "Data\\ObjectsDepot\\ZH_Objects_Props_Military\\AirDrop_01.sco";
		/*if (m_FirstTime == 1)
		{
			SetObjFlags(OBJFLAG_SkipDraw | OBJFLAG_DisableShadows);
			//AirCraft create			
			AirCraft = (obj_Building*)srv_CreateGameObject("obj_Building", "Data\\ObjectsDepot\\ZH_Vehicles_Static\\War_Plane_AirDrop.sco", GetPosition());
			AirCraft->DrawDistanceSq = FLT_MAX;			
			r3dscpy(AirCraft->m_sAnimName,"War_Plane_AirDrop.anm");
			AirCraft->m_bGlobalAnimFolder = 0;
			AirCraft->m_bAnimated = 1;			

			//sndAircraft = SoundSys.Play(SoundSys.GetEventIDByPath("Sounds/Vehicles/Helicopter/HelicopteStatic"), AirCraft->GetPosition());
			sndAircraft = SoundSys.Play(SoundSys.GetEventIDByPath("Sounds/Vehicles/C130/HerculesEnguine"), AirCraft->GetPosition());
			SoundSys.Stop(sndAircraft);
		}*/		
	}
	else if(m_Item.itemID == 'FLPS')
	{		
		cpMeshName = "Data\\ObjectsDepot\\Weapons\\item_flare_emergency01.sco";
	}		
	else
	{
		const ModelItemConfig* cfg = (const ModelItemConfig*)g_pWeaponArmory->getConfig(m_Item.itemID);
		switch(cfg->category)
		{
			case storecat_Account:
			case storecat_Boost:
			case storecat_LootBox:
			case storecat_HeroPackage:
				r3dError("spawned item is not model");
				break;
		}
		cpMeshName = cfg->m_ModelPath;
	}
	
	if(!parent::Load(cpMeshName)) 
		return FALSE;

	if(m_Item.itemID == 'GOLD')
	{
		m_ActionUI_Title = gLangMngr.getString("Money");
		m_ActionUI_Msg = gLangMngr.getString("HoldEToPickUpMoney");
	}
	else if(m_Item.itemID == 'LOOT')
	{
		m_ActionUI_Title = gLangMngr.getString("Item");
		m_ActionUI_Msg = gLangMngr.getString("HoldEToPickUpItem");
	}
	else if(m_Item.itemID == 'ARDR')
	{
		m_ActionUI_Title = gLangMngr.getString("Item");
		m_ActionUI_Msg = gLangMngr.getString("HoldEToPickUpItem");
	}
	else if(m_Item.itemID == 'FLPS')
	{
		m_ActionUI_Title = gLangMngr.getString("Item");
		m_ActionUI_Msg = gLangMngr.getString("HoldEToPickUpItem");
	}	
	else
	{
		const BaseItemConfig* cfg = g_pWeaponArmory->getConfig(m_Item.itemID);
		m_ActionUI_Title = cfg->m_StoreName;
		m_ActionUI_Msg = gLangMngr.getString("HoldEToPickUpItem");
	}

	if (m_Item.itemID >= WeaponConfig::ITEMID_AssaultCase && m_Item.itemID <= WeaponConfig::ITEMID_MedicCase)
	{
		if (hudMain)
			hudMain->addScreenIcon(m_CaseIcon, "icon");
	}

	if (m_Item.itemID != 'ARDR')
	{
		ReadPhysicsConfig();
		PhysicsConfig.isDynamic = true; // to prevent items from falling through the ground
		PhysicsConfig.isKinematic = false; // to prevent them from being completely static, as moving static object is causing physx performance issues, and we cannot place item at this point, as it is not loaded fully yet
		PhysicsConfig.group = PHYSCOLL_TINY_GEOMETRY; // skip collision with players
		PhysicsConfig.requireNoBounceMaterial = false;
		PhysicsConfig.isFastMoving = false; // fucked up PhysX CCD is crashing all the time		

		float offset = 0.05f;
		m_bEnablePhysics = true;// zp test:: disabled PhysX
		const BaseItemConfig* cfg = g_pWeaponArmory->getConfig(m_Item.itemID);
		if(cfg )
		{
			switch (cfg->category)
			{			
			case storecat_FPSAttachment:
			case storecat_GRENADE:
			case storecat_Water:
				offset = 0.1f;
				break;
			case storecat_ASR:
			case storecat_SNP:
			case storecat_SHTG:
			case storecat_MG:			
			case storecat_SMG:
			case storecat_MELEE:
			case storecat_SUPPORT:
				offset = 0.2f;
				break;
			case storecat_HG:
				offset = 0.15f;
				break;
			case storecat_Armor:
				offset = -1.0f;
				break;
			case storecat_Helmet:
				offset = -1.6f;
				break;
			case storecat_Backpack:
				offset = -1.0f;
				break;
			}						
		}

		switch (m_Item.itemID)
		{			
		case 400010: //STANAG 45
		case 400015: //STANAG 60
		case 400016: //STANAG 30		
			offset = 0.2f;
			break;
		case 101392: //Nail Gun
			offset = 0.3f;
			break;
		}	

		SetPosition(GetPosition()+r3dPoint3D(0,offset,0));
	}
	
	if(m_Item.itemID == 'ARDR')
	{
		//AirCraft Position Start and Sound
		if (AirCraft!=NULL) 
			AirCraft->SetPosition(m_spawnPos+r3dPoint3D(0,0,AirCraftDistance));

		m_spawnPos = AirDropPos;
		SetPosition(m_spawnPos);
		m_IsAirDrop = true;
		m_bEnablePhysics = true;
	}
	else {
		m_spawnPos = GetPosition();
	}

	// will have to create it later - when we are loaded.
	if( m_bEnablePhysics )
	{
		NeedInitPhysics = 1;
		m_bEnablePhysics = 0;
	}

	parent::OnCreate();

	return 1;
}

void obj_DroppedItem::StartLights()
{
	if (Light1==NULL)
	{
		Light1 = (obj_LightHelper*)srv_CreateGameObject("obj_LightHelper", "Omni", GetPosition() + r3dPoint3D(0,3,0));
		Light1->Color = r3dColor::white;
		Light1->LT.Intensity = 2.0f;
		Light1->bOn = true;
		Light1->innerRadius = 0.0f;
		Light1->outerRadius = 8.37f;
		Light1->bKilled = false;
		Light1->DrawDistanceSq = FLT_MAX;
		Light1->SetPosition(GetPosition()+r3dPoint3D(0,0,3.11f));
	}

	if (Light2==NULL)
	{
		Light2 = (obj_LightHelper*)srv_CreateGameObject("obj_LightHelper", "Omni", GetPosition() + r3dPoint3D(0,3,0));
		Light2->Color = r3dColor::white;
		Light2->LT.Intensity = 2.0f;
		Light2->bOn = true;
		Light2->innerRadius = 0.0f;
		Light2->outerRadius = 8.37f;
		Light2->bKilled = false;
		Light2->DrawDistanceSq = FLT_MAX;
		Light2->SetPosition(GetPosition()+r3dPoint3D(0,0,-3.11f));
	}
}

void obj_DroppedItem::UpdateObjectPositionAfterCreation()
{
	if(!PhysicsObject)
		return;

	PxActor* actor = PhysicsObject->getPhysicsActor();
	if(!actor)
		return;

	PxBounds3 pxBbox = actor->getWorldBounds();
	PxVec3 pxCenter = pxBbox.getCenter();

	// place object on the ground, to prevent excessive bouncing
	{
		PxRaycastHit hit;
		PxSceneQueryFilterData filter(PxFilterData(COLLIDABLE_STATIC_MASK, 0, 0, 0), PxSceneQueryFilterFlag::eSTATIC);
		if(g_pPhysicsWorld->raycastSingle(PxVec3(pxCenter.x, pxCenter.y, pxCenter.z), PxVec3(0, -1, 0), 50.0f, PxSceneQueryFlag::eIMPACT, hit, filter))
		{
			float diff = GetPosition().y - pxBbox.minimum.y;
			SetPosition(r3dPoint3D(hit.impact.x, hit.impact.y+diff, hit.impact.z));
		}
	}
}

BOOL obj_DroppedItem::OnDestroy()
{
#ifdef FINAL_BUILD
	if (SoundSys.IsHandleValid(sndAircraft))
	{
		SoundSys.Release(sndAircraft);
		sndAircraft = NULL;
	}
#endif
	if (AirCraft!=NULL)
		AirCraft = NULL;

	if (Light1!=NULL)
		Light1=NULL;
	if (Light2!=NULL)
		Light2=NULL;	

	if (hudMain && !m_CaseIcon.IsUndefined())
		hudMain->removeUserIcon(m_CaseIcon);

	return parent::OnDestroy();
}

BOOL obj_DroppedItem::Update()
{
	if( NeedInitPhysics && MeshLOD[ 0 ] && MeshLOD[ 0 ]->IsDrawable() && m_Item.itemID != 'ARDR' )
	{
		m_bEnablePhysics = 1;
		CreatePhysicsData();
		NeedInitPhysics = 0;
		UpdateObjectPositionAfterCreation();
	}	

	if (AirCraft!=NULL && NetworLocal)
	{
		float dist = (AirCraft->GetPosition() - m_spawnPos).Length();

		if(dist<1.8f)
		{
			ResetObjFlags(OBJFLAG_SkipDraw | OBJFLAG_DisableShadows);
			//StartLights();
		}

		AirCraft->SetPosition(m_spawnPos+r3dPoint3D(0,0,AirCraftDistance));
		SoundSys.SetSoundPos(sndAircraft,AirCraft->GetPosition());
		AirCraftDistance+=0.5f;		

		if (!SoundSys.isPlaying(sndAircraft))
			SoundSys.Start(sndAircraft);

		if (AirCraftDistance>700.0f)
		{
			SoundSys.Stop(sndAircraft);
			AirCraft->setActiveFlag(0);
			AirCraft = NULL;			
		}
	}

	float alpha = 0.0f;
	if (gClientLogic().localPlayer_) // show case icon only within 50 meters of player
		alpha = 1.0f - R3D_CLAMP((GetPosition() - gClientLogic().localPlayer_->GetPosition()).Length() / 50.0f, 0.0f, 1.0f);
	if (hudMain && !m_CaseIcon.IsUndefined())
	{
		hudMain->moveUserIcon(m_CaseIcon, GetPosition() + r3dPoint3D(0, 1.0f, 0), true);
		hudMain->setScreenIconAlpha(m_CaseIcon, alpha);
		hudMain->setScreenIconScale(m_CaseIcon, R3D_CLAMP(alpha, 0.8f, 1.0f));
	}

	parent::Update();
	
	r3dPoint3D pos = GetBBoxWorld().Center();

#ifndef FINAL_BUILD
	if( !g_bEditMode )
#endif
	/*{
		const ClientGameLogic& CGL = gClientLogic();
		if(CGL.localPlayer_ && (CGL.localPlayer_->GetPosition() - pos).Length() < 10.0f)
			SetHighlight(true);
		else
			SetHighlight(false);
	}*/

	return TRUE;
}

void obj_DroppedItem::ServerPost(float posY)
{
	if (!NetworLocal)
		return;

		m_spawnPos.y=posY;
		SetPosition(m_spawnPos);

		if (Light1!=NULL)
		{
			Light1->SetPosition(r3dPoint3D(Light1->GetPosition().x,posY+1,Light1->GetPosition().z));
		}
		if (Light2!=NULL)
		{
			Light2->SetPosition(r3dPoint3D(Light2->GetPosition().x,posY+1,Light2->GetPosition().z));
		}
}

r3dColor obj_DroppedItem::GetColor(int idx)
{
	r3d_assert(idx>=0);
	r3dColor hColor = r3dColor::blue;
	switch (idx)
	{
	case 0: hColor = r3dColor::blue; break;	
	case 1: hColor = r3dColor::green; break;
	case 2:	hColor = r3dColor::grey; break;
	case 3:	hColor = r3dColor::orange; break;
	case 4:	hColor = r3dColor::red;	break;
	case 5:	hColor = r3dColor::white; break;
	case 6:	hColor = r3dColor::yellow; break;
	}
	return hColor;
}

r3dColor obj_DroppedItem::GetCatOutLineColor()
{
	r3dColor hColor = r3dColor::blue;
	const BaseItemConfig* cfg = g_pWeaponArmory->getConfig(m_Item.itemID);
	if(cfg )
	{
		switch (cfg->category)
		{
		default: hColor = r3dColor::blue;
		case storecat_FPSAttachment:			
			hColor = GetColor(r_highlight_att_clr->GetInt());
			break;
		case storecat_GRENADE:
			hColor = GetColor(r_highlight_grenade_clr->GetInt());
			break;
		case storecat_ASR:
		case storecat_SNP:
		case storecat_SHTG:
		case storecat_MG:
		case storecat_HG:
		case storecat_SMG:
			hColor = GetColor(r_highlight_wpn_clr->GetInt());
			break;
		case storecat_MELEE:
			hColor = GetColor(r_highlight_melee_clr->GetInt());
			break;
		case storecat_Food:
		case storecat_Water:
			hColor = GetColor(r_highlight_food_clr->GetInt());
			break;
		case storecat_Armor:
		case storecat_Helmet:
			hColor = GetColor(r_highlight_gear_clr->GetInt());
			break;
		case storecat_Backpack:
			hColor = GetColor(r_highlight_bp_clr->GetInt());
			break;
		case storecat_UsableItem:
			hColor = GetColor(r_highlight_ui_clr->GetInt());
			break;
		case storecat_INVALID:		
		case storecat_LootBox:		
		case storecat_Components:
		case storecat_CraftRecipe:
			hColor = GetColor(r_highlight_other_clr->GetInt());
			break;
		}
	}
	return hColor;
}

r3dColor obj_DroppedItem::GetOutLineColor()
{
	r3dColor hColor = r3dColor::blue;
	switch (g_highlight_mode->GetInt())
	{
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
		hColor = GetColor(g_highlight_mode->GetInt());
		break;
	case 7:
		hColor = GetCatOutLineColor();
		break;
	}
	return hColor;
}

r3dColor obj_DroppedItem::GetOutLineColorNew()
{
	r3dColor hColor = r3dColor::white;
	return hColor;
}

void obj_DroppedItem::AppendRenderables( RenderArray ( & render_arrays )[ rsCount ], const r3dCamera& Cam )
{
	MeshGameObject::AppendRenderables( render_arrays, Cam );
	
	if (GetHighlight())
	{
		//return; //ZP : 20/06/2024 highlight kapatildi

		if (m_Item.itemID == 'ARDR' || m_Item.itemID == 'FLPS')
			return;			
		
		MeshObjDeferredHighlightRenderable rend;	
		rend.Init(MeshGameObject::GetObjectLodMesh(), this, GetOutLineColorNew().GetPacked()); //ZP: 20/06/2024 - Degistirildi.
		rend.SortValue = 0;
		rend.DoExtrude = 0;

		render_arrays[rsDrawHighlightPass0].PushBack(rend);

		rend.DoExtrude = 1;
		render_arrays[rsDrawHighlightPass1].PushBack(rend);
	}
}
