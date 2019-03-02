#pragma once

extern WORD xlive_base_port;
extern BOOL xlive_netsocket_abort;
extern SOCKET xlive_liveoverlan_socket;

INT WINAPI XWSAStartup(WORD wVersionRequested, LPWSADATA lpWSAData);
INT WINAPI XWSACleanup();
INT WINAPI XSocketSendTo(SOCKET s, const char *buf, int len, int flags, sockaddr *to, int tolen);
