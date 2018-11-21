#include "windows.h"
#include "xlln.h"
#include "DebugText.h"
#include "../xlive/xdefs.h"
#include "../xlive/xlive.h"

static LRESULT CALLBACK DLLWindowProc(HWND, UINT, WPARAM, LPARAM);

static HINSTANCE xlln_hModule = NULL;
HWND xlln_window_hwnd = NULL;
static HMENU xlln_window_hMenu = NULL;

static INT xlln_login_player = 0;
static INT xlln_login_player_h[] = { MYMENU_LOGIN1, MYMENU_LOGIN2, MYMENU_LOGIN3, MYMENU_LOGIN4 };

//Register the windows Class
static BOOL RegisterDLLWindowClass(HINSTANCE hModule, wchar_t szClassName[])
{
	WNDCLASSEXW wc;
	wc.hInstance = hModule;
	wc.lpszClassName = (LPCWSTR)szClassName;
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
	return TRUE;
}

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
	BOOL debug = TRUE;
	HINSTANCE hModule = reinterpret_cast<HINSTANCE>(lpParam);
	xlln_window_hMenu = CreateDLLWindowMenu(hModule);
	RegisterDLLWindowClass(hModule, L"XLLNDLLWindowClass");
	HWND hwdParent = NULL;// FindWindowW(L"Window Injected Into ClassName", L"Window Injected Into Caption");
	xlln_window_hwnd = CreateWindowExW(0, L"XLLNDLLWindowClass", L"XLLN", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 400, 600, hwdParent, xlln_window_hMenu, hModule, NULL);
	ShowWindow(xlln_window_hwnd, debug ? SW_NORMAL : SW_HIDE);

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
	return 1;
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
			exit(0);
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
			MessageBox(hwnd, "Created by Glitchy Scripts,\nwith thanks to PermaNulled.", "About", MB_OK);
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
		else if (wParam == MYWINDOW_BTN_LOGIN || wParam == MYWINDOW_BTN_LOGOUT) {

			if (wParam == MYWINDOW_BTN_LOGIN) {
				char jlbuffer[16];
				GetDlgItemText(hwnd, MYWINDOW_TBX_USERNAME, jlbuffer, 16);
				SetDlgItemText(hwnd, MYWINDOW_TBX_USERNAME, jlbuffer);

				xlive_users_info[xlln_login_player]->UserSigninState = IsDlgButtonChecked(xlln_window_hwnd, MYWINDOW_CHK_LIVEENABLE) == BST_CHECKED ? eXUserSigninState_SignedInToLive : eXUserSigninState_SignedInLocally;
				//0x0009000000000000 - online xuid?
				xlive_users_info[xlln_login_player]->xuid = 0xE000007300000000 + xlln_login_player;
				strncpy_s(xlive_users_info[xlln_login_player]->szUserName, XUSER_NAME_SIZE, jlbuffer, XUSER_NAME_SIZE);
				xlive_users_info[xlln_login_player]->dwInfoFlags;
			}
			else {
				xlive_users_info[xlln_login_player]->UserSigninState = eXUserSigninState_NotSignedIn;
			}

			xlive_users_info_changed[xlln_login_player] = TRUE;

			BOOL checked = GetMenuState(xlln_window_hMenu, xlln_login_player_h[xlln_login_player], 0) != MF_CHECKED;
			CheckMenuItem(xlln_window_hMenu, xlln_login_player_h[xlln_login_player], checked ? MF_CHECKED : MF_UNCHECKED);

			ShowWindow(GetDlgItem(xlln_window_hwnd, MYWINDOW_BTN_LOGIN), checked ? SW_HIDE : SW_SHOWNORMAL);
			ShowWindow(GetDlgItem(xlln_window_hwnd, MYWINDOW_BTN_LOGOUT), checked ? SW_SHOWNORMAL : SW_HIDE);
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

		CreateWindowA("edit", "Username", WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP,
			10, 10, 220, 22, hwnd, (HMENU)MYWINDOW_TBX_USERNAME, xlln_hModule, NULL);

		hWndControl = CreateWindowA("button", "Live Enabled", BS_CHECKBOX | WS_CHILD | WS_VISIBLE | WS_TABSTOP,
			10, 40, 110, 22, hwnd, (HMENU)MYWINDOW_CHK_LIVEENABLE, xlln_hModule, NULL);

		CreateWindowA("button", "Login", WS_CHILD | WS_VISIBLE | WS_TABSTOP,
			10, 70, 75, 25, hwnd, (HMENU)MYWINDOW_BTN_LOGIN, xlln_hModule, NULL);

		CreateWindowA("button", "Logout", WS_CHILD | WS_TABSTOP,
			10, 70, 75, 25, hwnd, (HMENU)MYWINDOW_BTN_LOGOUT, xlln_hModule, NULL);

		hWndControl = CreateWindowA("edit", "output", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | WS_SIZEBOX | WS_TABSTOP,
			10, 120, 350, 400, hwnd, (HMENU)MYWINDOW_TBX_TEST, xlln_hModule, NULL);

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
	initDebugText();

	xlln_hModule = hModule;
	CreateThread(0, NULL, ThreadProc, (LPVOID)hModule, NULL, NULL);
	return 0;
}

INT UninitXLLN()
{
	return 0;
}
