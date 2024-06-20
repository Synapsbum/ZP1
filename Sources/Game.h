#pragma once

#ifndef __GAME__
#define __GAME__

enum TypesofHUD
{
	T_TPSGameHUD = 0,
	T_CameraHUD,
	T_EditorGameHUD,
	T_ParticleHUD,
	T_PhysicsHUD,
	T_CharacterHUD

};

enum EnumRenderSceneType
{
 Render_None		= 0,
 Render_SceneSimple	= (1 << 1 ),
 Render_SceneAdvanced	= (1 << 2 ),
 Render_FOV		= (1 << 3 ),
 Render_Bloom		= (1 << 4 ),
 Render_NVG		= (1 << 5 ),
 Render_Thermal		= (1 << 6 ),
 Render_FogFilter	= (1 << 7 ),
 Render_FilmGrain	= (1 << 8 ),

};

extern int GameRenderFlags;
extern char CurLoadString[100];

extern int CurHUDID;
void SetHud(int iHud);

#endif __GAME__