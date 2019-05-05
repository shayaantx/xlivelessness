#pragma once

extern BOOL xlive_debug_pause;

#define XLIVE_LOCAL_USER_COUNT 4

extern BOOL xlive_users_info_changed[XLIVE_LOCAL_USER_COUNT];
extern XUSER_SIGNIN_INFO* xlive_users_info[XLIVE_LOCAL_USER_COUNT];

void Check_Overlapped(PXOVERLAPPED pOverlapped);

struct EligibleAdapter {
	char* name;
	ULONG unicastHAddr;
	ULONG unicastHMask;
	ULONG hBroadcast;
	UINT64 minLinkSpeed;
	BOOL hasDnsServer;
};

extern EligibleAdapter xlive_network_adapter;
extern CRITICAL_SECTION xlive_critsec_custom_local_user_hipv4;
extern unsigned long xlive_custom_local_user_hipv4;
extern BOOL xlive_online_initialized;

unsigned long LocalUserHostIpv4();
