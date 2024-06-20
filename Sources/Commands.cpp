#include "r3dPCH.h"
#include "r3d.h"

#include <limits>
#include <regex>

#include "r3dBackgroundTaskDispatcher.h"

#include "cvar.h"

#include "Commands.h"

#include "../SF/Console/CmdProcessor.h"

#include "../SF/Console/Console.h"

#include "GameCommon.h"
#include "GameLevel.h"

#include "ObjectsCode/AI/AI_Player.h"
#include "multiplayer/ClientGameLogic.h"

#include "r3dNetwork.h"
#include "multiplayer/ClientGameLogic.h"
#include "multiplayer/MasterServerLogic.h"
#include "multiplayer/LoginSessionPoller.h"
#include "multiplayer\P2PMessages.h"

#include "UI/FrontEndWarZ.h"
#include "UI/HUDVault.h"
#include "loadout.h"

extern HUDVault* hudVault;

//------------------------------------------------------------------------
extern bool g_bExit;

// for chat commands
const bool SendChatCommand(const char* Command)
{
	if (!gClientLogic().serverConnected_)
	{
		ConPrint("Not connected to a game server!");
		return false;
	}

	PKT_C2C_ChatMessage_s n;
	n.userFlag = 0; // server will init it for others
	n.msgChannel = 0;
	r3dscpy(n.msg, Command);
	r3dscpy(n.gamertag, "<DEV>");
	p2pSendToHost(gClientLogic().localPlayer_, &n, sizeof(n));
	return true;
}

template<typename T>
void removeSubstrs(std::basic_string<T>& s,
	const std::basic_string<T>& p) {
	std::basic_string<T>::size_type n = p.length();

	for (std::basic_string<T>::size_type i = s.find(p);
		i != std::basic_string<T>::npos;
		i = s.find(p))
		s.erase(i, n);
}
#ifndef FINAL_BUILD
void Console::GetNextPlayer(std::string& input, std::string remove)
{
	removeSubstrs(input, (std::string)remove);
	removeSubstrs(input, (std::string)" ");

	bool found = false;
	char username[64];
	for (int i = 0; i<MAX_NUM_PLAYERS; ++i)
	{
		obj_Player* plr = gClientLogic().GetPlayer(i);
		if (plr)
		{
			plr->GetUserName(username);

			std::string check_s = username;
			std::smatch sm;
			std::regex match(Va("(%s)(.*)", input.c_str()));
			if (std::regex_match(check_s, sm, match))
			{
				found = true;
				break;
			}
		}
	}

	if (found)
		input = remove + Va(" %s", username);
	else
	{
		static int index = 0;
		for (int i = 0; i<MAX_NUM_PLAYERS; ++i)
		{
			obj_Player* plr = gClientLogic().GetPlayer(index);
			if (plr)
			{
				plr->GetUserName(username);
				input = remove + Va(" %s", username);
				break;
			}
			else
				input = remove + Va(" ");

			index++;
		}
	}
}
#endif
DECLARE_CMD(exit)
{
	ConPrint("Exiting Application...");
	g_bExit = true;
}

DECLARE_CMD(testserver)
{
	//Destroy = true;
	//NeworkGame = true;
	//DevTest = true;
}

// Chat Commands
DECLARE_CMD(god)
{
	if (!SendChatCommand("/god"))
		return;
}

DECLARE_CMD(hide)
{
	if (!SendChatCommand("/hide"))
		return;
}

DECLARE_CMD(teleport)
{
	if (ev.NumArgs() < 3)
	{
		ConPrint("teleport [PosX] [PosZ]");
		return;
	}

	SendChatCommand(Va("/tp %f %f", ev.GetFloat(1), ev.GetFloat(2)));
}

DECLARE_CMD(repos)
{
	SendChatCommand("/repos");
}

DECLARE_CMD(giveitem)
{
	if (ev.NumArgs() < 2)
	{
		ConPrint("giveitem [ItemID] [Num]");
		return;
	}

	SendChatCommand(Va("/gi %d %d", ev.GetInteger(1), ev.NumArgs() > 2?ev.GetInteger(2):1));
}

DECLARE_CMD(spawnvehicle)
{
	if (ev.NumArgs() < 2)
	{
		ConPrint("spawnvehicle [vehicleType]");
		return;
	}

	SendChatCommand(Va("/vspawn %d", ev.GetInteger(1)));
}

DECLARE_CMD(spawnzombie)
{
	if (ev.NumArgs() < 2)
	{
		ConPrint("spawnzombie [Num]");
		return;
	}

	SendChatCommand(Va("/zspawn %d", ev.GetInteger(1)));
}

DECLARE_CMD(spawnsuperzombie)
{
	if (ev.NumArgs() < 2)
	{
		ConPrint("spawnsuperzombie [Num]");
		return;
	}

	SendChatCommand(Va("/szspawn %d", ev.GetInteger(1)));
}

DECLARE_CMD(message)
{
	if (ev.NumArgs() < 2)
	{
		ConPrint("message [Text]");
		return;
	}

	char buff[255];
	int i = 1;
	while (ev.NumArgs() > i)
	{
		strcat(buff, ev.GetString(i));
		strcat(buff, " ");
		i++;
	}

	SendChatCommand(Va("/msg %s", buff));
}

DECLARE_CMD(zombiekill)
{
	if (ev.NumArgs() < 2)
	{
		ConPrint("zombiekill [Num]");
		return;
	}

	SendChatCommand(Va("/zkill %d", ev.GetInteger(1)));
}

DECLARE_CMD(destroybarricades)
{
	if (ev.NumArgs() < 2)
	{
		ConPrint("destroybarricades [Num]");
		return;
	}

	SendChatCommand(Va("/db %d", ev.GetInteger(1)));
}

DECLARE_CMD(reportplayer)
{
	if (ev.NumArgs() < 2)
	{
		ConPrint("reportplayer [NickName]");
		return;
	}

	char buff[255];
	int i = 1;
	while (ev.NumArgs() > i)
	{
		strcat(buff, ev.GetString(i));
		strcat(buff, " ");
		i++;
	}

	SendChatCommand(Va("/report %s", buff));
}

DECLARE_CMD(daytime)
{
	int hour = 12;
	int min = 0;
	__int64 gameUtcTime = gClientLogic().GetServerGameTime();
	struct tm* tm = _gmtime64(&gameUtcTime);
	r3d_assert(tm);

	// adjust server time to match supplied hour
	gClientLogic().gameStartUtcTime_ -= tm->tm_sec;
	gClientLogic().gameStartUtcTime_ -= (tm->tm_min) * 60;
	gClientLogic().gameStartUtcTime_ += (hour - tm->tm_hour) * 60 * 60;
	gClientLogic().gameStartUtcTime_ += (min)* 60;
	gClientLogic().lastShadowCacheReset_ = -1;

	ConPrint("Time changed to %i:%i", hour, min);
}

DECLARE_CMD(tempban)
{
	if (ev.NumArgs() < 2)
	{
		ConPrint("tempban [NickName] [Reason]");
		return;
	}

	SendChatCommand(Va("/tempban %s %s", ev.GetString(1), ev.GetString(2)));
}

DECLARE_CMD(ban)
{
	if (ev.NumArgs() < 2)
	{
		ConPrint("ban [NickName] [Reason]");
		return;
	}

	SendChatCommand(Va("/ban %s %s", ev.GetString(1), ev.GetString(2)));
}

DECLARE_CMD(chatban)
{
	if (ev.NumArgs() < 2)
	{
		ConPrint("chatban [NickName] [Reason]");
		return;
	}

	SendChatCommand(Va("/chatban %s %s", ev.GetString(1), ev.GetString(2)));
}

//------------------------------------------------------------------------
void RegisterCommands()
{
	REG_CCOMMAND(exit, "Close Application");
	REG_CCOMMAND(testserver, "Connect fast to the test server");

	// Chat Commands
	REG_CCOMMAND(god, "Chat command for god mode");
	REG_CCOMMAND(hide, "Chat command for hide mode");
	REG_CCOMMAND(teleport, "Chat command for teleport");
	REG_CCOMMAND(repos, "Chat command to repos to Terrain Heigh");
	REG_CCOMMAND(giveitem, "Chat command to give an item");
	REG_CCOMMAND(spawnvehicle, "Chat command to spawn a vehicle");
	REG_CCOMMAND(spawnzombie, "Chat command to spawn a zombie");
	REG_CCOMMAND(spawnsuperzombie, "Chat command to spawn a super zombie");
	REG_CCOMMAND(message, "Chat command to send a message");	
	REG_CCOMMAND(zombiekill, "Chat command to kill a zombies");
	REG_CCOMMAND(destroybarricades, "Chat command to destroy a barricades");
	REG_CCOMMAND(reportplayer, "Chat command to report a player");
	REG_CCOMMAND(daytime, "Chat command to set a daytime");
	REG_CCOMMAND(tempban, "Temporarily ban");
	REG_CCOMMAND(ban, "Permanently ban");
	REG_CCOMMAND(chatban, "Temporarily ban from chat");
}