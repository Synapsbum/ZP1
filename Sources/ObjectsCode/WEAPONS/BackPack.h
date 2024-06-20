#ifndef __BackPack_H__
#define __BackPack_H__

#ifdef WO_SERVER
  #error "client BackPack.h included in SERVER"
#endif

#include "..\..\GameCode\UserProfile.h"
#include "GearConfig.h"

class BackPack
{
	friend class WeaponArmory;
public:
	BackPack(const BackpackConfig* conf);
	~BackPack();

	r3dMesh*	getModel(bool firstPersonModel) const
	{
		// always load regular model, as we need it in FPS mode too
		r3dMesh* res = m_pConfig->getMesh();
		if (firstPersonModel)
			return NULL;
		else
			return res;
	}
	const char*			getStoreIcon() const { return m_pConfig->m_StoreIcon; }

	STORE_CATEGORIES getCategory() const { return m_pConfig->category; }

	float GetWeight() const { return m_pConfig->m_Weight; }

	uint32_t getItemID() const { return m_pConfig->m_itemID; }

	struct ItemSkins
	{
		int itemID;
		char* FileName;
		char* StoreIcon;
	};
	ItemSkins BackPackSkins[99];
	int ActualSkin;

	int TotalSkins;
	void ChangeBackPackSKIN(int SkinID);
	r3dMesh* getMeshSkin(bool isFirstPerson);
	void SetSkins(int SkinsID, int itemID,char* FileName, char* StoreIcon);
	char* GetIconSkin(int SkinID);
	bool GetSkins();//skins

private:
	const BackpackConfig* m_pConfig;

};

#endif //__BackPack_H__
