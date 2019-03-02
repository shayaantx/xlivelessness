#include "xdefs.h"
#include "xshow.h"
#include "../xlln/DebugText.h"
#include "xlive.h"
#include "../xlln/xlln.h"

// #5206
VOID XShowMessagesUI()
{
	TRACE_FX();
	__debugbreak();
}

// #5208
VOID XShowGameInviteUI()
{
	TRACE_FX();
	__debugbreak();
}

// #5209
VOID XShowMessageComposeUI()
{
	TRACE_FX();
	__debugbreak();
}

// #5210
VOID XShowFriendRequestUI()
{
	TRACE_FX();
	__debugbreak();
}

// #5212
DWORD WINAPI XShowCustomPlayerListUI(
	DWORD dwUserIndex,
	DWORD dwFlags,
	LPCWSTR lpwszTitle,
	LPCWSTR lpwszDescription,
	CONST BYTE *pbImage,
	DWORD cbImage,
	CONST XPLAYERLIST_USER *rgPlayers,
	DWORD cPlayers,
	CONST XPLAYERLIST_BUTTON *pXButton,
	CONST XPLAYERLIST_BUTTON *pYButton,
	XPLAYERLIST_RESULT *pResult,
	XOVERLAPPED *pOverlapped)
{
	TRACE_FX();
	return ERROR_SUCCESS;
}

// #5214
VOID XShowPlayerReviewUI()
{
	TRACE_FX();
	__debugbreak();
}

// #5215
DWORD WINAPI XShowGuideUI(DWORD dwUserIndex)
{
	TRACE_FX();
	ShowXLLN(XLLN_SHOW_HOME);
	return ERROR_SUCCESS;
}

// #5216
VOID XShowKeyboardUI()
{
	TRACE_FX();
	__debugbreak();
}

// #5250
VOID XShowAchievementsUI()
{
	TRACE_FX();
	__debugbreak();
}

// #5252
DWORD WINAPI XShowGamerCardUI(DWORD dwUserIndex, XUID XuidPlayer)
{
	TRACE_FX();
	if (dwUserIndex >= XLIVE_LOCAL_USER_COUNT)
		return ERROR_NO_SUCH_USER;
	if (xlive_users_info[dwUserIndex]->UserSigninState == eXUserSigninState_NotSignedIn)
		return ERROR_NOT_LOGGED_ON;

	return ERROR_SUCCESS;
}

// #5260
DWORD WINAPI XShowSigninUI(DWORD cPanes, DWORD dwFlags)
{
	TRACE_FX();
	if (!(!(dwFlags & 0x40FFFC) && (!(dwFlags & XSSUI_FLAGS_LOCALSIGNINONLY) || !(dwFlags & XSSUI_FLAGS_SHOWONLYONLINEENABLED)))) {
		return ERROR_INVALID_PARAMETER;
	}

	// Number of users to sign in.
	if (!cPanes)
		return ERROR_INVALID_PARAMETER;

	ShowXLLN(XLLN_SHOW_LOGIN);

	//TODO XShowSigninUI
	return ERROR_SUCCESS;
	// no users signed in with multiplayer privilege. if XSSUI_FLAGS_SHOWONLYONLINEENABLED is flagged?
	return ERROR_FUNCTION_FAILED;
}

// #5271
VOID XShowPlayersUI()
{
	TRACE_FX();
	__debugbreak();
}

// #5275
VOID XShowFriendsUI()
{
	TRACE_FX();
	__debugbreak();
}

