#include <winsock2.h>
#include "xdefs.h"
#include "../xlln/DebugText.h"

// #69
INT WINAPI XNetQosListen(XNKID *pxnkid, PBYTE pb, UINT cb, DWORD dwBitsPerSec, DWORD dwFlags)
{
	TRACE_FX();
	return ERROR_FUNCTION_FAILED;
	//DO STUFF

	return S_OK;
}

// #70
DWORD WINAPI XNetQosLookup(UINT cxna, XNADDR * apxna[], XNKID * apxnkid[], XNKEY * apxnkey[], UINT cina, IN_ADDR aina[], DWORD adwServiceId[], UINT cProbes, DWORD dwBitsPerSec, DWORD dwFlags, WSAEVENT hEvent, XNQOS** pxnqos)
{
	TRACE_FX();
	return ERROR_FUNCTION_FAILED;

	//void ClientQoSLookUp(UINT cxna, XNADDR* apxna[],UINT cProbes,IN_ADDR  aina[], XNQOS** pxnqos,DWORD dwBitsPerSec)

	//XNADDR **axpna_copy = (XNADDR**)malloc(cxna * sizeof(XNADDR*));
	XNADDR** axpna_copy = (XNADDR**)malloc(cxna * sizeof(XNADDR*));

	for (unsigned int i = 0; i < cxna; i++)
	{
		XNADDR* xn = apxna[i];
		axpna_copy[i] = new XNADDR;
		memcpy(axpna_copy[i], xn, sizeof(XNADDR));
	}


	*pxnqos = (XNQOS*)malloc(sizeof(XNQOS) + (sizeof(XNQOSINFO) * (cxna - 1)));

	XNQOS* pqos = &**pxnqos;
	ZeroMemory(pqos, sizeof(XNQOS) + (sizeof(XNQOSINFO)*(cxna - 1)));

	pqos->cxnqosPending = cxna;
	pqos->cxnqos = cxna;
	pqos->axnqosinfo[0].wRttMedInMsecs = 500;
	pqos->axnqosinfo[0].wRttMinInMsecs = 100;
	pqos->axnqosinfo[0].dwDnBitsPerSec = dwBitsPerSec;
	pqos->axnqosinfo[0].dwUpBitsPerSec = dwBitsPerSec;
	pqos->axnqosinfo[0].bFlags = XNET_XNQOSINFO_TARGET_CONTACTED | XNET_XNQOSINFO_COMPLETE;



	/*
	This is gonna hit some CPUs hard when there's a lot of servers on the list, we'll probably want to queue this a bit and only allow X number of threads to run at a time.
	We want to abuse the CPU where possible considering more modern systems will have decent CPUs so we'll be able to force things to happen faster but still want to keep compatibility with older setups.
	*/

	//std::thread(ClientQoSLookUp, cxna, axpna_copy, cProbes, aina, pxnqos, dwBitsPerSec, pqos).detach();


	/* Memory Leak  - FIX ME! (Need to do some kind of garbage collection somewhere and store data like this in an array to be cleared later */
	/*
	*pxnqos = new XNQOS;
	XNQOS* pqos = *pxnqos;
	pqos->cxnqos = 1;
	pqos->cxnqosPending = 0;
	memset(pqos->axnqosinfo, 0x00, sizeof(XNQOSINFO));
	pqos->axnqosinfo->bReserved = 0;
	pqos->axnqosinfo->cProbesXmit = cProbes;
	pqos->axnqosinfo->cProbesRecv = cProbes;

	if (cProbes > 0)
	{
	pqos->axnqosinfo->wRttMedInMsecs = 5;
	pqos->axnqosinfo->wRttMinInMsecs = 10;
	pqos->axnqosinfo->dwUpBitsPerSec = dwBitsPerSec;
	pqos->axnqosinfo->dwDnBitsPerSec = dwBitsPerSec;
	pqos->axnqosinfo->bFlags = XNET_XNQOSINFO_TARGET_CONTACTED | XNET_XNQOSINFO_COMPLETE | XNET_XNQOSINFO_DATA_RECEIVED;
	}
	else
	{
	pqos->axnqosinfo->wRttMedInMsecs = 5;
	pqos->axnqosinfo->wRttMinInMsecs = 10;
	pqos->axnqosinfo->dwUpBitsPerSec = dwBitsPerSec;
	pqos->axnqosinfo->dwDnBitsPerSec = dwBitsPerSec;
	pqos->axnqosinfo->bFlags = XNET_XNQOSINFO_TARGET_CONTACTED | XNET_XNQOSINFO_COMPLETE;

	}*/

	return S_OK;
}

// #72
INT WINAPI XNetQosRelease(XNQOS* pxnqos)
{
	TRACE_FX();
	return ERROR_FUNCTION_FAILED;
	for (unsigned int i = 0; i == pxnqos->cxnqos; i++)
	{
		if (pxnqos->axnqosinfo[i].cbData > 0)
			delete[] pxnqos->axnqosinfo[i].pbData;

		delete[] & pxnqos->axnqosinfo[i];
		//XNQOSINFO *xnqos = &pqos->axnqosinfo[ ( pqos->cxnqosPending - 1) ];

	}
	// We need to clean-up all XNetQoSLookup data here, listener data should be cleaned up inside the Listen function Only.
	return S_OK;
}
