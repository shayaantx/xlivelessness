#include "xdefs.h"
#include "xrender.h"
#include "../xlln/DebugText.h"
#include <d3d9.h>
#include <chrono>
#include <thread>

BOOL Initialised_XRender = FALSE;

static LPDIRECT3DDEVICE9 pDevice;
static D3DPRESENT_PARAMETERS *pD3DPP;

DWORD xlive_fps_limit = 60;

static std::chrono::system_clock::time_point nextFrame;
static std::chrono::system_clock::duration desiredRenderTime;

INT InitXRender(XLIVE_INITIALIZE_INFO* pPii)
{
	TRACE_FX();
	pDevice = (LPDIRECT3DDEVICE9)pPii->pD3D;
	pD3DPP = (D3DPRESENT_PARAMETERS*)pPii->pD3DPP;

	SetFPSLimit(xlive_fps_limit);

	Initialised_XRender = TRUE;
	return S_OK;
}

INT UninitXRender()
{
	TRACE_FX();
	return S_OK;
}

VOID SetFPSLimit(DWORD fps_limit)
{
	xlive_fps_limit = fps_limit;
	nextFrame = std::chrono::system_clock::now();
	desiredRenderTime = std::chrono::duration_cast<std::chrono::system_clock::duration>(std::chrono::duration<double>(1.0 / (double)fps_limit));
}

void frameTimeManagement()
{
	std::this_thread::sleep_until(nextFrame);
	do {
		nextFrame += desiredRenderTime;
	} while (std::chrono::system_clock::now() > nextFrame);
}

// #5002
INT WINAPI XLiveRender()
{
	TRACE_FX();
	if (Initialised_XRender) {
		if (xlive_fps_limit > 0) {
			frameTimeManagement();
		}
	}
	return S_OK;
}
