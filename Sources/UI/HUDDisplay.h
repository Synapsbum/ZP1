#ifndef HUDDisplay_h
#define HUDDisplay_h

#include "r3d.h"
#include "APIScaleformGfx.h"
#include "../GameCode/UserProfile.h"
#include "../ObjectsCode/weapons/Weapon.h"
#include "../ObjectsCode/Gameplay/obj_Zombie.h"

#define MAX_HUD_ACHIEVEMENT_QUEUE 8

class obj_Player;
class HUDDisplay
{
protected:
	bool Inited;

	bool chatVisible;
	bool chatInputActive;
	float lastChatMessageSent;
	int	currentChatChannel;

	int playersListVisible;

	float bloodAlpha;

	int writeNoteSavedSlotIDFrom;
	float timeoutForNotes; // stupid UI design :(
	float timeoutNoteReadAbuseReportedHideUI;

	bool RangeFinderUIVisible;

	int weaponInfoVisible;
	bool SafeZoneWarningVisible;

	int TPSReticleVisible;
	float m_PrevBreathValue;//AlexRedd:: breathBar

	bool SendDisconnectRequest;	

	bool isReadNoteVisible;
	bool isWriteNoteVisible;

	bool isReadGravestoneVisible;

#ifdef MISSIONS
	uint32_t nextMissionIndex;
	std::map<uint32_t, uint32_t> missionIDtoIndex;
	std::map<uint32_t, uint32_t> actionIDtoIndex;
	std::map<uint32_t, std::vector<uint32_t>> missionIDtoActionIDs; // first element contains the NextActionIndex to be used.
	bool missionHUDVisible;
	bool radioctiveHUDVisible;
#ifdef MISSION_TRIGGERS
	bool isReadMissionVisible;
#endif
#endif
	bool chatHUDVisible;

	bool isFoundSafeZone;	

	bool isSafelockPinVisible;
	int  isSafeLockInputEnabled;
	int  currentSafelockNetID;
	bool isSafelockPinResetOp;

	bool localPlayer_inGroup;
	bool localPlayer_groupLeader;

	std::list<std::string> hudMessagesQueue;
	std::list<std::string> hudMessagesQueue2;
	float lastDisplayedHudMessageTime;
	
	std::list<std::string> hudGameMessagesQueue;
	float lastDisplayedHudGameMessageTime;	

public:
#ifdef VEHICLES_ENABLED
	bool isShowingYouAreDead;
#endif
	r3dScaleformMovie gfxHUD;
	r3dScaleformMovie gfxBloodStreak;
	r3dScaleformMovie gfxRangeFinder;

	enum HUDIconType
	{
		HUDIcon_Attack = 0,
		HUDIcon_Defend,
		HUDIcon_Spotted,
	};
	struct HUDIcon
	{
		Scaleform::GFx::Value icon;
		r3dVector pos;
		float lifetime;
		float spawnTime;
		bool enabled;
		HUDIconType type;
		HUDIcon() :enabled(false) {}
	};
	HUDIcon m_HUDIcons[32];
	void addHUDIcon(HUDIconType type, float lifetime, const r3dVector& pos);

	enum spawn_type {
	  SPAWN_NONE,
	  SPAWN_SAFE,
	  SPAWN_WORLD,
	  SPAWN_NEAR,
	  SPAWN_GROUP,
	  SPAWN_BATTLEROYALE,
	};
	int		m_spawnSelectedType_;

	void	eventChatMessage(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventNoteWritePost(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventNoteClosed(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventNoteReportAbuse(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventPlayerListAction(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventShowPlayerListContextMenu(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventSafelockPass(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventGravestoneClosed(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventDeadToMenu(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventRespawnPlayer(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventSetAutoRespawn(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventSetSelectedRespawn(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
#if defined(MISSIONS) && defined(MISSION_TRIGGERS)
	void	eventMissionAccepted(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void	eventMissionClosed(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
#endif

public:
	HUDDisplay();
	~HUDDisplay();

	bool 	Init();
	bool 	Unload();

	// will return pointer to TEMP string. You need to save it right away, next call to this function will overwrite that temp string
	char*	applyProfanityFilter(const char* input);

	int 	Update();
	int 	Draw();

	void	setBloodAlpha(float alpha);

	// HUD functions
	void	showVOIPIconTalking(bool visible);
	void	showPingFPS(int valueping, int valuefps);// AlexRedd:: show FPS and PING

	void	setThreatValue(int percent); // [1,100]; 1-no threat, 100-maximum threat
	void	setLifeParams(int food, int water, int health, int toxicity, int stamina);
	void	setLifeConditions(const char* type, bool visible); // conditions: IconBleed, IconTemp, IconVirus, IconBadfood, IconInfection
	
	void	setWeaponInfo(const char* name, int ammo, int clips, int firemode, int durability);
	void	showWeaponInfo(int state);
	void	setSlotInfo(int slotID, const char* name, int quantity, const char* icon);
	void	setSlotCooldown(int slotID, int progress, float timeLeft); // progress: 1-100%, 105 means hide it; timeLeft - in seconds, will show "999 S" "int timeLeft""
	void	updateSlotInfo(int slotID, int quantity);
	void	showSlots(bool state);
	void	setActiveSlot(int slotID);
	void	setActivatedSlot(int slotID);
	void	showMessage(const char* text);
	void	showMessage2(const char* text);

	bool	canRespawnPlayer;	

	// durability: [1,100], speed[1,100], speedText:int, gas: [1,100], rpm: [1,100]
	void	setCarInfo(int durability, int speed, int speedText, int gas, int rpm);
	void	showCarInfo(bool visible);
	// type: buggy, stryker, truck
	void	setCarTypeInfo(const char* type);
	// type: player, team, clan, filled, empty
	void	setCarSeatInfo(int seatID, const char* type);

	void	showChat(bool showChat, bool force=false, const char* defaultInputText="");
	void	showChatInput(const char* defaultInputText);
	void	addChatMessage(int tabIndex, const char* user, const char* text, uint32_t flags);
	void	addChatMessageBox(int tabIndex, const char* user, const char* text, DWORD weapon);//AlexRedd:: for colored open case chat message
	bool	isChatInputActive() const { return chatInputActive || (r3dGetTime()-lastChatMessageSent)<0.25f || writeNoteSavedSlotIDFrom || isSafelockPinVisible; }
	void	forceChatInputDisabled() { chatInputActive = false; }
	bool	isChatVisible() const { return chatVisible; }
	void	setChatTransparency(float alpha); //[0,1]
	void	setChatChannel(int index);
	void	enableClanChannel();
	int		isWeaponInfoVisible() const{return weaponInfoVisible;} 
	void	setChatVisibility(bool vis);

	// player list fn
	void	clearPlayersList();
	void	addPlayerToList(int index, int num, void* PlayerName_s_data);
	void	showPlayersList(int flag);
	int		isPlayersListVisible() const {return playersListVisible;}
	void	setPlayerInListVoipIconVisible(int index, bool visible, bool isMuted);

	// notes
	bool	canShowWriteNote() const { return r3dGetTime() > timeoutForNotes && r_render_in_game_HUD->GetBool(); }
	void	showWriteNote(int slotIDFrom);
	void	showReadNote(const char* msg);
	void	hideReadNote();
	void	hideWriteNote();

	// Gravestones
	void	showGravestone(const char* msg, const char* victim, const char* aggressor);
	void	hideGravestone();

	// AlexRedd:: Radioactive warning message
	bool	isRadioctiveHUDVisible() { return radioctiveHUDVisible; }
	void	showRadioactiveHUD();
	void	hideRadioactiveHUD();
	//AlexRedd:: Hide chat
	bool	isChatHUDVisible() { return chatHUDVisible; }
	void	showChatHUD();
	void	hideChatHUD();	

#ifdef MISSIONS
	bool	isMissionHUDVisible() { return missionHUDVisible; }
	void	showMissionHUD();
	void	hideMissionHUD();
	void	addMissionInfo(uint32_t missionID, const char* missionName);
	void	removeMissionInfo(uint32_t missionID);
	void	addMissionAction(uint32_t missionID, uint32_t actionID, bool isCompleted, const char* actionText, const char* progress, bool hasAreaRestriction);
	void	setMissionActionInArea(uint32_t missionID, uint32_t actionID, bool isInArea);
	void	setMissionActionProgress(uint32_t missionID, uint32_t actionID, const char* progress);
	void	setMissionActionComplete(uint32_t missionID, uint32_t actionID, bool isComplete);
#ifdef MISSION_TRIGGERS
	// Mission Triggers
	void	showMission(uint32_t missionID, const char* name, const char* desc);
	void	hideMission();
#endif
#endif

	// safelock
	void	showSafelockPin(bool isUnlock, int lockboxID, bool isDoingLockboxReset=false);
	void	hideSafelockPin();
	bool	isSafelockPinActive() const{ return isSafelockPinVisible;};

	void	showRangeFinderUI(bool set) { RangeFinderUIVisible = set; }

	void	showYouAreDead(const char* killerName, const char* pic, int dist, int killerHealth, bool headShot, int xp, int gd);
	void	updateDeadTimer(int timer, int progress, int timer2, int progress2);	
	void	updateDeadExitTimer(int timer);
	void	setMinimapPosition(const r3dPoint3D& pos, const r3dPoint3D& dir);
	void	showMinimapFunc();

	void	showSafeZoneWarning(bool flag);

	void	addCharTag(const char* name, bool isSameClan, int rep, Scaleform::GFx::Value& result);
	void	setCharTagTextVisible(Scaleform::GFx::Value& icon, bool isVisible, bool isSameGroup, bool isVoipTalking, bool isSpawnIcon);
	
	void	moveUserIcon(Scaleform::GFx::Value& icon, const r3dPoint3D& pos, bool alwaysShow, bool force_invisible = false, bool pos_in_screen_space=false); 
	void	removeUserIcon(Scaleform::GFx::Value& icon);

	void	setTPSReticleVisibility(int set);

	void	addPlayerToGroupList(const char* name, bool isLeader, bool isLeaving);
	void	removePlayerFromGroupList(const char* name);
	void	aboutToLeavePlayerFromGroup(const char* name);

	void	addPlayerToVoipList(const char* name);
	void	removePlayerFromVoipList(const char* name);

#ifdef VEHICLES_ENABLED
	bool	isYouAreDead();
#endif

	void	SetReloadingProgress(int progress, const char* time); //AlexRedd:: Reload bar | progress - 0-100
	void	showReloading(bool set);			  //AlexRedd:: Reload bar
	void	SetBreathValue(float value);		  //AlexRedd:: breathBar | value from [0.0..1.0]
	void    showKills(int kills);			  //AlexRedd:: KillStreak
	void    showKillStreakMsg(int kills);			  //AlexRedd:: KillStreak	
	void	showSpawnProtect(float protect);
	void	setCharTagHealth(Scaleform::GFx::Value& bar, int health, bool isVisible);//AlexRedd:: char health bar
	void	setWeaponStat(int dmg, const char* spread, const char* recoil);//AlexRedd:: Weapon Stats
	void	AddKillMessage(obj_Player* killer, obj_Player* victim, obj_Zombie* zombie, STORE_CATEGORIES damageType, int lastTimeHitItemID, bool isHeadShot);

	//AlexRedd:: BR mode
	/*==========================================================*/
	bool	ShowWaitingForPlayersMsg;
	bool	ShowBRGDMsg;	
	void    showBRMessage(const char* text);
	void	hideBRMessage();
	void    showBRGDMessage(const char* text);
	void	hideBRGDMessage();
	void	showBRSZDistance(int distance);
	void	showBRPlayersLeft(int playersLeft);
	void	showBRStartEndOfRoundAnim(int actionId);
	void	setThreatBRValue(int percent); // [1,100]; 1-no threat, 100-maximum threat
	void	ShowMsgDeath(const char* enable);
	void	showLeavingBattlezone(bool show);//AlexRedd:: BZ
	void	setLeavingBattlezoneTimer(float progress); // //AlexRedd:: BZ progress = [0.0,1.0f]	
	/*==========================================================*/	
	void	ToggleCompass(bool enable);
	void	ToggleMiniMap(bool enable);
	void	ToggleThreatIndicator(bool enable);

	void	setScreenIconAlpha(Scaleform::GFx::Value& icon, float alpha);
	void	setScreenIconScale(Scaleform::GFx::Value& icon, float scale); // 1.0f - default size
	void	setScreenIconDistance(Scaleform::GFx::Value& icon, const r3dPoint3D& pos);	
	void	addScreenIcon(Scaleform::GFx::Value& result, const char* iconname);	
	bool    isRangeFinderUIVisible() const{ return RangeFinderUIVisible; }
	void	addCommandsData(const char* text);
	void    toggleCommandsPopUp(bool enable);
	void	refreshReticle();
	void	setReticleScale(float scale);	
	void	setReticleColor(const char* color); // color="red", "blue", "green", "white"
};

#endif