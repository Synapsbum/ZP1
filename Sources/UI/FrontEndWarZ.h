#pragma once

#include "UIMenu.h"
#include "../../ServerNetPackets/NetPacketsMaster.h"

#include "CkHttp.h"

#include "FrontEndShared.h"
#include "GameCode\UserProfile.h"
#include "GameCode\UserClans.h"
#include "GameCode\UserServerRental.h"

#include "loadout.h"

#include <fstream>

class FrontEnd
{
private:
	bool isInited_;

	Scaleform::Render::D3D9::Texture* RTScaleformTexture;
	bool		needReInitScaleformTexture;
	int			frontendStage; // 0 - login, 1-frontend	

	EGameResult	prevGameResult;

	void bindRTsToScaleForm();

public:
	r3dScaleformMovie gfxFrontEnd;

	FrontEnd();
	~FrontEnd();

	bool IsInited() { return isInited_; }

	void Load();
	void Unload() { gfxFrontEnd.Unload(); }

	int	Draw();

	void SetLoadedThings(obj_Player* plr)
	{
		r3d_assert(m_Player == NULL);
		m_Player = plr;
	}

	void drawPlayer();

	void postLoginStepInit(EGameResult gameResult);
	void initLoginStep(const char* loginErrorMsg);

	// Process Packages from Master
	void Process_Master_Packages(DWORD peerId, const r3dNetPacketHeader* packetData, int packetSize);
private:
	// login part of frontend
	friend void FrontendWarZ_LoginProcessThread(void* in_data);
	friend class callbackEnterPassword;
	static unsigned int WINAPI LoginProcessThread(void* in_data);
	static unsigned int WINAPI LoginAuthThread(void* in_data);
	HANDLE	loginThread;
	enum {
		ANS_Unactive,
		ANS_Processing,
		ANS_Timeout,
		ANS_Error,

		ANS_Logged,
		ANS_BadPassword,
		ANS_Frozen,
	};
	volatile DWORD loginAnswerCode;
	bool	DecodeAuthParams();
	void	LoginCheckAnswerCode();
	float	loginProcessStartTime;
	bool	loginMsgBoxOK_Exit;
	bool    LoadComputerToken();
	void    CreateComputerToken();

private:
	void eventPlayGame(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventCancelQuickGameSearch(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventQuitGame(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventCreateCharacter(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventRenameCharacter(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventDeleteChar(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventShowSurvivorsMap(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventShowSurvivorsMapRefresh();
	void eventReviveChar(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventReviveCharMoney(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventBuyItem(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventBackpackFromInventory(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventBackpackToInventory(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventBackpackGridSwap(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventSetSelectedChar(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventOptionsReset(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventOptionsApply(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventExtraOptionsApply(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventOptionsLanguageSelection(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventOptionsControlsRequestKeyRemap(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventOptionsControlsReset(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventOptionsControlsApply(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventOptionsVoipApply(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventOptionsVoipReset(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventCreateChangeCharacter(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventCreateCancel(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventRequestPlayerRender(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventMsgBoxCallback(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventOpenBackpackSelector(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventChangeBackpack(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventLearnSkill(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventChangeOutfit(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventSetCurrentBrowseChannel(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventTrialRequestUpgrade(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventTrialUpgradeAccount(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventBuyPremiumAccount(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventBuyAccount(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventMarketplaceActive(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventMoveAllItems(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventOpenURL(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventShowRewardPopPUp(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);

	void eventBrowseGamesRequestFilterStatus(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventBrowseGamesSetFilter(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventBrowseGamesJoin(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventBrowseGamesOnAddToFavorites(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventBrowseGamesRequestList(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);

	void eventRequestMyClanInfo(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventRequestClanList(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventCreateClan(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventClanAdminDonateGC(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventClanAdminAction(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventClanLeaveClan(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventClanDonateGCToClan(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventRequestClanApplications(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventClanApplicationAction(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventClanInviteToClan(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventClanRespondToInvite(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventClanBuySlots(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventClanApplyToJoin(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);

	void eventRequestGCTransactionData(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventRequestLotteryData(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventRequestUpgradeAccBonusLootData(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);

	void eventStorePurchaseGPCallback(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventStorePurchaseGP(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventStorePurchaseGPRequest(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);

	void eventStorePurchaseGDCallback(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventStorePurchaseGD(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	int	m_buyGDForGC;
	static	unsigned int WINAPI as_BuyGDForGCThread(void* in_data);
	void		OnBuyGDForGCSuccess();

	void eventDonateGCtoServerCallback(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventDonateGCtoServer(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventRequestShowDonateGCtoServer(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);

	void eventRequestLeaderboardData(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);

	void eventRequestMyServerList(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventRequestMyServerInfo(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventMyServerKickPlayer(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventRentServerUpdatePrice(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventRentServer(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventMyServerJoinServer(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventMyServerUpdateSettings(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventRenewServerUpdatePrice(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventRenewServer(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventFriendlyFireSet(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventTeleportPlayers(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventLoad_Loadout(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventSave_Loadout(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventDelete_Loadout(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	// Blackmarket System Begin
	static unsigned int WINAPI async_DoLoadBlackmarket(void* in_data);
	void OnDoLoadBlackmarket();

	// same vars are used for buying and removing, because logic is same
	uint32_t m_BlackmarketBuyerPK_ID = 0; // PK_BlackmarketID
	uint32_t m_BlackmarketBuyerQuantity = 0; // Quantity
	uint32_t m_BlackmarketSellerSinglePrice = 0; // SinglePrice
	__int64 m_BlackmarketBuyerInventoryID = 0; // InventoryID of bought Item
	uint32_t m_BlackmarketBuyerItemID = 0; // ItemID of bought item
	uint32_t m_BlackmarketBuyerOutQuantity = 0; // Quantity to compare if we bought all or part of it
	uint32_t m_BlackmarketBuyerItemVar1 = 0;
	uint32_t m_BlackmarketBuyerItemVar2 = 0;
	uint32_t m_BlackmarketBuyerItemVar3 = 0;
	uint32_t m_BlackmarketCurrencyType = 0;
	bool m_BlackmarketRefreshDirty = false;
	bool m_BlackmarketListWasOutdated = false;
	std::vector<uint32_t> m_BlackmarketCategoriesRequest;

	static unsigned int WINAPI async_DoBuyItemFromBlackmarket(void* in_data);
	void OnBoughtItemFromBlackmarket();

	static unsigned int WINAPI async_DoRemoveItemFromBlackmarket(void* in_data);
	void OnRemovedItemFromBlackmarket();

	static unsigned int WINAPI async_DoSellItemOnBlackmarket(void* in_data);
	void OnSellItemOnBlackmarket();

	enum class eMarketplaceScreen
	{
		Market = 0,
		Blackmarket
	};

	eMarketplaceScreen cMarketplaceScreen;

	void eventMarketplaceShowBlackmarket(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventBuyItemFromBlackmarket(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventRemoveItemFromBlackmarket(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventSellItemOnBlackmarket(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventBlackmarketRefresh(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	void eventDoLoadBlackmarketWithSelectedCategory(r3dScaleformMovie* pMovie, const Scaleform::GFx::Value* args, unsigned argCount);
	// Blackmarket System End

	static unsigned int WINAPI as_RequestClanListThread(void* in_data);
	void		OnRequestClanListSuccess();
	int			clanListRequest_SortID;
	int			clanListRequest_StartPos;
	int			clanListRequest_ClanListSize;

	static unsigned int WINAPI as_RequestGCTransactionThread(void* in_data);
	void		OnRequestGCTransactionSuccess();

	//AlexRedd:: teleport system test
	static unsigned int WINAPI as_RequestTeleportThread(void* in_data);
	void		OnRequestTeleportSuccess();
	static	unsigned int WINAPI as_TeleportCharThread(void* in_data);
	void		OnTeleportCharSuccess();
	char m_Pos[128];
	//

	//AlexRedd:: 
	static unsigned int WINAPI as_RequestUpgradeAccBonusLootDataThread(void* in_data);
	void		OnRequestUpgradeAccBonusLootDataSuccess();
	int m_AccBonusType;

	static unsigned int WINAPI as_RequestLotteryThread(void* in_data);
	void		OnRequestLotterySuccess();


	static unsigned int WINAPI as_MoveAllItemsThread(void* in_data);
	void        OnMoveAllItemsSucessfully();

	static unsigned int WINAPI as_RequestLeaderboardThread(void* in_data);
	void		OnRequestLeaderboardSuccess();
	int			leaderboardSize;
	int			leaderboard_requestTableID;
	int			leaderboard_requestStartPos;
	int			leaderboard_BoardSelected;

	char		CharRename_PreviousName[128];

	void checkForInviteFromClan();
	void setClanInfo();
	CUserClans::CreateParams_s clanCreateParams;
	r3dSTLString					   clanCreateParam_Desc;
	static unsigned int WINAPI as_CreateClanThread(void* in_data);
	void		OnCreateClanSuccess();
	void		refreshClanUIMemberList();
	typedef r3dgameList(CUserClans::ClanInfo_s) ClanList;
	ClanList cachedClanList;
	bool processClanError(int api);

	static unsigned int WINAPI as_BrowseGamesThread(void* in_data);
	void		OnGameListReceived();
	void		processNewGameList();

	void		ProcessSupervisorPings();
	int		 GetSupervisorPing(DWORD ip);

	void initFrontend();
	void initVoipOptions();
	void initItems();
	void updateInventoryAndSkillItems();
	void addStore();

	void addClientSurvivor(const wiCharDataFull& slot, int slotIndex);
	void addBackpackItems(const wiCharDataFull& slot, int slotIndex);
	void updateClientSurvivor(const wiCharDataFull& slot);

	bool		exitRequested_;
	bool		needExitByGameJoin_;
	bool		needReturnFromQuickJoin;

	//
	// Async Function Calls
	//
	typedef void (FrontEnd::*fn_finish)();
	typedef unsigned int (WINAPI *fn_thread)(void*);

	float		lastServerReqTime_;
	void		DelayServerRequest();
	float		masterConnectTime_;
	bool		ConnectToMasterServer();
	bool		WaitForGameJoinAnswer();
	bool		ParseGameJoinAnswer();
	volatile bool CancelQuickJoinRequest;

	uint32_t		CurrentBrowseChannel;

	fn_finish	asyncFinish_;
	HANDLE		asyncThread_;
	char		asyncErr_[512];

	void		StartAsyncOperation(fn_thread threadFn, fn_finish finishFn = NULL);
	void		SetAsyncError(int apiCode, const char* msg);
	void		ProcessAsyncOperation();

	static unsigned int WINAPI as_CreateCharThread(void* in_data);
	void		OnCreateCharSuccess();
	static unsigned int WINAPI as_DeleteCharThread(void* in_data);
	void		OnDeleteCharSuccess();
	static unsigned int WINAPI as_BuyLootBoxThread(void* in_data);
	void OnBuyLootBoxSuccess();
	static unsigned int WINAPI as_BuyItemThread(void* in_data);
	void		OnBuyItemSuccess();
	void		OnBuyPremiumSuccess();
	void		OnBuyAccountSuccess();
	static	unsigned int WINAPI as_BackpackFromInventoryThread(void* in_data);
	static	unsigned int WINAPI as_BackpackFromInventorySwapThread(void* in_data);
	void		OnBackpackFromInventorySuccess();
	static	unsigned int WINAPI as_ReviveCharThread(void* in_data);
	static	unsigned int WINAPI as_ReviveCharThread2(void* in_data);
	int			ReviveCharSuccessfull; // 0 -failed, 1-success, 2-show money needed
	void		OnReviveCharSuccess();
	static	unsigned int WINAPI as_BackpackToInventoryThread(void* in_data);
	void		OnBackpackToInventorySuccess();
	static	unsigned int WINAPI as_BackpackGridSwapThread(void* in_data);
	void		OnBackpackGridSwapSuccess();
	static	unsigned int WINAPI as_BackpackChangeThread(void* in_data);
	void		OnBackpackChangeSuccess();
	static	unsigned int WINAPI as_ShowRewardPopUpThread(void* in_data);
	void		OnShowRewardPopUpSuccess();
	static	unsigned int WINAPI as_CloseRewardPopUpThread(void* in_data);
	void		OnCloseRewardPopUpSuccess();

	CUserServerRental::rentParams_s m_rentServerParams;
	int	m_rentServerPrice;
	static	unsigned int WINAPI as_RentServerThread(void* in_data);
	void		OnRentServerSuccess();
	static	unsigned int WINAPI as_RenewServerThread(void* in_data);
	void		OnRenewServerSuccess();
	static	unsigned int WINAPI as_DonateToServerThread(void* in_data);
	void		OnDonateToServerSuccess();

	static	unsigned int WINAPI as_MyServerListThread(void* in_data);
	void		OnMyServerListSuccess();

	DWORD	m_myGameServerId;
	DWORD	m_myGameServerAdminKey;
	static	unsigned int WINAPI as_MyServerInfoThread(void* in_data);
	void		OnMyServerInfoSuccess();

	char	m_myServerPwd[16];
	int	m_myServerFlags;
	uint32_t	 m_myServerGameTimeLimit;
	static	unsigned int WINAPI as_MyServerSetSettingsThread(void* in_data);
	void		OnMyServerSetSettingsSuccess();

	//AlexRedd:: FriendlyFire system
	int	m_FriendlyFire;
	static	unsigned int WINAPI as_FriendlyFireThread(void* in_data);
	void		OnFriendlyFireSuccess();

	DWORD	m_myKickCharID;
	static	unsigned int WINAPI as_MyServerKickPlayerThread(void* in_data);
	void		OnMyServerKickPlayerSuccess();

	int	m_buyGpPriceCents;
	static	unsigned int WINAPI as_SteamBuyGPThread(void* in_data);
	void		OnSteamBuyGPSuccess();

	uint32_t	 m_SkillID;
	static	unsigned int WINAPI as_LearnSkillThread(void* in_data);
	void		OnLearnSkillSuccess();

	uint32_t	m_ChangeOutfit_newHead;
	uint32_t	m_ChangeOutfit_newBody;
	uint32_t	m_ChangeOutfit_newLegs;
	uint32_t	m_ChangeOutfit_newHero;
	static	unsigned int WINAPI as_ChangeOutfitThread(void* in_data);
	void		OnChangeOutfitSuccess();

	static	unsigned int WINAPI as_CharRenameThread(void* in_data);
	void		OnCharRenameSuccess();

	static unsigned int WINAPI as_PlayGameThread(void* in_data);
	static unsigned int WINAPI as_JoinGameThread(void* in_data);

	int		StoreDetectBuyIdx();
	void	SyncGraphicsUI();
	void	AddSettingsChangeFlag(DWORD flag);
	void	SetNeedUpdateSettings();
	void	SetNeedUpdateMode();
	void	UpdateSettings();
	void	updateSurvivorTotalWeight(int survivor);

	// char create
	uint32_t		m_itemID;

	uint32_t		mStore_BuyItemID;
	int		mStore_BuyPrice;
	int		mStore_BuyPriceGD;
	int		mStore_BuyQuantity;
	__int64	m_inventoryID;
	int		m_gridTo;
	int		m_gridFrom;
	int		m_Amount;
	int		m_Amount2;
	__int64	mChangeBackpack_inventoryID;

	wiCharDataFull	PlayerCreateLoadout; // save loadout when creating player, so that user can see rendered player
	uint32_t		m_CreateHeroID;
	int		m_CreateBodyIdx;
	int		m_CreateHeadIdx;
	int		m_CreateLegsIdx;
	char		m_CreateGamerTag[64];
	int		m_CreateGameMode;

	DWORD		m_joinGameServerId;
	char		m_joinGamePwd[32];

	DWORD	settingsChangeFlags_;
	bool	needUpdateSettings_;
	bool	needUpdateMode_;
	int		m_waitingForKeyRemap;

	class obj_Player* m_Player;
	int m_needPlayerRenderingRequest;

	int m_browseGamesMode; // 0 - browse, 1-recent, 2-favorites
	char m_browseGamesNameFilter[64];
	int m_browseGamesSortMode; // 0-name, 1-map, 2-mode, 3-ping
	int m_browseGamesSortOrder;
	int m_browseGamesCurrentCur;
	int m_browseGamesRequestedOper;
	int	m_closeRewardMessageActionID;

	loadout loadout_;
	int as_LOADOUT_current_running_thread;
	static unsigned int WINAPI as_LOADOUT_BackpackFromInventoryThread_MAIN(void* in_data);
	void as_LOADOUT_BackpackFromInventoryThread(int currentThread, int ItemID = -1, int slotID = -1, int Quantity = -1);
	void OnLOADOUT_BackpackFromInventorySuccess();

	void clear_loadout();
	void add_loadout(const char* filename);
	void refresh_loadout();
	void clear_loadout_pictures();
	void add_loadout_picture(const char* filename, const char* picture);
	bool find_free_loadout_name(const char* fname);

public:
	void scan_loadout();
	void load_loadout(const char* name);
	void save_loadout(const char* fname, const char* name);
};
