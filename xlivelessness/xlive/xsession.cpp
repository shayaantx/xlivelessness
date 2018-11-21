#include "xdefs.h"
#include "xsession.h"
#include "../xlln/DebugText.h"
#include "xlive.h"

// #5318
DWORD WINAPI XSessionStart(HANDLE hSession, DWORD dwFlags, PXOVERLAPPED pXOverlapped)
{
	TRACE_FX();
	if (!hSession)
		return ERROR_INVALID_PARAMETER;

	//TODO
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
DWORD XSessionLeaveLocal(HANDLE hSession, DWORD dwUserCount, const DWORD *pdwUserIndexes, PXOVERLAPPED pXOverlapped)
{
	TRACE_FX();
	if (!hSession)
		return ERROR_INVALID_PARAMETER;
	if (!dwUserCount || dwUserCount > XLIVE_LOCAL_USER_COUNT)
		return ERROR_INVALID_PARAMETER;
	if (!pdwUserIndexes)
		return ERROR_INVALID_PARAMETER;

	//TODO
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

	//TODO
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

	//TODO
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

	//TODO
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

	//TODO
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

// #5336
DWORD WINAPI XSessionLeaveRemote(HANDLE hSession, DWORD dwXuidCount, const XUID *pXuids, PXOVERLAPPED pXOverlapped)
{
	TRACE_FX();
	if (!hSession)
		return ERROR_INVALID_PARAMETER;
	if (!dwXuidCount)
		return ERROR_INVALID_PARAMETER;
	if (!pXuids)
		return ERROR_INVALID_PARAMETER;

	//TODO
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
