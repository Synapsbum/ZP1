#include "r3dPCH.h"
#include "r3d.h"

#include "GameCommon.h"
#include "obj_LobbyArea.h"
#include "XMLHelpers.h"

extern bool g_bEditMode;

IMPLEMENT_CLASS(obj_LobbyArea, "obj_LobbyArea", "Object");
AUTOREGISTER_CLASS(obj_LobbyArea);

r3dgameVector(obj_LobbyArea*) obj_LobbyArea::LoadedLobbyAreas;

namespace
{
	struct obj_LobbyAreaCompositeRenderable: public Renderable
	{
		void Init()
		{
			DrawFunc = Draw;
		}

		static void Draw( Renderable* RThis, const r3dCamera& Cam )
		{
			obj_LobbyAreaCompositeRenderable *This = static_cast<obj_LobbyAreaCompositeRenderable*>(RThis);

			r3dRenderer->SetRenderingMode(R3D_BLEND_NZ | R3D_BLEND_PUSH);

			r3dDrawLine3D(This->Parent->GetPosition(), This->Parent->GetPosition() + r3dPoint3D(0, 20.0f, 0), Cam, 0.4f, r3dColor24::blue);
			r3dDrawCircle3D(This->Parent->GetPosition(), This->Parent->useRadius, Cam, 0.1f, r3dColor(2, 247, 231));

			r3dRenderer->Flush();
			r3dRenderer->SetRenderingMode(R3D_BLEND_POP);
		}

		obj_LobbyArea *Parent;	
	};
}

obj_LobbyArea::obj_LobbyArea()
{
	useRadius = 30.0f;
}

obj_LobbyArea::~obj_LobbyArea()
{
}

#define RENDERABLE_OBJ_USER_SORT_VALUE (3*RENDERABLE_USER_SORT_VALUE)
void obj_LobbyArea::AppendRenderables(RenderArray (& render_arrays  )[ rsCount ], const r3dCamera& Cam)
{
	parent::AppendRenderables(render_arrays, Cam);
#ifdef FINAL_BUILD
	return;
#else
	if(g_bEditMode)
	{
		obj_LobbyAreaCompositeRenderable rend;
		rend.Init();
		rend.Parent		= this;
		rend.SortValue	= RENDERABLE_OBJ_USER_SORT_VALUE;
		render_arrays[ rsDrawDebugData ].PushBack( rend );
	}
#endif
}

void obj_LobbyArea::ReadSerializedData(pugi::xml_node& node)
{
	GameObject::ReadSerializedData(node);

	pugi::xml_node objNode = node.child("lobby_area");
	GetXMLVal("useRadius", objNode, &useRadius);
}

void obj_LobbyArea::WriteSerializedData(pugi::xml_node& node)
{
	GameObject::WriteSerializedData(node);

	pugi::xml_node objNode = node.append_child();
	objNode.set_name("lobby_area");
	SetXMLVal("useRadius", objNode, &useRadius);
}

BOOL obj_LobbyArea::Load(const char *fname)
{
	const char* cpMeshName = "Data\\ObjectsDepot\\Capture_Points\\Flag_Pole_01.sco";

	if(!parent::Load(cpMeshName)) 
		return FALSE;

	return TRUE;
}

BOOL obj_LobbyArea::OnCreate()
{
	parent::OnCreate();

	LoadedLobbyAreas.push_back(this);
	return 1;
}


BOOL obj_LobbyArea::OnDestroy()
{
	LoadedLobbyAreas.erase(std::find(LoadedLobbyAreas.begin(), LoadedLobbyAreas.end(), this));
	return parent::OnDestroy();
}

BOOL obj_LobbyArea::Update()
{
	return parent::Update();
}

//------------------------------------------------------------------------
#ifndef FINAL_BUILD
float obj_LobbyArea::DrawPropertyEditor(float scrx, float scry, float scrw, float scrh, const AClass* startClass, const GameObjects& selected)
{
	float starty = scry;

	starty += parent::DrawPropertyEditor(scrx, scry, scrw,scrh, startClass, selected );

	if( IsParentOrEqual( &ClassData, startClass ) )
	{		
		starty += imgui_Static ( scrx, starty, "Lobby_Area Parameters" );
		starty += imgui_Value_Slider(scrx, starty, "Lobby Radius", &useRadius, 0, 500.0f, "%.0f");
	}

	return starty-scry;
}
#endif
