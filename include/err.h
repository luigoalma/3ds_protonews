#pragma once
#include <3ds/svc.h>
#include <3ds/types.h>
#include <3ds/result.h>

#define Err_Throw(failure) do {svcBreak(USERBREAK_ASSERT); svcBreak(USERBREAK_PANIC);} while(0)

#define Err_FailedThrow(failure) do {Result __tmp = failure; if (R_FAILED(__tmp)) Err_Throw(__tmp);} while(0)
