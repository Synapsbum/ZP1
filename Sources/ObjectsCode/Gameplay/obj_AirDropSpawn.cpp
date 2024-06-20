//=========================================================================
//	Module: obj_AirDropSpawn.cpp
//	Copyright (C) Online Warmongers Group Inc. 2012.
//=========================================================================

#include "r3dPCH.h"
#include "r3d.h"

#include "obj_AirDropSpawn.h"
#include "../../XMLHelpers.h"
#include "../../Editors/LevelEditor.h"
#include "../WEAPONS/WeaponArmory.h"
#include "../../../../GameEngine/ai/AI_Brain.h"

//////////////////////////////////////////////////////////////////////////

IMPLEMENT_CLASS(obj_AirDropSpawn, "obj_AirDropSpawn", "Object");
AUTOREGISTER_CLASS(obj_AirDropSpawn);

extern bool g_bEditMode;

//////////////////////////////////////////////////////////////////////////

namespace
{
//////////////////////////////////////////////////////////////////////////

	struct AirDropSpawnCompositeRenderable: public Renderable
	{
		void Init()
		{
			DrawFunc = Draw;
		}

		static void Draw( Renderable* RThis, const r3dCamera& Cam )
		{
			AirDropSpawnCompositeRenderable *This = static_cast<AirDropSpawnCompositeRenderable*>(RThis);

			r3dRenderer->SetRenderingMode(R3D_BLEND_NZ | R3D_BLEND_PUSH);

			r3dDrawLine3D(This->Parent->GetPosition(), This->Parent->GetPosition() + r3dPoint3D(0, 20.0f, 0), Cam, 0.4f, r3dColor24::yellow);
			r3dDrawCircle3D(This->Parent->GetPosition(), This->Parent->spawnRadius, Cam, 0.1f, r3dColor::orange);

			r3dRenderer->Flush();
			r3dRenderer->SetRenderingMode(R3D_BLEND_POP);
		}

		obj_AirDropSpawn *Parent;
	};
}

//////////////////////////////////////////////////////////////////////////

obj_AirDropSpawn::obj_AirDropSpawn()
: spawnRadius(30.0f)
{
	serializeFile = SF_ServerData;
	m_bEnablePhysics = false;	
}

//////////////////////////////////////////////////////////////////////////

obj_AirDropSpawn::~obj_AirDropSpawn()
{

}

//////////////////////////////////////////////////////////////////////////

#define RENDERABLE_OBJ_USER_SORT_VALUE (3*RENDERABLE_USER_SORT_VALUE)
void obj_AirDropSpawn::AppendRenderables(RenderArray (& render_arrays  )[ rsCount ], const r3dCamera& Cam)
{
#ifdef FINAL_BUILD
	return;
#else
	if ( !g_bEditMode )
		return;

	if(r_hide_icons->GetInt())
		return;

	float idd = r_icons_draw_distance->GetFloat();
	idd *= idd;

	if( ( Cam - GetPosition() ).LengthSq() > idd )
		return;

	AirDropSpawnCompositeRenderable rend;

	rend.Init();
	rend.Parent		= this;
	rend.SortValue	= RENDERABLE_OBJ_USER_SORT_VALUE;

	render_arrays[ rsDrawDebugData ].PushBack( rend );
#endif
}

//////////////////////////////////////////////////////////////////////////

BOOL obj_AirDropSpawn::OnCreate()
{
	parent::OnCreate();

	DrawOrder = OBJ_DRAWORDER_LAST;

	ObjFlags |= OBJFLAG_DisableShadows;

	r3dBoundBox bboxLocal ;
	bboxLocal.Size = r3dPoint3D(2, 2, 2);
	bboxLocal.Org = -bboxLocal.Size * 0.5f;
	SetBBoxLocal(bboxLocal) ;
	UpdateTransform();

	return 1;
}

//////////////////////////////////////////////////////////////////////////

BOOL obj_AirDropSpawn::Update()
{
	return parent::Update();
}

//////////////////////////////////////////////////////////////////////////

BOOL obj_AirDropSpawn::OnDestroy()
{
	return parent::OnDestroy();
}

//////////////////////////////////////////////////////////////////////////

#ifndef FINAL_BUILD
struct tempS
{
	char* name;
	uint32_t id;
};
static bool SortLootboxesByName(const tempS d1, const tempS d2)
{
	return stricmp(d1.name, d2.name)<0;
}
float obj_AirDropSpawn::DrawPropertyEditor(float scrx, float scry, float scrw, float scrh, const AClass* startClass, const GameObjects& selected)
{
	float y = scry;

	y += parent::DrawPropertyEditor(scrx, scry, scrw, scrh, startClass, selected);
	y += 5.0f;

	y += imgui_Static(scrx, y, "AirDrop parameters:");

	y += imgui_Value_Slider(scrx, y, "Radius", &spawnRadius, 5.0f, 400.0f, "%0.2f");

	y += imgui_Value_SliderI(scrx, y, "AirDrop ItemID:", (int*)&m_AirDropItemID, 0, 1000000, "%d", false);
	PropagateChange( m_AirDropItemID, &obj_AirDropSpawn::m_AirDropItemID, this, selected ) ;	

	return y - scry;
}
#endif

//////////////////////////////////////////////////////////////////////////

void obj_AirDropSpawn::WriteSerializedData(pugi::xml_node& node)
{
	parent::WriteSerializedData(node);
	pugi::xml_node AirDropSpawnNode = node.append_child();
	AirDropSpawnNode.set_name("LootID_parameters");
	SetXMLVal("spawn_radius", AirDropSpawnNode, &spawnRadius);
	SetXMLVal("m_AirDropItemID", AirDropSpawnNode, &m_AirDropItemID);
}

// NOTE: this function must stay in sync with server version
void obj_AirDropSpawn::ReadSerializedData(pugi::xml_node& node)
{
	parent::ReadSerializedData(node);
	pugi::xml_node AirDropSpawnNode = node.child("LootID_parameters");
	GetXMLVal("spawn_radius", AirDropSpawnNode, &spawnRadius);	
	GetXMLVal("m_AirDropItemID", AirDropSpawnNode, &m_AirDropItemID);
}

//////////////////////////////////////////////////////////////////////////

GameObject * obj_AirDropSpawn::Clone()
{
	obj_AirDropSpawn * newSpawnPoint = static_cast<obj_AirDropSpawn*>(srv_CreateGameObject("obj_AirDropSpawn", FileName.c_str(), GetPosition()));
	newSpawnPoint->CloneParameters(this);
	return newSpawnPoint;
}

//////////////////////////////////////////////////////////////////////////

void obj_AirDropSpawn::CloneParameters(obj_AirDropSpawn *o)
{
	spawnRadius = o->spawnRadius;
}
