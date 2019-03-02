#pragma once
#include <map>

struct CLocalUser {
	XNADDR pxna;
	IN_ADDR pina;
	BOOL bValid;
};

extern CLocalUser xlive_local_users[16];
extern std::map<DWORD, XNADDR*> xlive_users_secure;
extern std::map<std::pair<DWORD, WORD>, XNADDR*> xlive_users_hostpair;

extern BOOL xlive_net_initialized;

VOID CreateUser(XNADDR* pxna);
HRESULT WINAPI XNetCreateKey(XNKID *pxnkid, XNKEY *pxnkey);