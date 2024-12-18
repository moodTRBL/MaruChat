#include "pch.h"
#include "CoreTLS.h"

thread_local uint32 LThreadId;
thread_local kstring LNickname;
thread_local std::stack<int32> LLockStack;