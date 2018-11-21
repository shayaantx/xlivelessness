#include <winsock2.h>
#include "xdefs.h"
#include "xsocket.h"
#include "../xlln/DebugText.h"

INT Result_WSAStartup = WSANOTINITIALISED;

static INT xlive_base_port = 2000;
static SOCKET xlive_VDP_socket = NULL;

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
	int port = ntohs(((struct sockaddr_in*)name)->sin_port);

	if (s == xlive_VDP_socket) {
		//("[h2mod-voice] Game bound potential voice socket to : %i", ntohs(port));
	}
	else if (port == 1000)
		((struct sockaddr_in*)name)->sin_port = htons(xlive_base_port);
	else if (port == 1001)
		((struct sockaddr_in*)name)->sin_port = htons(xlive_base_port + 1);
	else if (port == 1002)
		((struct sockaddr_in*)name)->sin_port = htons(xlive_base_port + 2);
	else if (port == 1003)
		((struct sockaddr_in*)name)->sin_port = htons(xlive_base_port + 3);
	else if (port == 1004)
		((struct sockaddr_in*)name)->sin_port = htons(xlive_base_port + 4);
	else if (port == 1005)
		((struct sockaddr_in*)name)->sin_port = htons(xlive_base_port + 5);
	else if (port == 1006)
		((struct sockaddr_in*)name)->sin_port = htons(xlive_base_port + 6);
	else if (port == 1007)
		((struct sockaddr_in*)name)->sin_port = htons(xlive_base_port + 7);
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
	SOCKET_ERROR;
	if (result > 0)
	{
		u_long iplong = (((struct sockaddr_in*)from)->sin_addr.s_addr);
		short port = (((struct sockaddr_in*)from)->sin_port);
		//unsigned short normport = ntohs(port);

		//(((struct sockaddr_in*)from)->sin_addr.s_addr) = secure;

		//TODO
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
	short port = (((struct sockaddr_in*)to)->sin_port);
	//unsigned short normport = ntohs(port);
	u_long iplong = (((struct sockaddr_in*)to)->sin_addr.s_addr);
	ADDRESS_FAMILY af = (((struct sockaddr_in*)to)->sin_family);

	INT result = SOCKET_ERROR;

	if (iplong == INADDR_BROADCAST || iplong == 0x00) {
		//(((struct sockaddr_in*)to)->sin_addr.s_addr) = H2Config_master_ip;
		//((struct sockaddr_in*)to)->sin_port = ntohs(H2Config_master_port_relay);
		//TRACE("XSocketSendTo - Broadcast");

		result = sendto(s, buf, len, flags, to, tolen);
	}
	else {

		/*
			Create new SOCKADDR_IN structure,
			If we overwrite the original the game's security functions know it's not a secure address any longer.
			Worst case if this is found to cause performance issues we can handle the send and re-update to secure before return.
		*/

		SOCKADDR_IN SendStruct;
		SendStruct.sin_port = port;
		SendStruct.sin_addr.s_addr = iplong;
		SendStruct.sin_family = AF_INET;

		/*
		 Handle NAT map socket to port
		 Switch on port to determine which port we're intending to send to.
		 1000-> User.pmap_a[secureaddress]
		 1001-> User.pmap_b[secureaddress]
		*/
		int nPort = 0;
		switch (htons(port))
		{
		case 1000:
			//nPort = User.pmap_a[iplong];
			nPort = 1;

			if (nPort != 0)
			{
				//TRACE("XSocketSendTo() port 1000 nPort: %i secure: %08X", htons(nPort), iplong);
				SendStruct.sin_port = nPort;
			}
			else {
				SendStruct.sin_port = ntohs(2000);
			}

			break;

		case 1001:
			//nPort = User.pmap_b[iplong];
			nPort = 1;

			if (nPort != 0)
			{
				//TRACE("XSocketSendTo() port 1001 nPort: %i secure: %08X", htons(nPort), iplong);
				SendStruct.sin_port = nPort;
			}
			else {
				SendStruct.sin_port = ntohs(2001);
			}

			break;

		default:
			//TRACE("XSocketSendTo() port: %i not matched!", htons(port));
			break;
		}

		/*
		u_long xn = User.xnmap[iplong];

		if (xn != join_game_xn.ina.s_addr && join_game_xn.ina.s_addr != 0);
		{
			std::pair <ULONG, SHORT> hostpair = std::make_pair(xn, ntohs(2001));
			std::pair <ULONG, SHORT> hostpair2 = std::make_pair(xn, htons(2000));
			User.smap[hostpair] = iplong;
			User.smap[hostpair2] = iplong;

			//TRACE_GAME_N("[h2mod-socket] XSocketSendTo: ( secureAddr: %08X xn: %08X  - %i ) nPort: %i", iplong, xn, htons(port), htons(nPort));
			//xn = join_game_xn.ina.s_addr;
		}
		

		if (xn != 0)
		{
			SendStruct.sin_addr.s_addr = xn;
		}
		else
		{
			SendStruct.sin_addr.s_addr = User.GetXNFromSecure(iplong);
		}
		*/

		//TRACE("SendStruct.sin_addr.s_addr == %08X", SendStruct.sin_addr.s_addr);

		/*if (SendStruct.sin_addr.s_addr == H2Config_ip_wan)
		{
			//TRACE("Matched g_lWANIP:%08X", H2Config_ip_wan);
			SendStruct.sin_addr.s_addr = H2Config_ip_lan;
			//TRACE("Replaced send struct s_addr with g_lLANIP: %08X", H2Config_ip_lan);
		}*/

		result = sendto(s, buf, len, flags, (SOCKADDR *)&SendStruct, sizeof(SendStruct));
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
