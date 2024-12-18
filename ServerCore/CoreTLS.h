#pragma once
#include "Container.h"
#include <stack>

extern thread_local uint32 LThreadId;
extern thread_local kstring LNickname;
extern thread_local std::stack<int32> LLockStack;
