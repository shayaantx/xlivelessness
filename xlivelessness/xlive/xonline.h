#pragma once

int WINAPI XInviteSend(DWORD userIndex, DWORD invites, const XUID* xuidInvites, const WCHAR* invitationText, XOVERLAPPED *inviteOperation);
DWORD WINAPI XSessionModifySkill(HANDLE session, DWORD xuidCount, const XUID* xuids, PXOVERLAPPED modifySkillOperation);
DWORD XSessionModify(HANDLE hSession, DWORD dwFlags, DWORD dwMaxPublicSlots, DWORD dwMaxPrivateSlots, PXOVERLAPPED modifySessionOperation);
DWORD XSessionFlushStats(HANDLE hSession, XOVERLAPPED *pXOverlapped);
