#pragma once

class loadout
{
public:
	loadout();
	~loadout();

	char name[255];
	static const int max_items = 72;
	struct Items
	{
		int SlotID;
		int ItemID;
		int Quantity;
	};
	Items item[max_items];
	int BackpackID;
	int Backpacksize;

#ifndef WO_SERVER
	void load_loadout(const wiCharDataFull& slot, const char* fname);
	void save_loadout(const wiCharDataFull& slot, const char* fname, const char* name);
#endif
};