#ifndef __GEAR_H__
#define __GEAR_H__

#ifdef WO_SERVER
#error "client gear.h included in SERVER"
#endif

#include "..\..\GameCode\UserProfile.h"
#include "GearConfig.h"

class Gear
{
	friend class WeaponArmory;
public:
	Gear(const GearConfig* conf);
	~Gear();

	r3dMesh*	getModel(bool firstPersonModel) const
	{
		// always load regular model, as we need it in FPS mode too
		r3dMesh* res = m_pConfig->getMesh();
		if (firstPersonModel && (m_pConfig->category == storecat_HeroPackage)) // only body/heroes have FPS models, armor,heads,etc isn't rendered in fps mode
		{
			r3dMesh* fres = m_pConfig->getFirstPersonMesh();
			if (fres)
				return fres;
		}

		return res;
	}
	const char*			getStoreIcon() const { return m_pConfig->m_StoreIcon; }

	STORE_CATEGORIES getCategory() const { return m_pConfig->category; }

	float GetWeight() const { return m_pConfig->m_Weight; }

	uint32_t getItemID() const { return m_pConfig->m_itemID; }
//new add skin
	struct ItemSkins
	{
		int itemID;
		char* FileName;
		char* StoreIcon;
	};
	ItemSkins GearSkins[99];
	int ActualSkin;

	int TotalSkins;
	void ChangeGearSKIN(int SkinID);
	r3dMesh* getMeshSkin(bool isFirstPerson);
	void SetSkins(int SkinsID, int itemID,char* FileName, char* StoreIcon);
	char* GetIconSkin(int SkinID);
	bool GetSkins();//skins
//new add skin
private:
	const GearConfig* m_pConfig;

};

#endif //__GEAR_H__