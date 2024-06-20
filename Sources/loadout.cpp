#include "r3dPCH.h"
#include "r3d.h"

#ifndef WO_SERVER
#include "ObjectsCode/AI/AI_Player.H"
#endif
#include "loadout.h"

loadout::loadout()
{
	for (int i = 0; i < max_items; i++)
	{
		item[i].SlotID = -1;
		item[i].ItemID = -1;
		item[i].Quantity = -1;
	}
}

loadout::~loadout()
{
}

#ifndef WO_SERVER
void loadout::load_loadout(const wiCharDataFull& slot, const char* fname)
{
	r3dFile* f = r3d_open(fname, "rb");
	if (!f)
	{
		return;
	}

	const char* loadoutname;
	{
		char buf[256];
		buf[0] = 0;
		int len;
		fread(&len, sizeof(len), 1, f);
		if (len > 0)
		{
			fread(&buf[0], sizeof(char), len, f);
		}
		buf[len] = 0;
		loadoutname = buf;
	}
	r3dscpy(name, loadoutname);

	int backpackid = 0;
	int backpacksize = 0;
	fread(&backpackid, sizeof(backpackid), 1, f);
	fread(&backpacksize, sizeof(backpacksize), 1, f);
	BackpackID = backpackid;
	Backpacksize = backpacksize;
	for (int i = 0; i < backpacksize; i++)
	{
		const char* itemID;
		{
			char buf[256];
			buf[0] = 0;
			int len;
			fread(&len, sizeof(len), 1, f);
			if (len > 0)
			{
				fread(&buf[0], sizeof(char), len, f);
			}
			buf[len] = 0;
			itemID = buf;
		}

		const char* Quantity;
		{
			char buf[256];
			buf[0] = 0;
			int len;
			fread(&len, sizeof(len), 1, f);
			if (len > 0)
			{
				fread(&buf[0], sizeof(char), len, f);
			}
			buf[len] = 0;
			Quantity = buf;
		}

		int SlotID = -1;
		int valItemID = -1;
		int valQuantity = -1;
		std::sscanf(itemID, "ItemID%i: %i", &SlotID, &valItemID);
		std::sscanf(Quantity, "Quantity%i: %i", &SlotID, &valQuantity);

		item[i].SlotID = SlotID;
		item[i].ItemID = valItemID;
		item[i].Quantity = valQuantity;
	}

	fclose(f);
}

void loadout::save_loadout(const wiCharDataFull& slot, const char* fname, const char* name)
{
	FILE* f = fopen_for_write(fname, "wb");
	if (!f)
	{
		return;
	}
	
	r3dSTLString loadoutname = name;
	{
		int len = (int)loadoutname.size();
		fwrite(&len, sizeof(len), 1, f);
		fwrite(&loadoutname[0], sizeof(char), loadoutname.size(), f);
	}

	int backpackid = slot.BackpackID;
	int backpacksize = slot.BackpackSize;
	fwrite(&backpackid, sizeof(backpackid), 1, f);
	fwrite(&backpacksize, sizeof(backpacksize), 1, f);
	for (int i = 0; i < backpacksize; i++)
	{
		r3dSTLString itemID = Va("ItemID%i: %i", i, slot.Items[i].itemID);
		{
			int len = (int)itemID.size();
			fwrite(&len, sizeof(len), 1, f);
			fwrite(&itemID[0], sizeof(char), itemID.size(), f);
		}

		r3dSTLString Quantity = Va("Quantity%i: %i", i, slot.Items[i].quantity);
		{
			int len = (int)Quantity.size();
			fwrite(&len, sizeof(len), 1, f);
			fwrite(&Quantity[0], sizeof(char), Quantity.size(), f);
		}
	}
	fclose(f);
}
#endif