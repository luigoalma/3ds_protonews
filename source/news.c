#include <3ds/ipc.h>
#include <3ds/result.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/types.h>
#include <news.h>
#include <err.h>
#include <memset.h>

#define OS_REMOTE_SESSION_CLOSED MAKERESULT(RL_STATUS,    RS_CANCELED, RM_OS, 26)
#define OS_INVALID_HEADER        MAKERESULT(RL_PERMANENT, RS_WRONGARG, RM_OS, 47)
#define OS_INVALID_IPC_PARAMATER MAKERESULT(RL_PERMANENT, RS_WRONGARG, RM_OS, 48)

static const char* const NEWS_ServiceNames[] = {"news:u", "news:s"};
static __attribute__((section(".data.TerminationFlag"))) bool TerminationFlag = false;

inline static void HandleSRVNotification() {
	u32 id;
	Err_FailedThrow(srvReceiveNotification(&id));
	if (id == 0x100) {
		TerminationFlag = true;
	}
}

static void NEWS_U_IPCSession() {
	u32* cmdbuf = getThreadCommandBuffer();

	switch (cmdbuf[0] >> 16) {
	/*
	// Does prototype News have news:u?
	case 0x1:
	*/
	default:
		cmdbuf[0] = IPC_MakeHeader(0x0, 1, 0);
		cmdbuf[1] = OS_INVALID_HEADER;
	}
}

static void NEWS_S_IPCSession() {
	u32* cmdbuf = getThreadCommandBuffer();

	switch (cmdbuf[0] >> 16) {
	/*
	case 0x1: // unseen, maybe still is non-prototype 0x1?
	case 0x2: // unseen, unknown, stubbed one?
	case 0x3: // unseen, unknown, stubbed or resetnotifications (non-prototype 0x4)?
	*/
	case 0x4: // seems to be non-prototype 0x5
		cmdbuf[0] = IPC_MakeHeader(0x4, 2, 0);
		cmdbuf[1] = 0;
		cmdbuf[2] = 0; // written size, no data
		break;
	// non-prototype of 0x6 may not exist
	case 0x5: // seems to be non-prototype 0x7
		if (!IPC_CompareHeader(cmdbuf[0], 0x5, 2, 2) || !IPC_Is_Desc_Buffer(cmdbuf[3], IPC_BUFFER_R)) {
			cmdbuf[0] = IPC_MakeHeader(0x0, 1, 0);
			cmdbuf[1] = OS_INVALID_IPC_PARAMATER;
			break;
		}
		cmdbuf[0] = IPC_MakeHeader(0x5, 1, 2);
		cmdbuf[1] = 0;
		cmdbuf[2] = cmdbuf[3];
		cmdbuf[3] = cmdbuf[4];
		break;
	/*
	case 0x6: // may be non-prototype 0x8
	case 0x7: // may be non-prototype 0x9
	// non-prototype of 0xA may not exist
	*/
	case 0x8: // seems to be non-prototype 0xB
		if (!IPC_CompareHeader(cmdbuf[0], 0x8, 2, 2) || !IPC_Is_Desc_Buffer(cmdbuf[3], IPC_BUFFER_W)) {
			cmdbuf[0] = IPC_MakeHeader(0x0, 1, 0);
			cmdbuf[1] = OS_INVALID_IPC_PARAMATER;
			break;
		}
		cmdbuf[0] = IPC_MakeHeader(0x8, 2, 2);
		cmdbuf[1] = 0;
		cmdbuf[2] = 0; // written size, no data
		//cmdbuf[3] is the same as current
		//cmdbuf[4] is the same as current
		break;
	case 0x9: // seems to be non-prototype 0xC
		if (!IPC_CompareHeader(cmdbuf[0], 0x9, 2, 2) || !IPC_Is_Desc_Buffer(cmdbuf[3], IPC_BUFFER_W)) {
			cmdbuf[0] = IPC_MakeHeader(0x0, 1, 0);
			cmdbuf[1] = OS_INVALID_IPC_PARAMATER;
			break;
		}
		cmdbuf[0] = IPC_MakeHeader(0x9, 2, 2);
		cmdbuf[1] = 0;
		cmdbuf[2] = 0; // written size, no data
		//cmdbuf[3] is the same as current
		//cmdbuf[4] is the same as current
		break;
	case 0xA: // seems to be non-prototype 0xD
		if (!IPC_CompareHeader(cmdbuf[0], 0xA, 2, 2) || !IPC_Is_Desc_Buffer(cmdbuf[3], IPC_BUFFER_W)) {
			cmdbuf[0] = IPC_MakeHeader(0x0, 1, 0);
			cmdbuf[1] = OS_INVALID_IPC_PARAMATER;
			break;
		}
		cmdbuf[0] = IPC_MakeHeader(0xA, 2, 2);
		cmdbuf[1] = 0;
		cmdbuf[2] = 0; // written size, no data
		//cmdbuf[3] is the same as current
		//cmdbuf[4] is the same as current
		break;
	// unknown if more exist on prototype news
	default:
		cmdbuf[0] = IPC_MakeHeader(0x0, 1, 0);
		cmdbuf[1] = OS_INVALID_HEADER;
	}
}

static inline void initBSS() {
	extern void* __bss_start__;
	extern void* __bss_end__;
	_memset32_aligned(__bss_start__, 0, (size_t)__bss_end__ - (size_t)__bss_start__);
}

void ProtoNewsMain() {
	initBSS();

	const s32 SERVICE_COUNT = 2;
	const s32 INDEX_MAX = SERVICE_COUNT * 2 + 1;
	const s32 REMOTE_SESSION_INDEX = SERVICE_COUNT + 1;

	Handle session_handles[5];

	u32 service_indexes[2];

	s32 handle_count = SERVICE_COUNT + 1;

	Err_FailedThrow(srvInit());

	for (int i = 0; i < SERVICE_COUNT; i++) {
		Err_FailedThrow(srvRegisterService(&session_handles[i + 1], NEWS_ServiceNames[i]));
	}

	Err_FailedThrow(srvEnableNotification(&session_handles[0]));

	Handle target = 0;
	s32 target_index = -1;
	for (;;) {
		s32 index;

		if (!target) {
			if (TerminationFlag && handle_count == REMOTE_SESSION_INDEX) {
				break;
			} else {
				*getThreadCommandBuffer() = 0xFFFF0000;
			}
		}

		Result res = svcReplyAndReceive(&index, session_handles, handle_count, target);
		s32 last_target_index = target_index;
		target = 0;
		target_index = -1;

		if (R_FAILED(res)) {

			if (res != OS_REMOTE_SESSION_CLOSED) {
				Err_Throw(res);
			}

			else if (index == -1) {
				if (last_target_index == -1) {
					Err_Throw(NEWS_CANCELED_RANGE);
				} else {
					index = last_target_index;
				}
			}

			else if (index >= handle_count) {
				Err_Throw(NEWS_CANCELED_RANGE);
			}

			svcCloseHandle(session_handles[index]);
			handle_count--;
			for (s32 i = index - REMOTE_SESSION_INDEX; i < handle_count - REMOTE_SESSION_INDEX; i++) {
				session_handles[REMOTE_SESSION_INDEX + i] = session_handles[REMOTE_SESSION_INDEX + i + 1];
				service_indexes[i] = service_indexes[i + 1];
			}

			continue;
		}

		if (index == 0) {
			HandleSRVNotification();
		}

		else if (index >= 1 && index < REMOTE_SESSION_INDEX) {
			Handle newsession = 0;
			Err_FailedThrow(svcAcceptSession(&newsession, session_handles[index]));

			if (handle_count >= INDEX_MAX) {
				svcCloseHandle(newsession);
				continue;
			}

			session_handles[handle_count] = newsession;
			service_indexes[handle_count - REMOTE_SESSION_INDEX] = index - 1;
			handle_count++;

		} else if (index >= REMOTE_SESSION_INDEX && index < INDEX_MAX) {
			if (service_indexes[handle_count - REMOTE_SESSION_INDEX] == 0) {
				NEWS_U_IPCSession();
			} else {
				NEWS_S_IPCSession();
			}
			target = session_handles[index];
			target_index = index;

		} else {
			Err_Throw(NEWS_INTERNAL_RANGE);
		}
	}

	for (int i = 0; i < SERVICE_COUNT; i++) {
		Err_FailedThrow(srvUnregisterService(NEWS_ServiceNames[i]));
		svcCloseHandle(session_handles[i + 1]);
	}

	svcCloseHandle(session_handles[0]);

	srvExit();
}
