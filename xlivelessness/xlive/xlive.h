#pragma once

#define XLIVE_LOCAL_USER_COUNT 4

extern BOOL xlive_users_info_changed[XLIVE_LOCAL_USER_COUNT];
extern XUSER_SIGNIN_INFO* xlive_users_info[XLIVE_LOCAL_USER_COUNT];

void Check_Overlapped(PXOVERLAPPED pOverlapped);
