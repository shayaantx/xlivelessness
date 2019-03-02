#include "xdefs.h"
#include "xsession.h"
#include "../xlln/DebugText.h"
#include "xlive.h"

XSESSION_LOCAL_DETAILS xlive_session_details;

INT InitXSession()
{
	TRACE_FX();

	memset(&xlive_session_details, 0, sizeof(XSESSION_LOCAL_DETAILS));

	return S_OK;
}

INT UninitXSession()
{
	TRACE_FX();
	return S_OK;
}

// #5300
DWORD WINAPI XSessionCreate(DWORD dwFlags, DWORD dwUserIndex, DWORD dwMaxPublicSlots, DWORD dwMaxPrivateSlots, ULONGLONG *pqwSessionNonce, PXSESSION_INFO pSessionInfo, PXOVERLAPPED pXOverlapped, PHANDLE phEnum)
{
	TRACE_FX();
	if (dwUserIndex >= XLIVE_LOCAL_USER_COUNT)
		return ERROR_NO_SUCH_USER;
	if (xlive_users_info[dwUserIndex]->UserSigninState == eXUserSigninState_NotSignedIn)
		return ERROR_NOT_LOGGED_ON;
	if (!pqwSessionNonce)
		return ERROR_INVALID_PARAMETER;
	if (!pSessionInfo)
		return ERROR_INVALID_PARAMETER;
	if (!phEnum)
		return ERROR_INVALID_PARAMETER;
	if (dwFlags & ~(XSESSION_CREATE_USES_MASK | XSESSION_CREATE_MODIFIERS_MASK | 0x1000))//FIXME unknown macro or their mistake?
		return ERROR_INVALID_PARAMETER;
	if (dwFlags & XSESSION_CREATE_USES_MATCHMAKING && !(dwFlags & XSESSION_CREATE_USES_PEER_NETWORK))
		return ERROR_INVALID_PARAMETER;
	if (dwFlags & XSESSION_CREATE_USES_ARBITRATION && !(dwFlags & XSESSION_CREATE_USES_STATS))
		return ERROR_INVALID_PARAMETER;
	if (dwFlags & XSESSION_CREATE_USES_ARBITRATION && !(dwFlags & XSESSION_CREATE_USES_PEER_NETWORK))
		return ERROR_INVALID_PARAMETER;
	if (dwFlags & XSESSION_CREATE_HOST && !(dwFlags & (XSESSION_CREATE_USES_PEER_NETWORK | XSESSION_CREATE_USES_STATS | XSESSION_CREATE_USES_MATCHMAKING)))
		return ERROR_INVALID_PARAMETER;
	if (dwFlags & XSESSION_CREATE_MODIFIERS_MASK) {
		if (!(dwFlags & (XSESSION_CREATE_USES_PRESENCE | XSESSION_CREATE_USES_MATCHMAKING))) {
			return ERROR_INVALID_PARAMETER;
		}
		if (!(dwFlags & XSESSION_CREATE_USES_PRESENCE) && (dwFlags & XSESSION_CREATE_USES_MATCHMAKING) && (dwFlags & XSESSION_CREATE_MODIFIERS_MASK) != (dwFlags & XSESSION_CREATE_JOIN_IN_PROGRESS_DISABLED)) {
			return ERROR_INVALID_PARAMETER;
		}
		if ((dwFlags & XSESSION_CREATE_JOIN_VIA_PRESENCE_DISABLED) && (dwFlags & XSESSION_CREATE_JOIN_VIA_PRESENCE_FRIENDS_ONLY)) {
			return ERROR_INVALID_PARAMETER;
		}
	}

	*phEnum = CreateMutex(NULL, NULL, NULL);

	// local cache
	xlive_session_details.dwUserIndexHost = dwUserIndex;//XUSER_INDEX_NONE ?

	// already filled - SetContext
	//xlive_session_details.dwGameType = 0;
	//xlive_session_details.dwGameMode = 0;

	xlive_session_details.dwFlags = dwFlags;

	xlive_session_details.dwMaxPublicSlots = dwMaxPublicSlots;
	xlive_session_details.dwMaxPrivateSlots = dwMaxPrivateSlots;
	xlive_session_details.dwAvailablePublicSlots = dwMaxPublicSlots;
	xlive_session_details.dwAvailablePrivateSlots = dwMaxPrivateSlots;

	xlive_session_details.dwActualMemberCount = 0;
	xlive_session_details.dwReturnedMemberCount = 0;

	xlive_session_details.eState = XSESSION_STATE_LOBBY;
	xlive_session_details.qwNonce = *pqwSessionNonce;

	//xlive_session_details.sessionInfo = *pSessionInfo; //check this.


	//TODO XSessionCreate
	if (pXOverlapped) {
		//asynchronous

		pXOverlapped->InternalLow = ERROR_SUCCESS;
		pXOverlapped->InternalHigh = ERROR_SUCCESS;
		pXOverlapped->dwExtendedError = ERROR_SUCCESS;

		Check_Overlapped(pXOverlapped);

		return ERROR_IO_PENDING;
	}
	else {
		//synchronous
		//return result;
	}
	return ERROR_SUCCESS;
}

// #5317
VOID XSessionWriteStats()
{
	TRACE_FX();
	__debugbreak();
}

// #5318
DWORD WINAPI XSessionStart(HANDLE hSession, DWORD dwFlags, PXOVERLAPPED pXOverlapped)
{
	TRACE_FX();
	if (!hSession)
		return ERROR_INVALID_PARAMETER;

	//TODO XSessionStart
	if (pXOverlapped) {
		//asynchronous

		pXOverlapped->InternalLow = ERROR_SUCCESS;
		pXOverlapped->InternalHigh = ERROR_SUCCESS;
		pXOverlapped->dwExtendedError = ERROR_SUCCESS;

		Check_Overlapped(pXOverlapped);

		return ERROR_IO_PENDING;
	}
	else {
		//synchronous
		//return result;
	}
	return ERROR_SUCCESS;
}

// #5320
VOID XSessionSearchByID()
{
	TRACE_FX();
	__debugbreak();
}

// #5321
VOID XSessionSearch()
{
	TRACE_FX();
	__debugbreak();
}

// #5322
DWORD WINAPI XSessionModify(HANDLE hSession, DWORD dwFlags, DWORD dwMaxPublicSlots, DWORD dwMaxPrivateSlots, XOVERLAPPED *pXOverlapped)
{
	TRACE_FX();
	if (!hSession)
		return ERROR_INVALID_PARAMETER;
	if (!(dwFlags && 0x200) || !(dwFlags && 0x800))
		return ERROR_INVALID_PARAMETER;

	//TODO XSessionModify
	if (pXOverlapped) {
		//asynchronous

		pXOverlapped->InternalLow = ERROR_SUCCESS;
		pXOverlapped->InternalHigh = ERROR_SUCCESS;
		pXOverlapped->dwExtendedError = ERROR_SUCCESS;

		Check_Overlapped(pXOverlapped);

		return ERROR_IO_PENDING;
	}
	else {
		//synchronous
		//return result;
	}
	return ERROR_SUCCESS;
}

// #5325
DWORD WINAPI XSessionLeaveLocal(HANDLE hSession, DWORD dwUserCount, const DWORD *pdwUserIndexes, PXOVERLAPPED pXOverlapped)
{
	TRACE_FX();
	if (!hSession)
		return ERROR_INVALID_PARAMETER;
	if (!dwUserCount || dwUserCount > XLIVE_LOCAL_USER_COUNT)
		return ERROR_INVALID_PARAMETER;
	if (!pdwUserIndexes)
		return ERROR_INVALID_PARAMETER;

	//TODO XSessionLeaveLocal
	if (pXOverlapped) {
		//asynchronous

		pXOverlapped->InternalLow = ERROR_SUCCESS;
		pXOverlapped->InternalHigh = ERROR_SUCCESS;
		pXOverlapped->dwExtendedError = ERROR_SUCCESS;

		Check_Overlapped(pXOverlapped);

		return ERROR_IO_PENDING;
	}
	else {
		//synchronous
		//return result;
	}
	return ERROR_SUCCESS;
}

// #5326
DWORD WINAPI XSessionJoinRemote(HANDLE hSession, DWORD dwXuidCount, const XUID *pXuids, const BOOL *pfPrivateSlots, PXOVERLAPPED pXOverlapped)
{
	TRACE_FX();
	if (!hSession)
		return ERROR_INVALID_PARAMETER;
	if (!dwXuidCount)
		return ERROR_INVALID_PARAMETER;
	if (!pXuids)
		return ERROR_INVALID_PARAMETER;
	if (!pfPrivateSlots)
		return ERROR_INVALID_PARAMETER;

	//TODO XSessionJoinRemote
	if (pXOverlapped) {
		//asynchronous

		pXOverlapped->InternalLow = ERROR_SUCCESS;
		pXOverlapped->InternalHigh = ERROR_SUCCESS;
		pXOverlapped->dwExtendedError = ERROR_SUCCESS;

		Check_Overlapped(pXOverlapped);

		return ERROR_IO_PENDING;
	}
	else {
		//synchronous
		//return result;
	}
	return ERROR_SUCCESS;
}

// #5327
DWORD WINAPI XSessionJoinLocal(HANDLE hSession, DWORD dwUserCount, const DWORD *pdwUserIndexes, const BOOL *pfPrivateSlots, PXOVERLAPPED pXOverlapped)
{
	TRACE_FX();
	if (!hSession)
		return ERROR_INVALID_PARAMETER;
	if (!dwUserCount || dwUserCount > XLIVE_LOCAL_USER_COUNT)
		return ERROR_INVALID_PARAMETER;
	if (!pdwUserIndexes)
		return ERROR_INVALID_PARAMETER;
	if (!pfPrivateSlots)
		return ERROR_INVALID_PARAMETER;

	//TODO XSessionJoinLocal
	if (pXOverlapped) {
		//asynchronous

		pXOverlapped->InternalLow = ERROR_SUCCESS;
		pXOverlapped->InternalHigh = ERROR_SUCCESS;
		pXOverlapped->dwExtendedError = ERROR_SUCCESS;

		Check_Overlapped(pXOverlapped);

		return ERROR_IO_PENDING;
	}
	else {
		//synchronous
		//return result;
	}
	return ERROR_SUCCESS;
}

// #5329
DWORD WINAPI XSessionFlushStats(HANDLE hSession, XOVERLAPPED *pXOverlapped)
{
	TRACE_FX();
	if (!hSession)
		return ERROR_INVALID_PARAMETER;

	//TODO XSessionFlushStats
	if (pXOverlapped) {
		//asynchronous

		pXOverlapped->InternalLow = ERROR_SUCCESS;
		pXOverlapped->InternalHigh = ERROR_SUCCESS;
		pXOverlapped->dwExtendedError = ERROR_SUCCESS;

		Check_Overlapped(pXOverlapped);

		return ERROR_IO_PENDING;
	}
	else {
		//synchronous
		//return result;
	}
	return ERROR_SUCCESS;
}

// #5330
DWORD WINAPI XSessionDelete(HANDLE hSession, PXOVERLAPPED pXOverlapped)
{
	TRACE_FX();
	if (!hSession)
		return ERROR_INVALID_PARAMETER;

	//TODO XSessionDelete
	if (pXOverlapped) {
		//asynchronous

		pXOverlapped->InternalLow = ERROR_SUCCESS;
		pXOverlapped->InternalHigh = ERROR_SUCCESS;
		pXOverlapped->dwExtendedError = ERROR_SUCCESS;

		Check_Overlapped(pXOverlapped);

		return ERROR_IO_PENDING;
	}
	else {
		//synchronous
		//return result;
	}
	return ERROR_SUCCESS;
}

// #5332
DWORD WINAPI XSessionEnd(HANDLE hSession, PXOVERLAPPED pXOverlapped)
{
	TRACE_FX();
	if (!hSession)
		return ERROR_INVALID_PARAMETER;

	//TODO XSessionEnd
	if (pXOverlapped) {
		//asynchronous

		pXOverlapped->InternalLow = ERROR_SUCCESS;
		pXOverlapped->InternalHigh = ERROR_SUCCESS;
		pXOverlapped->dwExtendedError = ERROR_SUCCESS;

		Check_Overlapped(pXOverlapped);

		return ERROR_IO_PENDING;
	}
	else {
		//synchronous
		//return result;
	}
	return ERROR_SUCCESS;
}

// #5333
VOID XSessionArbitrationRegister()
{
	TRACE_FX();
	__debugbreak();
}

// #5336
DWORD WINAPI XSessionLeaveRemote(HANDLE hSession, DWORD dwXuidCount, const XUID *pXuids, XOVERLAPPED *pXOverlapped)
{
	TRACE_FX();
	if (!hSession)
		return ERROR_INVALID_PARAMETER;
	if (!dwXuidCount)
		return ERROR_INVALID_PARAMETER;
	if (!pXuids)
		return ERROR_INVALID_PARAMETER;

	//TODO XSessionLeaveRemote
	if (pXOverlapped) {
		//asynchronous

		pXOverlapped->InternalLow = ERROR_SUCCESS;
		pXOverlapped->InternalHigh = ERROR_SUCCESS;
		pXOverlapped->dwExtendedError = ERROR_SUCCESS;

		Check_Overlapped(pXOverlapped);

		return ERROR_IO_PENDING;
	}
	else {
		//synchronous
		//return result;
	}
	return ERROR_SUCCESS;
}

// #5342
DWORD WINAPI XSessionModifySkill(HANDLE hSession, DWORD dwXuidCount, const XUID *pXuids, XOVERLAPPED *pXOverlapped)
{
	TRACE_FX();
	if (!hSession)
		return ERROR_INVALID_PARAMETER;
	if (!dwXuidCount)
		return ERROR_INVALID_PARAMETER;
	if (!pXuids)
		return ERROR_INVALID_PARAMETER;

	//TODO XSessionModifySkill
	if (pXOverlapped) {
		//asynchronous

		pXOverlapped->InternalLow = ERROR_SUCCESS;
		pXOverlapped->InternalHigh = ERROR_SUCCESS;
		pXOverlapped->dwExtendedError = ERROR_SUCCESS;

		Check_Overlapped(pXOverlapped);

		return ERROR_IO_PENDING;
	}
	else {
		//synchronous
		//return result;
	}
	return ERROR_SUCCESS;
}
