#pragma once

#include "APIScaleformGfx.h"

class HUDSkins
{
	bool	isActive_;
	bool	isInit;

private:
	r3dScaleformMovie gfxMovie;

public:
	HUDSkins();
	~HUDSkins();

	bool 	Init();
	bool 	Unload();

	int Last_SlotID;

	bool	IsInited() const { return isInit; }

	void 	Update();
	void 	Draw();

	struct SkinsData
	{
		int DefaultItemID;
		int craftedItemID;
		int RecipeID;
		int SkinID;

		SkinsData()
		{
			Reset();
		}

		void	Reset()
		{
			DefaultItemID = 0;
			craftedItemID = 0;
			RecipeID	  = 0;
			SkinID        = 0;
		}
	};

	bool	isActive() const { return isActive_; }
	void	Activate();
	void	Deactivate();

	void	eventSelectAttachment(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
};
