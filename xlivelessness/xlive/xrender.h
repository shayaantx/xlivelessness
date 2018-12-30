#pragma once

extern BOOL Initialised_XRender;
extern DWORD xlive_fps_limit;
INT InitXRender(XLIVE_INITIALIZE_INFO* pPii);
INT UninitXRender();
