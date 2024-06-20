#pragma once

#include "r3dPCH.h"
#include "r3d.h"

#include "r3dNetwork.h"

static void* MainMenuSoundEvent = 0;
static int mainmenuTheme = -1;
void PlayMusic();
void StopMusic();

static char		_p2p_gameHost[MAX_PATH] = "";	// game server ip
static int		_p2p_gamePort = 0;	// game server port
static __int64	_p2p_gameSessionId = 0;

void ExecuteNetworkGame(bool quickJoin);