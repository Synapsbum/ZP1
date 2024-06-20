#include "r3dPCH.h"
#include "r3d.h"

#include "HUD_Base.h"


BaseHUD::BaseHUD () 
: bInited ( 0 )
{
}

BaseHUD::~BaseHUD() 
{
}

void BaseHUD::Init ()
{
	if ( !bInited )
	{
		bInited = 1;
		InitPure ();
	}
}

void BaseHUD::Destroy ()
{
	if ( bInited )
	{
		bInited = 0;
		DestroyPure ();
	}
}

void BaseHUD::SetCamera ( r3dCamera &Cam )
{
	SetCameraPure ( Cam );
}

void BaseHUD::HudSelected ()
{
	OnHudSelected ();
}

void BaseHUD::HudUnselected ()
{
	OnHudUnselected ();
}

/*virtual*/
r3dPoint3D
BaseHUD::GetCamOffset() const
{
	return r3dPoint3D( 0, 0, 0 );
}

//------------------------------------------------------------------------

/*virtual*/
void
BaseHUD::SetCamPos( const r3dPoint3D& pos )
{
	FPS_Position = pos - GetCamOffset();
}