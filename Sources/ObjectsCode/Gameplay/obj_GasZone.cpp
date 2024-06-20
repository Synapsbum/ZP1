#include "r3dPCH.h"
#include "r3d.h"

#include "GameCommon.h"
#include "obj_GasZone.h"
#include "XMLHelpers.h"
//#include "Particle.h"
#include "Particle_Int.h"
#include "../EFFECTS/obj_ParticleSystem.h"
#include "multiplayer/ClientGameLogic.h"


extern bool g_bEditMode;
IMPLEMENT_CLASS(obj_GasArea, "obj_GasArea", "Object");
AUTOREGISTER_CLASS(obj_GasArea);

std::vector<obj_GasArea*> obj_GasArea::LoadedGasArea;

namespace
{
	struct obj_GasAreaCompositeRenderable: public Renderable
	{
		void Init()
		{
			DrawFunc = Draw;
		}

		static void Draw( Renderable* RThis, const r3dCamera& Cam )
		{
			obj_GasAreaCompositeRenderable *This = static_cast<obj_GasAreaCompositeRenderable*>(RThis);

			r3dRenderer->SetRenderingMode(R3D_BLEND_NZ | R3D_BLEND_PUSH);

			r3dDrawLine3D(This->Parent->GetPosition(), This->Parent->GetPosition() + r3dPoint3D(0, 20.0f, 0), Cam, 0.4f, r3dColor24::orange);
			r3dDrawCircle3D(This->Parent->GetPosition(), This->Parent->useRadius, Cam, 0.1f, r3dColor24::orange);
			r3dRenderer->Flush();
			r3dRenderer->SetRenderingMode(R3D_BLEND_POP);
		}

		obj_GasArea *Parent;	
	};
}

obj_GasArea::obj_GasArea()
{
	useRadius = 10.0f;
	m_gasAreaParticle = NULL;
}

obj_GasArea::~obj_GasArea()
{
}

#define RENDERABLE_OBJ_USER_SORT_VALUE (3*RENDERABLE_USER_SORT_VALUE)
void obj_GasArea::AppendRenderables(RenderArray (& render_arrays  )[ rsCount ], const r3dCamera& Cam)
{
	parent::AppendRenderables(render_arrays, Cam);
#ifdef FINAL_BUILD
	return;
#else
	if(g_bEditMode)
	{
		obj_GasAreaCompositeRenderable rend;
		rend.Init();
		rend.Parent		= this;
		rend.SortValue	= RENDERABLE_OBJ_USER_SORT_VALUE;
		render_arrays[ rsDrawDebugData ].PushBack( rend );
	}
#endif
}

void obj_GasArea::ReadSerializedData(pugi::xml_node& node)
{
	GameObject::ReadSerializedData(node);

	pugi::xml_node objNode = node.child("Gas_Area");
	GetXMLVal("useRadius", objNode, &useRadius);
	GetXMLVal("GAreaX", objNode, &GAreaX);
	GetXMLVal("GAreaY", objNode, &GAreaY);
	GetXMLVal("GAreaZ", objNode, &GAreaZ);	
}

void obj_GasArea::WriteSerializedData(pugi::xml_node& node)
{
	GameObject::WriteSerializedData(node);

	pugi::xml_node objNode = node.append_child();
	objNode.set_name("Gas_Area");
	SetXMLVal("useRadius", objNode, &useRadius);
	SetXMLVal("GAreaX", objNode, &GetPosition().x);
	SetXMLVal("GAreaY", objNode, &GetPosition().y);
	SetXMLVal("GAreaZ", objNode, &GetPosition().z);
}

BOOL obj_GasArea::Load(const char *fname)
{
	const char* cpMeshName = "Data\\ObjectsDepot\\Capture_Points\\Flag_Pole_01.sco";

	if(!parent::Load(cpMeshName)) 
		return FALSE;

	return TRUE;
}

BOOL obj_GasArea::OnCreate()
{
	parent::OnCreate(); 
	canSpawnParticle = true;

	ObjTypeFlags |= OBJTYPE_GasArea;
	LoadedGasArea.push_back(this);
	return 1;
}


BOOL obj_GasArea::OnDestroy()
{
	LoadedGasArea.erase(std::find(LoadedGasArea.begin(), LoadedGasArea.end(), this));	
	return parent::OnDestroy();
}

BOOL obj_GasArea::Update()
{
	if(!g_bEditMode)
	{
#ifdef ENABLE_BATTLE_ROYALE
		//AlexRedd:: BR mode
		if(gClientLogic().m_gameInfo.IsGameBR() && gClientLogic().m_gasStaus==1)
		{
			if(canSpawnParticle)
			{
				m_gasAreaParticle = (obj_ParticleSystem*)srv_CreateGameObject("obj_ParticleSystem", "battle_royale_gas_area", GetPosition());
				canSpawnParticle = false;
			}
			m_gasAreaParticle->bSize = gClientLogic().m_gameGasRadius;
			m_gasAreaParticle->UpdateSize();
		}
#endif //ENABLE_BATTLE_ROYALE
	}
	
	return parent::Update();
}

//------------------------------------------------------------------------
#ifndef FINAL_BUILD
float obj_GasArea::DrawPropertyEditor(float scrx, float scry, float scrw, float scrh, const AClass* startClass, const GameObjects& selected)
{
	float starty = scry;

	starty += parent::DrawPropertyEditor(scrx, scry, scrw,scrh, startClass, selected );

	if( IsParentOrEqual( &ClassData, startClass ) )
	{		
		starty += imgui_Static ( scrx, starty, "Gas Area Parameters" );
		starty += imgui_Value_Slider(scrx, starty, "Gas Radius", &useRadius, 0, 4096.0f, "%.0f");
	}

	return starty-scry;
}
#endif
