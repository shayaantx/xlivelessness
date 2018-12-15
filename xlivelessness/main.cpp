#include "windows.h"
#include "xlln/xlln.h"
#include <iostream>

static UINT xlln_hinstLib_len = 0;
static UINT xlln_hinstLib_buflen = 0;
static HINSTANCE* xlln_hinstLib = NULL;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
		if (InitXLLN(hModule))
			return FALSE;

		// Load all additional modules.
		WIN32_FIND_DATA data;
		HANDLE hFind = FindFirstFile("./xlln/modules/*.dll", &data);
		if (hFind != INVALID_HANDLE_VALUE) {
			xlln_hinstLib_buflen += 10;
			xlln_hinstLib = (HINSTANCE*)malloc(sizeof(HINSTANCE) * xlln_hinstLib_buflen);
			do {
				int dll_path_len = 18 + strlen(data.cFileName);
				char* dll_path = (char*)malloc(sizeof(char) * dll_path_len);
				snprintf(dll_path, dll_path_len, "./xlln/modules/%s", data.cFileName);
				HINSTANCE hinstLib = LoadLibrary(dll_path);
				if (hinstLib) {
					if (xlln_hinstLib_len >= xlln_hinstLib_buflen) {
						xlln_hinstLib_buflen += 10;
						xlln_hinstLib = (HINSTANCE*)realloc(xlln_hinstLib, sizeof(HINSTANCE) * xlln_hinstLib_buflen);
					}
					xlln_hinstLib[xlln_hinstLib_len++] = hinstLib;
				}
				free(dll_path);
			} while (FindNextFile(hFind, &data));
			FindClose(hFind);
		}
	}
	else if (ul_reason_for_call == DLL_PROCESS_DETACH) {
		if (UninitXLLN())
			return FALSE;

		// Free all additional modules.
		if (xlln_hinstLib) {
			unsigned int i = xlln_hinstLib_len;
			if (i) {
				// Iterate backward and free all modules (in case order is important with undoing hooks for example).
				do {
					FreeLibrary(xlln_hinstLib[--i]);
				} while (i > 0);
			}
			free(xlln_hinstLib);
			xlln_hinstLib = NULL;
			xlln_hinstLib_buflen = xlln_hinstLib_len = 0;
		}
	}
	return TRUE;
}
