#include <winsock2.h>
#include "xdefs.h"
#include "xsocket.h"
#include "xlive.h"
#include "../xlln/DebugText.h"
#include "xnet.h"

INT Result_WSAStartup = WSANOTINITIALISED;

WORD xlive_base_port = 1000;
BOOL xlive_netsocket_abort = FALSE;
static SOCKET xlive_VDP_socket = NULL;

VOID CustomMemCpy(void *dst, void *src, rsize_t len)
{
	for (rsize_t i = 0; i < len; i++) {
		((BYTE*)dst)[i] = ((BYTE*)src)[i];
	}
}

// #1
INT WINAPI XWSAStartup(WORD wVersionRequested, LPWSADATA lpWSAData)
{
	TRACE_FX();
	Result_WSAStartup = WSAStartup(wVersionRequested, lpWSAData);
	return Result_WSAStartup;
}

// #2
INT WINAPI XWSACleanup()
{
	TRACE_FX();
	INT result = WSACleanup();
	if (result != ERROR_SUCCESS) {
		//TODO Store error in XWSAGetLastError().
		return SOCKET_ERROR;
	}
	return result;
}

// #3
SOCKET WINAPI XSocketCreate(int af, int type, int protocol)
{
	TRACE_FX();
	bool vdp = false;
	if (protocol == 254) {
		vdp = true;
		// We can't support VDP (Voice / Data Protocol) it's some encrypted crap which isn't standard.
		protocol = IPPROTO_UDP;
	}

	SOCKET result = socket(af, type, protocol);

	if (vdp) {
		//("Socket: %08X was VDP", result);
		xlive_VDP_socket = result;
	}

	if (result == INVALID_SOCKET) {
		__debugbreak();
	}

	return result;
}

// #4
INT WINAPI XSocketClose(SOCKET s)
{
	TRACE_FX();
	INT result = closesocket(s);
	return result;
}

// #5
INT WINAPI XSocketShutdown(SOCKET s, int how)
{
	TRACE_FX();
	INT result = shutdown(s, how);
	return result;
}

// #6
INT WINAPI XSocketIOCTLSocket(SOCKET s, __int32 cmd, ULONG *argp)
{
	TRACE_FX();
	INT result = ioctlsocket(s, cmd, argp);
	return result;
}

// #7
INT WINAPI XSocketSetSockOpt(SOCKET s, int level, int optname, const char *optval, int optlen)
{
	TRACE_FX();
	if ((level & SO_BROADCAST) > 0) {
		//"XSocketSetSockOpt - SO_BROADCAST";
	}

	INT result = setsockopt(s, level, optname, optval, optlen);
	if (result == SOCKET_ERROR) {
		__debugbreak();
	}

	return result;
}

// #8
INT WINAPI XSocketGetSockOpt(SOCKET s, int level, int optname, char *optval, int *optlen)
{
	TRACE_FX();
	INT result = getsockopt(s, level, optname, optval, optlen);
	return result;
}

// #11
SOCKET WINAPI XSocketBind(SOCKET s, const struct sockaddr *name, int namelen)
{
	TRACE_FX();
	int hPort = ntohs(((struct sockaddr_in*)name)->sin_port);

	if (hPort == 1000)
		((struct sockaddr_in*)name)->sin_port = htons(xlive_base_port);
	else if (hPort == 1001)
		((struct sockaddr_in*)name)->sin_port = htons(xlive_base_port + 1);
	else if (hPort == 1002)
		((struct sockaddr_in*)name)->sin_port = htons(xlive_base_port + 2);
	else if (hPort == 1003)
		((struct sockaddr_in*)name)->sin_port = htons(xlive_base_port + 3);
	else if (hPort == 1004)
		((struct sockaddr_in*)name)->sin_port = htons(xlive_base_port + 4);
	else if (hPort == 1005)
		((struct sockaddr_in*)name)->sin_port = htons(xlive_base_port + 5);
	else if (hPort == 1006)
		((struct sockaddr_in*)name)->sin_port = htons(xlive_base_port + 6);
	else if (hPort == 1007)
		((struct sockaddr_in*)name)->sin_port = htons(xlive_base_port + 7);
	else if (s == xlive_VDP_socket) {
		//("[h2mod-voice] Game bound potential voice socket to : %i", ntohs(port));
		((struct sockaddr_in*)name)->sin_port = htons(xlive_base_port + 10);
	}
	else
		__debugbreak();

	SOCKET result = bind(s, name, namelen);

	if (result == SOCKET_ERROR) {
		__debugbreak();
	}
	return result;
}

// #12
INT WINAPI XSocketConnect(SOCKET s, const struct sockaddr *name, int namelen)
{
	TRACE_FX();
	INT result = connect(s, name, namelen);
	return result;
}

// #13
INT WINAPI XSocketListen(SOCKET s, int backlog)
{
	TRACE_FX();
	INT result = listen(s, backlog);
	return result;
}

// #14
SOCKET WINAPI XSocketAccept(SOCKET s, struct sockaddr *addr, int *addrlen)
{
	TRACE_FX();
	SOCKET result = accept(s, addr, addrlen);
	return result;
}

// #15
INT WINAPI XSocketSelect(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, const struct timeval *timeout)
{
	TRACE_FX();
	INT result = select(nfds, readfds, writefds, exceptfds, timeout);
	return result;
}

// #18
INT WINAPI XSocketRecv(SOCKET s, char * buf, int len, int flags)
{
	TRACE_FX();
	INT result = recv(s, buf, len, flags);
	return result;
}

// #20
INT WINAPI XSocketRecvFrom(SOCKET s, char *buf, int len, int flags, sockaddr *from, int *fromlen)
{
	TRACE_FX();
	INT result = recvfrom(s, buf, len, flags, from, fromlen);
	
	if (result > 0)
	{
		//addDebugText(__func__);
		u_long niplong = (((struct sockaddr_in*)from)->sin_addr.s_addr);
		WORD hPort = ntohs(((struct sockaddr_in*)from)->sin_port);
		WORD port_base = (hPort / 1000) * 1000;
		WORD port_offset = hPort % 1000;

		if (result >= sizeof(XNADDR) + 8 && ((DWORD*)&buf[0])[0] == 0x01233210) {
			addDebugText("XLLN: Recv request Unknown User.");
			XNADDR* pAddr = (XNADDR*)&((DWORD*)&buf[0])[2];
			pAddr->ina.s_addr = niplong;
			CreateUser(pAddr);

			if (((DWORD*)&buf[0])[1] == 0x01233210) {
				addDebugText("XLLN: Send reply Unknown User.");
				((DWORD*)&buf[0])[1] = 0x01230123;
				memcpy_s(pAddr, sizeof(XNADDR), &xlive_local_users[0].pxna, sizeof(XNADDR));

				INT bytesSent = sendto(s, buf, len, 0, from, *fromlen);
			}
			else {
				DWORD buf2datasize = sizeof(XNADDR) + 8;
				INT result2 = result - buf2datasize;
				CustomMemCpy(buf, buf + buf2datasize, result2);
				addDebugText("XLLN: Passing original data back.");
				return result2;
			}
			return 0;
		}

		std::pair<DWORD, WORD> hostpair = std::make_pair(ntohl(((struct sockaddr_in*)from)->sin_addr.s_addr), port_base);
		XNADDR* userPxna = xlive_users_hostpair.count(hostpair) ? xlive_users_hostpair[hostpair] : NULL;
		if (userPxna) {
			(((struct sockaddr_in*)from)->sin_addr.s_addr) = userPxna->inaOnline.s_addr;
		}
		else {
			addDebugText("XLLN: Send request for Unknown User.");
			DWORD buf2datasize = sizeof(XNADDR) + 8;
			DWORD buf2len = result + buf2datasize;
			BYTE *buf2 = (BYTE*)malloc(buf2len);
			memcpy_s(buf2 + buf2datasize, result, buf, result);
			((DWORD*)&buf2[0])[0] = 0x01233210;
			((DWORD*)&buf2[0])[1] = 0x01233210;
			XNADDR* blarg = (XNADDR*)&((DWORD*)&buf2[0])[2];
			memset(blarg, 0x00, sizeof(XNADDR));
			memcpy_s(blarg, sizeof(XNADDR), &xlive_local_users[0].pxna, sizeof(XNADDR));

			INT bytesSent = sendto(s, (char*)buf2, buf2len, 0, from, *fromlen);

			free(buf2);

			return 0;
		}
	}
	return result;
}

// #22
INT WINAPI XSocketSend(SOCKET s, const char *buf, int len, int flags)
{
	TRACE_FX();
	INT result = send(s, buf, len, flags);
	return result;
}

// #24
INT WINAPI XSocketSendTo(SOCKET s, const char *buf, int len, int flags, sockaddr *to, int tolen)
{
	TRACE_FX();
	INT result = SOCKET_ERROR;

	WORD nPort = ((struct sockaddr_in*)to)->sin_port;
	WORD hPort = ntohs(nPort);
	u_long iplong = (((struct sockaddr_in*)to)->sin_addr.s_addr);
	ADDRESS_FAMILY af = (((struct sockaddr_in*)to)->sin_family);
	WORD port_base = (hPort / 1000) * 1000;
	WORD port_offset = hPort % 1000;

	if (iplong == INADDR_BROADCAST || iplong == 0x00) {
		//addDebugText("XSocketSendTo() - Broadcast.");
		(((struct sockaddr_in*)to)->sin_addr.s_addr) = htonl(xlive_network_adapter.hBroadcast);//inet_addr("192.168.0.255");
		
		for (WORD port_base = 1000; port_base <= 6000; port_base += 1000) {
			((struct sockaddr_in*)to)->sin_port = htons(port_base + port_offset);
			result = sendto(s, buf, len, 0, to, tolen);
		}
	}
	else {
		XNADDR* userPxna = xlive_users_secure.count(iplong) ? xlive_users_secure[iplong] : NULL;
		if (userPxna) {
			(((struct sockaddr_in*)to)->sin_addr.s_addr) = userPxna->ina.s_addr;
			((struct sockaddr_in*)to)->sin_port = htons(ntohs(userPxna->wPortOnline) + port_offset);
		}
		else {
			std::pair<DWORD, WORD> hostpair = std::make_pair(ntohl(((struct sockaddr_in*)to)->sin_addr.s_addr), port_base);
			XNADDR* userPxna = xlive_users_hostpair.count(hostpair) ? xlive_users_hostpair[hostpair] : NULL;
			if (userPxna) {
				//TODO fix port if remapped.
				((struct sockaddr_in*)to)->sin_port = htons(ntohs(userPxna->wPortOnline) + port_offset);
			}
			else {
				__debugbreak();
			}
		}

		
		result = sendto(s, buf, len, flags, to, tolen);
	}

	if (result == SOCKET_ERROR) {
		INT Error = WSAGetLastError();
		__debugbreak();
	}

	return result;
}

// #27
INT WINAPI XSocketWSAGetLastError()
{
	TRACE_FX();
	INT result = WSAGetLastError();
	return result;
}

// #37
ULONG WINAPI XSocketHTONL(ULONG hostlong)
{
	TRACE_FX();
	ULONG result = htonl(hostlong);
	return result;
}

// #38
USHORT WINAPI XSocketNTOHS(USHORT netshort)
{
	TRACE_FX();
	USHORT result = ntohs(netshort);
	return result;
}

// #39
ULONG WINAPI XSocketNTOHL(ULONG netlong)
{
	TRACE_FX();
	ULONG result = ntohl(netlong);
	return result;
}

// #40
USHORT WINAPI XSocketHTONS(USHORT hostshort)
{
	TRACE_FX();
	USHORT result = htons(hostshort);
	return result;
}
