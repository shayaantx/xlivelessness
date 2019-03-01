
#include "xdefs.h"
#include "xlive.h"
#include "xuser.h"
#include "../xlln/DebugText.h"

DWORD WINAPI XUserCreateStatsEnumeratorByRank(
	DWORD dwTitleId, DWORD dwRankStart, DWORD dwNumRows, 
	DWORD dwNuStatSpec, CONST XUSER_STATS_SPEC* pSpecs, 
	DWORD * pcbBuffer, PHANDLE ph)
{
	//TODO: supposed to return a list of gamers and their data for rankings for a specific rank value?
	//TODO: would need to query a backend server that stores title->{gamer:rankData}
	TRACE_FX();
	return 1;
}

DWORD WINAPI XUserReadStats(DWORD dwTitleId, DWORD dwNumXuids, CONST XUID *pXuids, DWORD dwNumStatsSpecs, DWORD *pSpecs, DWORD *pcbResults, DWORD *pResults, PXOVERLAPPED pOverlapped)
{
	//TODO: need backend serer to store leaderboards
	TRACE_FX();
	return ERROR_SUCCESS;
}

DWORD XUserCreateStatsEnumeratorByXuid(DWORD dwTitleId, XUID XuidPivot, DWORD dwNumRows, DWORD dwNumStatsSpecs, const XUSER_STATS_SPEC * pSpecs, PDWORD * pcbBuffer, PHANDLE * ph)
{
	TRACE_FX();
	return 0;
}

DWORD XUserAwardGamerPicture(
	DWORD dwUserIndex,
	DWORD dwPictureId,
	DWORD dwReserved,
	PXOVERLAPPED pXOverlapped
)
{
	TRACE_FX();
	return 0;
}

DWORD XUserWriteProfileSettings(
	DWORD dwUserIndex,
	DWORD dwNumSettings,
	const PXUSER_PROFILE_SETTING pSettings,
	PXOVERLAPPED pXOverlapped
) 
{
	TRACE_FX();
	return 0;
}

DWORD XUserGetSigninInfo(DWORD dwUserIndex, DWORD dwFlags, PXUSER_SIGNIN_INFO pSigninInfo)
{
	TRACE_FX();
	return 1;
}

DWORD XUserSetContextEx(DWORD dwUserIndex, DWORD dwContextId, DWORD dwContextValue, PXOVERLAPPED pOverlapped)
{
	TRACE_FX();
	return ERROR_IO_PENDING;
}

DWORD XUserSetPropertyEx(DWORD dwUserIndex, DWORD dwPropertyId, DWORD cbValue, const VOID * pvValue, PXOVERLAPPED pOverlapped)
{
	TRACE_FX();
	return ERROR_IO_PENDING;
}
