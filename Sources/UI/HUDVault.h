#pragma once

#include "APIScaleformGfx.h"

#include "GameCode\UserProfile.h"
#include "loadout.h"

#include <fstream>

class HUDVault
{
	bool	isActive_;
	bool	isInit;

	bool	isLockedUI;
	int		lockedOp;	

private:
	r3dScaleformMovie gfxMovie;
	r3dScaleformMovie* prevKeyboardCaptureMovie;
public:
	HUDVault();
	~HUDVault();

	bool 	Init();
	bool 	Unload();

	bool	IsInited() const { return isInit; }

	void 	Update();
	void 	Draw();

	bool	isActive() const { return isActive_; }
	void	Activate();
	void	Deactivate();
	int		m_DeadTimer_;
	int		m_DeadExitTimer_;

	bool	isUILocked() const { return isLockedUI; }
	void	setErrorMsg(const char* errMsg);
	void	unlockUI();

	void	reloadBackpackInfo();
	void	updateInventory();

	void	updateSurvivorTotalWeight();

	void	eventBackpackFromInventory(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventBackpackToInventory(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventBackpackGridSwap(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventReturnToGame(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventOpenStore(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventMoveAllItems(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventLoad_Loadout(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventSave_Loadout(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventDelete_Loadout(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);

	void clear_loadout();
	void add_loadout(const char* filename);
	void refresh_loadout();
	void clear_loadout_pictures();
	void add_loadout_picture(const char* filename, const char* picture);
	bool find_free_loadout_name(const char* fname);

	void scan_loadout();
	void load_loadout(const char* name);
	void save_loadout(const char* fname, const char* name);
};
