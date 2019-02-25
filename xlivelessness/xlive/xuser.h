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