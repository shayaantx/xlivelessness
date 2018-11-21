#include "xdefs.h"
#include "xnet.h"
#include "../xlln/DebugText.h"
#include "xsocket.h"

BOOL xlive_net_initialized = FALSE;

// #51
INT WINAPI XNetStartup(const XNetStartupParams *pxnsp)
{
	TRACE_FX();
	if (pxnsp) {
		if (pxnsp->cfgSizeOfStruct != sizeof(XNetStartupParams))
			return ERROR_INVALID_PARAMETER;
	}
	xlive_net_initialized = TRUE;
	if (!xlive_net_initialized)
		return ERROR_FUNCTION_FAILED;
	return ERROR_SUCCESS;
}

// #52
INT WINAPI XNetCleanup()
{
	TRACE_FX();
	if (Result_WSAStartup != 0)
		return WSANOTINITIALISED;
	return ERROR_SUCCESS;
}

// #53
INT WINAPI XNetRandom(BYTE *pb, UINT cb)
{
	TRACE_FX();
	if (cb)
		for (DWORD i = 0; i < cb; i++)
			pb[i] = static_cast<BYTE>(rand());

	return S_OK;
}

// #54
HRESULT WINAPI XNetCreateKey(XNKID *pxnkid, XNKEY *pxnkey)
{
	TRACE_FX();
	if (!pxnkid)
		return E_INVALIDARG;
	if (!pxnkey)
		return E_INVALIDARG;

	memset(pxnkid, 0x8B, sizeof(XNKID));
	//memset(pxnkid, 0xAB, sizeof(XNKID));
	memset(pxnkey, 0XAA, sizeof(XNKEY));

	/* These are un-necessary. */
	//pxnkid->ab[0] &= ~XNET_XNKID_MASK;
	//pxnkid->ab[0] |= XNET_XNKID_SYSTEM_LINK;
	
	return S_OK;
}

// #55: need #51
INT WINAPI XNetRegisterKey(XNKID *pxnkid, XNKEY *pxnkey)
{
	TRACE_FX();
	return S_OK;
}

// #56: need #51
INT WINAPI XNetUnregisterKey(const XNKID* pxnkid)
{
	TRACE_FX();
	return S_OK;
}

// #57
INT WINAPI XNetXnAddrToInAddr(XNADDR *pxna, XNKID *pnkid, IN_ADDR *pina)
{
	TRACE_FX();
	ULONG secure = pxna->inaOnline.s_addr;

	if (secure != 0) {
		pina->s_addr = secure;
	}
	else {
		__debugbreak();
		//*pina = 0;
	}

	return S_OK;
}

// #60
INT WINAPI XNetInAddrToXnAddr(const IN_ADDR ina, XNADDR *pxna, XNKID *pxnkid)
{
	TRACE_FX();
	void* user = NULL;

	// Zero memory of the current buffer passed to us by the game.
	memset(pxna, 0x00, sizeof(XNADDR));

	if (user != 0) {
		//memcpy(pxna, &user->pxna, sizeof(XNADDR));
	}
	else {
		__debugbreak();
	}

	return S_OK;
}

// #63
INT WINAPI XNetUnregisterInAddr(const IN_ADDR ina)
{
	TRACE_FX();
	return S_OK;
}

// #65
INT WINAPI XNetConnect(const IN_ADDR ina)
{
	TRACE_FX();
	return S_OK;
}

// #66
INT WINAPI XNetGetConnectStatus(const IN_ADDR ina)
{
	TRACE_FX();
	if (!xlive_net_initialized)
		return XNET_CONNECT_STATUS_PENDING;
	return XNET_CONNECT_STATUS_CONNECTED;
}

void sfdsagf(XNADDR *pAddr) {

	//BYTE blarg[] = {0x7C,0xBA,0x6C,0x6A,0x00,0x01,0x00,0x00,0x07,0xD0,0x01,0x00,0x00,0x10,0x13,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x10,0x13,0x00};

	//memcpy(pAddr, blarg, sizeof(XNADDR));

	//return;

	unsigned long resolvedAddr;
	if ((resolvedAddr = inet_addr("127.0.0.1")) == INADDR_NONE) {
		return;
	}
	XUID host_xuid = 0xE000007300000000;

	DWORD user_id = 0x6061B52F;
	DWORD mac_fix = 0x00131000;

	pAddr->ina.s_addr = resolvedAddr;
	pAddr->wPortOnline = 2000;
	pAddr->inaOnline.s_addr = 8192;

	memset(&(pAddr->abEnet), 0, 6);
	memset(&(pAddr->abOnline), 0, 6);

	memcpy(&(pAddr->abEnet), &user_id, 4);
	memcpy(&(pAddr->abOnline), &user_id, 4);

	memcpy((BYTE*)&(pAddr->abEnet) + 3, (BYTE*)&mac_fix + 1, 3);
	memcpy((BYTE*)&(pAddr->abOnline) + 17, (BYTE*)&mac_fix + 1, 3);
}

// #73
DWORD WINAPI XNetGetTitleXnAddr(XNADDR *pAddr)
{
	TRACE_FX();
	if (!xlive_net_initialized)
		return XNET_GET_XNADDR_PENDING;
	if (pAddr) {
		sfdsagf(pAddr);
	}
	return XNET_GET_XNADDR_STATIC | XNET_GET_XNADDR_ETHERNET;
}

// #75
DWORD WINAPI XNetGetEthernetLinkStatus()
{
	TRACE_FX();
	if (!xlive_net_initialized)
		return XNET_ETHERNET_LINK_INACTIVE;
	return XNET_ETHERNET_LINK_ACTIVE | XNET_ETHERNET_LINK_100MBPS | XNET_ETHERNET_LINK_FULL_DUPLEX;
}

// #84
INT WINAPI XNetSetSystemLinkPort(WORD wSystemLinkPort)
{
	TRACE_FX();
	//network byte (big-endian) to little-endian
	//WORD port = (((BYTE*)&wSystemLinkPort)[1] << 0) | (((BYTE*)&wSystemLinkPort)[0] << 8);
	WORD port = ntohs(wSystemLinkPort);

	return ERROR_SUCCESS;
}
