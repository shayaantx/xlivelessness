#pragma once

extern BOOL xlive_debug_pause;

#define XLIVE_LOCAL_USER_COUNT 4

extern BOOL xlive_users_info_changed[XLIVE_LOCAL_USER_COUNT];
extern XUSER_SIGNIN_INFO* xlive_users_info[XLIVE_LOCAL_USER_COUNT];

void Check_Overlapped(PXOVERLAPPED pOverlapped);

struct EligibleAdapter {
	char* name;
	ULONG unicastHAddr;
	ULONG unicastHMask;
	ULONG hBroadcast;
	UINT64 minLinkSpeed;
	BOOL hasDnsServer;
};

typedef enum _XLIVE_DEBUG_LEVEL
{
	XLIVE_DEBUG_LEVEL_OFF = 0,
	XLIVE_DEBUG_LEVEL_ERROR,
	XLIVE_DEBUG_LEVEL_WARNING,
	XLIVE_DEBUG_LEVEL_INFO,
	XLIVE_DEBUG_LEVEL_DEFAULT
} XLIVE_DEBUG_LEVEL;


extern EligibleAdapter xlive_network_adapter;

HRESULT XLiveOnDestroyDevice();
//this has to be marked as _stdcall otherwise you will get stack overflow exceptions
HRESULT _stdcall XLiveSetDebugLevel(
	XLIVE_DEBUG_LEVEL xdlLevel,
	XLIVE_DEBUG_LEVEL *pxdlOldLevel
);

DWORD XShowAchievementsUI(
	DWORD dwUserIndex
);

DWORD XShowFriendsUI(
	DWORD dwUserIndex
);

DWORD XShowFriendRequestUI(
	DWORD dwUserIndex,
	XUID xuidUser
);

DWORD XShowGameInviteUI(
	DWORD dwUserIndex,
	CONST XUID *pXuidRecipients,
	DWORD cRecipients,
	LPCWSTR wszUnused
);

DWORD XShowMessageComposeUI(
	DWORD dwUserIndex,
	CONST XUID *pXuidRecipients,
	DWORD cRecipients,
	LPCWSTR wszText
);

DWORD XShowMessagesUI(
	DWORD dwUserIndex
);

DWORD XShowPlayerReviewUI(
	DWORD dwUserIndex,
	XUID XuidFeedbackTarget
);

DWORD XShowPlayersUI(
	DWORD dwUserIndex
);

DWORD XUserAreUsersFriends(
	DWORD dwUserIndex,
	PXUID pXuids,
	DWORD dwXuidCount,
	PBOOL pfResult,
	PXOVERLAPPED pOverlapped
);