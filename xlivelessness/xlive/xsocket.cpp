#include <winsock2.h>
#include "xdefs.h"
#include "xsocket.h"
#include "xlive.h"
#include "../xlln/DebugText.h"
#include "xnet.h"
#include "xlocator.h"
#include <ctime>

#define IPPROTO_VDP 254

WORD xlive_base_port = 1000;
BOOL xlive_netsocket_abort = FALSE;
static SOCKET xlive_VDP_socket = NULL;
SOCKET xlive_liveoverlan_socket = NULL;

struct SocketInfo {
	int protocol;
	WORD hPort;
};
static std::map<SOCKET, SocketInfo*> xlive_sockets;

VOID SendUnknownUserAskRequest(SOCKET socket, char* data, int dataLen, sockaddr *to, int tolen)
{
	addDebugText("XLLN: Send UNKNOWN_USER_ASK.");
	const int cpHeaderLen = sizeof(XLLN_CUSTOM_PACKET_SENTINEL) + sizeof(XLLNCustomPacketType::Type);
	const int cpUUHeaderLen = cpHeaderLen + sizeof(XNADDR);
	int cpTotalLen = cpUUHeaderLen + dataLen;

	// Check overflow condition.
	if (cpTotalLen < 0) {
		// Send only UNKNOWN_USER_ASK.
		cpTotalLen = cpUUHeaderLen;
		return;
	}

	BYTE *cpBuf = (BYTE*)malloc(cpTotalLen);
	cpBuf[0] = XLLN_CUSTOM_PACKET_SENTINEL;
	cpBuf[sizeof(XLLN_CUSTOM_PACKET_SENTINEL)] = XLLNCustomPacketType::UNKNOWN_USER_ASK;
	XNADDR &xnAddr = *(XNADDR*)&cpBuf[cpHeaderLen];
	memcpy_s(&xnAddr, sizeof(xnAddr), &xlive_local_users[0].pxna, sizeof(xlive_local_users[0].pxna));

	if (cpTotalLen > cpUUHeaderLen) {
		memcpy_s(&cpBuf[cpUUHeaderLen], dataLen, data, dataLen);
	}

	INT bytesSent = sendto(socket, (char*)cpBuf, cpTotalLen, 0, to, tolen);

	if (bytesSent <= 0 && cpTotalLen > cpUUHeaderLen) {
		// Send only UNKNOWN_USER_ASK.
		bytesSent = sendto(socket, (char*)cpBuf, cpUUHeaderLen, 0, to, tolen);
	}

	free(cpBuf);
}

static VOID CustomMemCpy(void *dst, void *src, rsize_t len, bool directionAscending)
{
	if (directionAscending) {
		for (rsize_t i = 0; i < len; i++) {
			((BYTE*)dst)[i] = ((BYTE*)src)[i];
		}
	}
	else {
		if (len > 0) {
			for (rsize_t i = len - 1; true; i--) {
				((BYTE*)dst)[i] = ((BYTE*)src)[i];
				if (i == 0) {
					break;
				}
			}
		}
	}
}

// #1
INT WINAPI XWSAStartup(WORD wVersionRequested, LPWSADATA lpWSAData)
{
	TRACE_FX();
	INT result = WSAStartup(wVersionRequested, lpWSAData);
	return result;
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
	if (protocol == IPPROTO_VDP) {
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
	else {
		SocketInfo* si = (SocketInfo*)malloc(sizeof(SocketInfo));
		si->protocol = protocol;
		si->hPort = 0;
		xlive_sockets[result] = si;
	}

	return result;
}

// #4
INT WINAPI XSocketClose(SOCKET s)
{
	TRACE_FX();
	INT result = closesocket(s);

	SocketInfo *si = xlive_sockets[s];
	free(si);
	xlive_sockets.erase(s);

	return result;
}

// #5
INT WINAPI XSocketShutdown(SOCKET s, int how)
{
	TRACE_FX();
	INT result = shutdown(s, how);

	xlive_sockets[s]->hPort = 0;

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

// #9
INT WINAPI XSocketGetSockName(SOCKET s, struct sockaddr *name, int *namelen)
{
	TRACE_FX();
	INT result = getsockname(s, name, namelen);
	return result;
}

// #10
INT WINAPI XSocketGetPeerName(SOCKET s, struct sockaddr *name, int *namelen)
{
	TRACE_FX();
	INT result = getpeername(s, name, namelen);
	return result;
}

// #11
SOCKET WINAPI XSocketBind(SOCKET s, const struct sockaddr *name, int namelen)
{
	TRACE_FX();
	WORD hPort = ntohs(((struct sockaddr_in*)name)->sin_port);
	WORD port_base = (hPort / 1000) * 1000;
	WORD port_offset = hPort % 1000;

	if (port_offset == 1) {
		((struct sockaddr_in*)name)->sin_port = htons(xlive_base_port + 1);
		xlive_liveoverlan_socket = s;
	}
	//else if (s == xlive_VDP_socket) {
	//	__debugbreak();
	//	//("[h2mod-voice] Game bound potential voice socket to : %i", ntohs(port));
	//	((struct sockaddr_in*)name)->sin_port = htons(xlive_base_port + 10);
	//}
	else {
		((struct sockaddr_in*)name)->sin_port = htons(xlive_base_port + port_offset);
	}

	SOCKET result = bind(s, name, namelen);

	if (result == SOCKET_ERROR) {
		char debugText[200];
		snprintf(debugText, sizeof(debugText), "Socket Bind ERROR.\nAnother program has taken port:\nBase: %hd\nOffset: %hd\nOriginal: %hd", xlive_base_port, port_offset, hPort);
		XllnDebugBreak(debugText);
	}
	else {
		xlive_sockets[s]->hPort = hPort;
	}
	return result;
}

// #12
INT WINAPI XSocketConnect(SOCKET s, const struct sockaddr *name, int namelen)
{
	TRACE_FX();

	ULONG niplong = ((struct sockaddr_in*)name)->sin_addr.s_addr;
	ULONG hsecure = ntohl(niplong);
	WORD hPort = ntohs(((struct sockaddr_in*)name)->sin_port);
	WORD port_base = (hPort / 1000) * 1000;
	WORD port_offset = hPort % 1000;

	XNADDR *userPxna = xlive_users_secure.count(hsecure) ? xlive_users_secure[hsecure] : NULL;
	if (userPxna) {
		((struct sockaddr_in*)name)->sin_addr.s_addr = userPxna->ina.s_addr;
		((struct sockaddr_in*)name)->sin_port = htons(ntohs(userPxna->wPortOnline) + port_offset);
	}
	else {
		__debugbreak();
	}

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

INT WINAPI XSocketRecvFromHelper(INT result, SOCKET s, char *buf, int len, int flags, sockaddr *from, int *fromlen)
{
	if (result > 0) {
		//addDebugText(__func__);
		const u_long niplong = ((struct sockaddr_in*)from)->sin_addr.s_addr;
		const WORD hPort = ntohs(((struct sockaddr_in*)from)->sin_port);
		const WORD port_base = (hPort / 1000) * 1000;
		const WORD port_offset = hPort % 1000;
		const std::pair<DWORD, WORD> hostpair = std::make_pair(ntohl(niplong), port_base);

		if (buf[0] == XLLN_CUSTOM_PACKET_SENTINEL) {
			const int cpHeaderLen = sizeof(XLLN_CUSTOM_PACKET_SENTINEL) + sizeof(XLLNCustomPacketType::Type);
			if (result < cpHeaderLen) {
				addDebugText("XLLN: ERROR custom packet received was too short!");
				return 0;
			}
			switch (buf[sizeof(XLLN_CUSTOM_PACKET_SENTINEL)]) {
				case XLLNCustomPacketType::STOCK_PACKET: {
					CustomMemCpy(buf, (void*)((DWORD)buf + cpHeaderLen), result - cpHeaderLen, true);
					result -= cpHeaderLen;
					break;
				}
				case XLLNCustomPacketType::UNKNOWN_USER_ASK:
				case XLLNCustomPacketType::UNKNOWN_USER_REPLY: {
					// Less than since there is likely another packet pushed onto the end of this one.
					if (result < cpHeaderLen + sizeof(XNADDR)) {
						addDebugText("XLLN: ERROR INVALID UNKNOWN_USER_<?> Recieved.");
						return 0;
					}
					addDebugText("XLLN: UNKNOWN_USER_<?> Recieved.");
					XNADDR &xnAddr = *(XNADDR*)&buf[cpHeaderLen];
					xnAddr.ina.s_addr = niplong;
					CreateUser(&xnAddr);

					XLLNCustomPacketType::Type &packet_type = *(XLLNCustomPacketType::Type*)&buf[sizeof(XLLN_CUSTOM_PACKET_SENTINEL)];

					if (packet_type == XLLNCustomPacketType::UNKNOWN_USER_ASK) {
						addDebugText("XLLN: UNKNOWN_USER_ASK Sending REPLY.");
						packet_type = XLLNCustomPacketType::UNKNOWN_USER_REPLY;
						memcpy_s(&xnAddr, sizeof(xnAddr), &xlive_local_users[0].pxna, sizeof(xlive_local_users[0].pxna));

						INT bytesSent = sendto(s, buf, result, 0, from, *fromlen);
						return 0;
					}
					else {
						addDebugText("XLLN: UNKNOWN_USER_REPLY passing original data back.");
						const int UnknownUserPacketLen = cpHeaderLen + sizeof(XNADDR);
						CustomMemCpy(buf, (void*)((DWORD)buf + UnknownUserPacketLen), result - UnknownUserPacketLen, true);
						result -= UnknownUserPacketLen;
						return XSocketRecvFromHelper(result, s, buf, len, flags, from, fromlen);
					}
				}
				case XLLNCustomPacketType::LIVE_OVER_LAN_ADVERTISE:
				case XLLNCustomPacketType::LIVE_OVER_LAN_UNADVERTISE: {
					LiveOverLanRecieve(s, from, *fromlen, hostpair, buf, result);
					return 0;
				}
				default: {
					addDebugText("XLLN: ERROR unknown custom packet type received!");
					break;
				}
			}
		}
		
		if (result <= 0) {
			if (result < 0) {
				XllnDebugBreak("XLLN: ERROR XSocketRecvFrom result became less than 0!");
			}
			return 0;
		}

		XNADDR *userPxna = xlive_users_hostpair.count(hostpair) ? xlive_users_hostpair[hostpair] : NULL;
		if (userPxna) {
			((struct sockaddr_in*)from)->sin_addr.s_addr = userPxna->inaOnline.s_addr;
		}
		else {
			SendUnknownUserAskRequest(s, buf, result, from, *fromlen);
			return 0;
		}
	}
	return result;
}

// #20
INT WINAPI XSocketRecvFrom(SOCKET s, char *buf, int len, int flags, sockaddr *from, int *fromlen)
{
	TRACE_FX();
	INT result = recvfrom(s, buf, len, flags, from, fromlen);
	result = XSocketRecvFromHelper(result, s, buf, len, flags, from, fromlen);
	return result;
}

// #22
INT WINAPI XSocketSend(SOCKET s, const char *buf, int len, int flags)
{
	TRACE_FX();
	INT result = send(s, buf, len, flags);
	return result;
}

INT WINAPI XllnSocketSendTo(SOCKET s, const char *buf, int len, int flags, sockaddr *to, int tolen)
{
	TRACE_FX();
	INT result = SOCKET_ERROR;

	WORD nPort = ((struct sockaddr_in*)to)->sin_port;
	WORD hPort = ntohs(nPort);
	ULONG nIpv4 = ((struct sockaddr_in*)to)->sin_addr.s_addr;
	ULONG hIpv4 = ntohl(nIpv4);
	WORD port_base = (hPort / 1000) * 1000;
	WORD port_offset = hPort % 1000;
	ADDRESS_FAMILY af = ((struct sockaddr_in*)to)->sin_family;

	if (hIpv4 == INADDR_BROADCAST || hIpv4 == INADDR_ANY) {
		//addDebugText("XSocketSendTo() - Broadcast.");

		((struct sockaddr_in*)to)->sin_addr.s_addr = htonl(xlive_network_adapter.hBroadcast);

		for (WORD port_base = 1000; port_base <= 6000; port_base += 1000) {
			((struct sockaddr_in*)to)->sin_port = htons(port_base + port_offset);
			result = sendto(s, buf, len, 0, to, tolen);
		}
	}
	else {
		XNADDR *userPxna = xlive_users_secure.count(hIpv4) ? xlive_users_secure[hIpv4] : NULL;
		if (userPxna) {
			((struct sockaddr_in*)to)->sin_addr.s_addr = userPxna->ina.s_addr;
			((struct sockaddr_in*)to)->sin_port = htons(ntohs(userPxna->wPortOnline) + port_offset);
		}
		else {
			std::pair<DWORD, WORD> hostpair = std::make_pair(ntohl(((struct sockaddr_in*)to)->sin_addr.s_addr), port_base);
			XNADDR* userPxna = xlive_users_hostpair.count(hostpair) ? xlive_users_hostpair[hostpair] : NULL;
			if (userPxna) {
				//FIXME fix port if remapped.
				((struct sockaddr_in*)to)->sin_port = htons(ntohs(userPxna->wPortOnline) + port_offset);
			}
			else {
				addDebugText("ERROR: Unable to find XNADDR, sending anyway.");
				//__debugbreak();
				//return result;
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

// #24
INT WINAPI XSocketSendTo(SOCKET s, const char *buf, int len, int flags, sockaddr *to, int tolen)
{
	TRACE_FX();
	INT result = SOCKET_ERROR;

	// Check if the first byte is the same as the custom XLLN packets.
	// If so wrap the data in a new message.
	if (buf[0] == XLLN_CUSTOM_PACKET_SENTINEL) {
		int altBufLen = len + 2;
		// Check overflow condition.
		if (altBufLen < 0) {
			WSASetLastError(WSAEMSGSIZE);
			return SOCKET_ERROR;
		}

		char *altBuf = (char*)malloc(sizeof(char) * altBufLen);

		altBuf[0] = XLLN_CUSTOM_PACKET_SENTINEL;
		altBuf[1] = XLLNCustomPacketType::STOCK_PACKET;
		memcpy_s(&altBuf[2], altBufLen-2, buf, len);

		result = XllnSocketSendTo(s, altBuf, altBufLen, flags, to, tolen);

		free(altBuf);
	}
	else {
		result = XllnSocketSendTo(s, buf, len, flags, to, tolen);
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
