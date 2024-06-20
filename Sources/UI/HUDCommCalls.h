#pragma once

#include "APIScaleformGfx.h"

class HUDCommCalls
{
	struct CallCommand
	{
		int id;
		const char* txtCommand;
		int voiceCmdMale;
		int voiceCmdMaleRadio;
		int voiceCmdFamale;
		int voiceCmdFamaleRadio;
		CallCommand() :id(-1), txtCommand(0), voiceCmdMale(0), voiceCmdMaleRadio(0), voiceCmdFamale(0), voiceCmdFamaleRadio(0){}
		CallCommand(int i, const char* txt, int voiceMale, int voiceMaleRadio, int voiceFamale, int voiceFamaleRadio) : id(i), txtCommand(txt), voiceCmdMale(voiceMale), voiceCmdMaleRadio(voiceMaleRadio), voiceCmdFamale(voiceFamale), voiceCmdFamaleRadio(voiceFamaleRadio){}
	};

	CallCommand m_CommandsG[10];
	CallCommand m_CommandsH[10];	
//public:
	//r3dScaleformMovie gfxMovie;

public:
	HUDCommCalls();
	~HUDCommCalls();
	
	enum CommCallsTypeEnum
	{
		Command_G0 = 0, /* [NOT USED] */
		Command_G1, /* [Affirmative] */
		Command_G2, /* [Negative] */
		Command_G3, /* [Nice shot] */
		Command_G4, /* [Fight] */
		Command_G5, /* [Follow Me] */
		Command_G6, /* [Cover me] */
		Command_G7, /* [Forward] */
		Command_G8, /* [Sector Clear] */
		Command_G9, /* [Stick Together Team] */
		Command_H1, /* [WTF?] */
		Command_H2, /* [Need Medic] */
		Command_H3, /* [Need Ammo] */
		Command_H4, /* [Need Help] */
		Command_R1, /* [Attack] */
		Command_R2, /* [Defend] */
		Command_R3, /* [Spotted] */
	};

	bool 	Init();
	bool	IsInited () const { return m_bInited; }
	bool 	Unload();

	void 	Update();
	void 	Draw();

	bool	isVisible() const { return m_bVisible; }
	void    Deactivate();

	void	PlayMessage(class obj_Player* from, int id); // play voice over, send message to chat

	void	showHUDIcon(class obj_Player* from, int id, const r3dVector& pos);
private:
	void	eventPressButton(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	addCommRoseCommand(int id);


	void	ShowMessages(int msgType);
	void	HideMessages();
	bool	m_bInited;
	bool	m_bVisible;
	int		m_CurrentMsgType;
	bool	RUS_LANG;	
};
