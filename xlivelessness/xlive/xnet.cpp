#include <winsock2.h>
#include <WinDNS.h>
#include "xdefs.h"
#include "xnet.h"
#include "../xlln/DebugText.h"
#include "xsocket.h"
#include "dnshelper.h"

BOOL xlive_net_initialized = FALSE;

CLocalUser xlive_local_users[16];

std::map<DWORD, XNADDR*> xlive_users_secure;
std::map<std::pair<DWORD, WORD>, XNADDR*> xlive_users_hostpair;

DnsHelper dnsHelper;

VOID CreateUser(XNADDR* pxna)
{
	TRACE_FX();

	DWORD hsecure = ntohl(pxna->inaOnline.s_addr);
	XNADDR *userPxna = xlive_users_secure.count(hsecure) ? xlive_users_secure[hsecure] : NULL;
	if (userPxna)
		delete userPxna;

	userPxna = new XNADDR;
	memset(userPxna, 0x00, sizeof(XNADDR));
	memcpy(userPxna, pxna, sizeof(XNADDR));

	xlive_users_secure[hsecure] = userPxna;

	std::pair<DWORD, WORD> hostpair = std::make_pair(ntohl(pxna->ina.s_addr), ntohs(pxna->wPortOnline));
	xlive_users_hostpair[hostpair] = userPxna;
}

void UnregisterSecureAddr(const IN_ADDR ina)
{
	return;
	CLocalUser* deluser = NULL;// xlive_cUsers.count(ina.s_addr) ? xlive_cUsers[ina.s_addr] : NULL;
	//xlive_cUsers.erase(ina.s_addr);


	if (deluser != 0)
	{

	}
}

int WINAPI XSocketGetSockName(SOCKET s, struct sockaddr *name, int *namelen)
{
	return getsockname(s, name, namelen);
}

int WINAPI XSocketGetPeerName(SOCKET s, struct sockaddr *name, int *namelen)
{
	return getpeername(s, name, namelen);
}

int WINAPI XNetDnsLookup(const char *hostname, WSAEVENT onDnsLookupCompletedEvent, XNDNS** ppxndns)
{
	return dnsHelper.lookup(hostname, onDnsLookupCompletedEvent, ppxndns);
}

int WINAPI XNetDnsRelease(XNDNS* pxndns)
{
	return dnsHelper.release(pxndns);
}

// #51
INT WINAPI XNetStartup(const XNetStartupParams *pxnsp)
{
	TRACE_FX();
	if (pxnsp) {
		if (pxnsp->cfgSizeOfStruct != sizeof(XNetStartupParams))
			return ERROR_INVALID_PARAMETER;
	}
	if (!xlive_netsocket_abort)
		xlive_net_initialized = TRUE;
	if (!xlive_net_initialized)
		return ERROR_FUNCTION_FAILED;
	return ERROR_SUCCESS;
}

// #52
INT WINAPI XNetCleanup()
{
	TRACE_FX();
	xlive_net_initialized = FALSE;
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

	// These are un-necessary.
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

	CreateUser(pxna);

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

	DWORD hsecure = ntohl(ina.s_addr);
	XNADDR *userPxna = xlive_users_secure.count(hsecure) ? xlive_users_secure[hsecure] : NULL;

	// Zero memory of the current buffer passed to us by the game.
	memset(pxna, 0x00, sizeof(XNADDR));

	if (userPxna != 0) {
		memcpy(pxna, userPxna, sizeof(XNADDR));
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
	UnregisterSecureAddr(ina);
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

// #73
DWORD WINAPI XNetGetTitleXnAddr(XNADDR *pAddr)
{
	TRACE_FX();
	if (!xlive_net_initialized)
		return XNET_GET_XNADDR_PENDING;
	if (pAddr) {
		memcpy(pAddr, &xlive_local_users[0].pxna, sizeof(xlive_local_users[0].pxna));
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
	//network byte (big-endian) to little-endian host
	WORD hPort = ntohs(wSystemLinkPort);

	return ERROR_SUCCESS;
}

XONLINE_NAT_TYPE WINAPI XOnlineGetNatType()
{
	TRACE_FX();
	return XONLINE_NAT_OPEN;
}