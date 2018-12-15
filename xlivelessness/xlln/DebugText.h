#pragma once
#ifndef DEBUGTEXT
#define DEBUGTEXT

int getDebugTextArrayMaxLen();
void addDebugText(char* text);
void addDebugText(const char* text);
void addDebugText(wchar_t* wtext);
char* getDebugText(int ordered_index);
void initDebugText(DWORD dwInstanceId);

void setDebugTextDisplay(bool setOn);
bool getDebugTextDisplay();

void trace_func(const char *fxname);

#define TRACE_FX() \
    trace_func(__func__)

#endif