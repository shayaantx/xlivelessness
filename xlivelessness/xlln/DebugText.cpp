#include "../xlive/xdefs.h"
#include "DebugText.h"
#include <string>
#include "../xlln/xlln.h"
#include "../xlive/xsocket.h"


static char** DebugStr;
static int DebugTextArrayLenMax = 160;
static int DebugTextArrayPos = 0;
static bool DebugTextDisplay = false;
static FILE* debugFile = NULL;
static bool initialisedDebugText = false;

static const int blacklistlen = 11;
static const char* blacklist[blacklistlen] = { "XLiveRender", "XNetGetEthernetLinkStatus", "XLiveInput", "XLivePreTranslateMessage", "XLivePBufferGetByte", "XSocketWSAGetLastError", "XSocketRecvFrom", "XNotifyGetNext", "XUserCheckPrivilege", "XNetGetConnectStatus", "XSocketHTONL" };


int getDebugTextArrayMaxLen() {
	return DebugTextArrayLenMax;
}

void addDebugText(wchar_t* wtext) {
	int lenInput = wcslen(wtext);
	char* text = (char*)malloc(sizeof(char) * lenInput + 1);
	snprintf(text, lenInput + 1, "%ls", wtext);
	addDebugText(text);
	free(text);
}

void addDebugText(const char* text) {
	int buflen = strlen(text) + 1;
	char* text2 = (char*)malloc(sizeof(char) * buflen);
	memcpy(text2, text, sizeof(char) * buflen);
	addDebugText(text2);
	free(text2);
}

static CRITICAL_SECTION log_section;

static void addDebugTextHelper(char* text) {
	int lenInput = strlen(text);

	char* endChar = strchr(text, '\n');
	if (endChar) {
		lenInput = endChar - text;
	}

	DebugTextArrayPos++;
	if (DebugTextArrayPos >= DebugTextArrayLenMax) {
		DebugTextArrayPos = 0;
	}

	free(DebugStr[DebugTextArrayPos]);
	DebugStr[DebugTextArrayPos] = (char*)malloc(sizeof(char) * lenInput + 1);
	strncpy(DebugStr[DebugTextArrayPos], text, lenInput);
	memset(DebugStr[DebugTextArrayPos] + lenInput, 0, 1);

	if (debugFile != NULL) {
		char* debug_text = (char*)malloc(sizeof(char) * lenInput + 2);
		strncpy(debug_text, text, lenInput);
		memset(debug_text + lenInput, '\n', 1);
		memset(debug_text + lenInput + 1, 0, 1);
		fputs(debug_text, debugFile);
		fflush(debugFile);
		free(debug_text);
	}

	if (endChar) {
		addDebugText(endChar + 1);
	}
}

static char debug_blarg[1000];

void addDebugText(char* text) {
	if (!initialisedDebugText)
		return;

	EnterCriticalSection(&log_section);
	addDebugTextHelper(text);
	LeaveCriticalSection(&log_section);

	if (xlln_debug) {
		char* iblarg = debug_blarg;
		for (int i = 0; i < 30; i++) {
			iblarg += snprintf(iblarg, 1000, "%s\r\n", getDebugText(i));
		}
		SetDlgItemText(xlln_window_hwnd, MYWINDOW_TBX_TEST, debug_blarg);
	}
	/*if (getDebugTextDisplay()) {
		for (int i = 0; i < getDebugTextArrayMaxLen(); i++) {
			const char* text = getDebugText(i);

		}
	}*/
}

void initDebugText(DWORD dwInstanceId) {
	InitializeCriticalSection(&log_section);
	initialisedDebugText = true;
	DebugStr = (char**)malloc(sizeof(char*) * DebugTextArrayLenMax);
	for (int i = 0; i < DebugTextArrayLenMax; i++) {
		DebugStr[i] = (char*)calloc(1, sizeof(char));
	}
	wchar_t debug_file_path[1024];
	swprintf(debug_file_path, 1024, L"%wsxlln_debug_%d.log", L"./", dwInstanceId);
	debugFile = _wfopen(debug_file_path, L"w");
	char awerg[1034];
	sprintf(awerg, "PATH: %ws", debug_file_path);
	addDebugText(awerg);
	addDebugText("Initialised Debug Logger.");
}

char* getDebugText(int ordered_index) {
	if (initialisedDebugText) {
		EnterCriticalSection(&log_section);
		if (ordered_index < DebugTextArrayLenMax) {
			int array_index = ((DebugTextArrayPos - ordered_index) + DebugTextArrayLenMax) % DebugTextArrayLenMax;
			char* result = DebugStr[array_index];
			LeaveCriticalSection(&log_section);
			//FIXME race condition on string returned before it is free'd if logging too fast.
			return result;
		}
		LeaveCriticalSection(&log_section);
	}
	return const_cast<char*>("");
}

void setDebugTextDisplay(bool setOn) {
	DebugTextDisplay = setOn;
}

bool getDebugTextDisplay() {
	return DebugTextDisplay;
}

//SYSTEMTIME t;
//GetLocalTime(&t);
//fwprintf(log_handle, L"%02d/%02d/%04d %02d:%02d:%02d.%03d ", t.wDay, t.wMonth, t.wYear, t.wHour, t.wMinute, t.wSecond, t.wMilliseconds);

void trace_func(const char *fxname)
{
	for (int i = 0; i < blacklistlen; i++) {
		if (strcmp(fxname, blacklist[i]) == 0)
			return;
	}
	int leng = (30 + strlen(fxname));
	char* guibsig = (char*)malloc(sizeof(char) * leng);
	snprintf(guibsig, leng, "%s()", fxname);
	addDebugText(guibsig);
	free(guibsig);
}
