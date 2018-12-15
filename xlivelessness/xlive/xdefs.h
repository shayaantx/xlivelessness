#pragma once
#ifndef XLIVEDEFS_H
#define XLIVEDEFS_H
#pragma comment(lib, "Ws2_32.lib")
#include <windows.h>


#define LODWORD(ll) ((DWORD)(ll))
#define HIDWORD(ll) ((DWORD)(((ULONGLONG)(ll) >> 32) & 0xFFFFFFFF))
//#define LOWORD(l) ((WORD)(l))
//#define HIWORD(l) ((WORD)(((DWORD)(l) >> 16) & 0xFFFF))
//#define LOBYTE(w) ((BYTE)(w))
//#define HIBYTE(w) ((BYTE)(((WORD)(w) >> 8) & 0xFF))


//typedef unsigned __int64 ULONG_PTR, *PULONG_PTR;
//typedef unsigned long DWORD_PTR;


// Xbox Secure Network Library ------------------------------------------------

//
// XNetStartup is called to load the Xbox Secure Network Library.  It takes an
// optional pointer to a parameter structure.  To initialize the library with
// the default set of parameters, simply pass NULL for this argument.  To
// initialize the library with a different set of parameters, place an
// XNetStartupParams on the stack, zero it out, set the cfgSizeOfStruct to
// sizeof(XNetStartupParams), set any of the parameters you want to configure
// (leaving the remaining ones zeroed), and pass a pointer to this structure to
// XNetStartup.
//
// By default the Xbox Secure Network Library operates in secure mode, which
// means that communication to untrusted hosts (such as a PC) is disallowed.
// However, the devkit version of the library allows you to bypass this
// security by specifying the XNET_STARTUP_BYPASS_SECURITY flag in the cfgFlags
// parameter.  Here is an example of how to do this:
//
//      XNetStartupParams xnsp;
//      memset(&xnsp, 0, sizeof(xnsp));
//      xnsp.cfgSizeOfStruct = sizeof(XNetStartupParams);
//      xnsp.cfgFlags = XNET_STARTUP_BYPASS_SECURITY;
//      INT err = XNetStartup(&xnsp);
//
// Attempting to bypass security when not using the devkit version of the
// library does not return an error, it is simply ignored.  Attempts to send or
// receive packets from untrusted hosts will fail.
//


//
// This devkit-only flag tells the XNet stack to allow insecure
// communication to untrusted hosts (such as a PC).  This flag is silently
// ignored by the secure versions of the library.
//
#define XNET_STARTUP_BYPASS_SECURITY                0x01

//
// This flag instructs XNet to pre-allocate memory for the maximum number of
// datagram (UDP and VDP) sockets during the 'XNetStartup' call and store the
// objects in an internal pool.  Otherwise, sockets are allocated on demand (by
// the 'socket' function).  In either case, SOCK_DGRAM sockets are returned to
// the internal pool once closed.  The memory will remain allocated until
// XNetCleanup is called.
//
#define XNET_STARTUP_ALLOCATE_MAX_DGRAM_SOCKETS     0x02

//
// This flag instructs XNet to pre-allocate memory for the maximum number of
// stream (TCP) sockets during the 'XNetStartup' call and store the objects in
// an internal pool.  Otherwise, sockets are allocated on demand (by the
// 'socket', 'listen', and 'accept' functions).  Note that 'listen' will still
// attempt to pre-allocate the specified maximum backlog number of sockets even
// without this flag set.  The 'accept' function will always return a socket
// retrieved from the pool, though it will also attempt to allocate a
// replacement if the cfgSockMaxStreamSockets limit and memory permit.
// In all cases, SOCK_STREAM sockets are returned to the internal pool once
// closed. The memory will remain allocated until XNetCleanup is called.
//
#define XNET_STARTUP_ALLOCATE_MAX_STREAM_SOCKETS    0x04

//
// This devkit-only flag tells the XNet stack to disable encryption for
// communication between peers.  This flag is silently ignored by the secure
// versions of the library.
//
#define XNET_STARTUP_DISABLE_PEER_ENCRYPTION        0x08




typedef struct _XLIVE_INITIALIZE_INFO {
	UINT cbSize;
	DWORD dwFlags;
	IUnknown *pD3D;
	VOID *pD3DPP;
	LANGID langID;
	WORD wReserved1;
	PCHAR pszAdapterName;
	WORD wLivePortOverride;
	WORD wReserved2;
} XLIVE_INITIALIZE_INFO;

typedef struct XLIVE_INPUT_INFO {
	UINT cbSize;
	HWND hWnd;
	UINT uMSG;
	WPARAM wParam;
	LPARAM lParam;
	BOOL fHandled;
	LRESULT lRet;
} XLIVE_INPUT_INFO;



typedef struct _XLIVEUPDATE_INFORMATION {
	DWORD cbSize;
	BOOL bSystemUpdate;
	DWORD dwFromVersion;
	DWORD dwToVersion;
	WCHAR szUpdateDownloadPath[MAX_PATH];
} XLIVEUPDATE_INFORMATION, *PXLIVEUPDATE_INFORMATION;


typedef struct {
	DWORD dwSize;
	// bData is first element of array of length/size dwSize.
	BYTE bData;
} XLIVE_PROTECTED_BUFFER;


#define XUSER_DATA_TYPE_CONTEXT     ((BYTE)0)
#define XUSER_DATA_TYPE_INT32       ((BYTE)1)
#define XUSER_DATA_TYPE_INT64       ((BYTE)2)
#define XUSER_DATA_TYPE_DOUBLE      ((BYTE)3)
#define XUSER_DATA_TYPE_UNICODE     ((BYTE)4)
#define XUSER_DATA_TYPE_FLOAT       ((BYTE)5)
#define XUSER_DATA_TYPE_BINARY      ((BYTE)6)
#define XUSER_DATA_TYPE_DATETIME    ((BYTE)7)
#define XUSER_DATA_TYPE_NULL        ((BYTE)0xFF)

// Creation Flags
#define XSESSION_CREATE_USES_MASK                       0x0000003F

#define XSESSION_CREATE_HOST                            0x00000001  // Set only on the host of a multiplayer session. The user who sets the host flag is the user that interacts with Live
#define XSESSION_CREATE_USES_PRESENCE                   0x00000002  // Session is used across games to keep players together. Advertises state via Presence
#define XSESSION_CREATE_USES_STATS                      0x00000004  // Session is used for stats tracking
#define XSESSION_CREATE_USES_MATCHMAKING                0x00000008  // Session is advertised in matchmaking for searching
#define XSESSION_CREATE_USES_ARBITRATION                0x00000010  // Session stats are arbitrated (and therefore tracked for everyone in the game)
#define XSESSION_CREATE_USES_PEER_NETWORK               0x00000020  // Session XNKey is registered and PC settings are enforced

// Optional modifiers to sessions that are created with XSESSION_CREATE_USES_PRESENCE
#define XSESSION_CREATE_MODIFIERS_MASK                  0x00000F80

#define XSESSION_CREATE_SOCIAL_MATCHMAKING_ALLOWED      0x00000080  // Session may be converted to an Social Matchmaking session
#define XSESSION_CREATE_INVITES_DISABLED                0x00000100  // Game Invites cannot be sent by the HUD for this session
#define XSESSION_CREATE_JOIN_VIA_PRESENCE_DISABLED      0x00000200  // Session will not ever be displayed as joinable via Presence
#define XSESSION_CREATE_JOIN_IN_PROGRESS_DISABLED       0x00000400  // Session will not be joinable between XSessionStart and XSessionEnd
#define XSESSION_CREATE_JOIN_VIA_PRESENCE_FRIENDS_ONLY  0x00000800  // Session is only joinable via presence for friends of the host


#define XSESSION_CREATE_SINGLEPLAYER_WITH_STATS		XSESSION_CREATE_USES_PRESENCE | XSESSION_CREATE_USES_STATS | XSESSION_CREATE_INVITES_DISABLED | XSESSION_CREATE_JOIN_VIA_PRESENCE_DISABLED | XSESSION_CREATE_JOIN_IN_PROGRESS_DISABLED
#define XSESSION_CREATE_LIVE_MULTIPLAYER_STANDARD	XSESSION_CREATE_USES_PRESENCE | XSESSION_CREATE_USES_STATS | XSESSION_CREATE_USES_MATCHMAKING | XSESSION_CREATE_USES_PEER_NETWORK
#define XSESSION_CREATE_LIVE_MULTIPLAYER_RANKED		XSESSION_CREATE_LIVE_MULTIPLAYER_STANDARD | XSESSION_CREATE_USES_ARBITRATION
#define XSESSION_CREATE_SYSTEMLINK					XSESSION_CREATE_USES_PEER_NETWORK
#define XSESSION_CREATE_GROUP_LOBBY					XSESSION_CREATE_USES_PRESENCE | XSESSION_CREATE_USES_PEER_NETWORK
#define XSESSION_CREATE_GROUP_GAME					XSESSION_CREATE_USES_STATS | XSESSION_CREATE_USES_MATCHMAKING | XSESSION_CREATE_USES_PEER_NETWORK



typedef struct
{

	//
	// Must be set to sizeof(XNetStartupParams).  There is no default.
	//
	BYTE        cfgSizeOfStruct;

	//
	// One or more of the XNET_STARTUP_xxx flags OR'd together.
	//
	// The default is 0 (no flags specified).
	BYTE        cfgFlags;

	//
	// The maximum number of SOCK_DGRAM (UDP or VDP) sockets that can be
	// opened at once.
	//
	// The default is 8 sockets.
	//
	BYTE        cfgSockMaxDgramSockets;

	//
	// The maximum number of SOCK_STREAM (TCP) sockets that can be opened at
	// once, including those sockets created as a result of incoming connection
	// requests.  Remember that a TCP socket may not be closed immediately
	// after 'closesocket' is called, depending on the linger options in place
	// (by default a TCP socket will linger).
	//
	// The default is 32 sockets.
	//
	BYTE        cfgSockMaxStreamSockets;

	//
	// The default receive buffer size for a socket, in units of K (1024 bytes).
	//
	// The default is 16 units (16K).
	//
	BYTE        cfgSockDefaultRecvBufsizeInK;

	//
	// The default send buffer size for a socket, in units of K (1024 bytes).
	//
	// The default is 16 units (16K).
	//
	BYTE        cfgSockDefaultSendBufsizeInK;

	//
	// The maximum number of XNKID / XNKEY pairs that can be registered at the
	// same time by calling XNetRegisterKey.
	//
	// The default is 8 key pair registrations.
	//
	BYTE        cfgKeyRegMax;

	//
	// The maximum number of security associations that can be registered at
	// the same time.  Security associations are created for each unique
	// XNADDR / XNKID pair passed to XNetXnAddrToInAddr.  Security associations
	// are also implicitly created for each secure host that establishes an
	// incoming connection with this host on a given registered XNKID.  Note
	// that there will only be one security association between a pair of hosts
	// on a given XNKID no matter how many sockets are actively communicating
	// on that secure connection.
	//
	// The default is 32 security associations.
	//
	BYTE        cfgSecRegMax;

	//
	// The maximum amount of QoS data, in units of DWORD (4 bytes), that can be
	// supplied to a call to XNetQosListen or returned in the result set of a
	// call to XNetQosLookup.
	//
	// The default is 64 (256 bytes).
	//
	BYTE        cfgQosDataLimitDiv4;

	//
	// The amount of time to wait for a response after sending a QoS packet
	// before sending it again (or giving up).  This should be set to the same
	// value on clients (XNetQosLookup callers) and servers (XNetQosListen
	// callers).
	//
	// The default is 2 seconds.
	//
	BYTE        cfgQosProbeTimeoutInSeconds;

	//
	// The maximum number of times to retry a given QoS packet when no response
	// is received.  This should be set to the same value on clients
	// (XNetQosLookup callers) and servers (XNetQosListen callers).
	//
	// The default is 3 retries.
	//
	BYTE        cfgQosProbeRetries;

	//
	// The maximum number of simultaneous QoS lookup responses that a QoS
	// listener supports.  Note that the bandwidth throttling parameter passed
	// to XNetQosListen may impact the number of responses queued, and thus
	// affects how quickly this limit is reached.
	//
	// The default is 8 responses.
	//
	BYTE        cfgQosSrvMaxSimultaneousResponses;

	//
	// The maximum amount of time for QoS listeners to wait for the second
	// packet in a packet pair.
	//
	// The default is 2 seconds.
	//
	BYTE        cfgQosPairWaitTimeInSeconds;

} XNetStartupParams;

typedef struct
{
	IN_ADDR     ina;                            // IP address (zero if not static/DHCP)
	IN_ADDR     inaOnline;                      // Online IP address (zero if not online)
	WORD        wPortOnline;                    // Online port
	BYTE        abEnet[6];                      // Ethernet MAC address
	BYTE        abOnline[20];                   // Online identification
} XNADDR;

typedef struct
{
	BYTE        ab[8];                          // xbox to xbox key identifier
} XNKID;

typedef XNADDR TSADDR;

#define XONLINE_S_LOGON_CONNECTION_ESTABLISHED          _HRESULT_TYPEDEF_(0x001510F0L)
#define XONLINE_S_LOGON_DISCONNECTED                    _HRESULT_TYPEDEF_(0x001510F1L)

#define XNET_XNKID_MASK                 0xF0    // Mask of flag bits in first byte of XNKID
#define XNET_XNKID_SYSTEM_LINK          0x00    // Peer to peer system link session
#define XNET_XNKID_SYSTEM_LINK_XPLAT    0x40    // Peer to peer system link session for cross-platform
#define XNET_XNKID_ONLINE_PEER          0x80    // Peer to peer online session
#define XNET_XNKID_ONLINE_SERVER        0xC0    // Client to server online session
#define XNET_XNKID_ONLINE_TITLESERVER   0xE0    // Client to title server online session

#define XNetXnKidIsSystemLinkXbox(pxnkid)       (((pxnkid)->ab[0] & 0xE0) == XNET_XNKID_SYSTEM_LINK)
#define XNetXnKidIsSystemLinkXPlat(pxnkid)      (((pxnkid)->ab[0] & 0xE0) == XNET_XNKID_SYSTEM_LINK_XPLAT)
#define XNetXnKidIsSystemLink(pxnkid)           (XNetXnKidIsSystemLinkXbox(pxnkid) || XNetXnKidIsSystemLinkXPlat(pxnkid))
#define XNetXnKidIsOnlinePeer(pxnkid)           (((pxnkid)->ab[0] & 0xE0) == XNET_XNKID_ONLINE_PEER)
#define XNetXnKidIsOnlineServer(pxnkid)         (((pxnkid)->ab[0] & 0xE0) == XNET_XNKID_ONLINE_SERVER)
#define XNetXnKidIsOnlineTitleServer(pxnkid)    (((pxnkid)->ab[0] & 0xE0) == XNET_XNKID_ONLINE_TITLESERVER)


typedef struct
{
	BYTE        ab[16];                         // xbox to xbox key exchange key
} XNKEY;

typedef struct
{
	INT         iStatus;                        // WSAEINPROGRESS if pending; 0 if success; error if failed
	UINT        cina;                           // Count of IP addresses for the given host
	IN_ADDR     aina[8];                        // Vector of IP addresses for the given host
} XNDNS;



#define XNET_XNQOSINFO_COMPLETE         0x01    // Qos has finished processing this entry
#define XNET_XNQOSINFO_TARGET_CONTACTED 0x02    // Target host was successfully contacted
#define XNET_XNQOSINFO_TARGET_DISABLED  0x04    // Target host has disabled its Qos listener
#define XNET_XNQOSINFO_DATA_RECEIVED    0x08    // Target host supplied Qos data
#define XNET_XNQOSINFO_PARTIAL_COMPLETE 0x10    // Qos has unfinished estimates for this entry
#define XNET_XNQOSINFO_UNK	0x11


#define XNET_QOS_LISTEN_ENABLE              0x00000001 // Responds to queries on the given XNKID
#define XNET_QOS_LISTEN_DISABLE             0x00000002 // Rejects queries on the given XNKID
#define XNET_QOS_LISTEN_SET_DATA            0x00000004 // Sets the block of data to send back to queriers
#define XNET_QOS_LISTEN_SET_BITSPERSEC      0x00000008 // Sets max bandwidth that query reponses may consume
#define XNET_QOS_LISTEN_RELEASE             0x00000010 // Stops listening on given XNKID and releases memory

#define XNET_QOS_LOOKUP_RESERVED            0x00000000 // No flags defined yet for XNetQosLookup

#define XNET_QOS_SERVICE_LOOKUP_RESERVED    0x00000000 // No flags defined yet for XNetQosServiceLookup

//INT   WINAPI XNetQosListen(const XNKID * pxnkid, const BYTE * pb, UINT cb, DWORD dwBitsPerSec, DWORD dwFlags);
//INT   WINAPI XNetQosLookup(UINT cxna, const XNADDR * apxna[], const XNKID * apxnkid[], const XNKEY * apxnkey[], UINT cina, const IN_ADDR aina[], const DWORD adwServiceId[], UINT cProbes, DWORD dwBitsPerSec, DWORD dwFlags, WSAEVENT hEvent, XNQOS ** ppxnqos);
//INT   WINAPI XNetQosServiceLookup(DWORD dwFlags, WSAEVENT hEvent, XNQOS ** ppxnqos);
//INT   WINAPI XNetQosRelease(XNQOS* pxnqos);
//INT   WINAPI XNetQosGetListenStats(const XNKID * pxnkid, XNQOSLISTENSTATS * pQosListenStats);



typedef struct
{
	BYTE        bFlags;                         // See XNET_XNQOSINFO_*
	BYTE        bReserved;                      // Reserved
	WORD        cProbesXmit;                    // Count of Qos probes transmitted
	WORD        cProbesRecv;                    // Count of Qos probes successfully received
	WORD        cbData;                         // Size of Qos data supplied by target (may be zero)
	BYTE *      pbData;                         // Qos data supplied by target (may be NULL)
	WORD        wRttMinInMsecs;                 // Minimum round-trip time in milliseconds
	WORD        wRttMedInMsecs;                 // Median round-trip time in milliseconds
	DWORD       dwUpBitsPerSec;                 // Upstream bandwidth in bits per second
	DWORD       dwDnBitsPerSec;                 // Downstream bandwidth in bits per second
} XNQOSINFO;

typedef struct
{
	UINT        cxnqos;                         // Count of items in axnqosinfo[] array
	UINT        cxnqosPending;                  // Count of items still pending
	XNQOSINFO   axnqosinfo[1];                  // Vector of Qos results
} XNQOS;

typedef struct
{
	DWORD       dwSizeOfStruct;                 // Structure size, must be set prior to calling XNetQosGetListenStats
	DWORD       dwNumDataRequestsReceived;      // Number of client data request probes received
	DWORD       dwNumProbesReceived;            // Number of client probe requests received
	DWORD       dwNumSlotsFullDiscards;         // Number of client requests discarded because all slots are full
	DWORD       dwNumDataRepliesSent;           // Number of data replies sent
	DWORD       dwNumDataReplyBytesSent;        // Number of data reply bytes sent
	DWORD       dwNumProbeRepliesSent;          // Number of probe replies sent
} XNQOSLISTENSTATS;



//INT   WINAPI XNetStartup(const XNetStartupParams * pxnsp);
//INT   WINAPI XNetCleanup();

//INT   WINAPI XNetRandom(BYTE * pb, UINT cb);

//INT   WINAPI XNetCreateKey(XNKID * pxnkid, XNKEY * pxnkey);
//INT   WINAPI XNetRegisterKey(const XNKID * pxnkid, const XNKEY * pxnkey);
//INT   WINAPI XNetUnregisterKey(const XNKID * pxnkid);
//INT   WINAPI XNetReplaceKey(const XNKID * pxnkidUnregister, const XNKID * pxnkidReplace);

//INT   WINAPI XNetXnAddrToInAddr(const XNADDR * pxna, const XNKID * pxnkid, IN_ADDR * pina);
//INT   WINAPI XNetServerToInAddr(const IN_ADDR ina, DWORD dwServiceId, IN_ADDR * pina);
//INT   WINAPI XNetTsAddrToInAddr(const TSADDR * ptsa, DWORD dwServiceId, const XNKID * pxnkid, IN_ADDR * pina);
//INT   WINAPI XNetInAddrToXnAddr(const IN_ADDR ina, XNADDR * pxna, XNKID * pxnkid);
//INT   WINAPI XNetInAddrToServer(const IN_ADDR ina, IN_ADDR *pina);
//INT   WINAPI XNetInAddrToString(const IN_ADDR ina, char * pchBuf, INT cchBuf);
//INT   WINAPI XNetUnregisterInAddr(const IN_ADDR ina);
//INT   WINAPI XNetXnAddrToMachineId(const XNADDR * pxnaddr, ULONGLONG * pqwMachineId);


#define XNET_XNADDR_PLATFORM_XBOX1          0x00000000 // Platform type is original Xbox
#define XNET_XNADDR_PLATFORM_XBOX360        0x00000001 // Platform type is Xbox 360
#define XNET_XNADDR_PLATFORM_WINPC          0x00000002 // Platform type is Windows PC

//INT   WINAPI XNetGetXnAddrPlatform(const XNADDR * pxnaddr, DWORD * pdwPlatform);


#define XNET_CONNECT_STATUS_IDLE            0x00000000 // Connection not started; use XNetConnect or send packet
#define XNET_CONNECT_STATUS_PENDING         0x00000001 // Connecting in progress; not complete yet
#define XNET_CONNECT_STATUS_CONNECTED       0x00000002 // Connection is established
#define XNET_CONNECT_STATUS_LOST            0x00000003 // Connection was lost

//INT   WINAPI XNetConnect(const IN_ADDR ina);
//INT WINAPI XNetGetConnectStatus(const IN_ADDR ina);


//INT   WINAPI XNetDnsLookup(const char * pszHost, WSAEVENT hEvent, XNDNS ** ppxndns);
//INT   WINAPI XNetDnsRelease(XNDNS * pxndns);

#define XNET_GET_XNADDR_PENDING             0x00000000 // Address acquisition is not yet complete
#define XNET_GET_XNADDR_NONE                0x00000001 // XNet is uninitialized or no debugger found
#define XNET_GET_XNADDR_ETHERNET            0x00000002 // Host has ethernet address (no IP address)
#define XNET_GET_XNADDR_STATIC              0x00000004 // Host has statically assigned IP address
#define XNET_GET_XNADDR_DHCP                0x00000008 // Host has DHCP assigned IP address
#define XNET_GET_XNADDR_PPPOE               0x00000010 // Host has PPPoE assigned IP address
#define XNET_GET_XNADDR_GATEWAY             0x00000020 // Host has one or more gateways configured
#define XNET_GET_XNADDR_DNS                 0x00000040 // Host has one or more DNS servers configured
#define XNET_GET_XNADDR_ONLINE              0x00000080 // Host is currently connected to online service
#define XNET_GET_XNADDR_TROUBLESHOOT        0x00008000 // Network configuration requires troubleshooting

//DWORD WINAPI XNetGetTitleXnAddr(XNADDR * pxna);
//DWORD WINAPI XNetGetDebugXnAddr(XNADDR * pxna);


#define XNET_ETHERNET_LINK_INACTIVE           0x00000000
#define XNET_ETHERNET_LINK_ACTIVE           0x00000001 // Ethernet cable is connected and active
#define XNET_ETHERNET_LINK_100MBPS          0x00000002 // Ethernet link is set to 100 Mbps
#define XNET_ETHERNET_LINK_10MBPS           0x00000004 // Ethernet link is set to 10 Mbps
#define XNET_ETHERNET_LINK_FULL_DUPLEX      0x00000008 // Ethernet link is in full duplex mode
#define XNET_ETHERNET_LINK_HALF_DUPLEX      0x00000010 // Ethernet link is in half duplex mode
#define XNET_ETHERNET_LINK_WIRELESS         0x00000020 // Ethernet link is wireless (802.11 based)

//DWORD WINAPI XNetGetEthernetLinkStatus();


#define XNET_BROADCAST_VERSION_OLDER        0x00000001 // Got broadcast packet(s) from incompatible older version of title
#define XNET_BROADCAST_VERSION_NEWER        0x00000002 // Got broadcast packet(s) from incompatible newer version of title

//DWORD WINAPI XNetGetBroadcastVersionStatus(BOOL fReset);


//
// Value = XNetStartupParams
// Get   = Returns the XNetStartupParams values that were used at
//         initialization time.
// Set   = Not allowed.
//
#define XNET_OPTID_STARTUP_PARAMS                   1

//
// Value = ULONGLONG
// Get   = Returns total number of bytes sent by the NIC hardware since system
//         boot, including sizes of all protocol headers.
// Set   = Not allowed.
//
#define XNET_OPTID_NIC_XMIT_BYTES                   2

//
// Value = DWORD
// Get   = Returns total number of frames sent by the NIC hardware since system
//         boot.
// Set   = Not allowed.
//
#define XNET_OPTID_NIC_XMIT_FRAMES                  3

//
// Value = ULONGLONG
// Get   = Returns total number of bytes received by the NIC hardware since
//         system boot, including sizes of all protocol headers.
// Set   = Not allowed.
//
#define XNET_OPTID_NIC_RECV_BYTES                   4

//
// Value = DWORD
// Get   = Returns total number of frames received by the NIC hardware since
//         system boot.
// Set   = Not allowed.
//
#define XNET_OPTID_NIC_RECV_FRAMES                  5

//
// Value = ULONGLONG
// Get   = Returns the number of bytes sent by the caller since XNetStartup/
//         WSAStartup, including sizes of all protocol headers.
// Set   = Not allowed.
//
#define XNET_OPTID_CALLER_XMIT_BYTES                6

//
// Value = DWORD
// Get   = Returns total number of frames sent by the caller since XNetStartup/
//         WSAStartup.
// Set   = Not allowed.
//
#define XNET_OPTID_CALLER_XMIT_FRAMES               7

//
// Value = ULONGLONG
// Get   = Returns total number of bytes received by the caller since
//         XNetStartup/WSAStartup, including sizes of all protocol headers.
// Set   = Not allowed.
//
#define XNET_OPTID_CALLER_RECV_BYTES                8

//
// Value = DWORD
// Get   = Returns total number of frames received by the caller since
//         XNetStartup/WSAStartup.
// Set   = Not allowed.
//
#define XNET_OPTID_CALLER_RECV_FRAMES               9

// Get   = Retrieves 0 for default behavior
// Set   = Set 0 for default behavior

//INT    WINAPI XNetGetOpt(DWORD dwOptId, BYTE * pbValue, DWORD * pdwValueSize);
//INT    WINAPI XNetSetOpt(DWORD dwOptId, const BYTE * pbValue, DWORD dwValueSize);

#define XNET_CONNECT_STATUS_IDLE            0x00000000 // Connection not started; use XNetConnect or send packet
#define XNET_CONNECT_STATUS_PENDING         0x00000001 // Connecting in progress; not complete yet
#define XNET_CONNECT_STATUS_CONNECTED       0x00000002 // Connection is established
#define XNET_CONNECT_STATUS_LOST            0x00000003 // Connection was lost


//
// Since our socket handles are not file handles, apps can NOT call CancelIO API to cancel
// outstanding overlapped I/O requests. Apps must call WSACancelOverlappedIO function instead.
//

typedef ULONGLONG XUID;
typedef XUID *PXUID;

#define INVALID_XUID                    ((XUID) 0)

#define XUSER_NAME_SIZE                 16
#define XUSER_MAX_NAME_LENGTH           (XUSER_NAME_SIZE - 1)

#define XUSER_GET_SIGNIN_INFO_ONLINE_XUID_ONLY      0x00000002
#define XUSER_GET_SIGNIN_INFO_OFFLINE_XUID_ONLY     0x00000001

#define XUSER_INFO_FLAG_LIVE_ENABLED    0x00000001
#define XUSER_INFO_FLAG_GUEST           0x00000002

typedef struct _XUSER_DATA
{
	BYTE                                type;

	union
	{
		LONG                            nData;     // XUSER_DATA_TYPE_INT32
		LONGLONG                        i64Data;   // XUSER_DATA_TYPE_INT64
		double                          dblData;   // XUSER_DATA_TYPE_DOUBLE
		struct                                     // XUSER_DATA_TYPE_UNICODE
		{
			DWORD                       cbData;    // Includes null-terminator
			LPWSTR                      pwszData;
		} string;
		FLOAT                           fData;     // XUSER_DATA_TYPE_FLOAT
		struct                                     // XUSER_DATA_TYPE_BINARY
		{
			DWORD                       cbData;
			PBYTE                       pbData;
		} binary;
		FILETIME                        ftData;    // XUSER_DATA_TYPE_DATETIME
	};
} XUSER_DATA, *PXUSER_DATA;

typedef enum _XPRIVILEGE_TYPE
{
	XPRIVILEGE_MULTIPLAYER_SESSIONS = 254, // on|off

	XPRIVILEGE_COMMUNICATIONS = 252, // on (communicate w/everyone) | off (check _FO)
	XPRIVILEGE_COMMUNICATIONS_FRIENDS_ONLY = 251, // on (communicate w/friends only) | off (blocked)

	XPRIVILEGE_PROFILE_VIEWING = 249, // on (viewing allowed) | off (check _FO)
	XPRIVILEGE_PROFILE_VIEWING_FRIENDS_ONLY = 248, // on (view friend's only) | off (no details)

	XPRIVILEGE_USER_CREATED_CONTENT = 247, // on (allow viewing of UCC) | off (check _FO)
	XPRIVILEGE_USER_CREATED_CONTENT_FRIENDS_ONLY = 246, // on (view UCC from friends only) | off (blocked)

	XPRIVILEGE_PURCHASE_CONTENT = 245, // on (allow purchase) | off (blocked)

	XPRIVILEGE_PRESENCE = 244, // on (share presence info) | off (check _FO)
	XPRIVILEGE_PRESENCE_FRIENDS_ONLY = 243, // on (share w/friends only | off (don't share)

	XPRIVILEGE_TRADE_CONTENT = 238, // on (allow trading) | off (blocked)

	XPRIVILEGE_VIDEO_COMMUNICATIONS = 235, // on (communicate w/everyone) | off (check _FO)
	XPRIVILEGE_VIDEO_COMMUNICATIONS_FRIENDS_ONLY = 234, // on (communicate w/friends only) | off (blocked)

	XPRIVILEGE_MULTIPLAYER_DEDICATED_SERVER = 226, // on (allow) | off (disallow)
} XPRIVILEGE_TYPE;


typedef struct _XUSER_PROPERTY
{
	DWORD                               dwPropertyId;
	XUSER_DATA                          value;
} XUSER_PROPERTY, *PXUSER_PROPERTY;

typedef enum _XUSER_SIGNIN_STATE
{
	eXUserSigninState_NotSignedIn,
	eXUserSigninState_SignedInLocally,
	eXUserSigninState_SignedInToLive
} XUSER_SIGNIN_STATE;

typedef struct {
	XUID                 xuid;
	DWORD                dwInfoFlags;
	XUSER_SIGNIN_STATE   UserSigninState;
	DWORD                dwGuestNumber;
	DWORD                dwSponsorUserIndex;
	CHAR                 szUserName[XUSER_NAME_SIZE];
} XUSER_SIGNIN_INFO, *PXUSER_SIGNIN_INFO;

// Xbox-specific Overlapped

typedef VOID (WINAPI *PXOVERLAPPED_COMPLETION_ROUTINE)(
	DWORD dwErrorCode,
	DWORD dwNumberOfBytesTransfered,
	DWORD pOverlapped
	);


typedef struct {
	ULONG_PTR                           InternalLow;
	ULONG_PTR                           InternalHigh;
	ULONG_PTR                           InternalContext;
	HANDLE                              hEvent;
	PXOVERLAPPED_COMPLETION_ROUTINE     pCompletionRoutine;
	DWORD_PTR                           dwCompletionContext;
	DWORD                               dwExtendedError;
} XOVERLAPPED, *PXOVERLAPPED;

typedef enum {
	XSOURCE_NO_VALUE = 0,
	XSOURCE_DEFAULT,
	XSOURCE_TITLE,
	XSOURCE_PERMISSION_DENIED
} XUSER_PROFILE_SOURCE;

typedef struct _XUSER_PROFILE_SETTING {
	XUSER_PROFILE_SOURCE source;

	union {
		DWORD dwUserIndex;
		XUID xuid;
	}user;

	DWORD dwSettingId;

	XUSER_DATA data;
} XUSER_PROFILE_SETTING, *PXUSER_PROFILE_SETTING;

typedef struct _XUSER_READ_PROFILE_SETTING_RESULT {
	DWORD dwSettingsLen;
	XUSER_PROFILE_SETTING *pSettings;
} XUSER_READ_PROFILE_SETTING_RESULT, *PXUSER_READ_PROFILE_SETTING_RESULT;

// XContent

// Content types
#define XCONTENTTYPE_SAVEDGAME                      0x00000001
#define XCONTENTTYPE_MARKETPLACE                    0x00000002
#define XCONTENTTYPE_PUBLISHER                      0x00000003

#define XCONTENTTYPE_GAMEDEMO                       0x00080000
#define XCONTENTTYPE_ARCADE                         0x000D0000

//  Content creation/open flags
#define XCONTENTFLAG_NONE                           0x00000000
#define XCONTENTFLAG_CREATENEW                      CREATE_NEW
#define XCONTENTFLAG_CREATEALWAYS                   CREATE_ALWAYS
#define XCONTENTFLAG_OPENEXISTING                   OPEN_EXISTING
#define XCONTENTFLAG_OPENALWAYS                     OPEN_ALWAYS
#define XCONTENTFLAG_TRUNCATEEXISTING               TRUNCATE_EXISTING

//  Content attributes
#define XCONTENTFLAG_NOPROFILE_TRANSFER             0x00000010
#define XCONTENTFLAG_NODEVICE_TRANSFER              0x00000020
#define XCONTENTFLAG_STRONG_SIGNED                  0x00000040
#define XCONTENTFLAG_ALLOWPROFILE_TRANSFER          0x00000080
#define XCONTENTFLAG_MOVEONLY_TRANSFER              0x00000800

//  Device selector flags
#define XCONTENTFLAG_MANAGESTORAGE                  0x00000100
#define XCONTENTFLAG_FORCE_SHOW_UI                  0x00000200

//  Enumeration scoping
#define XCONTENTFLAG_ENUM_EXCLUDECOMMON             0x00001000


#define XCONTENT_MAX_DISPLAYNAME_LENGTH 128
#define XCONTENT_MAX_FILENAME_LENGTH    42
#define XCONTENTDEVICE_MAX_NAME_LENGTH  27

typedef DWORD                           XCONTENTDEVICEID, *PXCONTENTDEVICEID;

typedef struct _XCONTENT_DATA {
	DWORD ContentNum;
	DWORD TitleId;
	DWORD ContentPackageType;
	BYTE ContentId[20];
} XCONTENT_DATA, *PXCONTENT_DATA;


typedef struct _XUSER_ACHIEVEMENT {
	DWORD dwUserIndex;
	DWORD dwAchievementId;
} XUSER_ACHIEVEMENT, *PXUSER_ACHIEVEMENT;


typedef struct {
	XNKID sessionID;
	XNADDR hostAddress;
	XNKEY keyExchangeKey;
} XSESSION_INFO, *PXSESSION_INFO;



typedef enum _XSESSION_STATE
{
	XSESSION_STATE_LOBBY = 0,
	XSESSION_STATE_REGISTRATION,
	XSESSION_STATE_INGAME,
	XSESSION_STATE_REPORTING,
	XSESSION_STATE_DELETED
} XSESSION_STATE;


typedef struct {
	XUID xuidOnline;
	DWORD dwUserIndex;
	DWORD dwFlags;
} XSESSION_MEMBER;


typedef struct {
	DWORD dwUserIndexHost;
	DWORD dwGameType;
	DWORD dwGameMode;
	DWORD dwFlags;
	DWORD dwMaxPublicSlots;
	DWORD dwMaxPrivateSlots;
	DWORD dwAvailablePublicSlots;
	DWORD dwAvailablePrivateSlots;
	DWORD dwActualMemberCount;
	DWORD dwReturnedMemberCount;
	XSESSION_STATE eState;
	ULONGLONG qwNonce;
	XSESSION_INFO sessionInfo;
	XNKID xnkidArbitration;
	XSESSION_MEMBER *pSessionMembers;
} XSESSION_LOCAL_DETAILS, *PXSESSION_LOCAL_DETAILS;


typedef enum
{
	XONLINE_NAT_OPEN = 1,
	XONLINE_NAT_MODERATE,
	XONLINE_NAT_STRICT
} XONLINE_NAT_TYPE;

typedef struct _XUSER_CONTEXT
{
	DWORD                               dwContextId;
	DWORD                               dwValue;
} XUSER_CONTEXT, *PXUSER_CONTEXT;

typedef struct _XSESSION_SEARCHRESULT
{
	XSESSION_INFO   info;
	DWORD           dwOpenPublicSlots;
	DWORD           dwOpenPrivateSlots;
	DWORD           dwFilledPublicSlots;
	DWORD           dwFilledPrivateSlots;
	DWORD           cProperties;
	DWORD           cContexts;
	PXUSER_PROPERTY pProperties;
	PXUSER_CONTEXT  pContexts;
} XSESSION_SEARCHRESULT, *PXSESSION_SEARCHRESULT;


typedef struct _XSESSION_SEARCHRESULT_HEADER
{
	DWORD dwSearchResults;
	XSESSION_SEARCHRESULT *pResults;
} XSESSION_SEARCHRESULT_HEADER, *PXSESSION_SEARCHRESULT_HEADER;



typedef struct _XSESSION_REGISTRANT
{
	ULONGLONG qwMachineID;
	DWORD bTrustworthiness;
	DWORD bNumUsers;
	XUID *rgUsers;
} XSESSION_REGISTRANT;


typedef struct _XSESSION_REGISTRATION_RESULTS
{
	DWORD wNumRegistrants;
	XSESSION_REGISTRANT *rgRegistrants;
} XSESSION_REGISTRATION_RESULTS, *PXSESSION_REGISTRATION_RESULTS;



//#define X_CONTEXT_PRESENCE              0x00008001	// ??
//#define X_CONTEXT_GAME_TYPE             0x0000800A	// DR2
//#define X_CONTEXT_GAME_MODE             0x0000800B


// Values for X_CONTEXT_GAME_TYPE
//#define X_CONTEXT_GAME_TYPE_RANKED      0
//#define X_CONTEXT_GAME_TYPE_STANDARD    1

typedef enum
{
	XMARKETPLACE_OFFERING_TYPE_CONTENT = 0x00000002,
	XMARKETPLACE_OFFERING_TYPE_GAME_DEMO = 0x00000020,
	XMARKETPLACE_OFFERING_TYPE_GAME_TRAILER = 0x00000040,
	XMARKETPLACE_OFFERING_TYPE_THEME = 0x00000080,
	XMARKETPLACE_OFFERING_TYPE_TILE = 0x00000800,
	XMARKETPLACE_OFFERING_TYPE_ARCADE = 0x00002000,
	XMARKETPLACE_OFFERING_TYPE_VIDEO = 0x00004000,
	XMARKETPLACE_OFFERING_TYPE_CONSUMABLE = 0x00010000,
	XMARKETPLACE_OFFERING_TYPE_AVATARITEM = 0x00100000
} XMARKETPLACE_OFFERING_TYPE;


typedef struct _MESSAGEBOX_RESULT {
	union {
		DWORD dwButtonPressed;
		WORD rgwPasscode[4];
	};
}	MESSAGEBOX_RESULT, *PMESSAGEBOX_RESULT;


typedef enum _XSTORAGE_FACILITY
{
	XSTORAGE_FACILITY_GAME_CLIP = 1,
	XSTORAGE_FACILITY_PER_TITLE = 2,
	XSTORAGE_FACILITY_PER_USER_TITLE = 3
} XSTORAGE_FACILITY;


typedef struct _XSTORAGE_DOWNLOAD_TO_MEMORY_RESULTS {
	DWORD dwBytesTotal;
	XUID xuidOwner;
	FILETIME ftCreated;
} XSTORAGE_DOWNLOAD_TO_MEMORY_RESULTS;


typedef struct {
	DWORD dwNewOffers;
	DWORD dwTotalOffers;
} XOFFERING_CONTENTAVAILABLE_RESULT;



#define XMARKETPLACE_CONTENT_ID_LEN 20

typedef struct {
	ULONGLONG qwOfferID;
	ULONGLONG qwPreviewOfferID;
	DWORD dwOfferNameLength;
	WCHAR *wszOfferName;
	DWORD dwOfferType;
	BYTE contentId[XMARKETPLACE_CONTENT_ID_LEN];
	BOOL fIsUnrestrictedLicense;
	DWORD dwLicenseMask;
	DWORD dwTitleID;
	DWORD dwContentCategory;
	DWORD dwTitleNameLength;
	WCHAR *wszTitleName;
	BOOL fUserHasPurchased;
	DWORD dwPackageSize;
	DWORD dwInstallSize;
	DWORD dwSellTextLength;
	WCHAR *wszSellText;
	DWORD dwAssetID;
	DWORD dwPurchaseQuantity;
	DWORD dwPointsPrice;
} XMARKETPLACE_CONTENTOFFER_INFO, *PXMARKETPLACE_CONTENTOFFER_INFO;



typedef struct _STRING_DATA {
	WORD wStringSize;
	WCHAR *pszString;
} STRING_DATA;


#pragma pack( push,1 )
typedef struct _STRING_VERIFY_RESPONSE {
	WORD wNumStrings;
	HRESULT *pStringResult;
} STRING_VERIFY_RESPONSE;
#pragma pack( pop )



#define XNID(Version, Area, Index)      (DWORD)( (WORD)(Area) << 25 | (WORD)(Version) << 16 | (WORD)(Index))
#define XNID_VERSION(msgid)             (((msgid) >> 16) & 0x1FF)
#define XNID_AREA(msgid)                (((msgid) >> 25) & 0x3F)
#define XNID_INDEX(msgid)               ((msgid) & 0xFFFF)


//
// Notification Areas
//

#define XNOTIFY_SYSTEM                  (0x00000001)
#define XNOTIFY_LIVE                    (0x00000002)
#define XNOTIFY_FRIENDS                 (0x00000004)
#define XNOTIFY_CUSTOM                  (0x00000008)
#define XNOTIFY_XMP                     (0x00000020)
#define XNOTIFY_MSGR                    (0x00000040)
#define XNOTIFY_PARTY                   (0x00000080)
#define XNOTIFY_ALL                     (XNOTIFY_SYSTEM | XNOTIFY_LIVE | XNOTIFY_FRIENDS | XNOTIFY_CUSTOM | XNOTIFY_XMP | XNOTIFY_MSGR | XNOTIFY_PARTY)

//
// Bit numbers of each area (bit 0 is the least significant bit)
//

#define _XNAREA_SYSTEM                  (0)
#define _XNAREA_LIVE                    (1)
#define _XNAREA_FRIENDS                 (2)
#define _XNAREA_CUSTOM                  (3)
#define _XNAREA_XMP                     (5)
#define _XNAREA_MSGR                    (6)
#define _XNAREA_PARTY                   (7)

//
// System notifications
//

#define XN_SYS_FIRST                    XNID(0, _XNAREA_SYSTEM, 0x0001)
#define XN_SYS_UI                       XNID(0, _XNAREA_SYSTEM, 0x0009)
#define XN_SYS_SIGNINCHANGED            XNID(0, _XNAREA_SYSTEM, 0x000a)
#define XN_SYS_STORAGEDEVICESCHANGED    XNID(0, _XNAREA_SYSTEM, 0x000b)
#define XN_SYS_PROFILESETTINGCHANGED    XNID(0, _XNAREA_SYSTEM, 0x000e)
#define XN_SYS_MUTELISTCHANGED          XNID(0, _XNAREA_SYSTEM, 0x0011)
#define XN_SYS_INPUTDEVICESCHANGED      XNID(0, _XNAREA_SYSTEM, 0x0012)
#define XN_SYS_INPUTDEVICECONFIGCHANGED XNID(1, _XNAREA_SYSTEM, 0x0013)
#define XN_SYS_PLAYTIMERNOTICE          XNID(3, _XNAREA_SYSTEM, 0x0015)
#define XN_SYS_AVATARCHANGED            XNID(4, _XNAREA_SYSTEM, 0x0017)
#define XN_SYS_NUIHARDWARESTATUSCHANGED XNID(6, _XNAREA_SYSTEM, 0x0019)
#define XN_SYS_NUIPAUSE                 XNID(6, _XNAREA_SYSTEM, 0x001a)
#define XN_SYS_NUIUIAPPROACH            XNID(6, _XNAREA_SYSTEM, 0x001b)
#define XN_SYS_DEVICEREMAP              XNID(6, _XNAREA_SYSTEM, 0x001c)
#define XN_SYS_NUIBINDINGCHANGED        XNID(6, _XNAREA_SYSTEM, 0x001d)
#define XN_SYS_AUDIOLATENCYCHANGED      XNID(8, _XNAREA_SYSTEM, 0x001e)
#define XN_SYS_NUICHATBINDINGCHANGED    XNID(8, _XNAREA_SYSTEM, 0x001f)
#define XN_SYS_INPUTACTIVITYCHANGED     XNID(9, _XNAREA_SYSTEM, 0x0020)
#define XN_SYS_LAST                     XNID(0, _XNAREA_SYSTEM, 0x0023)

//
// Live notifications
//

#define XN_LIVE_FIRST                   XNID(0, _XNAREA_LIVE, 0x0001)
#define XN_LIVE_CONNECTIONCHANGED       XNID(0, _XNAREA_LIVE, 0x0001)
#define XN_LIVE_INVITE_ACCEPTED         XNID(0, _XNAREA_LIVE, 0x0002)
#define XN_LIVE_LINK_STATE_CHANGED      XNID(0, _XNAREA_LIVE, 0x0003)
#define XN_LIVE_CONTENT_INSTALLED       XNID(0, _XNAREA_LIVE, 0x0007)
#define XN_LIVE_MEMBERSHIP_PURCHASED    XNID(0, _XNAREA_LIVE, 0x0008)
#define XN_LIVE_VOICECHAT_AWAY          XNID(0, _XNAREA_LIVE, 0x0009)
#define XN_LIVE_PRESENCE_CHANGED        XNID(0, _XNAREA_LIVE, 0x000A)
#define XN_LIVE_LAST                    XNID(XNID_CURRENTVERSION+1, _XNAREA_LIVE, 0x0014)

//
// Friends notifications
//

#define XN_FRIENDS_FIRST                XNID(0, _XNAREA_FRIENDS, 0x0001)
#define XN_FRIENDS_PRESENCE_CHANGED     XNID(0, _XNAREA_FRIENDS, 0x0001)
#define XN_FRIENDS_FRIEND_ADDED         XNID(0, _XNAREA_FRIENDS, 0x0002)
#define XN_FRIENDS_FRIEND_REMOVED       XNID(0, _XNAREA_FRIENDS, 0x0003)
#define XN_FRIENDS_LAST                 XNID(XNID_CURRENTVERSION+1, _XNAREA_FRIENDS, 0x0009)

//
// Custom notifications
//

#define XN_CUSTOM_FIRST                 XNID(0, _XNAREA_CUSTOM, 0x0001)
#define XN_CUSTOM_ACTIONPRESSED         XNID(0, _XNAREA_CUSTOM, 0x0003)
#define XN_CUSTOM_GAMERCARD             XNID(1, _XNAREA_CUSTOM, 0x0004)
#define XN_CUSTOM_LAST                  XNID(XNID_CURRENTVERSION+1, _XNAREA_CUSTOM, 0x0005)


//
// XMP notifications
//

#define XN_XMP_FIRST                                     XNID(0, _XNAREA_XMP, 0x0001)
#define XN_XMP_STATECHANGED                              XNID(0, _XNAREA_XMP, 0x0001)
#define XN_XMP_PLAYBACKBEHAVIORCHANGED                   XNID(0, _XNAREA_XMP, 0x0002)
#define XN_XMP_PLAYBACKCONTROLLERCHANGED                 XNID(0, _XNAREA_XMP, 0x0003)
#define XN_XMP_LAST                                      XNID(XNID_CURRENTVERSION+1, _XNAREA_XMP, 0x000D)


//
// Party notifications
//

#define XN_PARTY_FIRST                                   XNID(0, _XNAREA_PARTY, 0x0001)
#define XN_PARTY_MEMBERS_CHANGED                         XNID(4, _XNAREA_PARTY, 0x0002)
#define XN_PARTY_LAST                                    XNID(XNID_CURRENTVERSION+1, _XNAREA_PARTY, 0x0006)






// xbox.h

#define CUSTOMACTION_FLAG_CLOSESUI      1

typedef enum
{
	XMSG_FLAG_DISABLE_EDIT_RECIPIENTS = 0x00000001
} XMSG_FLAGS;

#define XMSG_MAX_CUSTOM_IMAGE_SIZE      (36*1024)

typedef enum
{
	XCUSTOMACTION_FLAG_CLOSES_GUIDE = 0x00000001,
	XCUSTOMACTION_FLAG_DELETES_MESSAGE = 0x00000002
} XCUSTOMACTION_FLAGS;

#define XMSG_MAX_CUSTOMACTION_TRANSLATIONS      11


#define XCUSTOMACTION_MAX_PAYLOAD_SIZE  1024

#define XPLAYERLIST_CUSTOMTEXT_MAX_LENGTH   31
#define XPLAYERLIST_TITLE_MAX_LENGTH        36
#define XPLAYERLIST_DESCRIPTION_MAX_LENGTH  83
#define XPLAYERLIST_IMAGE_MAX_SIZE          36864
#define XPLAYERLIST_MAX_PLAYERS             100
#define XPLAYERLIST_BUTTONTEXT_MAX_LENGTH   23

typedef struct
{
	XUID        xuid;
	WCHAR       wszCustomText[XPLAYERLIST_CUSTOMTEXT_MAX_LENGTH];
} XPLAYERLIST_USER;

typedef struct {
	DWORD dwType;
	WCHAR wszCustomText[2000];
} XPLAYERLIST_BUTTON;

typedef struct {
	XUID xuidSelected;
	DWORD dwKeyCode;
} XPLAYERLIST_RESULT;

typedef enum
{
	XPLAYERLIST_FLAG_CUSTOMTEXT = 0x00000001,
} XPLAYERLIST_FLAGS;

typedef enum _XSHOWMARKETPLACEUI_ENTRYPOINTS {
	XSHOWMARKETPLACEUI_ENTRYPOINT_CONTENTLIST,
	XSHOWMARKETPLACEUI_ENTRYPOINT_CONTENTITEM,
	XSHOWMARKETPLACEUI_ENTRYPOINT_MEMBERSHIPLIST,
	XSHOWMARKETPLACEUI_ENTRYPOINT_MEMBERSHIPITEM,
	XSHOWMARKETPLACEUI_ENTRYPOINT_CONTENTLIST_BACKGROUND,
	XSHOWMARKETPLACEUI_ENTRYPOINT_CONTENTITEM_BACKGROUND,
	XSHOWMARKETPLACEUI_ENTRYPOINT_MAX
} XSHOWMARKETPLACEUI_ENTRYPOINTS;

#define MPDI_E_CANCELLED            _HRESULT_TYPEDEF_(0x8057F001)
#define MPDI_E_INVALIDARG           _HRESULT_TYPEDEF_(0x8057F002)
#define MPDI_E_OPERATION_FAILED     _HRESULT_TYPEDEF_(0x8057F003)

typedef enum _XSHOWMARKETPLACEDOWNLOADITEMSUI_ENTRYPOINTS {
	XSHOWMARKETPLACEDOWNLOADITEMS_ENTRYPOINT_FREEITEMS = 1000,
	XSHOWMARKETPLACEDOWNLOADITEMS_ENTRYPOINT_PAIDITEMS,
	XSHOWMARKETPLACEDOWNLOADITEMS_ENTRYPOINT_MAX
} XSHOWMARKETPLACEDOWNLOADITEMSUI_ENTRYPOINTS;

typedef struct
{
	WORD                                wActionId;
	WCHAR                               wszActionText[23];
	DWORD                               dwFlags;
} XCUSTOMACTION;

#define XSSUI_FLAGS_LOCALSIGNINONLY                 0x00000001
#define XSSUI_FLAGS_SHOWONLYONLINEENABLED           0x00000002

#define X_STATS_COLUMN_SKILL_SKILL              61
#define X_STATS_COLUMN_SKILL_GAMESPLAYED        62
#define X_STATS_COLUMN_SKILL_MU                 63
#define X_STATS_COLUMN_SKILL_SIGMA              64

#define X_STATS_SKILL_SKILL_DEFAULT             1
#define X_STATS_SKILL_MU_DEFAULT                3.0
#define X_STATS_SKILL_SIGMA_DEFAULT             1.0

#define X_STATS_COLUMN_ATTACHMENT_SIZE          ((WORD)0xFFFA)


#define XUSER_STATS_ATTRS_IN_SPEC       64

typedef struct _XUSER_STATS_COLUMN
{
	WORD                                wColumnId;
	XUSER_DATA                          Value;
} XUSER_STATS_COLUMN, *PXUSER_STATS_COLUMN;

typedef struct _XUSER_STATS_ROW
{
	XUID                                xuid;
	DWORD                               dwRank;
	LONGLONG                            i64Rating;
	CHAR                                szGamertag[XUSER_NAME_SIZE];
	DWORD                               dwNumColumns;
	PXUSER_STATS_COLUMN                 pColumns;
} XUSER_STATS_ROW, *PXUSER_STATS_ROW;

typedef struct _XUSER_STATS_VIEW
{
	DWORD                               dwViewId;
	DWORD                               dwTotalViewRows;
	DWORD                               dwNumRows;
	PXUSER_STATS_ROW                    pRows;
} XUSER_STATS_VIEW, *PXUSER_STATS_VIEW;

typedef struct _XUSER_STATS_READ_RESULTS
{
	DWORD                               dwNumViews;
	PXUSER_STATS_VIEW                   pViews;
} XUSER_STATS_READ_RESULTS, *PXUSER_STATS_READ_RESULTS;

typedef struct _XUSER_STATS_SPEC
{
	DWORD                               dwViewId;
	DWORD                               dwNumColumnIds;
	WORD                                rgwColumnIds[XUSER_STATS_ATTRS_IN_SPEC];
} XUSER_STATS_SPEC, *PXUSER_STATS_SPEC;

#define X_CONTEXT_GAME_TYPE_RANKED      0
#define X_CONTEXT_GAME_TYPE_STANDARD    1

#define X_PROPERTY_ATTACHMENT_SIZE                  XPROPERTYID(1, XUSER_DATA_TYPE_INT32,   0x11)

#define X_PROPERTY_PLAYER_PARTIAL_PLAY_PERCENTAGE   XPROPERTYID(1, XUSER_DATA_TYPE_INT32,   0xC)
#define X_PROPERTY_PLAYER_SKILL_UPDATE_WEIGHTING_FACTOR XPROPERTYID(1, XUSER_DATA_TYPE_INT32,   0xD)
#define X_PROPERTY_SESSION_SKILL_BETA               XPROPERTYID(1, XUSER_DATA_TYPE_DOUBLE,  0xE)
#define X_PROPERTY_SESSION_SKILL_TAU                XPROPERTYID(1, XUSER_DATA_TYPE_DOUBLE,  0xF)
#define X_PROPERTY_SESSION_SKILL_DRAW_PROBABILITY   XPROPERTYID(1, XUSER_DATA_TYPE_INT32,   0x10)

#define X_PROPERTY_RELATIVE_SCORE                   XPROPERTYID(1, XUSER_DATA_TYPE_INT32,   0xA)
#define X_PROPERTY_SESSION_TEAM                     XPROPERTYID(1, XUSER_DATA_TYPE_INT32,   0xB)

#define X_PROPERTY_GAMER_ZONE           XPROPERTYID(1, XUSER_DATA_TYPE_INT32,   0x101)
#define X_PROPERTY_GAMER_COUNTRY        XPROPERTYID(1, XUSER_DATA_TYPE_INT32,   0x102)
#define X_PROPERTY_GAMER_LANGUAGE       XPROPERTYID(1, XUSER_DATA_TYPE_INT32,   0x103)
#define X_PROPERTY_GAMER_RATING         XPROPERTYID(1, XUSER_DATA_TYPE_FLOAT,   0x104)
#define X_PROPERTY_GAMER_MU             XPROPERTYID(1, XUSER_DATA_TYPE_DOUBLE,  0x105)
#define X_PROPERTY_GAMER_SIGMA          XPROPERTYID(1, XUSER_DATA_TYPE_DOUBLE,  0x106)
#define X_PROPERTY_GAMER_PUID           XPROPERTYID(1, XUSER_DATA_TYPE_INT64,   0x107)
#define X_PROPERTY_AFFILIATE_SCORE      XPROPERTYID(1, XUSER_DATA_TYPE_INT64,   0x108)
#define X_PROPERTY_GAMER_HOSTNAME       XPROPERTYID(1, XUSER_DATA_TYPE_UNICODE, 0x109)

#define X_CONTEXT_PRESENCE              XCONTEXTID(1, 0x1)
#define X_CONTEXT_GAME_TYPE             XCONTEXTID(1, 0xA)
#define X_CONTEXT_GAME_MODE             XCONTEXTID(1, 0xB)
#define X_CONTEXT_SESSION_JOINABLE      XCONTEXTID(1, 0xC)

#define X_PROPERTY_RANK                 XPROPERTYID(1, XUSER_DATA_TYPE_INT32,   0x1)
#define X_PROPERTY_GAMERNAME            XPROPERTYID(1, XUSER_DATA_TYPE_UNICODE, 0x2)
#define X_PROPERTY_SESSION_ID           XPROPERTYID(1, XUSER_DATA_TYPE_INT64,   0x3)

#define X_PROPERTY_TYPE_MASK            0xF0000000
#define X_PROPERTY_SCOPE_MASK           0x00008000
#define X_PROPERTY_ID_MASK              0x00007FFF


#define XPROPERTYID(global, type, id)   ((global ? X_PROPERTY_SCOPE_MASK : 0) | ((type << 28) & X_PROPERTY_TYPE_MASK) | (id & X_PROPERTY_ID_MASK))
#define XCONTEXTID(global, id)          XPROPERTYID(global, XUSER_DATA_TYPE_CONTEXT, id)
#define XPROPERTYTYPEFROMID(id)         ((id >> 28) & 0xf)
#define XISSYSTEMPROPERTY(id)           (id & X_PROPERTY_SCOPE_MASK)





//Achievements

#define XACHIEVEMENT_MAX_COUNT 200

#define XACHIEVEMENT_TYPE_COMPLETION            1
#define XACHIEVEMENT_TYPE_LEVELING              2
#define XACHIEVEMENT_TYPE_UNLOCK                3
#define XACHIEVEMENT_TYPE_EVENT                 4
#define XACHIEVEMENT_TYPE_TOURNAMENT            5
#define XACHIEVEMENT_TYPE_CHECKPOINT            6
#define XACHIEVEMENT_TYPE_OTHER                 7

#define XACHIEVEMENT_DETAILS_ALL                0xFFFFFFFF
#define XACHIEVEMENT_DETAILS_LABEL              0x00000001
#define XACHIEVEMENT_DETAILS_DESCRIPTION        0x00000002
#define XACHIEVEMENT_DETAILS_UNACHIEVED         0x00000004
#define XACHIEVEMENT_DETAILS_TFC                0x00000020

#define XACHIEVEMENT_DETAILS_MASK_TYPE          0x00000007
#define XACHIEVEMENT_DETAILS_SHOWUNACHIEVED     0x00000008
#define XACHIEVEMENT_DETAILS_ACHIEVED_ONLINE    0x00010000
#define XACHIEVEMENT_DETAILS_ACHIEVED           0x00020000

#define AchievementType(dwFlags)           (dwFlags & XACHIEVEMENT_DETAILS_MASK_TYPE)
#define AchievementShowUnachieved(dwFlags) (dwFlags & XACHIEVEMENT_DETAILS_SHOWUNACHIEVED ? TRUE : FALSE)
#define AchievementEarnedOnline(dwFlags)   (dwFlags & XACHIEVEMENT_DETAILS_ACHIEVED_ONLINE ? TRUE : FALSE)
#define AchievementEarned(dwFlags)         (dwFlags & XACHIEVEMENT_DETAILS_ACHIEVED ? TRUE : FALSE)

#define XACHIEVEMENT_MAX_LABEL_LENGTH   33
#define XACHIEVEMENT_MAX_DESC_LENGTH    101
#define XACHIEVEMENT_MAX_UNACH_LENGTH   101

#define XACHIEVEMENT_SIZE_BASE          (sizeof(XACHIEVEMENT_DETAILS))
#define XACHIEVEMENT_SIZE_STRINGS       (sizeof(WCHAR) * (XACHIEVEMENT_MAX_LABEL_LENGTH  + XACHIEVEMENT_MAX_DESC_LENGTH + XACHIEVEMENT_MAX_UNACH_LENGTH))
#define XACHIEVEMENT_SIZE_FULL          (XACHIEVEMENT_SIZE_BASE + XACHIEVEMENT_SIZE_STRINGS)

#define XACHIEVEMENT_INVALID_ID         ((DWORD)0xFFFFFFFF)

typedef struct
{
	DWORD                               dwId;
	LPWSTR                              pwszLabel;
	LPWSTR                              pwszDescription;
	LPWSTR                              pwszUnachieved;
	DWORD                               dwImageId;
	DWORD                               dwCred;
	FILETIME                            ftAchieved;
	DWORD                               dwFlags;
} XACHIEVEMENT_DETAILS, *PXACHIEVEMENT_DETAILS;



#define XSTRING_MAX_LENGTH 512
#define XSTRING_MAX_STRINGS 10


#define XFRIENDS_MAX_C_RESULT 100


typedef struct {
	XUID xuidInvitee;
	XUID xuidInviter;
	DWORD dwTitleID;
	XSESSION_INFO hostInfo;
	BOOL fFromGameInvite;
} XINVITE_INFO, *PXINVITE_INFO;


#define XLOCATOR_PROPERTY_LIVE_COUNT_TOTAL 0
#define XLOCATOR_PROPERTY_LIVE_COUNT_PUBLIC 1
#define XLOCATOR_PROPERTY_LIVE_COUNT_GOLD 2
#define XLOCATOR_PROPERTY_LIVE_COUNT_PEER 3



#define XUSER_PROPERTY_GAMETYPE_NAME		0x40008228
#define XUSER_PROPERTY_GAMETYPE_NAME_2		0x4000822B
#define XUSER_PROPERTY_SERVER_NAME			0x40008230
#define XUSER_PROPERTY_SERVER_DESC			0x40008225
#define XUSER_PROPERTY_MAP_ID				0x10008207
#define XUSER_PROPERTY_MAP_ID_2				0x1000820A
#define XUSER_PROPERTY_MAP_NAME				0x40008226
#define XUSER_PROPERTY_MAP_NAME_2			0x40008229
#define XUSER_PROPERTY_GAMETYPE_ID			0x10008209
#define XUSER_PROPERTY_GAME_STATUS			0x10008211
#define XUSER_PROPERTY_VERSION_1			0x1000820E
#define XUSER_PROPERTY_VERSION_2			0x1000820F
#define XUSER_PROPERTY_UNKNOWN_INT64		0x2000821B
#define XUSER_PROPERTY_MAP_HASH_1			0x40008227
#define XUSER_PROPERTY_MAP_HASH_2			0x4000822A
#define XUSER_PROPERTY_USER_INT				0x10008202
#define XUSER_PROPERTY_UNKNOWN_INT32_1		0x10008208
#define XUSER_PROPERTY_UNKNOWN_INT32_2		0x1000820B
#define XUSER_PROPERTY_UNKNOWN_INT32_3		0x1000820C
#define XUSER_PROPERTY_UNKNOWN_INT32_4		0x1000820D
#define XUSER_PROPERTY_PARTY_PRIVACY		0x10008210
#define XUSER_PROPERTY_UNKNOWN_INT32_6		0x10008212
#define XUSER_PROPERTY_UNKNOWN_INT32_7		0x10008213
#define XUSER_PROPERTY_USERNAME_2			0x4000822C

#define XLOCATOR_DEDICATEDSERVER_PROPERTY_START     0x200

// These properties are used for search only.
// The search result header should already contains the information, and the query should not request these properties again.
#define X_PROPERTY_DEDICATEDSERVER_IDENTITY             XPROPERTYID(1, XUSER_DATA_TYPE_INT64,  XLOCATOR_DEDICATEDSERVER_PROPERTY_START + XTS_SEARCH_FIELD_ServerIdentity)   // server id. supports '=' operator onl$
#define X_PROPERTY_DEDICATEDSERVER_TYPE                 XPROPERTYID(1, XUSER_DATA_TYPE_INT32,  XLOCATOR_DEDICATEDSERVER_PROPERTY_START + XTS_SEARCH_FIELD_ServerType)
#define X_PROPERTY_DEDICATEDSERVER_MAX_PUBLIC_SLOTS     XPROPERTYID(1, XUSER_DATA_TYPE_INT32,  XLOCATOR_DEDICATEDSERVER_PROPERTY_START + XTS_SEARCH_FIELD_MaxPublicSlots)
#define X_PROPERTY_DEDICATEDSERVER_MAX_PRIVATE_SLOTS    XPROPERTYID(1, XUSER_DATA_TYPE_INT32,  XLOCATOR_DEDICATEDSERVER_PROPERTY_START + XTS_SEARCH_FIELD_MaxPrivateSlots)
#define X_PROPERTY_DEDICATEDSERVER_AVAILABLE_PUBLIC_SLOTS   XPROPERTYID(1, XUSER_DATA_TYPE_INT32,  XLOCATOR_DEDICATEDSERVER_PROPERTY_START + XTS_SEARCH_FIELD_AvailablePublicSlots)
#define X_PROPERTY_DEDICATEDSERVER_AVAILABLE_PRIVATE_SLOTS  XPROPERTYID(1, XUSER_DATA_TYPE_INT32,  XLOCATOR_DEDICATEDSERVER_PROPERTY_START + XTS_SEARCH_FIELD_AvailablePrivateSlots)
#define X_PROPERTY_DEDICATEDSERVER_FILLED_PUBLIC_SLOTS      XPROPERTYID(1, XUSER_DATA_TYPE_INT32,   XLOCATOR_DEDICATEDSERVER_PROPERTY_START + XTS_SEARCH_FIELD_FilledPublicSlots)
#define X_PROPERTY_DEDICATEDSERVER_FILLED_PRIVATE_SLOTS     XPROPERTYID(1, XUSER_DATA_TYPE_INT32,   XLOCATOR_DEDICATEDSERVER_PROPERTY_START + XTS_SEARCH_FIELD_FilledPrivateSlots)


// the following properties only support XTS_FILTER_COMPARE_OPERATOR_Equals operator
#define X_PROPERTY_DEDICATEDSERVER_OWNER_XUID           XPROPERTYID(1, XUSER_DATA_TYPE_INT64,   XLOCATOR_DEDICATEDSERVER_PROPERTY_START + XTS_SEARCH_FIELD_OwnerXuid)
#define X_PROPERTY_DEDICATEDSERVER_OWNER_GAMERTAG       XPROPERTYID(1, XUSER_DATA_TYPE_UNICODE,   XLOCATOR_DEDICATEDSERVER_PROPERTY_START + XTS_SEARCH_FIELD_OwnerGamerTag)
#define X_PROPERTY_DEDICATEDSERVER_REGIONID             XPROPERTYID(1, XUSER_DATA_TYPE_INT32,   XLOCATOR_DEDICATEDSERVER_PROPERTY_START + XTS_SEARCH_FIELD_RegionID)
#define X_PROPERTY_DEDICATEDSERVER_LANGUAGEID           XPROPERTYID(1, XUSER_DATA_TYPE_INT32,   XLOCATOR_DEDICATEDSERVER_PROPERTY_START + XTS_SEARCH_FIELD_LanguageID)

// Predefined dedicated server types
#define XLOCATOR_SERVERTYPE_PUBLIC          0   // dedicated server is for all players.
#define XLOCATOR_SERVERTYPE_GOLD_ONLY       1   // dedicated server is for Gold players only.
#define XLOCATOR_SERVERTYPE_PEER_HOSTED     2   // dedicated server is a peer-hosted game server.
#define XLOCATOR_SERVERTYPE_PEER_HOSTED_GOLD_ONLY   3   // dedicated server is a peer-hosted game server (gold only).
#define XLOCATOR_SERVICESTATUS_PROPERTY_START     0x100

typedef struct _XLOCATOR_SEARCHRESULT {
	ULONGLONG serverID;                     // the ID of dedicated server
	DWORD dwServerType;                     // see XLOCATOR_SERVERTYPE_PUBLIC, etc
	XNADDR serverAddress;                   // the address dedicated server
	XNKID xnkid;
	XNKEY xnkey;
	DWORD dwMaxPublicSlots;
	DWORD dwMaxPrivateSlots;
	DWORD dwFilledPublicSlots;
	DWORD dwFilledPrivateSlots;
	DWORD cProperties;                      // number of custom properties.
	PXUSER_PROPERTY pProperties;            // an array of custom properties.

} XLOCATOR_SEARCHRESULT, *PXLOCATOR_SEARCHRESULT;



#define XLMGRCREDS_FLAG_SAVE 1
#define XLMGRCREDS_FLAG_DELETE 2

#define XLSIGNIN_FLAG_SAVECREDS 1
#define XLSIGNIN_FLAG_ALLOWTITLEUPDATES 2
#define XLSIGNIN_FLAG_ALLOWSYSTEMUPDATES 4

#endif