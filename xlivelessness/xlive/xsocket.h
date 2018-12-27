#pragma once

extern INT Result_WSAStartup;
extern WORD xlive_base_port;
extern BOOL xlive_netsocket_abort;
extern SOCKET xlive_liveoverlan_socket;

INT WINAPI XSocketSendTo(SOCKET s, const char *buf, int len, int flags, sockaddr *to, int tolen);
