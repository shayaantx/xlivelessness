
#include "xdefs.h"
#include "xlive.h"
#include "xuser.h"
#include "../xlln/DebugText.h"

// #5284
DWORD WINAPI XUserCreateStatsEnumeratorByRank(DWORD dwTitleId, DWORD dwRankStart, DWORD dwNumRows, DWORD dwNumStatsSpecs, const XUSER_STATS_SPEC *pSpecs, DWORD *pcbBuffer, PHANDLE ph)
{
	TRACE_FX();
	if (!dwRankStart)
		return ERROR_INVALID_PARAMETER;
	if (!dwNumRows || dwNumRows > 0x64)
		return ERROR_INVALID_PARAMETER;
	if (!dwNumStatsSpecs || dwNumStatsSpecs > 0x40)
		return ERROR_INVALID_PARAMETER;
	if (!pSpecs)
		return ERROR_INVALID_PARAMETER;
	if (!pcbBuffer)
		return ERROR_INVALID_PARAMETER;
	if (!ph)
		return ERROR_INVALID_PARAMETER;

	DWORD v9 = dwNumStatsSpecs;
	DWORD v12 = v9 * (48 * dwNumRows + 16) + 8;
	if (v9)
	{
		DWORD *v13 = (DWORD*)((char *)pSpecs + 4);
		do
		{
			v12 += 28 * dwNumRows * *v13;
			v13 += 34;
			--v9;
		} while (v9);
	}
	*pcbBuffer = v12;
	*ph = CreateMutex(NULL, NULL, NULL);

	return ERROR_SUCCESS;
}

// #5267
DWORD XUserGetSigninInfo(
	DWORD dwUserIndex,
	DWORD dwFlags,
	PXUSER_SIGNIN_INFO pSigninInfo)
{
	TRACE_FX();
	__debugbreak();
	return 0;
}

// #5281
DWORD WINAPI XUserReadStats(DWORD dwTitleId, DWORD dwNumXuids, CONST XUID *pXuids, DWORD dwNumStatsSpecs, CONST XUSER_STATS_SPEC *pSpecs, DWORD *pcbResults, XUSER_STATS_READ_RESULTS *pResults, XOVERLAPPED *pXOverlapped)
{
	TRACE_FX();
	if (!dwNumXuids || dwNumXuids > 0x65)
		return ERROR_INVALID_PARAMETER;
	if (!pXuids)
		return ERROR_INVALID_PARAMETER;
	if (!dwNumStatsSpecs || dwNumStatsSpecs > 0x40)
		return ERROR_INVALID_PARAMETER;
	if (!pSpecs)
		return ERROR_INVALID_PARAMETER;
	if (!pcbResults)
		return ERROR_INVALID_PARAMETER;
	if (*pcbResults && !pResults)
		return ERROR_INVALID_PARAMETER;
	if (!*pcbResults && pResults)
		return ERROR_INVALID_PARAMETER;

	DWORD *v9 = pcbResults;
	DWORD v10 = *pcbResults;
	DWORD v11 = dwNumStatsSpecs * (52 * dwNumXuids + 16) + 8;
	if (dwNumStatsSpecs)
	{
		DWORD *v12 = (DWORD*)((char*)pSpecs + 4);
		do
		{
			v11 += 28 * dwNumXuids * *v12;
			v12 += 34;
			--dwNumStatsSpecs;
		} while (dwNumStatsSpecs);
		v9 = pcbResults;
	}
	if (v11 > v10)
	{
		*v9 = v11;
		return ERROR_INSUFFICIENT_BUFFER;
	}

	//TODO XUserReadStats
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

// #5286
DWORD WINAPI XUserCreateStatsEnumeratorByXuid(DWORD dwTitleId, XUID XuidPivot, DWORD dwNumRows, DWORD dwNumStatsSpecs, const XUSER_STATS_SPEC *pSpecs, DWORD *pcbBuffer, HANDLE *ph)
{
	TRACE_FX();
	if (!XuidPivot)
		return ERROR_INVALID_PARAMETER;
	if (!dwNumRows || dwNumRows > 0x64)
		return ERROR_INVALID_PARAMETER;
	if (!dwNumStatsSpecs || dwNumStatsSpecs > 0x40)
		return ERROR_INVALID_PARAMETER;
	if (!pSpecs)
		return ERROR_INVALID_PARAMETER;
	if (!pcbBuffer)
		return ERROR_INVALID_PARAMETER;
	if (!ph)
		return ERROR_INVALID_PARAMETER;

	DWORD v9 = dwNumStatsSpecs;
	DWORD v12 = v9 * (48 * dwNumRows + 16) + 8;
	if (v9)
	{
		DWORD *v13 = (DWORD*)((char *)pSpecs + 4);
		do
		{
			v12 += 28 * dwNumRows * *v13;
			v13 += 34;
			--v9;
		} while (v9);
	}
	*pcbBuffer = v12;
	*ph = CreateMutex(NULL, NULL, NULL);

	return ERROR_SUCCESS;
}

// #5274
VOID XUserAwardGamerPicture()
{
	TRACE_FX();
	__debugbreak();
}

DWORD XUserWriteProfileSettings(
	DWORD dwUserIndex,
	DWORD dwNumSettings,
	const PXUSER_PROFILE_SETTING pSettings,
	PXOVERLAPPED pXOverlapped
) 
{
	TRACE_FX();
	__debugbreak();
	return 0;
}

DWORD XUserSetContextEx(DWORD dwUserIndex, DWORD dwContextId, DWORD dwContextValue, PXOVERLAPPED pOverlapped)
{
	TRACE_FX();
	__debugbreak();
	return ERROR_IO_PENDING;
}

DWORD XUserSetPropertyEx(DWORD dwUserIndex, DWORD dwPropertyId, DWORD cbValue, const VOID * pvValue, PXOVERLAPPED pOverlapped)
{
	TRACE_FX();
	__debugbreak();
	return ERROR_IO_PENDING;
}
