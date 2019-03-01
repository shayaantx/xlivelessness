#pragma once

typedef struct {
	DWORD dwViewId;
	DWORD dwNumProperties;
	XUSER_PROPERTY *pProperties;
} XSESSION_VIEW_PROPERTIES;

int WINAPI XInviteSend(DWORD userIndex, DWORD invites, const XUID* xuidInvites, const WCHAR* invitationText, XOVERLAPPED *inviteOperation);
DWORD WINAPI XSessionModifySkill(HANDLE session, DWORD xuidCount, const XUID* xuids, PXOVERLAPPED modifySkillOperation);
DWORD XSessionModify(HANDLE hSession, DWORD dwFlags, DWORD dwMaxPublicSlots, DWORD dwMaxPrivateSlots, PXOVERLAPPED modifySessionOperation);
DWORD XSessionFlushStats(HANDLE hSession, XOVERLAPPED *pXOverlapped);

DWORD XSessionArbitrationRegister(
	HANDLE hSession,
	DWORD dwFlags,
	ULONGLONG qwSessionNonce,
	DWORD *pcbResultsBuffer,
	PXSESSION_REGISTRATION_RESULTS pRegistrationResults,
	PXOVERLAPPED pXOverlapped
);

DWORD XShowKeyboardUI(
	DWORD dwUserIndex,
	DWORD dwFlags,
	LPCWSTR wseDefaultText,
	LPCWSTR wszTitleText,
	LPCWSTR wszDescriptionText,
	LPWSTR wszResultText,
	DWORD cchResultText,
	PXOVERLAPPED pOverlapped
);

DWORD XSessionSearch(
	DWORD dwProcedureIndex,
	DWORD dwUserIndex,
	DWORD dwNumResults,
	WORD wNumProperties,
	WORD wNumContexts,
	PXUSER_PROPERTY pSearchProperties,
	PXUSER_CONTEXT pSearchContexts,
	DWORD *pcbResultsBuffer,
	PXSESSION_SEARCHRESULT_HEADER pSearchResults,
	PXOVERLAPPED pXOverlapped
);

DWORD XSessionSearchByID(
	XNKID sessionID,
	DWORD dwUserIndex,
	DWORD *pcbResultsBuffer,
	PXSESSION_SEARCHRESULT_HEADER pSearchResults,
	PXOVERLAPPED pXOverlapped
);

DWORD XSessionWriteStats(
	HANDLE hSession,
	XUID xuid,
	DWORD dwNumViews,
	CONST XSESSION_VIEW_PROPERTIES *pViews,
	XOVERLAPPED *pXOverlapped
);