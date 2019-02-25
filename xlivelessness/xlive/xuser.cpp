
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