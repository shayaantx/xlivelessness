#include "windows.h"
#include "xlln.h"
#include "DebugText.h"
#include "../xlive/xdefs.h"
#include "../xlive/xlive.h"
#include "../xlive/xlocator.h"
#include "../xlive/xrender.h"
#include "../xlive/xsocket.h"
#include "RandName.h"
#include "../resource.h"
#include <string>
#include <time.h>

static LRESULT CALLBACK DLLWindowProc(HWND, UINT, WPARAM, LPARAM);

static HINSTANCE xlln_hModule = NULL;
HWND xlln_window_hwnd = NULL;
static HMENU xlln_window_hMenu = NULL;
static int xlln_instance = 0;

static INT xlln_login_player = 0;
static INT xlln_login_player_h[] = { MYMENU_LOGIN1, MYMENU_LOGIN2, MYMENU_LOGIN3, MYMENU_LOGIN4 };

BOOL xlln_debug = FALSE;

static HMENU CreateDLLWindowMenu(HINSTANCE hModule)
{
	HMENU hMenu;
	hMenu = CreateMenu();
	HMENU hMenuPopup;
	if (hMenu == NULL)
		return FALSE;

	hMenuPopup = CreatePopupMenu();
	AppendMenu(hMenuPopup, MF_STRING, MYMENU_EXIT, TEXT("Exit"));
	AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hMenuPopup, TEXT("File"));

	hMenuPopup = CreatePopupMenu();
	AppendMenu(hMenuPopup, MF_UNCHECKED, MYMENU_LOGIN1, TEXT("Login P1"));
	AppendMenu(hMenuPopup, MF_UNCHECKED, MYMENU_LOGIN2, TEXT("Login P2"));
	AppendMenu(hMenuPopup, MF_UNCHECKED, MYMENU_LOGIN3, TEXT("Login P3"));
	AppendMenu(hMenuPopup, MF_UNCHECKED, MYMENU_LOGIN4, TEXT("Login P4"));
	AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hMenuPopup, TEXT("Login"));

	hMenuPopup = CreatePopupMenu();
	AppendMenu(hMenuPopup, MF_UNCHECKED, MYMENU_ALWAYSTOP, TEXT("Always on top"));
	AppendMenu(hMenuPopup, MF_STRING, MYMENU_ABOUT, TEXT("About"));
	AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hMenuPopup, TEXT("Help"));

	EnableMenuItem(hMenu, MYMENU_LOGIN2, MF_GRAYED);
	EnableMenuItem(hMenu, MYMENU_LOGIN3, MF_GRAYED);
	EnableMenuItem(hMenu, MYMENU_LOGIN4, MF_GRAYED);


	return hMenu;
}

static DWORD WINAPI ThreadProc(LPVOID lpParam)
{
	srand((unsigned int)time(NULL));

	const wchar_t* windowclassname = L"XLLNDLLWindowClass";
	HINSTANCE hModule = reinterpret_cast<HINSTANCE>(lpParam);
	xlln_window_hMenu = CreateDLLWindowMenu(hModule);

	// Register the windows Class.
	WNDCLASSEXW wc;
	wc.hInstance = hModule;
	wc.lpszClassName = windowclassname;
	wc.lpfnWndProc = DLLWindowProc;
	wc.style = CS_DBLCLKS;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpszMenuName = NULL;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hbrBackground = (HBRUSH)COLOR_BACKGROUND;
	if (!RegisterClassExW(&wc))
		return FALSE;

	wchar_t title[80];
	swprintf_s(title, 80, L"XLLN v%d.%d.%d.%d", DLL_VERSION);

	HWND hwdParent = NULL;// FindWindowW(L"Window Injected Into ClassName", L"Window Injected Into Caption");
	xlln_window_hwnd = CreateWindowExW(0, windowclassname, title, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 400, xlln_debug ? 700 : 165, hwdParent, xlln_window_hMenu, hModule, NULL);
	ShowWindow(xlln_window_hwnd, xlln_debug ? SW_NORMAL : SW_HIDE);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		// Handle tab ordering
		if (!IsDialogMessage(xlln_window_hwnd, &msg)) {
			// Translate virtual-key msg into character msg
			TranslateMessage(&msg);
			// Send msg to WindowProcedure(s)
			DispatchMessage(&msg);
		}
	}
	return TRUE;
}


// #41140
DWORD WINAPI XLLNLogin(DWORD dwUserIndex, BOOL bLiveEnabled, DWORD dwUserId, const CHAR *szUsername)
{
	TRACE_FX();
	if (dwUserIndex >= XLIVE_LOCAL_USER_COUNT)
		return ERROR_NO_SUCH_USER;
	if (szUsername && (!*szUsername || strlen(szUsername) > XUSER_MAX_NAME_LENGTH))
		return ERROR_INVALID_ACCOUNT_NAME;
	if (xlive_users_info[dwUserIndex]->UserSigninState != eXUserSigninState_NotSignedIn)
		return ERROR_ALREADY_ASSIGNED;

	if (!dwUserId) {
		//Generate Random Number?
		dwUserId = rand();
	}
	if (szUsername) {
		strncpy_s(xlive_users_info[dwUserIndex]->szUserName, XUSER_NAME_SIZE, szUsername, XUSER_NAME_SIZE);
	}
	else {
		wchar_t generated_name[XUSER_NAME_SIZE];
		GetName(generated_name, XUSER_NAME_SIZE);
		snprintf(xlive_users_info[dwUserIndex]->szUserName, XUSER_NAME_SIZE, "%ls", generated_name);
	}

	xlive_users_info[dwUserIndex]->UserSigninState = bLiveEnabled ? eXUserSigninState_SignedInToLive : eXUserSigninState_SignedInLocally;
	//0x0009000000000000 - online xuid?
	xlive_users_info[dwUserIndex]->xuid = 0xE000007300000000 + dwUserId;
	xlive_users_info[dwUserIndex]->dwInfoFlags;

	xlive_users_info_changed[dwUserIndex] = TRUE;

	SetDlgItemText(xlln_window_hwnd, MYWINDOW_TBX_USERNAME, xlive_users_info[dwUserIndex]->szUserName);
	CheckDlgButton(xlln_window_hwnd, MYWINDOW_CHK_LIVEENABLE, bLiveEnabled ? BST_CHECKED : BST_UNCHECKED);

	BOOL checked = TRUE;//GetMenuState(xlln_window_hMenu, xlln_login_player_h[xlln_login_player], 0) != MF_CHECKED;
	CheckMenuItem(xlln_window_hMenu, xlln_login_player_h[xlln_login_player], checked ? MF_CHECKED : MF_UNCHECKED);

	ShowWindow(GetDlgItem(xlln_window_hwnd, MYWINDOW_BTN_LOGIN), checked ? SW_HIDE : SW_SHOWNORMAL);
	ShowWindow(GetDlgItem(xlln_window_hwnd, MYWINDOW_BTN_LOGOUT), checked ? SW_SHOWNORMAL : SW_HIDE);

	return ERROR_SUCCESS;
}

// #41141
DWORD WINAPI XLLNLogout(DWORD dwUserIndex)
{
	TRACE_FX();
	if (dwUserIndex >= XLIVE_LOCAL_USER_COUNT)
		return ERROR_NO_SUCH_USER;
	if (xlive_users_info[dwUserIndex]->UserSigninState == eXUserSigninState_NotSignedIn)
		return ERROR_NOT_LOGGED_ON;

	xlive_users_info[dwUserIndex]->UserSigninState = eXUserSigninState_NotSignedIn;
	xlive_users_info_changed[dwUserIndex] = TRUE;

	BOOL checked = FALSE;//GetMenuState(xlln_window_hMenu, xlln_login_player_h[xlln_login_player], 0) != MF_CHECKED;
	CheckMenuItem(xlln_window_hMenu, xlln_login_player_h[xlln_login_player], checked ? MF_CHECKED : MF_UNCHECKED);

	ShowWindow(GetDlgItem(xlln_window_hwnd, MYWINDOW_BTN_LOGIN), checked ? SW_HIDE : SW_SHOWNORMAL);
	ShowWindow(GetDlgItem(xlln_window_hwnd, MYWINDOW_BTN_LOGOUT), checked ? SW_SHOWNORMAL : SW_HIDE);

	return ERROR_SUCCESS;
}

static LRESULT CALLBACK DLLWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_PAINT) {
		/*PAINTSTRUCT ps;
		HDC hdc = BeginPaint(xlln_window_hwnd, &ps);
		TCHAR greeting[] = TEXT("Hello, World!");
		TextOut(hdc, 5, 5, greeting, strlen(greeting));

		EndPaint(xlln_window_hwnd, &ps);*/
	}
	else if (message == WM_SYSCOMMAND) {
		if (wParam == SC_CLOSE) {
			ShowXLLN(XLLN_SHOW_HIDE);
			return 0;
		}
	}
	else if (message == WM_COMMAND) {
		if (wParam == MYMENU_EXIT) {
			//SendMessage(hwnd, WM_CLOSE, 0, 0);
			LiveOverLanAbort();
			exit(EXIT_SUCCESS);
		}
		else if (wParam == MYMENU_ALWAYSTOP) {
			BOOL checked = GetMenuState(xlln_window_hMenu, MYMENU_ALWAYSTOP, 0) != MF_CHECKED;
			if (checked) {
				SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
			}
			else {
				SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
			}
			CheckMenuItem(xlln_window_hMenu, MYMENU_ALWAYSTOP, checked ? MF_CHECKED : MF_UNCHECKED);
		}
		else if (wParam == MYMENU_ABOUT) {
			MessageBox(hwnd,
"Created by Glitchy Scripts,\n\
with thanks to PermaNulled.\n\
\n\
Executable Launch Parameters:\n\
-xlivefps=<uint> ? 0 to disable fps limiter.\n\
-xllndebug ? Sleep until debugger attach.\n\
-xllndebuglog ? Enable debug log.\n\
-xlivedebug ? Sleep XLiveInitialize until debugger attach.\n\
-xlivenetdisable ? Disable all network functionality."
				, "About", MB_OK);
		}
		else if (wParam == MYMENU_LOGIN1) {
			xlln_login_player = 0;
		}
		else if (wParam == MYMENU_LOGIN2) {
			xlln_login_player = 1;
		}
		else if (wParam == MYMENU_LOGIN3) {
			xlln_login_player = 2;
		}
		else if (wParam == MYMENU_LOGIN4) {
			xlln_login_player = 3;
		}
		else if (wParam == MYWINDOW_CHK_LIVEENABLE) {
			BOOL checked = IsDlgButtonChecked(xlln_window_hwnd, MYWINDOW_CHK_LIVEENABLE) != BST_CHECKED;
			CheckDlgButton(xlln_window_hwnd, MYWINDOW_CHK_LIVEENABLE, checked ? BST_CHECKED : BST_UNCHECKED);
		}
		else if (wParam == MYWINDOW_BTN_LOGIN) {
			char jlbuffer[16];
			GetDlgItemText(xlln_window_hwnd, MYWINDOW_TBX_USERNAME, jlbuffer, 16);
			SetDlgItemText(xlln_window_hwnd, MYWINDOW_TBX_USERNAME, jlbuffer);

			BOOL live_enabled = IsDlgButtonChecked(xlln_window_hwnd, MYWINDOW_CHK_LIVEENABLE) == BST_CHECKED;
			DWORD result_login = XLLNLogin(xlln_login_player, live_enabled, NULL, strnlen_s(jlbuffer, 16) == 0 ? NULL : jlbuffer);
		}
		else if (wParam == MYWINDOW_BTN_LOGOUT) {
			DWORD result_logout = XLLNLogout(xlln_login_player);
		}
		return 0;
	}
	else if (message == WM_CTLCOLORSTATIC) {
		HDC hdc = reinterpret_cast<HDC>(wParam);
		SetTextColor(hdc, RGB(0, 0, 0));
		SetBkColor(hdc, 0x00C8C8C8);
		return (INT_PTR)CreateSolidBrush(0x00C8C8C8);
	}
	else if (message == WM_CREATE) {

		HWND hWndControl;

		CreateWindowA("edit", "", WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP,
			10, 10, 260, 22, hwnd, (HMENU)MYWINDOW_TBX_USERNAME, xlln_hModule, NULL);

		hWndControl = CreateWindowA("button", "Live Enabled", BS_CHECKBOX | WS_CHILD | WS_VISIBLE | WS_TABSTOP,
			10, 40, 150, 22, hwnd, (HMENU)MYWINDOW_CHK_LIVEENABLE, xlln_hModule, NULL);

		CreateWindowA("button", "Login", WS_CHILD | WS_VISIBLE | WS_TABSTOP,
			10, 70, 75, 25, hwnd, (HMENU)MYWINDOW_BTN_LOGIN, xlln_hModule, NULL);

		CreateWindowA("button", "Logout", WS_CHILD | WS_TABSTOP,
			10, 70, 75, 25, hwnd, (HMENU)MYWINDOW_BTN_LOGOUT, xlln_hModule, NULL);

		hWndControl = CreateWindowA("edit", "", WS_CHILD | (xlln_debug ? WS_VISIBLE : 0) | WS_BORDER | ES_MULTILINE | WS_SIZEBOX | WS_TABSTOP | WS_HSCROLL,
			10, 120, 350, 500, hwnd, (HMENU)MYWINDOW_TBX_TEST, xlln_hModule, NULL);

	}
	else if (message == WM_DESTROY) {
		PostQuitMessage(0);
		return 0;
	}
	else if (message == WM_CLOSE) {
		// Stupid textbox causes the window to close.
		return 0;
	}
	
	return DefWindowProcW(hwnd, message, wParam, lParam);
}

INT ShowXLLN(DWORD dwShowType)
{
	if (dwShowType == XLLN_SHOW_HIDE) {
		ShowWindow(xlln_window_hwnd, SW_HIDE);
	}
	else if (dwShowType == XLLN_SHOW_HOME) {
		ShowWindow(xlln_window_hwnd, SW_SHOWNORMAL);
		SetWindowPos(xlln_window_hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	}
	else if (dwShowType == XLLN_SHOW_LOGIN) {
		ShowWindow(xlln_window_hwnd, SW_SHOWNORMAL);
		SetWindowPos(xlln_window_hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		SendMessage(xlln_window_hwnd, WM_NEXTDLGCTL, (WPARAM)GetDlgItem(xlln_window_hwnd, MYWINDOW_TBX_USERNAME), TRUE);
	}
	return 0;
}

INT InitXLLN(HMODULE hModule)
{
	BOOL xlln_debug_pause = FALSE;

	int nArgs;
	LPWSTR* lpwszArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);
	if (lpwszArglist != NULL) {
		for (int i = 1; i < nArgs; i++) {
			if (wcsstr(lpwszArglist[i], L"-xlivefps=") != NULL) {
				DWORD tempuint = 0;
				if (swscanf_s(lpwszArglist[i], L"-xlivefps=%u", &tempuint) == 1) {
					xlive_fps_limit = tempuint;
				}
			}
			else if (wcscmp(lpwszArglist[i], L"-xllndebug") == 0) {
				xlln_debug_pause = TRUE;
			}
			else if (wcscmp(lpwszArglist[i], L"-xllndebuglog") == 0) {
				xlln_debug = TRUE;
			}
			else if (wcscmp(lpwszArglist[i], L"-xlivedebug") == 0) {
				xlive_debug_pause = TRUE;
			}
			else if (wcscmp(lpwszArglist[i], L"-xlivenetdisable") == 0) {
				xlive_netsocket_abort = TRUE;
			}
		}
	}
	LocalFree(lpwszArglist);

	while (xlln_debug_pause && !IsDebuggerPresent())
		Sleep(500L);

	wchar_t mutex_name[40];
	DWORD mutex_last_error;
	HANDLE mutex = NULL;
	do {
		if (mutex)
			mutex_last_error = CloseHandle(mutex);
		xlln_instance++;
		swprintf(mutex_name, 40, L"Global\\XLLNInstance#%d", xlln_instance);
		mutex = CreateMutexW(0, FALSE, mutex_name);
		mutex_last_error = GetLastError();
	} while (mutex_last_error != ERROR_SUCCESS);

	if (xlln_debug)
		initDebugText(xlln_instance);

	xlln_hModule = hModule;
	CreateThread(0, NULL, ThreadProc, (LPVOID)hModule, NULL, NULL);
	return 0;
}

INT UninitXLLN()
{
	return 0;
}
