#include "r3dPCH.h"
#include "r3d.h"

#include "../ObjectsCode/Gameplay/BattleZone.h"
#include "../multiplayer/clientgamelogic.h"

#include "HUDDisplay.h"

extern HUDDisplay*	hudMain;

//////////////////////////////////////////////////////////////////////////

r3dVector g_vBattleZonePlayerPos = r3dVector(0,0,0);
const float g_fKickCountdownTime = 10.0f;
const float g_fSaturatePowerMax = 0.5f;
static float g_fCountdownKickPlayer = 0.0f;

//////////////////////////////////////////////////////////////////////////

static void KickPlayer()
{

}

//bool g_bExplicitlyShowBattleZoneWarning = false;

void BattleZoneWork()
{
	extern int g_CCBlackWhite;
	extern float g_fCCBlackWhitePwr;

	static bool g_bInBattleOld = true;
	static float g_fPowerNew = 0.0f;
	static float g_fTimeOffset = 0.0f;
	static float g_fFadeInSec = 0.0f;
	
	bool isInBattle = g_BattleZone.IsInBattle(g_vBattleZonePlayerPos);

#ifdef ENABLE_BATTLE_ROYALE
	if (gClientLogic().m_gameInfo.IsGameBR() && gClientLogic().m_gameHasStarted)
		return;
#endif //ENABLE_BATTLE_ROYALE

	if(gClientLogic().m_gameInfo.mapId==GBGameInfo::MAPID_ServerTest)
		return;

	if ( isInBattle )
	{
		if(!g_bInBattleOld)
		{
			if(hudMain)
				hudMain->showLeavingBattlezone(false);
			g_fPowerNew = 0.0f;
			g_fTimeOffset = 1.0f;
			g_fFadeInSec = ( g_fPowerNew - g_fCCBlackWhitePwr ) / g_fTimeOffset;
			g_bInBattleOld = true;
			g_CCBlackWhite = false;
			g_fCountdownKickPlayer = 0.0f;
		}
	}
	else
	{
		if(g_bInBattleOld)
		{
			g_fCountdownKickPlayer = g_fKickCountdownTime;
			if(hudMain)
				hudMain->showLeavingBattlezone(true);			

			g_fPowerNew = g_fSaturatePowerMax;
			g_fTimeOffset = 1.0f;
			g_fFadeInSec = ( g_fPowerNew - g_fCCBlackWhitePwr ) / g_fTimeOffset;
			g_bInBattleOld = false;
			g_CCBlackWhite = true;
		}
	}


	float fDT = r3dGetFrameTime ();

	if(g_fTimeOffset > 0.0f)
	{
		fDT = r3dTL::Min(g_fTimeOffset, fDT);
		g_fTimeOffset -= fDT;

		g_fCCBlackWhitePwr = g_fCCBlackWhitePwr + g_fFadeInSec * fDT;

		if(g_fTimeOffset < 0.0001f )
		{
			g_fTimeOffset = 0.0f;
			g_fCCBlackWhitePwr = g_fPowerNew;

			if(g_bInBattleOld)
				g_CCBlackWhite = false;
		}
	}

	if(g_fCountdownKickPlayer>0.0f)
	{
		g_fCountdownKickPlayer = r3dTL::Max ( 0.0f, g_fCountdownKickPlayer - fDT );

		if(hudMain)
			hudMain->setLeavingBattlezoneTimer((g_fCountdownKickPlayer/g_fKickCountdownTime)*100);

		if(g_fCountdownKickPlayer<0.00001f && !g_bInBattleOld )
		{
			KickPlayer ();
			g_fCountdownKickPlayer = 0.0f;
		}
	}
}
