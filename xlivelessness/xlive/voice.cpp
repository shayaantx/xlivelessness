#include <windows.h>
#include "Voice.h"
#include "../xlln/DebugText.h"

// #5008
INT WINAPI XHVCreateEngine(PXHV_INIT_PARAMS pParams, PHANDLE phWorkerThread, PIXHV2ENGINE *ppEngine)
{
	TRACE_FX();
	if (!pParams)
		return E_INVALIDARG;
	if (!ppEngine)
		return E_INVALIDARG;
	if (phWorkerThread)
		return E_INVALIDARG;
	if (!pParams->dwMaxLocalTalkers)
		return E_INVALIDARG;
	if (pParams->dwMaxLocalTalkers > XHV_MAX_LOCAL_TALKERS)
		return E_INVALIDARG;
	if (pParams->dwMaxRemoteTalkers > XHV_MAX_REMOTE_TALKERS)
		return E_INVALIDARG;
	if (pParams->dwNumLocalTalkerEnabledModes > XHV_MAX_PROCESSING_MODES)
		return E_INVALIDARG;
	if (pParams->dwNumRemoteTalkerEnabledModes > XHV_MAX_PROCESSING_MODES)
		return E_INVALIDARG;
	if (!pParams->dwNumLocalTalkerEnabledModes)
		return E_INVALIDARG;
	if (!pParams->dwMaxRemoteTalkers && pParams->dwNumRemoteTalkerEnabledModes)
		return E_INVALIDARG;
	if (pParams->dwMaxRemoteTalkers && !pParams->dwNumRemoteTalkerEnabledModes)
		return E_INVALIDARG;
	if (pParams->dwNumLocalTalkerEnabledModes > 0 && !pParams->localTalkerEnabledModes)
		return E_INVALIDARG;
	if (pParams->dwNumRemoteTalkerEnabledModes > 0 && !pParams->remoteTalkerEnabledModes)
		return E_INVALIDARG;
	if (pParams->bCustomVADProvided && !pParams->pfnMicrophoneRawDataReady)
		return E_INVALIDARG;
	if (!pParams->hwndFocus)
		return E_INVALIDARG;

	return E_FAIL; // disable live voice
}

// #5314
INT WINAPI XUserMuteListQuery(DWORD dwUserIndex, XUID XuidRemoteTalker, BOOL *pfOnMuteList)
{
	TRACE_FX();
	*pfOnMuteList = FALSE;
	return S_OK;
}
