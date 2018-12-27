#include "xdefs.h"
#include "xlocator.h"
#include "xsession.h"
#include "xsocket.h"
#include "../xlln/DebugText.h"
#include "xnet.h"
#include "xlive.h"
#include <thread>
#include <condition_variable>
#include <atomic>
#include <vector>

#pragma pack(push) // save current alignment setting
#pragma pack(1)   // specify single-byte alignment
typedef struct {
	DWORD padding1;
	XUID xuid;
	DWORD dwServerType;
	XNKID xnkid;
	XNKEY xnkey;
	DWORD dwMaxPublicSlots;
	DWORD dwMaxPrivateSlots;
	DWORD dwFilledPublicSlots;
	DWORD dwFilledPrivateSlots;
	DWORD cProperties;
	DWORD pProperties;
} LIVE_SERVER_DETAILS;
#pragma pack(pop) // return to original alignment setting

//FIXME Needs a critical section
std::map<HANDLE, std::vector<std::pair<DWORD, WORD>>> xlive_xlocator_enumerators;

std::map<std::pair<DWORD, WORD>, XLOCATOR_SESSION*> liveoverlan_sessions;
CRITICAL_SECTION liveoverlan_sessions_lock;
static std::condition_variable liveoverlan_cond_empty;
static std::thread liveoverlan_empty_thread;
static std::atomic<bool> liveoverlan_empty_exit = FALSE;

static CRITICAL_SECTION liveoverlan_broadcast_lock;
static std::condition_variable liveoverlan_cond_broadcast;
static std::thread liveoverlan_thread;
static BOOL liveoverlan_running = FALSE;
static std::atomic<bool> liveoverlan_exit = FALSE;

static LIVE_SERVER_DETAILS *local_session_details = 0;

VOID LiveOverLanClone(XLOCATOR_SEARCHRESULT **dst, XLOCATOR_SEARCHRESULT *src)
{
	XLOCATOR_SEARCHRESULT *xlocator_result;
	if (dst)
		xlocator_result = *dst;
	else
		*dst = xlocator_result = new XLOCATOR_SEARCHRESULT;
	memcpy_s(xlocator_result, sizeof(XLOCATOR_SEARCHRESULT), src, sizeof(XLOCATOR_SEARCHRESULT));
	xlocator_result->pProperties = new XUSER_PROPERTY[xlocator_result->cProperties];
	for (DWORD i = 0; i < xlocator_result->cProperties; i++) {
		memcpy_s(&xlocator_result->pProperties[i], sizeof(XUSER_PROPERTY), &src->pProperties[i], sizeof(XUSER_PROPERTY));
		if (xlocator_result->pProperties[i].value.type == XUSER_DATA_TYPE_UNICODE) {
			if (xlocator_result->pProperties[i].value.string.cbData % 2)
				__debugbreak();
			xlocator_result->pProperties[i].value.string.pwszData = new WCHAR[xlocator_result->pProperties[i].value.string.cbData/2];
			memcpy_s(xlocator_result->pProperties[i].value.string.pwszData, xlocator_result->pProperties[i].value.string.cbData, src->pProperties[i].value.string.pwszData, src->pProperties[i].value.string.cbData);
			xlocator_result->pProperties[i].value.string.pwszData[(xlocator_result->pProperties[i].value.string.cbData/2) - 1] = 0;
		}
		else if (xlocator_result->pProperties[i].value.type == XUSER_DATA_TYPE_BINARY) {
			xlocator_result->pProperties[i].value.binary.pbData = new BYTE[xlocator_result->pProperties[i].value.binary.cbData];
			memcpy_s(xlocator_result->pProperties[i].value.binary.pbData, xlocator_result->pProperties[i].value.binary.cbData, src->pProperties[i].value.binary.pbData, src->pProperties[i].value.binary.cbData);
		}
	}
}
VOID LiveOverLanDelete(XLOCATOR_SEARCHRESULT *xlocator_result)
{
	for (DWORD i = 0; i < xlocator_result->cProperties; i++) {
		if (xlocator_result->pProperties[i].value.type == XUSER_DATA_TYPE_UNICODE) {
			delete[] xlocator_result->pProperties[i].value.string.pwszData;
		}
		else if (xlocator_result->pProperties[i].value.type == XUSER_DATA_TYPE_BINARY) {
			delete[] xlocator_result->pProperties[i].value.binary.pbData;
		}
	}
	delete[] xlocator_result->pProperties;
	delete xlocator_result;
}

static VOID LiveOverLanBroadcast()
{
	std::mutex mymutex;
	while (1) {
		EnterCriticalSection(&liveoverlan_broadcast_lock);
		if (local_session_details) {
			// Broadcast.
			addDebugText("LiveOverLan Broadcast");

			SOCKADDR_IN SendStruct;
			SendStruct.sin_port = htons(1001);
			SendStruct.sin_addr.s_addr = INADDR_BROADCAST;
			SendStruct.sin_family = AF_INET;

			XSocketSendTo(xlive_liveoverlan_socket, (char*)local_session_details, local_session_details->pProperties + sizeof(LIVE_SERVER_DETAILS), 0, (SOCKADDR*)&SendStruct, sizeof(SendStruct));

		}
		LeaveCriticalSection(&liveoverlan_broadcast_lock);
		std::unique_lock<std::mutex> lock(mymutex);
		liveoverlan_cond_broadcast.wait_for(lock, std::chrono::seconds(5), []() { return liveoverlan_exit == true; });
		if (liveoverlan_exit)
			break;
	}
}
BOOL LiveOverLanBroadcastReceive(PXLOCATOR_SEARCHRESULT *result, BYTE *buf, DWORD buflen)
{
	DWORD buflenread = sizeof(LIVE_SERVER_DETAILS);
	if (buflenread > buflen) {
		return FALSE;
	}
	XLOCATOR_SEARCHRESULT* xlocator_result = new XLOCATOR_SEARCHRESULT;
	LIVE_SERVER_DETAILS* session_details = (LIVE_SERVER_DETAILS*)buf;
	xlocator_result->serverID = session_details->xuid;
	xlocator_result->dwServerType = session_details->dwServerType;
	xlocator_result->xnkid = session_details->xnkid;
	xlocator_result->xnkey = session_details->xnkey;
	xlocator_result->dwMaxPublicSlots = session_details->dwMaxPublicSlots;
	xlocator_result->dwMaxPrivateSlots = session_details->dwMaxPrivateSlots;
	xlocator_result->dwFilledPublicSlots = session_details->dwFilledPublicSlots;
	xlocator_result->dwFilledPrivateSlots = session_details->dwFilledPrivateSlots;
	xlocator_result->cProperties = session_details->cProperties;
	try {
		xlocator_result->pProperties = new XUSER_PROPERTY[xlocator_result->cProperties];
	}
	catch (std::bad_alloc) {
		delete xlocator_result;
		return FALSE;
	}
	
	// account for all dwPropertyId and value.type
	buflenread += (sizeof(DWORD) + sizeof(BYTE)) * xlocator_result->cProperties;

	if (buflenread > buflen) {
		delete[] xlocator_result->pProperties;
		delete xlocator_result;
		return FALSE;
	}

	BYTE *propertiesBuf = (BYTE*)((DWORD)&session_details->pProperties + sizeof(DWORD));
	DWORD i = 0;
	for (; i < xlocator_result->cProperties; i++) {
		xlocator_result->pProperties[i].dwPropertyId = *((DWORD*)propertiesBuf);
		propertiesBuf += sizeof(DWORD); //dwPropertyId
		xlocator_result->pProperties[i].value.type = *propertiesBuf;
		propertiesBuf += sizeof(BYTE); //value.type

		if (xlocator_result->pProperties[i].value.type == XUSER_DATA_TYPE_INT32) {
			buflenread += sizeof(LONG);
			propertiesBuf += sizeof(LONG);
		}
		else if (xlocator_result->pProperties[i].value.type == XUSER_DATA_TYPE_INT64) {
			buflenread += sizeof(LONGLONG);
			propertiesBuf += sizeof(LONGLONG);
		}
		else if (xlocator_result->pProperties[i].value.type == XUSER_DATA_TYPE_DOUBLE) {
			buflenread += sizeof(DOUBLE);
			propertiesBuf += sizeof(DOUBLE);
		}
		else if (xlocator_result->pProperties[i].value.type == XUSER_DATA_TYPE_FLOAT) {
			buflenread += sizeof(FLOAT);
			propertiesBuf += sizeof(FLOAT);
		}
		else if (xlocator_result->pProperties[i].value.type == XUSER_DATA_TYPE_UNICODE) {
			buflenread += sizeof(DWORD); //value.string.cbData
			if (buflenread > buflen) {
				break;
			}
			xlocator_result->pProperties[i].value.string.cbData = *((DWORD*)propertiesBuf);
			propertiesBuf += sizeof(DWORD);
			buflenread += xlocator_result->pProperties[i].value.string.cbData;
			propertiesBuf += xlocator_result->pProperties[i].value.string.cbData;
			if (buflenread > buflen || xlocator_result->pProperties[i].value.string.cbData % 2 || *(WCHAR*)(propertiesBuf - sizeof(WCHAR)) != 0) { // Must be null-terminated.
				break;
			}
		}
		else if (xlocator_result->pProperties[i].value.type == XUSER_DATA_TYPE_BINARY) {
			buflenread += sizeof(DWORD); //value.binary.cbData
			if (buflenread > buflen) {
				break;
			}
			xlocator_result->pProperties[i].value.binary.cbData = *((DWORD*)propertiesBuf);
			propertiesBuf += sizeof(DWORD);
			buflenread += xlocator_result->pProperties[i].value.binary.cbData;
			propertiesBuf += xlocator_result->pProperties[i].value.binary.cbData;
			if (buflenread > buflen) {
				break;
			}
		}
		else if (xlocator_result->pProperties[i].value.type == XUSER_DATA_TYPE_DATETIME) {
			buflenread += sizeof(FILETIME);
			propertiesBuf += sizeof(FILETIME);
		}
		else if (xlocator_result->pProperties[i].value.type == XUSER_DATA_TYPE_NULL) {
			__debugbreak();
		}
		else if (xlocator_result->pProperties[i].value.type == XUSER_DATA_TYPE_CONTEXT) {
			__debugbreak();
		}
		else {
			__debugbreak();
		}
	}

	if (buflenread != buflen || i != xlocator_result->cProperties) {
		delete[] xlocator_result->pProperties;
		delete xlocator_result;
		return FALSE;
	}
	if (propertiesBuf - buf != buflenread) {
		// Misaligned buffer!
		__debugbreak();
	}

	propertiesBuf = (BYTE*)((DWORD)&session_details->pProperties + sizeof(DWORD));
	i = 0;
	for (; i < xlocator_result->cProperties; i++) {
		propertiesBuf += sizeof(DWORD); //dwPropertyId
		propertiesBuf += sizeof(BYTE); //value.type

		if (xlocator_result->pProperties[i].value.type == XUSER_DATA_TYPE_INT32) {
			xlocator_result->pProperties[i].value.nData = *((LONG*)propertiesBuf);
			propertiesBuf += sizeof(LONG);
		}
		else if (xlocator_result->pProperties[i].value.type == XUSER_DATA_TYPE_INT64) {
			xlocator_result->pProperties[i].value.i64Data = *((LONGLONG*)propertiesBuf);
			propertiesBuf += sizeof(LONGLONG);
		}
		else if (xlocator_result->pProperties[i].value.type == XUSER_DATA_TYPE_DOUBLE) {
			xlocator_result->pProperties[i].value.dblData = *((DOUBLE*)propertiesBuf);
			propertiesBuf += sizeof(DOUBLE);
		}
		else if (xlocator_result->pProperties[i].value.type == XUSER_DATA_TYPE_FLOAT) {
			xlocator_result->pProperties[i].value.fData = *((FLOAT*)propertiesBuf);
			propertiesBuf += sizeof(FLOAT);
		}
		else if (xlocator_result->pProperties[i].value.type == XUSER_DATA_TYPE_UNICODE) {
			propertiesBuf += sizeof(DWORD); //value.string.cbData
			xlocator_result->pProperties[i].value.string.pwszData = new WCHAR[xlocator_result->pProperties[i].value.string.cbData/2];
			memcpy_s(xlocator_result->pProperties[i].value.string.pwszData, xlocator_result->pProperties[i].value.string.cbData, propertiesBuf, xlocator_result->pProperties[i].value.string.cbData);
			propertiesBuf += xlocator_result->pProperties[i].value.string.cbData;
			xlocator_result->pProperties[i].value.string.pwszData[(xlocator_result->pProperties[i].value.string.cbData/2) - 1] = 0;
		}
		else if (xlocator_result->pProperties[i].value.type == XUSER_DATA_TYPE_BINARY) {
			propertiesBuf += sizeof(DWORD); //value.binary.cbData
			xlocator_result->pProperties[i].value.binary.pbData = new BYTE[xlocator_result->pProperties[i].value.binary.cbData];
			memcpy_s(xlocator_result->pProperties[i].value.binary.pbData, xlocator_result->pProperties[i].value.binary.cbData, propertiesBuf, xlocator_result->pProperties[i].value.binary.cbData);
			propertiesBuf += xlocator_result->pProperties[i].value.binary.cbData;
		}
		else if (xlocator_result->pProperties[i].value.type == XUSER_DATA_TYPE_DATETIME) {
			xlocator_result->pProperties[i].value.ftData = *((FILETIME*)propertiesBuf);
			propertiesBuf += sizeof(FILETIME);
		}
		else if (xlocator_result->pProperties[i].value.type == XUSER_DATA_TYPE_NULL) {
			__debugbreak();
		}
		else if (xlocator_result->pProperties[i].value.type == XUSER_DATA_TYPE_CONTEXT) {
			__debugbreak();
		}
		else {
			__debugbreak();
		}
	}
	if (propertiesBuf - buf != buflenread) {
		// Misaligned buffer!
		__debugbreak();
	}

	*result = xlocator_result;
	return TRUE;
}

static VOID LiveOverLanBroadcastData(XUID *xuid,
	DWORD dwServerType,
	XNKID xnkid,
	XNKEY xnkey,
	DWORD dwMaxPublicSlots,
	DWORD dwMaxPrivateSlots,
	DWORD dwFilledPublicSlots,
	DWORD dwFilledPrivateSlots,
	DWORD cProperties,
	PXUSER_PROPERTY pProperties)
{
	std::map<DWORD, XUSER_PROPERTY*> local_session_properties;
	for (DWORD i = 0; i < cProperties; i++) {
		local_session_properties[pProperties[i].dwPropertyId] = &pProperties[i];
	}
	if (local_session_properties.count(0)) {
		// A null property id should not exist.
		__debugbreak();
	}
	if (!local_session_properties.count(0x40008230) && xlive_users_info[0]->xuid == *xuid) {
		XUSER_PROPERTY* prop = new XUSER_PROPERTY;
		
		prop->dwPropertyId = 0;//so we know to delete this one later.
		prop->value.type = XUSER_DATA_TYPE_UNICODE;
		DWORD strlen = strnlen(xlive_users_info[0]->szUserName, XUSER_MAX_NAME_LENGTH) + 1;
		prop->value.string.cbData = sizeof(WCHAR) * strlen;
		prop->value.string.pwszData = new WCHAR[strlen];
		swprintf_s(prop->value.string.pwszData, strlen, L"%hs", xlive_users_info[0]->szUserName);

		local_session_properties[0x40008230] = prop;
	}

	DWORD buflen = sizeof(LIVE_SERVER_DETAILS);
	for (auto const &prop : local_session_properties) {
		buflen += sizeof(DWORD); //dwPropertyId
		buflen += sizeof(BYTE); //value.type
		if (prop.second->value.type == XUSER_DATA_TYPE_INT32) {
			buflen += sizeof(LONG);
		}
		else if (prop.second->value.type == XUSER_DATA_TYPE_INT64) {
			buflen += sizeof(LONGLONG);
		}
		else if (prop.second->value.type == XUSER_DATA_TYPE_DOUBLE) {
			buflen += sizeof(DOUBLE);
		}
		else if (prop.second->value.type == XUSER_DATA_TYPE_FLOAT) {
			buflen += sizeof(FLOAT);
		}
		else if (prop.second->value.type == XUSER_DATA_TYPE_UNICODE) {
			buflen += sizeof(DWORD); //value.string.cbData
			if (prop.second->value.string.cbData % 2)
				__debugbreak();
			buflen += prop.second->value.string.cbData;
			//if (prop.second->value.string.cbData == 2)
			//	buflen += 2;
		}
		else if (prop.second->value.type == XUSER_DATA_TYPE_BINARY) {
			buflen += sizeof(DWORD); //value.binary.cbData
			buflen += prop.second->value.binary.cbData;
		}
		else if (prop.second->value.type == XUSER_DATA_TYPE_DATETIME) {
			buflen += sizeof(FILETIME);
		}
		else if (prop.second->value.type == XUSER_DATA_TYPE_NULL) {
			__debugbreak();
		}
		else if (prop.second->value.type == XUSER_DATA_TYPE_CONTEXT) {
			__debugbreak();
		}
		else {
			__debugbreak();
		}
	}

	LIVE_SERVER_DETAILS *new_local_sd = (LIVE_SERVER_DETAILS*)malloc(buflen);
	new_local_sd->xuid = *xuid;
	new_local_sd->dwServerType = dwServerType;
	new_local_sd->xnkid = xnkid;
	new_local_sd->xnkey = xnkey;
	new_local_sd->dwMaxPublicSlots = dwMaxPublicSlots;
	new_local_sd->dwMaxPrivateSlots = dwMaxPrivateSlots;
	new_local_sd->dwFilledPublicSlots = dwFilledPublicSlots;
	new_local_sd->dwFilledPrivateSlots = dwFilledPrivateSlots;
	new_local_sd->cProperties = local_session_properties.size();
	BYTE *propertiesBuf = (BYTE*)&new_local_sd->pProperties;
	*((DWORD*)propertiesBuf) = buflen - sizeof(LIVE_SERVER_DETAILS);
	propertiesBuf += sizeof(DWORD);
	for (auto const &prop : local_session_properties) {
		*((DWORD*)propertiesBuf) = prop.first; //dwPropertyId
		propertiesBuf += sizeof(DWORD);
		*propertiesBuf = prop.second->value.type;
		propertiesBuf += sizeof(BYTE);
		if (prop.second->value.type == XUSER_DATA_TYPE_INT32) {
			*((LONG*)propertiesBuf) = prop.second->value.nData;
			propertiesBuf += sizeof(LONG);
		}
		else if (prop.second->value.type == XUSER_DATA_TYPE_INT64) {
			*((LONGLONG*)propertiesBuf) = prop.second->value.i64Data;
			propertiesBuf += sizeof(LONGLONG);
		}
		else if (prop.second->value.type == XUSER_DATA_TYPE_DOUBLE) {
			*((DOUBLE*)propertiesBuf) = prop.second->value.dblData;
			propertiesBuf += sizeof(DOUBLE);
		}
		else if (prop.second->value.type == XUSER_DATA_TYPE_FLOAT) {
			*((FLOAT*)propertiesBuf) = prop.second->value.fData;
			propertiesBuf += sizeof(FLOAT);
		}
		else if (prop.second->value.type == XUSER_DATA_TYPE_UNICODE) {
			*((DWORD*)propertiesBuf) = prop.second->value.string.cbData;
			//if (prop.second->value.string.cbData == 2)
			//	*((DWORD*)propertiesBuf) = 4;
			propertiesBuf += sizeof(DWORD);
			memcpy_s(propertiesBuf, prop.second->value.string.cbData, prop.second->value.string.pwszData, prop.second->value.string.cbData);
			//if (prop.second->value.string.cbData == 2) {
			//	((WCHAR*)propertiesBuf)[0] = 'a';
			//	((WCHAR*)propertiesBuf)[1] = 0;
			//	propertiesBuf += 2;
			//}
			propertiesBuf += prop.second->value.string.cbData;
		}
		else if (prop.second->value.type == XUSER_DATA_TYPE_BINARY) {
			*((DWORD*)propertiesBuf) = prop.second->value.binary.cbData;
			propertiesBuf += sizeof(DWORD);
			memcpy_s(propertiesBuf, prop.second->value.binary.cbData, prop.second->value.binary.pbData, prop.second->value.binary.cbData);
			propertiesBuf += prop.second->value.binary.cbData;
		}
		else if (prop.second->value.type == XUSER_DATA_TYPE_DATETIME) {
			*((FILETIME*)propertiesBuf) = prop.second->value.ftData;
			propertiesBuf += sizeof(FILETIME);
		}
		else if (prop.second->value.type == XUSER_DATA_TYPE_NULL) {
			__debugbreak();
		}
		else if (prop.second->value.type == XUSER_DATA_TYPE_CONTEXT) {
			__debugbreak();
		}
		else {
			__debugbreak();
		}
	}

	for (auto const &prop : local_session_properties) {
		if (!prop.second->dwPropertyId) {
			if (prop.second->value.type == XUSER_DATA_TYPE_UNICODE) {
				delete[] prop.second->value.string.pwszData;
			}
			else if (prop.second->value.type == XUSER_DATA_TYPE_BINARY) {
				delete[] prop.second->value.binary.pbData;
			}
			delete prop.second;
		}
	}

	new_local_sd->padding1 = 0x01230123;

	EnterCriticalSection(&liveoverlan_broadcast_lock);
	if (local_session_details)
		free(local_session_details);
	local_session_details = new_local_sd;
	LeaveCriticalSection(&liveoverlan_broadcast_lock);
}
static VOID LiveOverLanStartBroadcast()
{
	EnterCriticalSection(&liveoverlan_broadcast_lock);

	if (!liveoverlan_running) {
		liveoverlan_running = TRUE;
		liveoverlan_exit = FALSE;
		liveoverlan_thread = std::thread(LiveOverLanBroadcast);
	}

	LeaveCriticalSection(&liveoverlan_broadcast_lock);
}
static VOID LiveOverLanStopBroadcast()
{
	EnterCriticalSection(&liveoverlan_broadcast_lock);
	if (local_session_details)
		free(local_session_details);
	local_session_details = 0;

	if (liveoverlan_running) {
		liveoverlan_running = FALSE;
		liveoverlan_exit = TRUE;
		liveoverlan_cond_broadcast.notify_all();
		liveoverlan_thread.join();
	}
	
	LeaveCriticalSection(&liveoverlan_broadcast_lock);
}

static VOID LiveOverLanEmpty()
{
	std::mutex mymutex;
	while (1) {
		addDebugText("LiveOverLan Empty");
		EnterCriticalSection(&liveoverlan_sessions_lock);
		
		std::vector<std::pair<DWORD, WORD>> removesessions;
		time_t ltime;
		time(&ltime);//seconds since epoch.
		DWORD timetoremove = ((DWORD)ltime) - 15;
		for (auto const &session : liveoverlan_sessions) {
			if (session.second->broadcastTime > timetoremove)
				continue;
			LiveOverLanDelete(session.second->searchresult);
			delete session.second;
			removesessions.push_back(session.first);
		}
		for (auto const &session : removesessions) {
			liveoverlan_sessions.erase(session);
		}

		LeaveCriticalSection(&liveoverlan_sessions_lock);

		std::unique_lock<std::mutex> lock(mymutex);
		liveoverlan_cond_empty.wait_for(lock, std::chrono::seconds(10), []() { return liveoverlan_empty_exit == true; });
		if (liveoverlan_empty_exit)
			break;
	}
}
static VOID LiveOverLanStartEmpty()
{
	liveoverlan_empty_exit = FALSE;
	liveoverlan_empty_thread = std::thread(LiveOverLanEmpty);
}
static VOID LiveOverLanStopEmpty()
{
	EnterCriticalSection(&liveoverlan_sessions_lock);
	for (auto const &session : liveoverlan_sessions) {
		LiveOverLanDelete(session.second->searchresult);
		delete session.second;
	}
	liveoverlan_sessions.clear();
	LeaveCriticalSection(&liveoverlan_sessions_lock);
	liveoverlan_empty_exit = TRUE;
	liveoverlan_cond_empty.notify_all();
	liveoverlan_empty_thread.join();
}

// #5230
HRESULT WINAPI XLocatorServerAdvertise(
	DWORD dwUserIndex,
	DWORD dwServerType,
	XNKID xnkid,
	XNKEY xnkey,
	DWORD dwMaxPublicSlots,
	DWORD dwMaxPrivateSlots,
	DWORD dwFilledPublicSlots,
	DWORD dwFilledPrivateSlots,
	DWORD cProperties,
	PXUSER_PROPERTY pProperties,
	PXOVERLAPPED pXOverlapped)
{
	TRACE_FX();
	if (dwUserIndex >= XLIVE_LOCAL_USER_COUNT)
		return E_INVALIDARG;
	if (xlive_users_info[dwUserIndex]->UserSigninState == eXUserSigninState_NotSignedIn)
		return E_INVALIDARG;
	if (dwServerType > INT_MAX)
		return E_INVALIDARG;
	if ((((BYTE*)&(xnkid))[0] & 0xE0) != 0x80)
		return E_INVALIDARG;
	if (dwMaxPublicSlots < dwFilledPublicSlots)
		return E_INVALIDARG;
	if (dwMaxPrivateSlots < dwFilledPrivateSlots)
		return E_INVALIDARG;
	if (dwMaxPublicSlots > INT_MAX)
		return E_INVALIDARG;
	if (dwMaxPrivateSlots > INT_MAX)
		return E_INVALIDARG;
	if (!cProperties && pProperties || cProperties && !pProperties)
		return E_INVALIDARG;
	if (cProperties > INT_MAX)
		return E_INVALIDARG;

	LiveOverLanBroadcastData(&xlive_users_info[dwUserIndex]->xuid, dwServerType, xnkid, xnkey, dwMaxPublicSlots, dwMaxPrivateSlots, dwFilledPublicSlots, dwFilledPrivateSlots, cProperties, pProperties);
	LiveOverLanStartBroadcast();

	if (pXOverlapped) {
		//asynchronous

		pXOverlapped->InternalLow = ERROR_SUCCESS;
		pXOverlapped->InternalHigh = ERROR_SUCCESS;
		pXOverlapped->dwExtendedError = ERROR_SUCCESS;

		Check_Overlapped(pXOverlapped);

		return HRESULT_FROM_WIN32(ERROR_IO_PENDING);
	}
	else {
		//synchronous
		//return result;
	}
	return S_OK;
}

// #5231
HRESULT WINAPI XLocatorServerUnAdvertise(DWORD dwUserIndex, PXOVERLAPPED pXOverlapped)
{
	TRACE_FX();
	if (dwUserIndex >= XLIVE_LOCAL_USER_COUNT)
		return E_INVALIDARG;
	if (xlive_users_info[dwUserIndex]->UserSigninState == eXUserSigninState_NotSignedIn)
		return E_INVALIDARG;

	LiveOverLanStopBroadcast();

	SOCKADDR_IN SendStruct;
	SendStruct.sin_port = htons(1001);
	SendStruct.sin_addr.s_addr = INADDR_BROADCAST;
	SendStruct.sin_family = AF_INET;

	//FIXME write a better packet and handler (padding1) for this.
	LIVE_SERVER_DETAILS ohboy;
	ohboy.padding1 = 0x01230124;
	ohboy.xuid = xlive_users_info[dwUserIndex]->xuid;

	XSocketSendTo(xlive_liveoverlan_socket, (char*)&ohboy, sizeof(LIVE_SERVER_DETAILS), 0, (SOCKADDR*)&SendStruct, sizeof(SendStruct));

	if (pXOverlapped) {
		//asynchronous

		pXOverlapped->InternalLow = ERROR_SUCCESS;
		pXOverlapped->InternalHigh = ERROR_SUCCESS;
		pXOverlapped->dwExtendedError = ERROR_SUCCESS;

		Check_Overlapped(pXOverlapped);

		return HRESULT_FROM_WIN32(ERROR_IO_PENDING);
	}
	else {
		//synchronous
		//return result;
	}
	return S_OK;
}

// #5233
HRESULT WINAPI XLocatorGetServiceProperty(DWORD dwUserIndex, DWORD cNumProperties, PXUSER_PROPERTY pProperties, PXOVERLAPPED pXOverlapped)
{
	TRACE_FX();
	if (dwUserIndex >= XLIVE_LOCAL_USER_COUNT)
		return E_INVALIDARG;
	if (xlive_users_info[dwUserIndex]->UserSigninState == eXUserSigninState_NotSignedIn)
		return E_INVALIDARG;
	if (!cNumProperties || cNumProperties > INT_MAX)
		return E_INVALIDARG;
	if (!pProperties)
		return E_POINTER;

	EnterCriticalSection(&liveoverlan_sessions_lock);
	if (cNumProperties > XLOCATOR_PROPERTY_LIVE_COUNT_TOTAL)
		pProperties[XLOCATOR_PROPERTY_LIVE_COUNT_TOTAL].value.nData = liveoverlan_sessions.size();
	if (cNumProperties > XLOCATOR_PROPERTY_LIVE_COUNT_PUBLIC)
		pProperties[XLOCATOR_PROPERTY_LIVE_COUNT_PUBLIC].value.nData = -1;
	if (cNumProperties > XLOCATOR_PROPERTY_LIVE_COUNT_GOLD)
		pProperties[XLOCATOR_PROPERTY_LIVE_COUNT_GOLD].value.nData = -1;
	if (cNumProperties > XLOCATOR_PROPERTY_LIVE_COUNT_PEER)
		pProperties[XLOCATOR_PROPERTY_LIVE_COUNT_PEER].value.nData = -1;
	LeaveCriticalSection(&liveoverlan_sessions_lock);

	if (pXOverlapped) {
		//asynchronous

		pXOverlapped->InternalLow = ERROR_SUCCESS;
		pXOverlapped->InternalHigh = ERROR_SUCCESS;
		pXOverlapped->dwExtendedError = ERROR_SUCCESS;

		Check_Overlapped(pXOverlapped);

		return HRESULT_FROM_WIN32(ERROR_IO_PENDING);
	}
	else {
		//synchronous
		//return result;
	}
	return S_OK;
}

typedef struct {
	DWORD ukn1;
	void *ukn2;
} XLOCATOR_FILTER_GROUP;

// #5234
DWORD WINAPI XLocatorCreateServerEnumerator(
	DWORD dwUserIndex,
	DWORD cItems,
	DWORD cRequiredPropertyIDs,
	DWORD *pRequiredPropertyIDs,
	DWORD cFilterGroupItems,
	XLOCATOR_FILTER_GROUP *pxlFilterGroups,
	DWORD cSorterItems,
	struct _XLOCATOR_SORTER *pxlSorters,
	PDWORD pcbBuffer,
	PHANDLE phEnum)
{
	TRACE_FX();
	if (dwUserIndex >= XLIVE_LOCAL_USER_COUNT)
		return ERROR_NO_SUCH_USER;
	if (xlive_users_info[dwUserIndex]->UserSigninState == eXUserSigninState_NotSignedIn)
		return ERROR_NOT_LOGGED_ON;

	if (!cItems || cItems > INT_MAX)
		return ERROR_INVALID_PARAMETER;
	if (cRequiredPropertyIDs > INT_MAX || cRequiredPropertyIDs && !pRequiredPropertyIDs)
		return ERROR_INVALID_PARAMETER;
	if (cRequiredPropertyIDs) {
		for (unsigned int i = 0; i < cRequiredPropertyIDs; i++) {
			if (!pRequiredPropertyIDs[i]) {
				return ERROR_INVALID_PARAMETER;
			}
		}
	}
	if (cFilterGroupItems > INT_MAX || cFilterGroupItems && !pxlFilterGroups)
		return ERROR_INVALID_PARAMETER;
	if (cFilterGroupItems) {
		XLOCATOR_FILTER_GROUP *list = pxlFilterGroups;
		for (unsigned int i = 0; i < cFilterGroupItems; i++) {
			if (!list[i].ukn1 || !list[i].ukn2) {
				return ERROR_INVALID_PARAMETER;
			}
		}
	}
	if (cSorterItems > INT_MAX || cSorterItems && !pxlSorters)
		return ERROR_INVALID_PARAMETER;
	if (cSorterItems) {
		unsigned int v12 = 0;
		struct _XLOCATOR_SORTER **v13 = (struct _XLOCATOR_SORTER **)pxlSorters;
		while (2)
		{
			v12++;
			struct _XLOCATOR_SORTER *v14 = v13[1];
			for (unsigned int i = v12; i < cSorterItems; i++)
			{
				if (*v13 == *((struct _XLOCATOR_SORTER **)pxlSorters + 2 * i))
				{
					// L"XLocatorCreateServerEnumerator: Invalid parameter - duplicate sorters in pxlSorters.";
					return ERROR_INVALID_PARAMETER;
				}
			}
			v13 += 2;
			if (v12 < cSorterItems)
				continue;
			break;
		}
	}
	if (!pcbBuffer)
		return ERROR_INVALID_PARAMETER;
	if (!phEnum)
		return ERROR_INVALID_PARAMETER;

	if (cItems > 200)
		cItems = 200;
	*pcbBuffer = (cItems) * sizeof(XLOCATOR_SEARCHRESULT);
	*phEnum = CreateMutex(NULL, NULL, NULL);
	xlive_xlocator_enumerators[*phEnum];

	return S_OK;
}

typedef struct {
	DWORD ukn1;
	DWORD ukn2;
	WORD ukn3;
	char ukn4[0x22];
} XLOCATOR_INIT_INFO;

// #5236
HRESULT WINAPI XLocatorServiceInitialize(XLOCATOR_INIT_INFO *pXii, PHANDLE phLocatorService)
{
	TRACE_FX();
	if (!pXii)
		return E_POINTER;

	InitializeCriticalSection(&liveoverlan_broadcast_lock);
	InitializeCriticalSection(&liveoverlan_sessions_lock);

	LiveOverLanStartEmpty();
	
	if (phLocatorService)
		*phLocatorService = CreateMutex(NULL, NULL, NULL);
	
	return S_OK;
	return S_FALSE;
}

// #5237
HRESULT WINAPI XLocatorServiceUnInitialize(HANDLE hLocatorService)
{
	TRACE_FX();

	LiveOverLanStopBroadcast();
	LiveOverLanStopEmpty();
	DeleteCriticalSection(&liveoverlan_broadcast_lock);
	DeleteCriticalSection(&liveoverlan_sessions_lock);

	if (!hLocatorService || (DWORD)hLocatorService == -1) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return S_FALSE;
	}
	if (!CloseHandle(hLocatorService)) {
		SetLastError(ERROR_INVALID_HANDLE);
		return S_FALSE;
	}
	return S_OK;
}

// #5238
HRESULT WINAPI XLocatorCreateKey(XNKID *pxnkid, XNKEY *pxnkey)
{
	TRACE_FX();
	HRESULT result = XNetCreateKey(pxnkid, pxnkey);
	return result;
}
