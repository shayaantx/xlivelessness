#pragma once
#include <map>
#include <vector>

typedef struct {
	XLOCATOR_SEARCHRESULT *searchresult;
	DWORD broadcastTime;
} XLOCATOR_SESSION;

extern CRITICAL_SECTION xlive_xlocator_enumerators_lock;
extern std::map<HANDLE, std::vector<std::pair<DWORD, WORD>>> xlive_xlocator_enumerators;
extern std::map<std::pair<DWORD, WORD>, XLOCATOR_SESSION*> liveoverlan_sessions;
extern CRITICAL_SECTION liveoverlan_sessions_lock;

VOID LiveOverLanAbort();
VOID LiveOverLanClone(XLOCATOR_SEARCHRESULT **dst, XLOCATOR_SEARCHRESULT *src);
VOID LiveOverLanDelete(XLOCATOR_SEARCHRESULT *xlocator_result);
BOOL LiveOverLanBroadcastReceive(PXLOCATOR_SEARCHRESULT *result, BYTE *buf, DWORD buflen);
BYTE LiveOverLanRecieve(const std::pair<DWORD, WORD> hostpair, char* buf, INT& result);
