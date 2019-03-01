#include "xdefs.h"
#include "xonline.h"
#include "../xlln/DebugText.h"
#include "xlive.h"

int WINAPI XInviteSend(DWORD userIndex, DWORD invites, const XUID* xuidInvites, const WCHAR* invitationText, XOVERLAPPED *inviteOperation)
{
	//TODO: when an invite is sent, we should tell the backend, so its state is persisted outside of peer to peer communication
	//TODO: we would need to also implement XNotifyCreateListener such that when they create a listener for events
	//the implementation would talk with the backend for notifications
	TRACE_FX();
	return 0;
}

DWORD WINAPI XSessionModifySkill(HANDLE session, DWORD xuidCount, const XUID* xuids, PXOVERLAPPED modifySkillOperation) 
{
	//TODO: backend needs a session persistence layer, with a field for skill
	TRACE_FX();
	return 0;
}

DWORD XSessionModify(HANDLE hSession, DWORD dwFlags, DWORD dwMaxPublicSlots, DWORD dwMaxPrivateSlots, PXOVERLAPPED modifySessionOperation) 
{
	TRACE_FX();
	return 0;
}

DWORD XSessionFlushStats(HANDLE hSession, XOVERLAPPED *pXOverlapped) 
{
	TRACE_FX();
	return 0;
}

DWORD XSessionArbitrationRegister(
	HANDLE hSession,
	DWORD dwFlags,
	ULONGLONG qwSessionNonce,
	DWORD *pcbResultsBuffer,
	PXSESSION_REGISTRATION_RESULTS pRegistrationResults,
	PXOVERLAPPED pXOverlapped)
{
	TRACE_FX();
	return 0;
}

DWORD XShowKeyboardUI(
	DWORD dwUserIndex,
	DWORD dwFlags,
	LPCWSTR wseDefaultText,
	LPCWSTR wszTitleText,
	LPCWSTR wszDescriptionText,
	LPWSTR wszResultText,
	DWORD cchResultText,
	PXOVERLAPPED pOverlapped
)
{
	TRACE_FX();
	return 0;
}

DWORD XSessionSearch(DWORD dwProcedureIndex, DWORD dwUserIndex, DWORD dwNumResults, WORD wNumProperties, WORD wNumContexts, PXUSER_PROPERTY pSearchProperties, PXUSER_CONTEXT pSearchContexts, DWORD * pcbResultsBuffer, PXSESSION_SEARCHRESULT_HEADER pSearchResults, PXOVERLAPPED pXOverlapped)
{
	TRACE_FX();
	return 0;
}

DWORD XSessionSearchByID(XNKID sessionID, DWORD dwUserIndex, DWORD * pcbResultsBuffer, PXSESSION_SEARCHRESULT_HEADER pSearchResults, PXOVERLAPPED pXOverlapped)
{
	TRACE_FX();
	if (pcbResultsBuffer)
		*pcbResultsBuffer = 0;


	Check_Overlapped(pXOverlapped);

	return 0;
}

DWORD XSessionWriteStats(
	HANDLE hSession,
	XUID xuid,
	DWORD dwNumViews,
	CONST XSESSION_VIEW_PROPERTIES *pViews,
	XOVERLAPPED *pXOverlapped
) 
{
	TRACE_FX();
	Check_Overlapped(pXOverlapped);
	return 0;
}
