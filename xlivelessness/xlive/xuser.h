#pragma once

DWORD WINAPI XUserCreateStatsEnumeratorByRank(
	DWORD dwTitleId, DWORD dwRankStart, DWORD dwNumRows, 
	DWORD dwNuStatSpec, CONST XUSER_STATS_SPEC* pSpecs, 
	DWORD * pcbBuffer, PHANDLE ph);

DWORD WINAPI XUserReadStats(
	DWORD dwTitleId, DWORD dwNumXuids, CONST XUID *pXuids,
	DWORD dwNumStatsSpecs, DWORD *pSpecs, DWORD *pcbResults,
	DWORD *pResults, PXOVERLAPPED pOverlapped);

DWORD XUserCreateStatsEnumeratorByXuid(
	DWORD dwTitleId,
	XUID XuidPivot,
	DWORD dwNumRows,
	DWORD dwNumStatsSpecs,
	CONST XUSER_STATS_SPEC *pSpecs,
	PDWORD *pcbBuffer,
	PHANDLE *ph
);

DWORD XUserCreateStatsEnumeratorByXuid(
	DWORD dwTitleId,
	XUID XuidPivot,
	DWORD dwNumRows,
	DWORD dwNumStatsSpecs,
	CONST XUSER_STATS_SPEC *pSpecs,
	PDWORD *pcbBuffer,
	PHANDLE *ph
);

DWORD XUserAwardGamerPicture(
	DWORD dwUserIndex,
	DWORD dwPictureId,
	DWORD dwReserved,
	PXOVERLAPPED pXOverlapped
);

DWORD XUserWriteProfileSettings(
	DWORD dwUserIndex,
	DWORD dwNumSettings,
	const PXUSER_PROFILE_SETTING pSettings,
	PXOVERLAPPED pXOverlapped
);

DWORD XUserGetSigninInfo(
	DWORD dwUserIndex,
	DWORD dwFlags,
	PXUSER_SIGNIN_INFO pSigninInfo
);

DWORD XUserSetContextEx(
	DWORD dwUserIndex,
	DWORD dwContextId,
	DWORD dwContextValue,
	PXOVERLAPPED pOverlapped
);

DWORD XUserSetPropertyEx(
	DWORD dwUserIndex,
	DWORD dwPropertyId,
	DWORD cbValue,
	CONST VOID *pvValue,
	PXOVERLAPPED pOverlapped
);