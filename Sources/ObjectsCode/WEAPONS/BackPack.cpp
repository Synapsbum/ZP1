#include "r3dPCH.h"
#include "r3d.h"

#include "GameCommon.h"
#include "BackPack.h"
#include "WeaponArmory.h"

int g_BackPackBalance ;

BackPack::BackPack(const BackpackConfig* conf) : m_pConfig(conf)
{
	g_BackPackBalance ++ ;
	ActualSkin=0;
	TotalSkins=0;
	m_pConfig->aquireMesh() ;
	GetSkins();

	if (TotalSkins>0)
		ChangeBackPackSKIN(0);
}

BackPack::~BackPack()
{
	g_BackPackBalance -- ;

	m_pConfig->releaseMesh() ;
}

void BackPack::ChangeBackPackSKIN(int SkinID)
{
	if (this == NULL)
		return;

	if (TotalSkins>0)
	{
		ActualSkin=SkinID;
	}
}

r3dMesh* BackPack::getMeshSkin(bool isFirstPerson)
{
	if (this == NULL)
		return NULL;

	if (TotalSkins>0)
	{
		const BaseItemConfig* SkinCfg = g_pWeaponArmory->getConfig(BackPackSkins[ActualSkin].itemID);
		if (SkinCfg)
		{
			if (isFirstPerson)
			{
				r3dOutToLog("##### ES FIRST PERSON 1\n");
				return NULL;
			}
			else {
				return r3dGOBAddMesh(SkinCfg->m_ModelFile, true, false, true, true );
			}
		}
	}

	return m_pConfig->getMesh();
}

void BackPack::SetSkins(int SkinsID, int itemID, char* FileName, char* StoreIcon)
{
	BackPackSkins[SkinsID].itemID = itemID;
	BackPackSkins[SkinsID].FileName = FileName;
	BackPackSkins[SkinsID].StoreIcon = StoreIcon;
}

char* BackPack::GetIconSkin(int SkinID)
{
	if (SkinID<TotalSkins)
		return BackPackSkins[SkinID].StoreIcon;
	else
		return "Not found";
}

bool BackPack::GetSkins()
{
		const char* SkinDBFile = "Data/Weapons/SkinsSystem.xml";

		r3dFile* f = r3d_open(SkinDBFile, "rb");
		if ( !f )
		{
			r3dError("Failed to open %s\n", SkinDBFile);
			return false;
		}

		char* fileBuffer = game_new char[f->size + 1];
		r3d_assert(fileBuffer);
		fread(fileBuffer, f->size, 1, f);
		fileBuffer[f->size] = 0;

		pugi::xml_document xmlFile;
		pugi::xml_parse_result parseResult = xmlFile.load_buffer_inplace(fileBuffer, f->size);
		fclose(f);
		if(!parseResult)
			r3dError("Failed to parse XML, error: %s", parseResult.description());

		pugi::xml_node xmlSkinSystem = xmlFile.child("SkinsDB");
	{
		pugi::xml_node xmlSkinsArmory = xmlSkinSystem.child("SkinsArmory");
		uint32_t itemID = getItemID();
		pugi::xml_node xmlSkinBackPack = xmlSkinsArmory.child("BackPack");


		while(!xmlSkinBackPack.empty())
		{

			if (itemID!=0)
			{


				uint32_t itemIDSkin = xmlSkinBackPack.attribute("itemID").as_uint();

				if (itemIDSkin == itemID)
				{
					const BaseItemConfig* Defaultconfig = g_pWeaponArmory->getConfig(itemID);
					if (Defaultconfig)
					{
						r3dMesh* SkinMesh = r3dGOBAddMesh(Defaultconfig->m_ModelFile, true, false, true, true );
						TotalSkins=1;
						if (SkinMesh != 0)
							SetSkins(0,itemID,Defaultconfig->m_ModelFile,Defaultconfig->m_StoreIcon);
						else
							r3dOutToLog("##### This Model not is correct %s\n",Defaultconfig->m_ModelFile);

						SkinMesh=0;
					}
					int SkinsFound=1;

					for (int i=0;i<99;i++)
					{
						char buf[64];
						sprintf(buf, "sk%d", i);
						uint32_t SKINID = xmlSkinBackPack.attribute(buf).as_uint();

						if ((int)SKINID == 0)
							break;


						const BaseItemConfig* config = g_pWeaponArmory->getConfig(SKINID);

						if (config != NULL)
						{
							r3dMesh* SkinMesh = r3dGOBAddMesh(config->m_ModelFile, true, false, true, true  );

							TotalSkins++;

							if (SkinMesh != 0)
								SetSkins(SkinsFound,SKINID,config->m_ModelFile,config->m_StoreIcon);
							else
								r3dOutToLog("##### This Model not is correct %s\n",config->m_ModelFile);

							SkinMesh = 0;

							SkinsFound++;
						}
						else {
							SkinsFound++;
						}
					}
				}

			}
			xmlSkinBackPack = xmlSkinBackPack.next_sibling();
		}
	}
	return false;
}


