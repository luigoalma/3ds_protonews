#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/srv.h>
#include <3ds/svc.h>
#include <3ds/ipc.h>

// NOTE! Modified for a version of prototype SRV!

static inline int str_len_limited(const char* name) {
	int s = 0;
	while (*name) {
		if(++s == 8)
			return s;
		name++;
	}
	return s;
}

static Handle srvHandle;
static int srvRefCount;

Result srvInit(void)
{
	Result rc = 0;

	if (srvRefCount++) return 0;

	rc = svcConnectToPort(&srvHandle, "srv:");
	if (R_SUCCEEDED(rc)) rc = srvRegisterClient();

	if (R_FAILED(rc)) srvExit();
	return rc;
}

void srvExit(void)
{
	if (--srvRefCount) return;

	if (srvHandle != 0) svcCloseHandle(srvHandle);
	srvHandle = 0;
}

Result srvRegisterClient(void)
{
	Result rc = 0;
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x1,0,2); // 0x10002
	cmdbuf[1] = IPC_Desc_CurProcessId();

	if(R_FAILED(rc = svcSendSyncRequest(srvHandle)))return rc;

	return cmdbuf[1];
}

Result srvEnableNotification(Handle* semaphoreOut)
{
	Result rc = 0;
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x2,0,0); // 0x20000

	if(R_FAILED(rc = svcSendSyncRequest(srvHandle)))return rc;

	if(semaphoreOut) *semaphoreOut = cmdbuf[3];

	return cmdbuf[1];
}

Result srvRegisterService(Handle* out, const char* name)
{
	Result rc = 0;
	u32* cmdbuf = getThreadCommandBuffer();

	int len = str_len_limited(name);

	cmdbuf[0] = IPC_MakeHeader(0x3,1,2); // 0x30042
	cmdbuf[1] = len;
	cmdbuf[2] = IPC_Desc_StaticBuffer(len, 0);
	cmdbuf[3] = (u32)name;

	if(R_FAILED(rc = svcSendSyncRequest(srvHandle)))return rc;

	if(out) *out = cmdbuf[3];

	return cmdbuf[1];
}

Result srvUnregisterService(const char* name)
{
	Result rc = 0;
	u32* cmdbuf = getThreadCommandBuffer();

	int len = str_len_limited(name);

	cmdbuf[0] = IPC_MakeHeader(0x4,1,2); // 0x40042
	cmdbuf[1] = len;
	cmdbuf[2] = IPC_Desc_StaticBuffer(len, 0);
	cmdbuf[3] = (u32)name;

	if(R_FAILED(rc = svcSendSyncRequest(srvHandle)))return rc;

	return cmdbuf[1];
}
Result srvReceiveNotification(u32* notificationIdOut)
{
	Result rc = 0;
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xA,0,0); // 0xA0000

	if(R_FAILED(rc = svcSendSyncRequest(srvHandle)))return rc;

	if(notificationIdOut) *notificationIdOut = cmdbuf[2];

	return cmdbuf[1];
}
