#include <winsock2.h>
#include "xdefs.h"
#include "../xlln/DebugText.h"
#include "xlive.h"
#include "xrender.h"
#include "xnet.h"
#include "../xlln/xlln.h"
#include "xsocket.h"
#include "xlocator.h"
#include "xsession.h"
#include <time.h>
#include <d3d9.h>
#include <string>
#include <vector>
// Link with iphlpapi.lib
#include <iphlpapi.h>
#include "xuser.h"

BOOL xlive_debug_pause = FALSE;

BOOL xlive_users_info_changed[XLIVE_LOCAL_USER_COUNT];
XUSER_SIGNIN_INFO* xlive_users_info[XLIVE_LOCAL_USER_COUNT];
CHAR temporaryUserSettingsByteBuffer[8192];

struct NOTIFY_LISTENER {
	HANDLE id;
	ULONGLONG area;
};
static NOTIFY_LISTENER g_listener[50];
static int g_dwListener = 0;

bool xlive_invite_to_game = false;

static CRITICAL_SECTION d_lock;

static char* xlive_preferred_network_adapter_name = NULL;
EligibleAdapter xlive_network_adapter;

BOOL xlive_online_initialized = FALSE;
XLIVE_DEBUG_LEVEL xlive_xdlLevel = XLIVE_DEBUG_LEVEL_OFF;

INT GetNetworkAdapter()
{
	// Declare and initialize variables
	DWORD dwRetVal = 0;

	// Set the flags to pass to GetAdaptersAddresses
	ULONG flags = GAA_FLAG_INCLUDE_PREFIX;

	// IPv4
	ULONG family = AF_INET;

	PIP_ADAPTER_ADDRESSES pAddresses = NULL;
	// Allocate a 15 KB buffer to start with.
	ULONG outBufLen = 15000;
	ULONG Iterations = 0;

	PIP_ADAPTER_ADDRESSES pCurrAddresses = NULL;
	PIP_ADAPTER_UNICAST_ADDRESS pUnicast = NULL;

	INT result = ERROR_UNIDENTIFIED_ERROR;

	do {
		pAddresses = (IP_ADAPTER_ADDRESSES*)HeapAlloc(GetProcessHeap(), 0, (outBufLen));
		if (pAddresses == NULL) {
			addDebugText("Memory allocation failed for IP_ADAPTER_ADDRESSES struct");
			dwRetVal = ERROR_NOT_ENOUGH_MEMORY;
			break;
		}

		dwRetVal = GetAdaptersAddresses(family, flags, NULL, pAddresses, &outBufLen);

		if (dwRetVal == ERROR_BUFFER_OVERFLOW) {
			HeapFree(GetProcessHeap(), 0, pAddresses);
			pAddresses = NULL;
		}
		else {
			break;
		}

		Iterations++;
		// 3 attempts max
	} while ((dwRetVal == ERROR_BUFFER_OVERFLOW) && (Iterations < 3));

	if (dwRetVal == NO_ERROR) {
		std::vector<EligibleAdapter*> eligible_adapters;

		// If successful, output some information from the data we received
		pCurrAddresses = pAddresses;
		while (pCurrAddresses) {

			if (pCurrAddresses->OperStatus == 1) {

				//addDebugText("\tAdapter name: %s", pCurrAddresses->AdapterName);

				pUnicast = pCurrAddresses->FirstUnicastAddress;
				
				for (int i = 0; pUnicast != NULL; i++)
				{
					if (pUnicast->Address.lpSockaddr->sa_family == AF_INET)
					{
						sockaddr_in *sa_in = (sockaddr_in *)pUnicast->Address.lpSockaddr;
						ULONG dwMask = 0;
						dwRetVal = ConvertLengthToIpv4Mask(pUnicast->OnLinkPrefixLength, &dwMask);
						if (dwRetVal == NO_ERROR) {
							EligibleAdapter *ea = new EligibleAdapter;
							ea->name = pCurrAddresses->AdapterName;
							ea->unicastHAddr = ntohl(((sockaddr_in *)pUnicast->Address.lpSockaddr)->sin_addr.s_addr);
							ea->unicastHMask = ntohl(dwMask);
							ea->minLinkSpeed = pCurrAddresses->ReceiveLinkSpeed;
							if (pCurrAddresses->TransmitLinkSpeed < pCurrAddresses->ReceiveLinkSpeed)
								ea->minLinkSpeed = pCurrAddresses->TransmitLinkSpeed;
							ea->hasDnsServer = pCurrAddresses->FirstDnsServerAddress ? TRUE : FALSE;
							eligible_adapters.push_back(ea);
						}
					}
					pUnicast = pUnicast->Next;
				}
			}

			pCurrAddresses = pCurrAddresses->Next;
		}

		EligibleAdapter* chosenAdapter = NULL;

		for (EligibleAdapter* ea : eligible_adapters) {
			if (xlive_preferred_network_adapter_name && strncmp(ea->name, xlive_preferred_network_adapter_name, 50) == 0) {
				chosenAdapter = ea;
				break;
			}
			if (ea->unicastHAddr == INADDR_LOOPBACK || ea->unicastHAddr == INADDR_BROADCAST || ea->unicastHAddr == INADDR_NONE)
				continue;
			if (!chosenAdapter) {
				chosenAdapter = ea;
				continue;
			}
			if ((ea->hasDnsServer && !chosenAdapter->hasDnsServer) ||
				(ea->minLinkSpeed > chosenAdapter->minLinkSpeed)) {
				chosenAdapter = ea;
			}
		}

		if (chosenAdapter) {
			memcpy_s(&xlive_network_adapter, sizeof(EligibleAdapter), chosenAdapter, sizeof(EligibleAdapter));
			xlive_network_adapter.hBroadcast = ~(xlive_network_adapter.unicastHMask) | xlive_network_adapter.unicastHAddr;

			unsigned int adapter_name_buflen = strnlen_s(chosenAdapter->name, 49) + 1;
			xlive_network_adapter.name = (char*)malloc(adapter_name_buflen);
			memcpy_s(xlive_network_adapter.name, adapter_name_buflen, chosenAdapter->name, adapter_name_buflen);
			xlive_network_adapter.name[adapter_name_buflen - 1] = 0;

			result = ERROR_SUCCESS;
		}
		else {
			result = ERROR_NETWORK_UNREACHABLE;
		}

		for (EligibleAdapter* ea : eligible_adapters) {
			delete ea;
		}
		eligible_adapters.clear();
	}
	else {
		char errorMsg[400];
		snprintf(errorMsg, 400, "Call to GetAdaptersAddresses failed with error: %d", dwRetVal);
		addDebugText(errorMsg);
		if (dwRetVal == ERROR_NO_DATA)
			addDebugText("No addresses were found for the requested parameters");
		else {
			LPSTR lpMsgBuf = NULL;
			if (FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL, dwRetVal, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				// Default language
				(LPSTR)&lpMsgBuf, 0, NULL)) {
				snprintf(errorMsg, 400, "Error: %s", lpMsgBuf);
				addDebugText(errorMsg);
				LocalFree(lpMsgBuf);
			}
		}
		result = ERROR_UNIDENTIFIED_ERROR;
	}

	if (pAddresses) {
		HeapFree(GetProcessHeap(), 0, pAddresses);
	}

	if (result != ERROR_SUCCESS) {
		xlive_network_adapter.name = NULL;
		xlive_network_adapter.unicastHAddr = INADDR_LOOPBACK;
		xlive_network_adapter.unicastHMask = 0;
		xlive_network_adapter.hBroadcast = INADDR_BROADCAST;
		xlive_network_adapter.hasDnsServer = FALSE;
		xlive_network_adapter.minLinkSpeed = 0;
	}

	return result;
}

void CreateLocalUser()
{
	INT error_network_adapter = GetNetworkAdapter();

	XNADDR *pAddr = &xlive_local_users[0].pxna;

	unsigned long resolvedAddr;
	if ((resolvedAddr = xlive_network_adapter.unicastHAddr) == INADDR_NONE) {//inet_addr("192.168.0.6")
		return;
	}

	//DWORD user_id = 0x6061B52F;
	DWORD user_id = rand();
	DWORD mac_fix = 0x00131000;

	pAddr->ina.s_addr = htonl(resolvedAddr);
	pAddr->wPortOnline = htons(xlive_base_port);
	pAddr->inaOnline.s_addr = user_id << 8;

	memset(&(pAddr->abEnet), 0, 6);
	memset(&(pAddr->abOnline), 0, 6);

	memcpy(&(pAddr->abEnet), &user_id, 4);
	memcpy(&(pAddr->abOnline), &user_id, 4);

	memcpy((BYTE*)&(pAddr->abEnet) + 3, (BYTE*)&mac_fix + 1, 3);
	memcpy((BYTE*)&(pAddr->abOnline) + 17, (BYTE*)&mac_fix + 1, 3);

	xlive_local_users[0].pina.s_addr = pAddr->inaOnline.s_addr;

	xlive_local_users[0].bValid = TRUE;

	//CreateUser(pAddr, TRUE);
}

void Check_Overlapped(PXOVERLAPPED pOverlapped)
{
	if (!pOverlapped)
		return;

	if (pOverlapped->hEvent) {
		SetEvent(pOverlapped->hEvent);
	}

	if (pOverlapped->pCompletionRoutine) {
		pOverlapped->pCompletionRoutine(pOverlapped->InternalLow, pOverlapped->InternalHigh, pOverlapped->dwCompletionContext);
	}
}


BOOL XLivepIsPropertyIdValid(DWORD dwPropertyId, BOOL a2)
{
	return !(dwPropertyId & X_PROPERTY_SCOPE_MASK)
		|| dwPropertyId == X_PROPERTY_RANK
		|| dwPropertyId == X_PROPERTY_SESSION_ID
		|| dwPropertyId == X_PROPERTY_GAMER_ZONE
		|| dwPropertyId == X_PROPERTY_GAMER_COUNTRY
		|| dwPropertyId == X_PROPERTY_GAMER_LANGUAGE
		|| dwPropertyId == X_PROPERTY_GAMER_RATING
		|| dwPropertyId == X_PROPERTY_GAMER_MU
		|| dwPropertyId == X_PROPERTY_GAMER_SIGMA
		|| dwPropertyId == X_PROPERTY_GAMER_PUID
		|| dwPropertyId == X_PROPERTY_AFFILIATE_SCORE
		|| dwPropertyId == X_PROPERTY_RELATIVE_SCORE
		|| dwPropertyId == X_PROPERTY_SESSION_TEAM
		|| !a2 && dwPropertyId == X_PROPERTY_GAMER_HOSTNAME;
}

// #472
VOID WINAPI XCustomSetAction(DWORD dwActionIndex, LPCWSTR lpwszActionText, DWORD dwFlags)
{
	TRACE_FX();
}

// #473
BOOL WINAPI XCustomGetLastActionPress(DWORD *pdwUserIndex, DWORD *pdwActionIndex, XUID *pXuid)
{
	TRACE_FX();
	return FALSE;
}

BOOL XNotifyGetNextHelper(ULONGLONG notificationArea, PDWORD pdwId, PULONG_PTR pParam)
{
	if (notificationArea & XNOTIFY_SYSTEM) {

		*pParam = 0x00000000;

		for (int i = 0; i < XLIVE_LOCAL_USER_COUNT; i++) {
			if (xlive_users_info_changed[i]) {
				xlive_users_info_changed[i] = FALSE;
				*pParam |= 1 << i;
			}
		}

		if (*pParam) {
			*pdwId = XN_SYS_SIGNINCHANGED;
			return TRUE;
		}
	}
	if (false && notificationArea & XNOTIFY_SYSTEM) {

		//*pParam = XONLINE_S_LOGON_CONNECTION_ESTABLISHED;
		*pParam = XONLINE_S_LOGON_DISCONNECTED;

		if (*pParam) {
			*pdwId = XN_LIVE_CONNECTIONCHANGED;
			return TRUE;
		}
	}
	if (notificationArea & XNOTIFY_LIVE) {
		if (xlive_invite_to_game) {
			*pdwId = XN_LIVE_INVITE_ACCEPTED;
			*pParam = 0x00000000;
			return TRUE;
		}
	}
	return FALSE;
}

// #651
BOOL WINAPI XNotifyGetNext(HANDLE hNotification, DWORD dwMsgFilter, PDWORD pdwId, PULONG_PTR pParam)
{
	TRACE_FX();
	EnterCriticalSection(&d_lock);
	ResetEvent(hNotification);

	BOOL result = FALSE;

	int noteId = 0;
	for (; noteId < g_dwListener; noteId++) {
		if (g_listener[noteId].id == hNotification)
			break;
	}
	if (noteId == g_dwListener) {
		//noteId = -1;
		__debugbreak();
	}
	else {
		result = XNotifyGetNextHelper(dwMsgFilter ? dwMsgFilter : g_listener[noteId].area, pdwId, pParam);
	}

	if (result)
		SetEvent(hNotification);
	LeaveCriticalSection(&d_lock);
	return result;
}

#define XNOTIFYUI_POS_TOPLEFT ?
#define XNOTIFYUI_POS_TOPCENTER ?
#define XNOTIFYUI_POS_TOPRIGHT ?
#define XNOTIFYUI_POS_CENTERLEFT ?
#define XNOTIFYUI_POS_CENTER ?
#define XNOTIFYUI_POS_CENTERRIGHT ?
#define XNOTIFYUI_POS_BOTTOMLEFT ?
#define XNOTIFYUI_POS_BOTTOMCENTER ?
#define XNOTIFYUI_POS_BOTTOMRIGHT ?

// #652
VOID WINAPI XNotifyPositionUI(DWORD dwPosition)
{
	TRACE_FX();
	// Invalid dwPos--check XNOTIFYUI_POS_* bits.  Do not specify both TOP and BOTTOM or both LEFT and RIGHT.
	if (dwPosition & 0xFFFFFFF0 || dwPosition & 1 && dwPosition & 2 || dwPosition & 8 && dwPosition & 4)
		return;
	// TODO XNotifyPositionUI
}

// #653
DWORD WINAPI XNotifyDelayUI(ULONG ulMilliSeconds)
{
	TRACE_FX();
	return ERROR_SUCCESS;
}

// #1082
DWORD WINAPI XGetOverlappedExtendedError(PXOVERLAPPED pOverlapped)
{
	TRACE_FX();
	if (!pOverlapped) {
		return ERROR_INVALID_PARAMETER;
	}

	return pOverlapped->dwExtendedError;
}

// #1083
DWORD WINAPI XGetOverlappedResult(PXOVERLAPPED pOverlapped, LPDWORD pdwResult, BOOL bWait)
{
	TRACE_FX();
	if (bWait) {
		while (pOverlapped->InternalLow == ERROR_IO_INCOMPLETE) {
			Sleep(50L);
		}
	}

	if (pdwResult) {
		*pdwResult = pOverlapped->InternalHigh;
	}

	return pOverlapped->InternalLow;
}

// #5000
HRESULT WINAPI XLiveInitialize(XLIVE_INITIALIZE_INFO *pPii)
{
	TRACE_FX();

	while (xlive_debug_pause && !IsDebuggerPresent()) {
		Sleep(500L);
	}

	srand((unsigned int)time(NULL));

	if (pPii->pszAdapterName && pPii->pszAdapterName[0]) {
		unsigned int adapter_name_buflen = strnlen_s(pPii->pszAdapterName, 49) + 1;
		xlive_preferred_network_adapter_name = (char*)malloc(adapter_name_buflen);
		memcpy_s(xlive_preferred_network_adapter_name, adapter_name_buflen, pPii->pszAdapterName, adapter_name_buflen);
		xlive_preferred_network_adapter_name[adapter_name_buflen-1] = 0;
	}

	memset(&xlive_network_adapter, 0x00, sizeof(EligibleAdapter));

	for (int i = 0; i < XLIVE_LOCAL_USER_COUNT; i++) {
		xlive_users_info[i] = (XUSER_SIGNIN_INFO*)malloc(sizeof(XUSER_SIGNIN_INFO));
		memset(xlive_users_info[i], 0, sizeof(XUSER_SIGNIN_INFO));
		xlive_users_info_changed[i] = FALSE;
	}

	InitializeCriticalSection(&d_lock);
	InitializeCriticalSection(&xlive_xlocator_enumerators_lock);

	wchar_t mutex_name[40];
	DWORD mutex_last_error;
	HANDLE mutex = NULL;
	do {
		if (mutex) {
			mutex_last_error = CloseHandle(mutex);
		}
		xlive_base_port += 1000;
		if (xlive_base_port > 65000) {
			xlive_netsocket_abort = TRUE;
			xlive_base_port = 1000;
			break;
		}
		swprintf(mutex_name, 40, L"Global\\XLLNBasePort#%hd", xlive_base_port);
		mutex = CreateMutexW(0, TRUE, mutex_name);
		mutex_last_error = GetLastError();
	} while (mutex_last_error != ERROR_SUCCESS);

	char debugText[50];
	snprintf(debugText, 50, "XLive Base Port %hd.", xlive_base_port);
	addDebugText(debugText);

	CreateLocalUser();

	//TODO If the title's graphics system has not yet been initialized, D3D will be passed in XLiveOnCreateDevice(...).
	INT error_XRender = InitXRender(pPii);
	INT error_XSession = InitXSession();

	return S_OK;
}

// #5001
HRESULT WINAPI XLiveInput(XLIVE_INPUT_INFO *pPii)
{
	TRACE_FX();
	if (!pPii)
		return E_POINTER;
	pPii->fHandled = FALSE;
	return S_OK;
}

// #5003
VOID WINAPI XLiveUninitialize()
{
	TRACE_FX();
	INT error_XRender = UninitXRender();
	DeleteCriticalSection(&xlive_xlocator_enumerators_lock);
	DeleteCriticalSection(&d_lock);
}

// #5005
HRESULT WINAPI XLiveOnCreateDevice(IUnknown *pD3D, VOID *pD3DPP)
{
	TRACE_FX();
	if (!pD3D)
		return E_POINTER;
	if (!pD3DPP)
		return E_POINTER;
	//TODO XLiveOnCreateDevice
	return S_OK;
}

// #5006
HRESULT WINAPI XLiveOnDestroyDevice()
{
	TRACE_FX();
	return S_OK;
}

// #5007
HRESULT WINAPI XLiveOnResetDevice(void *pD3DPP)
{
	TRACE_FX();
	D3DPRESENT_PARAMETERS *pD3DPP2 = (D3DPRESENT_PARAMETERS*)pD3DPP;
	//ID3D10Device *pD3DPP2 = (ID3D10Device*)pD3DPP;
	return S_OK;
}

// #5010: This function is deprecated.
HRESULT WINAPI XLiveRegisterDataSection(int a1, int a2, int a3)
{
	TRACE_FX();
	return ERROR_SUCCESS;
	//if (XLivepGetTitleXLiveVersion() < 0x20000000)
	return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
}

// #5011: This function is deprecated.
HRESULT WINAPI XLiveUnregisterDataSection(int a1)
{
	TRACE_FX();
	return ERROR_SUCCESS;
	//if (XLivepGetTitleXLiveVersion() < 0x20000000)
	return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
}

// #5016
HRESULT WINAPI XLivePBufferAllocate(DWORD dwSize, XLIVE_PROTECTED_BUFFER **pxebBuffer)
{
	TRACE_FX();
	if (!dwSize)
		return E_INVALIDARG;
	if (!pxebBuffer)
		return E_POINTER;
	if (dwSize + 4 < dwSize)
		//Overflow experienced.
		return E_UNEXPECTED;

	HANDLE hHeap = GetProcessHeap();
	*pxebBuffer = (XLIVE_PROTECTED_BUFFER*)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, dwSize + 4);
	if (!*pxebBuffer)
		return E_OUTOFMEMORY;

	(*pxebBuffer)->dwSize = dwSize;

	return S_OK;
}

// #5018
HRESULT WINAPI XLivePBufferGetByte(XLIVE_PROTECTED_BUFFER *xebBuffer, DWORD dwOffset, BYTE *pucValue)
{
	TRACE_FX();
	if (!xebBuffer)
		return E_POINTER;
	if (!pucValue)
		return E_POINTER;
	if (dwOffset + 4 < dwOffset)
		//Overflow experienced.
		return E_UNEXPECTED;
	if (dwOffset >= xebBuffer->dwSize)
		return E_UNEXPECTED;

	*pucValue = ((BYTE*)&xebBuffer->bData)[dwOffset];
	return S_OK;
}

// #5019
HRESULT WINAPI XLivePBufferSetByte(XLIVE_PROTECTED_BUFFER *xebBuffer, DWORD dwOffset, BYTE ucValue)
{
	TRACE_FX();
	if (!xebBuffer)
		return E_POINTER;
	if (dwOffset + 4 < dwOffset)
		//Overflow experienced.
		return E_UNEXPECTED;
	if (dwOffset >= xebBuffer->dwSize)
		return E_UNEXPECTED;

	((BYTE*)&xebBuffer->bData)[dwOffset] = ucValue;

	return S_OK;
}

// #5022
HRESULT WINAPI XLiveGetUpdateInformation(PXLIVEUPDATE_INFORMATION pXLiveUpdateInfo)
{
	TRACE_FX();
	if (!pXLiveUpdateInfo)
		return E_POINTER;
	if (pXLiveUpdateInfo->cbSize != sizeof(XLIVEUPDATE_INFORMATION))
		return HRESULT_FROM_WIN32(ERROR_INVALID_USER_BUFFER);//0x800706F8;
	// No update?
	return S_FALSE;
}

// #5024
HRESULT WINAPI XLiveUpdateSystem(LPCWSTR lpwszRelaunchCmdLine)
{
	TRACE_FX();
	return S_OK;
	// No update?
	return S_FALSE;
}

// #5026
HRESULT WINAPI XLiveSetSponsorToken(LPCWSTR lpwszToken, DWORD dwTitleId)
{
	TRACE_FX();
	if (!lpwszToken || wcsnlen_s(lpwszToken, 30) != 29)
		return E_INVALIDARG;
	return S_OK;
}

// #5028
DWORD WINAPI XLiveLoadLibraryEx(LPCWSTR lpwszModuleFileName, HINSTANCE *phModule, DWORD dwFlags)
{
	TRACE_FX();
	if (!lpwszModuleFileName)
		return E_POINTER;
	if (!*lpwszModuleFileName)
		return E_INVALIDARG;
	if (!phModule)
		return E_INVALIDARG;

	HINSTANCE hInstance = LoadLibraryExW(lpwszModuleFileName, NULL, dwFlags);
	if (!hInstance)
		return E_INVALIDARG;

	*phModule = hInstance;
	return S_OK;
}

// #5029
HRESULT WINAPI XLiveFreeLibrary(HMODULE hModule)
{
	TRACE_FX();
	if (!hModule)
		return E_INVALIDARG;
	if (!FreeLibrary(hModule)) {
		signed int last_error = GetLastError();
		if (last_error > 0)
			last_error = (unsigned __int16)last_error | 0x80070000;
		return last_error;
	}
	return S_OK;
}

// #5030
BOOL WINAPI XLivePreTranslateMessage(const LPMSG lpMsg)
{
	TRACE_FX();
	if (!lpMsg)
		return FALSE;
	return FALSE;
	//return TRUE;
}

// #5031
HRESULT WINAPI XLiveSetDebugLevel(XLIVE_DEBUG_LEVEL xdlLevel, XLIVE_DEBUG_LEVEL *pxdlOldLevel)
{
	TRACE_FX();
	if (xdlLevel != XLIVE_DEBUG_LEVEL_OFF && xdlLevel != XLIVE_DEBUG_LEVEL_ERROR && xdlLevel != XLIVE_DEBUG_LEVEL_WARNING && xdlLevel != XLIVE_DEBUG_LEVEL_INFO && xdlLevel != XLIVE_DEBUG_LEVEL_DEFAULT)
		return E_INVALIDARG;
	if (pxdlOldLevel) {
		*pxdlOldLevel = xlive_xdlLevel;
	}
	xlive_xdlLevel = xdlLevel;
	return S_OK;
}

// #5251
BOOL WINAPI XCloseHandle(HANDLE hObject)
{
	TRACE_FX();
	if (!hObject || (DWORD)hObject == -1) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}
	EnterCriticalSection(&xlive_xlocator_enumerators_lock);
	if (xlive_xlocator_enumerators.count(hObject)) {
		xlive_xlocator_enumerators.erase(hObject);
	}
	LeaveCriticalSection(&xlive_xlocator_enumerators_lock);
	if (!CloseHandle(hObject)) {
		SetLastError(ERROR_INVALID_HANDLE);
		return FALSE;
	}
	return TRUE;
}

// #5254
DWORD WINAPI XCancelOverlapped(PXOVERLAPPED pOverlapped)
{
	TRACE_FX();
	if (!pOverlapped)
		return ERROR_INVALID_PARAMETER;
	//TODO XCancelOverlapped
	return ERROR_SUCCESS;
}

// #5256
DWORD WINAPI XEnumerate(HANDLE hEnum, PVOID pvBuffer, DWORD cbBuffer, PDWORD pcItemsReturned, PXOVERLAPPED pXOverlapped)
{
	TRACE_FX();
	if (!hEnum)
		return ERROR_INVALID_PARAMETER;
	if (!pvBuffer)
		return ERROR_INVALID_PARAMETER;
	if (!cbBuffer)
		return ERROR_INVALID_PARAMETER;
	if (pcItemsReturned && pXOverlapped)
		return ERROR_INVALID_PARAMETER;
	if (!pcItemsReturned && !pXOverlapped)
		return ERROR_INVALID_PARAMETER;

	EnterCriticalSection(&xlive_xlocator_enumerators_lock);
	if (xlive_xlocator_enumerators.count(hEnum)) {
		xlive_xlocator_enumerators[hEnum];

		DWORD max_result_len = cbBuffer / sizeof(XLOCATOR_SEARCHRESULT);
		DWORD total_server_count = 0;

		EnterCriticalSection(&liveoverlan_sessions_lock);
		for (auto const &session : liveoverlan_sessions) {
			if (total_server_count >= max_result_len)
				break;
			if (std::find(xlive_xlocator_enumerators[hEnum].begin(), xlive_xlocator_enumerators[hEnum].end(), session.first) != xlive_xlocator_enumerators[hEnum].end())
				continue;
			XLOCATOR_SEARCHRESULT* server = &((XLOCATOR_SEARCHRESULT*)pvBuffer)[total_server_count++];
			xlive_xlocator_enumerators[hEnum].push_back(session.first);
			LiveOverLanClone(&server, session.second->searchresult);
		}
		LeaveCriticalSection(&liveoverlan_sessions_lock);
		LeaveCriticalSection(&xlive_xlocator_enumerators_lock);

		if (pXOverlapped) {
			//pXOverlapped->InternalHigh = ERROR_IO_INCOMPLETE;
			//pXOverlapped->InternalLow = ERROR_IO_INCOMPLETE;
			//pXOverlapped->dwExtendedError = ERROR_SUCCESS;

			if (total_server_count) {
				pXOverlapped->InternalHigh = total_server_count;
				pXOverlapped->InternalLow = ERROR_SUCCESS;
			}
			else {
				pXOverlapped->InternalHigh = ERROR_SUCCESS;
				pXOverlapped->InternalLow = ERROR_NO_MORE_FILES;
				XCloseHandle(hEnum);
			}
			Check_Overlapped(pXOverlapped);

			return ERROR_IO_PENDING;
		}
		else {
			*pcItemsReturned = total_server_count;
			return ERROR_SUCCESS;
		}
	}
	LeaveCriticalSection(&xlive_xlocator_enumerators_lock);

	//TODO XEnumerate
	if (pXOverlapped) {
		//asynchronous

		pXOverlapped->InternalLow = ERROR_SUCCESS;
		pXOverlapped->InternalHigh = ERROR_SUCCESS;
		pXOverlapped->dwExtendedError = ERROR_SUCCESS;

		Check_Overlapped(pXOverlapped);

		return ERROR_IO_PENDING;
	}
	else {
		//synchronous
		//return result;
	}
	return ERROR_SUCCESS;
}

// #5257
HRESULT WINAPI XLiveManageCredentials(LPCWSTR lpwszLiveIdName, LPCWSTR lpszLiveIdPassword, DWORD dwCredFlags, PXOVERLAPPED pXOverlapped)
{
	TRACE_FX();
	if (dwCredFlags & XLMGRCREDS_FLAG_SAVE && dwCredFlags & XLMGRCREDS_FLAG_DELETE || !(dwCredFlags & XLMGRCREDS_FLAG_SAVE) && !(dwCredFlags & XLMGRCREDS_FLAG_DELETE))
		return E_INVALIDARG;
	if (!lpwszLiveIdName || !*lpwszLiveIdName)
		return E_INVALIDARG;
	if (dwCredFlags & XLMGRCREDS_FLAG_SAVE && (!lpszLiveIdPassword || !*lpszLiveIdPassword))
		return E_INVALIDARG;

	//TODO XLiveManageCredentials
	if (pXOverlapped) {
		//asynchronous

		pXOverlapped->InternalLow = ERROR_SUCCESS;
		pXOverlapped->InternalHigh = ERROR_SUCCESS;
		pXOverlapped->dwExtendedError = ERROR_SUCCESS;

		Check_Overlapped(pXOverlapped);

		return ERROR_IO_PENDING;
	}
	else {
		//synchronous
		//return result;
	}
	return S_OK;
}

// #5258
HRESULT WINAPI XLiveSignout(PXOVERLAPPED  pXOverlapped)
{
	TRACE_FX();

	XLLNLogout(0);

	if (pXOverlapped) {
		//asynchronous

		pXOverlapped->InternalLow = ERROR_SUCCESS;
		pXOverlapped->InternalHigh = ERROR_SUCCESS;
		pXOverlapped->dwExtendedError = ERROR_SUCCESS;

		Check_Overlapped(pXOverlapped);

		return ERROR_IO_PENDING;
	}
	else {
		//synchronous
		//return result;
	}
	return S_OK;
}

// #5259
HRESULT WINAPI XLiveSignin(PWSTR pszLiveIdName, PWSTR pszLiveIdPassword, DWORD dwFlags, PXOVERLAPPED pXOverlapped)
{
	TRACE_FX();
	if (!pszLiveIdName || !*pszLiveIdName)
		return E_INVALIDARG;
	//FIXME password isn't being passed in.
	//if (!pszLiveIdPassword || !*pszLiveIdPassword)
	//	return E_INVALIDARG;

	if (dwFlags & XLSIGNIN_FLAG_SAVECREDS) {

	}
	//XLSIGNIN_FLAG_ALLOWTITLEUPDATES XLSIGNIN_FLAG_ALLOWSYSTEMUPDATES

	XLLNLogin(0, TRUE, 0, 0);

	if (pXOverlapped) {
		//asynchronous

		pXOverlapped->InternalLow = ERROR_SUCCESS;
		pXOverlapped->InternalHigh = ERROR_SUCCESS;
		pXOverlapped->dwExtendedError = ERROR_SUCCESS;

		Check_Overlapped(pXOverlapped);

		return ERROR_IO_PENDING;
	}
	else {
		//synchronous
		//return result;
	}
	return S_OK;
}

// #5261
DWORD WINAPI XUserGetXUID(DWORD dwUserIndex, XUID *pxuid)
{
	TRACE_FX();
	if (!pxuid)
		return ERROR_INVALID_PARAMETER;
	if (dwUserIndex >= XLIVE_LOCAL_USER_COUNT)
		return ERROR_NO_SUCH_USER;
	if (xlive_users_info[dwUserIndex]->UserSigninState == eXUserSigninState_NotSignedIn)
		return ERROR_NOT_LOGGED_ON;

	if (xlive_users_info[dwUserIndex]->UserSigninState & (eXUserSigninState_SignedInLocally | eXUserSigninState_SignedInToLive)) {
		*pxuid = xlive_users_info[dwUserIndex]->xuid;
		return ERROR_SUCCESS;
	}
	return ERROR_NOT_LOGGED_ON;
}

// #5262
XUSER_SIGNIN_STATE WINAPI XUserGetSigninState(DWORD dwUserIndex)
{
	TRACE_FX();
	if (dwUserIndex >= XLIVE_LOCAL_USER_COUNT)
		return eXUserSigninState_NotSignedIn;
	return xlive_users_info[dwUserIndex]->UserSigninState;
}

// #5263
DWORD WINAPI XUserGetName(DWORD dwUserIndex, LPSTR szUserName, DWORD cchUserName)
{
	TRACE_FX();
	if (dwUserIndex >= XLIVE_LOCAL_USER_COUNT)
		return ERROR_NO_SUCH_USER;
	if (xlive_users_info[dwUserIndex]->UserSigninState == eXUserSigninState_NotSignedIn)
		return ERROR_NOT_LOGGED_ON;
	if (!szUserName)
		return ERROR_INVALID_PARAMETER;
	if (!cchUserName)
		return ERROR_INVALID_PARAMETER;

	if (cchUserName > XUSER_NAME_SIZE)
		cchUserName = XUSER_NAME_SIZE;

	memcpy(szUserName, xlive_users_info[dwUserIndex]->szUserName, cchUserName);
	return ERROR_SUCCESS;
}

// #5264
VOID XUserAreUsersFriends()
{
	TRACE_FX();
	__debugbreak();
}

// #5265
DWORD WINAPI XUserCheckPrivilege(DWORD dwUserIndex, XPRIVILEGE_TYPE PrivilegeType, PBOOL pfResult)
{
	TRACE_FX();
	if (!pfResult)
		return ERROR_INVALID_PARAMETER;
	*pfResult = FALSE;
	if (PrivilegeType != XPRIVILEGE_MULTIPLAYER_SESSIONS &&
		PrivilegeType != XPRIVILEGE_COMMUNICATIONS &&
		PrivilegeType != XPRIVILEGE_COMMUNICATIONS_FRIENDS_ONLY &&
		PrivilegeType != XPRIVILEGE_PROFILE_VIEWING &&
		PrivilegeType != XPRIVILEGE_PROFILE_VIEWING_FRIENDS_ONLY &&
		PrivilegeType != XPRIVILEGE_USER_CREATED_CONTENT &&
		PrivilegeType != XPRIVILEGE_USER_CREATED_CONTENT_FRIENDS_ONLY &&
		PrivilegeType != XPRIVILEGE_PURCHASE_CONTENT &&
		PrivilegeType != XPRIVILEGE_PRESENCE &&
		PrivilegeType != XPRIVILEGE_PRESENCE_FRIENDS_ONLY &&
		PrivilegeType != XPRIVILEGE_TRADE_CONTENT &&
		PrivilegeType != XPRIVILEGE_VIDEO_COMMUNICATIONS &&
		PrivilegeType != XPRIVILEGE_VIDEO_COMMUNICATIONS_FRIENDS_ONLY &&
		PrivilegeType != XPRIVILEGE_MULTIPLAYER_DEDICATED_SERVER)
		return ERROR_INVALID_PARAMETER;
	if (dwUserIndex >= XLIVE_LOCAL_USER_COUNT)
		return ERROR_NO_SUCH_USER;
	if (xlive_users_info[dwUserIndex]->UserSigninState == eXUserSigninState_NotSignedIn)
		return ERROR_NOT_LOGGED_ON;

	if (TRUE)//TODO XUserCheckPrivilege
		*pfResult = TRUE;

	return ERROR_SUCCESS;
}

// #5270: Requires XNotifyGetNext to process the listener.
HANDLE WINAPI XNotifyCreateListener(ULONGLONG qwAreas)
{
	TRACE_FX();
	if (HIDWORD(qwAreas) | qwAreas & 0xFFFFFF10)
		return NULL;

	HANDLE g_dwFakeListener = CreateMutex(NULL, NULL, NULL);

	g_listener[g_dwListener].id = g_dwFakeListener;
	g_listener[g_dwListener].area = qwAreas;
	g_dwListener++;

	SetEvent(g_dwFakeListener);
	return g_dwFakeListener;
}

// #5276
VOID WINAPI XUserSetProperty(DWORD dwUserIndex, DWORD dwPropertyId, DWORD cbValue, CONST VOID *pvValue)
{
	TRACE_FX();
	if (dwUserIndex >= XLIVE_LOCAL_USER_COUNT)
		return;// ERROR_NO_SUCH_USER;
	if (xlive_users_info[dwUserIndex]->UserSigninState == eXUserSigninState_NotSignedIn)
		return;// ERROR_NOT_LOGGED_ON;
	if (!cbValue)
		return;// ERROR_INVALID_PARAMETER;
	if (!pvValue)
		return;// ERROR_INVALID_PARAMETER;
	if (!XLivepIsPropertyIdValid(dwPropertyId, TRUE))
		return;

	//TODO XUserSetProperty
}

// #5277
VOID WINAPI XUserSetContext(DWORD dwUserIndex, DWORD dwContextId, DWORD dwContextValue)
{
	TRACE_FX();
	if (dwUserIndex >= XLIVE_LOCAL_USER_COUNT)
		return;// ERROR_NO_SUCH_USER;
	if (xlive_users_info[dwUserIndex]->UserSigninState == eXUserSigninState_NotSignedIn)
		return;// ERROR_NOT_LOGGED_ON;

	if (dwContextId == X_CONTEXT_PRESENCE) {

	}
	else if (dwContextId == X_CONTEXT_GAME_TYPE) {
		xlive_session_details.dwGameType = dwContextValue;
	}
	else if (dwContextId == X_CONTEXT_GAME_MODE) {
		xlive_session_details.dwGameMode = dwContextValue;
	}
	else if (dwContextId == X_CONTEXT_SESSION_JOINABLE) {

	}
}

// #5278
DWORD WINAPI XUserWriteAchievements(DWORD dwNumAchievements, CONST XUSER_ACHIEVEMENT *pAchievements, PXOVERLAPPED pXOverlapped)
{
	TRACE_FX();
	if (!dwNumAchievements)
		return ERROR_INVALID_PARAMETER;
	if (!pAchievements)
		return ERROR_INVALID_PARAMETER;

	if (pAchievements->dwUserIndex >= XLIVE_LOCAL_USER_COUNT)
		return ERROR_NO_SUCH_USER;

	//TODO XUserWriteAchievements
	if (pXOverlapped) {
		//asynchronous

		pXOverlapped->InternalLow = ERROR_SUCCESS;
		pXOverlapped->InternalHigh = ERROR_SUCCESS;
		pXOverlapped->dwExtendedError = ERROR_SUCCESS;

		Check_Overlapped(pXOverlapped);

		return ERROR_IO_PENDING;
	}
	else {
		//synchronous
		//return result;
	}
	return ERROR_SUCCESS;
}

// #5280
DWORD WINAPI XUserCreateAchievementEnumerator(DWORD dwTitleId, DWORD dwUserIndex, XUID xuid, DWORD dwDetailFlags, DWORD dwStartingIndex, DWORD cItem, PDWORD pcbBuffer, PHANDLE phEnum)
{
	TRACE_FX();
	if (dwUserIndex >= XLIVE_LOCAL_USER_COUNT)
		return ERROR_NO_SUCH_USER;
	if (xlive_users_info[dwUserIndex]->UserSigninState == eXUserSigninState_NotSignedIn)
		return ERROR_NOT_LOGGED_ON;
	if (cItem > XACHIEVEMENT_MAX_COUNT)
		return ERROR_INVALID_PARAMETER;
	if (dwStartingIndex >= XACHIEVEMENT_MAX_COUNT)
		return ERROR_INVALID_PARAMETER;
	if (dwStartingIndex + cItem < dwStartingIndex || dwStartingIndex + cItem >= XACHIEVEMENT_MAX_COUNT)
		return ERROR_INVALID_PARAMETER;
	if (!dwDetailFlags || (dwDetailFlags != XACHIEVEMENT_DETAILS_ALL && dwDetailFlags & ~(XACHIEVEMENT_DETAILS_LABEL | XACHIEVEMENT_DETAILS_DESCRIPTION | XACHIEVEMENT_DETAILS_UNACHIEVED | XACHIEVEMENT_DETAILS_TFC)))
		return ERROR_INVALID_PARAMETER;
	if (!pcbBuffer)
		return ERROR_INVALID_PARAMETER;
	if (!phEnum)
		return ERROR_INVALID_PARAMETER;

	if (xuid == INVALID_XUID) {
		//enumerate the local signed-in gamer's achievements.
	}

	for (DWORD i = dwStartingIndex; i < dwStartingIndex + cItem; i++) {
		//?
	}
	//TODO XUserCreateAchievementEnumerator

	*pcbBuffer = cItem * sizeof(XACHIEVEMENT_DETAILS);
	*phEnum = CreateMutex(NULL, NULL, NULL);

	return ERROR_SUCCESS;
}

// #5303
DWORD WINAPI XStringVerify(DWORD dwFlags, const CHAR *szLocale, DWORD dwNumStrings, const STRING_DATA *pStringData, DWORD cbResults, STRING_VERIFY_RESPONSE *pResults, XOVERLAPPED *pXOverlapped)
{
	TRACE_FX();
	if (dwFlags)
		// Not implemented.
		return ERROR_INVALID_PARAMETER;
	if (!szLocale)
		return ERROR_INVALID_PARAMETER;
	if (strlen(szLocale) >= XSTRING_MAX_LENGTH)
		return ERROR_INVALID_PARAMETER;
	if (dwNumStrings > XSTRING_MAX_STRINGS)
		return ERROR_INVALID_PARAMETER;
	if (!pStringData)
		return ERROR_INVALID_PARAMETER;
	if (!pResults)
		return ERROR_INVALID_PARAMETER;

	pResults->wNumStrings = (WORD)dwNumStrings;
	pResults->pStringResult = (HRESULT*)((BYTE*)pResults + sizeof(STRING_VERIFY_RESPONSE));

	for (int lcv = 0; lcv < (WORD)dwNumStrings; lcv++)
		pResults->pStringResult[lcv] = S_OK;
	
	if (pXOverlapped) {
		//asynchronous

		pXOverlapped->InternalLow = ERROR_SUCCESS;
		pXOverlapped->InternalHigh = ERROR_SUCCESS;
		pXOverlapped->dwExtendedError = ERROR_SUCCESS;

		Check_Overlapped(pXOverlapped);

		return ERROR_IO_PENDING;
	}
	else {
		//synchronous
		//return result;
	}
	return ERROR_SUCCESS;
}

// #5305
DWORD WINAPI XStorageUploadFromMemory(DWORD dwUserIndex, const WCHAR *wszServerPath, DWORD dwBufferSize, const BYTE *pbBuffer, XOVERLAPPED *pXOverlapped)
{
	TRACE_FX();
	if (dwUserIndex >= XLIVE_LOCAL_USER_COUNT)
		return ERROR_NO_SUCH_USER;
	if (xlive_users_info[dwUserIndex]->UserSigninState == eXUserSigninState_NotSignedIn)
		return ERROR_NOT_LOGGED_ON;
	if (!wszServerPath)
		return ERROR_INVALID_PARAMETER;
	if (!dwBufferSize)
		return ERROR_INVALID_PARAMETER;
	if (!pbBuffer)
		return ERROR_INVALID_PARAMETER;

	//TODO XStorageUploadFromMemory
	if (pXOverlapped) {
		//asynchronous

		pXOverlapped->InternalLow = ERROR_SUCCESS;
		pXOverlapped->InternalHigh = ERROR_SUCCESS;
		pXOverlapped->dwExtendedError = ERROR_SUCCESS;

		Check_Overlapped(pXOverlapped);

		return ERROR_IO_PENDING;
	}
	else {
		//synchronous
		//return result;
	}
	return ERROR_SUCCESS;
}

// #5308
DWORD WINAPI XStorageDelete(DWORD dwUserIndex, const WCHAR *wszServerPath, XOVERLAPPED *pXOverlapped)
{
	TRACE_FX();
	if (dwUserIndex >= XLIVE_LOCAL_USER_COUNT)
		return ERROR_NO_SUCH_USER;
	if (xlive_users_info[dwUserIndex]->UserSigninState == eXUserSigninState_NotSignedIn)
		return ERROR_NOT_LOGGED_ON;
	if (!wszServerPath)
		return ERROR_INVALID_PARAMETER;

	//TODO XStorageDelete
	if (pXOverlapped) {
		//asynchronous

		pXOverlapped->InternalLow = ERROR_SUCCESS;
		pXOverlapped->InternalHigh = ERROR_SUCCESS;
		pXOverlapped->dwExtendedError = ERROR_SUCCESS;

		Check_Overlapped(pXOverlapped);

		return ERROR_IO_PENDING;
	}
	else {
		//synchronous
		//return result;
	}
	return ERROR_SUCCESS;
}

// #5310
DWORD WINAPI XOnlineStartup()
{
	TRACE_FX();
	if (!xlive_net_initialized)
		return ERROR_FUNCTION_FAILED;

	WSADATA wsaData;
	DWORD result = XWSAStartup(2, &wsaData);
	xlive_online_initialized = result == 0;
	return result;
}

// #5311
DWORD WINAPI XOnlineCleanup()
{
	TRACE_FX();
	if (xlive_online_initialized) {
		return XWSACleanup();
	}
	return WSANOTINITIALISED;
}

// #5324
VOID XOnlineGetNatType()
{
	TRACE_FX();
	__debugbreak();
}

// #5312
DWORD WINAPI XFriendsCreateEnumerator(DWORD dwUserIndex, DWORD dwStartingIndex, DWORD dwFriendsToReturn, DWORD *pcbBuffer, HANDLE *ph)
{
	TRACE_FX();
	if (dwUserIndex >= XLIVE_LOCAL_USER_COUNT)
		return ERROR_NO_SUCH_USER;
	if (xlive_users_info[dwUserIndex]->UserSigninState == eXUserSigninState_NotSignedIn)
		return ERROR_NOT_LOGGED_ON;
	if (dwFriendsToReturn > XFRIENDS_MAX_C_RESULT)
		return ERROR_INVALID_PARAMETER;
	if (dwStartingIndex >= XFRIENDS_MAX_C_RESULT)
		return ERROR_INVALID_PARAMETER;
	if (dwStartingIndex + dwFriendsToReturn < dwStartingIndex || dwStartingIndex + dwFriendsToReturn >= XFRIENDS_MAX_C_RESULT)
		return ERROR_INVALID_PARAMETER;
	if (!pcbBuffer)
		return ERROR_INVALID_PARAMETER;
	if (!ph)
		return ERROR_INVALID_PARAMETER;

	*pcbBuffer = dwFriendsToReturn * sizeof(XCONTENT_DATA);
	*ph = CreateMutex(NULL, NULL, NULL);

	return ERROR_SUCCESS;
}

// #5315
DWORD WINAPI XInviteGetAcceptedInfo(DWORD dwUserIndex, XINVITE_INFO *pInfo)
{
	TRACE_FX();
	if (dwUserIndex >= XLIVE_LOCAL_USER_COUNT)
		return ERROR_NO_SUCH_USER;
	if (xlive_users_info[dwUserIndex]->UserSigninState == eXUserSigninState_NotSignedIn)
		return ERROR_NOT_LOGGED_ON;
	if (!pInfo)
		return ERROR_INVALID_PARAMETER;

	//TODO XInviteGetAcceptedInfo
	if (xlive_invite_to_game) {
		xlive_invite_to_game = false;
		unsigned long resolvedAddr;
		if ((resolvedAddr = inet_addr("192.168.0.22")) == INADDR_NONE) {
			return ERROR;
		}

		pInfo->hostInfo.hostAddress.ina.s_addr = htonl(resolvedAddr);
		pInfo->hostInfo.hostAddress.wPortOnline = htons(2000);

		XUID host_xuid = 1234561000000032;
		pInfo->hostInfo.hostAddress.inaOnline.s_addr = 8192;

		DWORD user_id = host_xuid % 1000000000;
		DWORD mac_fix = 0x00131000;

		memset(&(pInfo->hostInfo.hostAddress.abEnet), 0, 6);
		memset(&(pInfo->hostInfo.hostAddress.abOnline), 0, 6);

		memcpy(&(pInfo->hostInfo.hostAddress.abEnet), &user_id, 4);
		memcpy(&(pInfo->hostInfo.hostAddress.abOnline), &user_id, 4);

		memcpy((BYTE*)&(pInfo->hostInfo.hostAddress.abEnet) + 3, (BYTE*)&mac_fix + 1, 3);
		memcpy((BYTE*)&(pInfo->hostInfo.hostAddress.abOnline) + 17, (BYTE*)&mac_fix + 1, 3);

		pInfo->fFromGameInvite = TRUE;
		pInfo->dwTitleID = 0x4D53080F;
		XNetCreateKey(&(pInfo->hostInfo.sessionID), &(pInfo->hostInfo.keyExchangeKey));
		pInfo->xuidInvitee = xlive_users_info[dwUserIndex]->xuid;
		pInfo->xuidInviter = host_xuid;

		return ERROR_SUCCESS;
	}
	return ERROR_FUNCTION_FAILED;
}

// #5316
DWORD XInviteSend(DWORD dwUserIndex, DWORD cInvitees, const XUID *pXuidInvitees, const WCHAR *pszText, XOVERLAPPED *pXOverlapped)
{
	TRACE_FX();
	if (dwUserIndex >= XLIVE_LOCAL_USER_COUNT)
		return ERROR_NO_SUCH_USER;
	if (xlive_users_info[dwUserIndex]->UserSigninState == eXUserSigninState_NotSignedIn)
		return ERROR_NOT_LOGGED_ON;
	if (!cInvitees || cInvitees > 0x64)
		return E_INVALIDARG;
	if (!pXuidInvitees)
		return E_POINTER;
	if (pszText && wcsnlen_s(pszText, 0x100+1) > 0x100)
		return E_INVALIDARG;


	//TODO XInviteSend
	if (pXOverlapped) {
		//asynchronous

		pXOverlapped->InternalLow = ERROR_SUCCESS;
		pXOverlapped->InternalHigh = ERROR_SUCCESS;
		pXOverlapped->dwExtendedError = ERROR_SUCCESS;

		Check_Overlapped(pXOverlapped);

		return ERROR_IO_PENDING;
	}
	else {
		//synchronous
		//return result;
	}
	return ERROR_SUCCESS;
}


void writeTemporaryUserSettingsBuffer(DWORD* sizeOfSettingsBytes, DWORD numberOfSettings, const DWORD* settingsToRetrieve) {
	int size = 0;
	for (DWORD setting = 0; setting < numberOfSettings; setting++)
	{
		int settingType, settingSize, settingId;

		settingType = (settingsToRetrieve[setting] >> 28) & 0x0F;
		settingSize = (settingsToRetrieve[setting] >> 16) & 0xFFF;
		settingId = (settingsToRetrieve[setting] >> 0) & 0x3FFF;

		wsprintf(temporaryUserSettingsByteBuffer, "- Settings %d: %X  (Type = %X, Size = %d", setting + 1, settingsToRetrieve[setting], settingType, settingSize);

		switch (settingId)
		{
			case 3: wsprintf(temporaryUserSettingsByteBuffer, "%s, id = XPROFILE_OPTION_CONTROLLER_VIBRATION)", temporaryUserSettingsByteBuffer); break;
			case 1: wsprintf(temporaryUserSettingsByteBuffer, "%s, id = XPROFILE_GAMERCARD_NXE)", temporaryUserSettingsByteBuffer); break;
			case 2: wsprintf(temporaryUserSettingsByteBuffer, "%s, id = XPROFILE_GAMER_YAXIS_INVERSION)", temporaryUserSettingsByteBuffer); break;
			case 24: wsprintf(temporaryUserSettingsByteBuffer, "%s, id = XPROFILE_GAMER_CONTROL_SENSITIVITY)", temporaryUserSettingsByteBuffer); break;

			case 4: wsprintf(temporaryUserSettingsByteBuffer, "%s, id = XPROFILE_GAMERCARD_ZONE)", temporaryUserSettingsByteBuffer); break;
			case 5: wsprintf(temporaryUserSettingsByteBuffer, "%s, id = XPROFILE_GAMERCARD_REGION)", temporaryUserSettingsByteBuffer); break;
			case 6: wsprintf(temporaryUserSettingsByteBuffer, "%s, id = XPROFILE_GAMERCARD_CRED)", temporaryUserSettingsByteBuffer); break;
			case 11: wsprintf(temporaryUserSettingsByteBuffer, "%s, id = XPROFILE_GAMERCARD_REP)", temporaryUserSettingsByteBuffer); break;
			case 71: wsprintf(temporaryUserSettingsByteBuffer, "%s, id = XPROFILE_GAMERCARD_YEARS)", temporaryUserSettingsByteBuffer); break;
			case 72: wsprintf(temporaryUserSettingsByteBuffer, "%s, id = XPROFILE_GAMERCARD_BOUBLES)", temporaryUserSettingsByteBuffer); break;
			case 15: wsprintf(temporaryUserSettingsByteBuffer, "%s, id = XPROFILE_GAMERCARD_PICTURE_KEY)", temporaryUserSettingsByteBuffer); break;
			case 64: wsprintf(temporaryUserSettingsByteBuffer, "%s, id = XPROFILE_GAMERCARD_NAME)", temporaryUserSettingsByteBuffer); break;
			case 65: wsprintf(temporaryUserSettingsByteBuffer, "%s, id = XPROFILE_GAMERCARD_LOCATION)", temporaryUserSettingsByteBuffer); break;
			case 17: wsprintf(temporaryUserSettingsByteBuffer, "%s, id = XPROFILE_GAMERCARD_MOTTO)", temporaryUserSettingsByteBuffer); break;
			case 18: wsprintf(temporaryUserSettingsByteBuffer, "%s, id = XPROFILE_GAMERCARD_TITLES_PLAYED)", temporaryUserSettingsByteBuffer); break;
			case 19: wsprintf(temporaryUserSettingsByteBuffer, "%s, id = XPROFILE_GAMERCARD_ACHIEVEMENTS_EARNED)", temporaryUserSettingsByteBuffer); break;

			case 21: wsprintf(temporaryUserSettingsByteBuffer, "%s, id = XPROFILE_GAMER_DIFFICULTY)", temporaryUserSettingsByteBuffer); break;

			case 29: wsprintf(temporaryUserSettingsByteBuffer, "%s, id = XPROFILE_GAMER_PREFERRED_COLOR_FIRST)", temporaryUserSettingsByteBuffer); break;
			case 30: wsprintf(temporaryUserSettingsByteBuffer, "%s, id = XPROFILE_GAMER_PREFERRED_COLOR_SECOND)", temporaryUserSettingsByteBuffer); break;

			case 34: wsprintf(temporaryUserSettingsByteBuffer, "%s, id = XPROFILE_GAMER_ACTION_AUTO_AIM)", temporaryUserSettingsByteBuffer); break;
			case 35: wsprintf(temporaryUserSettingsByteBuffer, "%s, id = XPROFILE_GAMER_ACTION_AUTO_CENTER)", temporaryUserSettingsByteBuffer); break;
			case 36: wsprintf(temporaryUserSettingsByteBuffer, "%s, id = XPROFILE_GAMER_ACTION_MOVEMENT_CONTROL)", temporaryUserSettingsByteBuffer); break;

			case 38: wsprintf(temporaryUserSettingsByteBuffer, "%s, id = XPROFILE_GAMER_RACE_TRANSMISSION)", temporaryUserSettingsByteBuffer); break;
			case 39: wsprintf(temporaryUserSettingsByteBuffer, "%s, id = XPROFILE_GAMER_RACE_CAMERA_LOCATION)", temporaryUserSettingsByteBuffer); break;
			case 40: wsprintf(temporaryUserSettingsByteBuffer, "%s, id = XPROFILE_GAMER_RACE_BRAKE_CONTROL)", temporaryUserSettingsByteBuffer); break;
			case 41: wsprintf(temporaryUserSettingsByteBuffer, "%s, id = XPROFILE_GAMER_RACE_ACCELERATOR_CONTROL)", temporaryUserSettingsByteBuffer); break;

			case 56: wsprintf(temporaryUserSettingsByteBuffer, "%s, id = XPROFILE_GAMERCARD_TITLE_CRED_EARNED)", temporaryUserSettingsByteBuffer); break;
			case 57: wsprintf(temporaryUserSettingsByteBuffer, "%s, id = XPROFILE_GAMERCARD_TITLE_ACHIEVEMENTS_EARNED)", temporaryUserSettingsByteBuffer); break;
			case 67: wsprintf(temporaryUserSettingsByteBuffer, "%s, id = XPROFILE_GAMERCARD_BIO)", temporaryUserSettingsByteBuffer); break;
			case 68: wsprintf(temporaryUserSettingsByteBuffer, "%s, id = XPROFILE_AVATAR_METADATA)", temporaryUserSettingsByteBuffer); break;

			case 0x3FFF: wsprintf(temporaryUserSettingsByteBuffer, "%s, id = XPROFILE_TITLE_SPECIFIC1)", temporaryUserSettingsByteBuffer); break;
			case 0x3FFE: wsprintf(temporaryUserSettingsByteBuffer, "%s, id = XPROFILE_TITLE_SPECIFIC2)", temporaryUserSettingsByteBuffer); break;
			case 0x3FFD: wsprintf(temporaryUserSettingsByteBuffer, "%s, id = XPROFILE_TITLE_SPECIFIC3)", temporaryUserSettingsByteBuffer); break;
			default: wsprintf(temporaryUserSettingsByteBuffer, "%s, id = Unknown)", temporaryUserSettingsByteBuffer); break;
		}

		size += settingSize;
	}

	*sizeOfSettingsBytes = size;
	*sizeOfSettingsBytes += numberOfSettings * sizeof(XUSER_PROFILE_SETTING);
	*sizeOfSettingsBytes += sizeof(XUSER_READ_PROFILE_SETTING_RESULT);
}

void writeSettingsResults(PXUSER_READ_PROFILE_SETTING_RESULT userSettingsResults, DWORD *sizeOfResultsBuffer, const DWORD* userSettingsToBeRetrieved, DWORD numberOfSettings) {

	memset(userSettingsResults, 0, *sizeOfResultsBuffer);

	userSettingsResults->dwSettingsLen = numberOfSettings;
	userSettingsResults->pSettings = (XUSER_PROFILE_SETTING *)((BYTE *)userSettingsResults + sizeof(XUSER_READ_PROFILE_SETTING_RESULT));

	XUSER_PROFILE_SETTING *settingPointer = userSettingsResults->pSettings;
	BYTE *pSettingData = (BYTE *)settingPointer + numberOfSettings * sizeof(XUSER_PROFILE_SETTING);

	for (DWORD setting = 0; setting < numberOfSettings; setting++)
	{
		int settingType, settingSize, settingId;

		settingType = (userSettingsToBeRetrieved[setting] >> 28) & 0x0F;
		settingSize = (userSettingsToBeRetrieved[setting] >> 16) & 0xFFF;
		settingId = (userSettingsToBeRetrieved[setting] >> 0) & 0x3FFF;

		strcat(temporaryUserSettingsByteBuffer, "\\Offline\\");
		switch (settingId)
		{
			case 0x3FFF: strcat(temporaryUserSettingsByteBuffer, "Title1.dat");	break;
			case 0x3FFE: strcat(temporaryUserSettingsByteBuffer, "Title2.dat"); break;
			case 0x3FFD: strcat(temporaryUserSettingsByteBuffer, "Title3.dat"); break;
			default: strcat(temporaryUserSettingsByteBuffer, "Settings.txt"); break;
		}

		FILE *fp = fopen(temporaryUserSettingsByteBuffer, "rb");
		if (!fp)
		{
			settingPointer->source = XSOURCE_NO_VALUE;
		}
		else
		{
			if (settingType == 6)
			{
				fread(&settingPointer->data.binary.cbData, 1, 4, fp);
				fread(pSettingData, 1, settingPointer->data.binary.cbData, fp);
				settingPointer->data.binary.pbData = pSettingData;
				settingPointer->source = XSOURCE_TITLE;
			}
			fclose(fp);
		}

		settingPointer->data.type = settingType;
		settingPointer->dwSettingId = userSettingsToBeRetrieved[setting];
		settingPointer->user.dwUserIndex = 0;
		pSettingData += settingSize;
		settingPointer++;
	}
}

// #5331
DWORD WINAPI XUserReadProfileSettings(
	DWORD titleId,
	DWORD gamerIndex,
	DWORD numberOfSettings,
	const DWORD* userSettingsToBeRetrieved,
	DWORD* sizeOfResultsBuffer,
	PXUSER_READ_PROFILE_SETTING_RESULT userSettingsResults,
	PXOVERLAPPED pXOverlapped)
{
	TRACE_FX();
	if (gamerIndex >= XLIVE_LOCAL_USER_COUNT)
		return ERROR_NO_SUCH_USER;
	if (xlive_users_info[gamerIndex]->UserSigninState == eXUserSigninState_NotSignedIn)
		return ERROR_NOT_LOGGED_ON;
	if (!numberOfSettings || numberOfSettings > 0x20)
		return ERROR_INVALID_PARAMETER;
	if (!userSettingsToBeRetrieved)
		return ERROR_INVALID_PARAMETER;
	if (!sizeOfResultsBuffer)
		return ERROR_INVALID_PARAMETER;
	if (*sizeOfResultsBuffer && !userSettingsResults)
		return ERROR_INVALID_PARAMETER;
	if (!userSettingsResults) {
		//userSettingsResults can be null the first time this method is called
		addDebugText("No read profile settings results pointer found");
	}

	if (sizeOfResultsBuffer)
	{
		if (*sizeOfResultsBuffer == 0)
		{
			addDebugText("Writing temporary user settings to temp buffer");
			writeTemporaryUserSettingsBuffer(sizeOfResultsBuffer, numberOfSettings, userSettingsToBeRetrieved);
		}
		if (userSettingsResults) {
			addDebugText("Writing user settings to user results buffer");
			writeSettingsResults(userSettingsResults, sizeOfResultsBuffer, userSettingsToBeRetrieved, numberOfSettings);
		}
	}

	if (pXOverlapped)
	{
		pXOverlapped->InternalLow = ERROR_SUCCESS;
		pXOverlapped->InternalHigh = *sizeOfResultsBuffer;
		pXOverlapped->dwExtendedError = ERROR_SUCCESS;

		Check_Overlapped(pXOverlapped);

		return ERROR_IO_PENDING;
	}

	return ERROR_SUCCESS;
}

// #5344
DWORD WINAPI XStorageBuildServerPath(
	DWORD dwUserIndex,
	XSTORAGE_FACILITY StorageFacility,
	CONST void *pvStorageFacilityInfo,
	DWORD dwStorageFacilityInfoSize,
	LPCWSTR *lpwszItemName,
	WCHAR *pwszServerPath,
	DWORD *pdwServerPathLength)
{
	TRACE_FX();
	if (dwUserIndex >= XLIVE_LOCAL_USER_COUNT)
		return ERROR_NO_SUCH_USER;
	if (xlive_users_info[dwUserIndex]->UserSigninState == eXUserSigninState_NotSignedIn)
		return ERROR_NOT_LOGGED_ON;
	if (!pvStorageFacilityInfo && dwStorageFacilityInfoSize)
		return ERROR_INVALID_PARAMETER;
	if (!lpwszItemName)
		return ERROR_INVALID_PARAMETER;
	if (!*lpwszItemName)
		return ERROR_INVALID_PARAMETER;
	if (!pwszServerPath)
		return ERROR_INVALID_PARAMETER;
	if (!pdwServerPathLength)
		return ERROR_INVALID_PARAMETER;
	if (StorageFacility == XSTORAGE_FACILITY_GAME_CLIP && !pvStorageFacilityInfo)
		return ERROR_INVALID_PARAMETER;
	if (pvStorageFacilityInfo && dwStorageFacilityInfoSize < sizeof(XSTORAGE_FACILITY_GAME_CLIP))
		return ERROR_INVALID_PARAMETER;

	//TODO XStorageBuildServerPath

	return ERROR_FUNCTION_FAILED;
	return ERROR_SUCCESS;
}

// #5345
DWORD WINAPI XStorageDownloadToMemory(
	DWORD dwUserIndex,
	const WCHAR *wszServerPath,
	DWORD dwBufferSize,
	const BYTE *pbBuffer,
	DWORD cbResults,
	XSTORAGE_DOWNLOAD_TO_MEMORY_RESULTS *pResults,
	XOVERLAPPED *pXOverlapped)
{
	TRACE_FX();
	if (dwUserIndex >= XLIVE_LOCAL_USER_COUNT)
		return ERROR_NO_SUCH_USER;
	if (xlive_users_info[dwUserIndex]->UserSigninState == eXUserSigninState_NotSignedIn)
		return ERROR_NOT_LOGGED_ON;
	if (!wszServerPath)
		return ERROR_INVALID_PARAMETER;
	if (!*wszServerPath)
		return ERROR_INVALID_PARAMETER;
	if (!dwBufferSize)
		return ERROR_INVALID_PARAMETER;
	if (!pbBuffer)
		return ERROR_INVALID_PARAMETER;
	if (cbResults != sizeof(XSTORAGE_DOWNLOAD_TO_MEMORY_RESULTS))
		return ERROR_INVALID_PARAMETER;
	if (!pResults)
		return ERROR_INVALID_PARAMETER;

	//TODO XStorageDownloadToMemory
	if (pXOverlapped) {
		//asynchronous

		pXOverlapped->InternalLow = ERROR_SUCCESS;
		pXOverlapped->InternalHigh = ERROR_SUCCESS;
		pXOverlapped->dwExtendedError = ERROR_SUCCESS;

		Check_Overlapped(pXOverlapped);

		return ERROR_IO_PENDING;
	}
	else {
		//synchronous
		//return result;
	}
	return ERROR_SUCCESS;
}
