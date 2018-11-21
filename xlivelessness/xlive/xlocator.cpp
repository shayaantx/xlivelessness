#include "xdefs.h"
#include "xsession.h"
#include "../xlln/DebugText.h"
#include "xnet.h"
#include "xlive.h"

// #5230
HRESULT WINAPI XLocatorServerAdvertise(
	DWORD dwUserIndex,
	DWORD dwServerType,
	XNKID *pxnkid,
	DWORD a4,
	DWORD a5,
	DWORD a6,
	DWORD xnaddr1,
	DWORD xnaddr2,
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
	if (!pxnkid)
		return E_POINTER;
	if ((((BYTE*)&(pxnkid))[0] & 0xE0) != 128)
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

	//TODO
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

	//TODO
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

	if (cNumProperties > XLOCATOR_PROPERTY_LIVE_COUNT_TOTAL)
		pProperties[XLOCATOR_PROPERTY_LIVE_COUNT_TOTAL].value.nData = -1;
	if (cNumProperties > XLOCATOR_PROPERTY_LIVE_COUNT_PUBLIC)
		pProperties[XLOCATOR_PROPERTY_LIVE_COUNT_PUBLIC].value.nData = -1;
	if (cNumProperties > XLOCATOR_PROPERTY_LIVE_COUNT_GOLD)
		pProperties[XLOCATOR_PROPERTY_LIVE_COUNT_GOLD].value.nData = -1;
	if (cNumProperties > XLOCATOR_PROPERTY_LIVE_COUNT_PEER)
		pProperties[XLOCATOR_PROPERTY_LIVE_COUNT_PEER].value.nData = -1;
	

	//TODO
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

// #5234
DWORD WINAPI XLocatorCreateServerEnumerator(
	DWORD dwUserIndex,
	DWORD cItems,
	DWORD cRequiredPropertyIDs,
	DWORD *pRequiredPropertyIDs,
	DWORD cFilterGroupItems,
	struct _XLOCATOR_FILTER_GROUP *pxlFilterGroups,
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
		//TODO
	}
	if (cFilterGroupItems > INT_MAX || cFilterGroupItems && !pxlFilterGroups)
		return ERROR_INVALID_PARAMETER;
	if (cFilterGroupItems) {
		BOOL check = FALSE;
		unsigned int v4 = 0;
		struct _XLOCATOR_FILTER_GROUP **v3 = (struct _XLOCATOR_FILTER_GROUP **)pxlFilterGroups;
		while (!*v3 || v3[1])
		{
			++v4;
			v3 += 2;
			if (v4 >= cFilterGroupItems) {
				check = TRUE;
				break;
			}
		}
		if (!check)
			return ERROR_INVALID_PARAMETER;
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

	int total_count = 0;
	*pcbBuffer = (DWORD)(sizeof(_XLOCATOR_SEARCHRESULT) * (total_count + 10));
	*phEnum = CreateMutex(NULL, NULL, NULL);

	return S_OK;
}

// #5236
HRESULT WINAPI XLocatorServiceInitialize(struct XLOCATOR_INIT_INFO *pXii, DWORD *a2)
{
	TRACE_FX();
	if (!pXii)
		return E_POINTER;

	// Remove this and Live is not available apparently.
	if (a2)
		*a2 = 0;

	return S_OK;
	return S_FALSE;
}

// #5237
HRESULT WINAPI XLocatorServiceUnInitialize(DWORD *a1)
{
	TRACE_FX();
	return S_OK;
}

// #5238
HRESULT WINAPI XLocatorCreateKey(XNKID *pxnkid, XNKEY *pxnkey)
{
	TRACE_FX();
	HRESULT result = XNetCreateKey(pxnkid, pxnkey);
	return result;
}
