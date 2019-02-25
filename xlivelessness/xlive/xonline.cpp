#include "xdefs.h"
#include "xonline.h"
#include "../xlln/DebugText.h"

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