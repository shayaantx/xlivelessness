#pragma once

#define XLLN_SHOW_HIDE 0
#define XLLN_SHOW_HOME 1
#define XLLN_SHOW_LOGIN 2

#define MYMENU_EXIT         (WM_APP + 101)
#define MYMENU_ABOUT   (WM_APP + 102)
#define MYMENU_ALWAYSTOP   (WM_APP + 103)
#define MYMENU_LOGIN1   (WM_APP + 110)
#define MYMENU_LOGIN2   (WM_APP + 111)
#define MYMENU_LOGIN3   (WM_APP + 112)
#define MYMENU_LOGIN4   (WM_APP + 113)
#define MYWINDOW_BTN_LOGIN   (WM_APP + 120)
#define MYWINDOW_TBX_USERNAME   (WM_APP + 121)
#define MYWINDOW_TBX_TEST   (WM_APP + 122)
#define MYWINDOW_CHK_LIVEENABLE   (WM_APP + 123)
#define MYWINDOW_BTN_LOGOUT   (WM_APP + 124)

DWORD WINAPI XLLNLogin(DWORD dwUserIndex, BOOL bLiveEnabled, DWORD dwUserId, const CHAR *szUsername);
DWORD WINAPI XLLNLogout(DWORD dwUserIndex);
INT InitXLLN(HMODULE hModule);
INT UninitXLLN();
INT ShowXLLN(DWORD dwShowType);

extern HWND xlln_window_hwnd;
extern BOOL xlln_debug;
