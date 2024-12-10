#include "pch.h"
#include "Lock.h"
#include "CoreTLS.h"
#include "DeadLockProfiler.h"

void Lock::WriteLock(const char* name) {
#if _DEBUG
	GDeadLockProfiler->PushLock(name);
#endif

	const uint32 lockThreadId = (_lockFlag.load() & WRITE_THREAD_MASK) >> 16;
	if (lockThreadId == LThreadId) {
		_writeCount++;
		return;
	}

	const int64 beginTick = ::GetTickCount64();
	const uint32 desired = ((LThreadId << 16) & WRITE_THREAD_MASK);
	while (true) {
		for (uint32 spinCount = 0; spinCount < MAX_SPIN_COUNT; spinCount++) {
			uint32 expected = EMPTY_FLAG;
			if (_lockFlag.compare_exchange_strong(expected, desired)) {
				_writeCount++;
				return;
			}
		}
		const int64 endTick = ::GetTickCount64();
		if (endTick - beginTick >= ACQUIRE_TIMEOUT_TICK) {
			CRASH("Lock Timeout 발생");
		}
		this_thread::yield();
	}
}

void Lock::WriteUnlock(const char* name) {
#if _DEBUG
	GDeadLockProfiler->PopLock(name);
#endif

	if ((_lockFlag.load() & READ_COUNT_MASK) != 0) {
		CRASH("아직 Reader가 락을 잡고 있습니다")
	}
	const int32 lockCount = --_writeCount;
	if (lockCount == 0) {
		_lockFlag = EMPTY_FLAG;
	}
}

void Lock::ReadLock(const char* name) {
#if _DEBUG
	GDeadLockProfiler->PushLock(name);
#endif

	const uint32 lockThreadId = (_lockFlag.load() & WRITE_THREAD_MASK) >> 16;
	if (lockThreadId == LThreadId) {
		_lockFlag.fetch_add(1);
		return;
	}

	const int64 beginTick = ::GetTickCount64();
	while (true) {
		for (uint32 spinCount = 0; spinCount < MAX_SPIN_COUNT; spinCount++) {
			uint32 expected = (_lockFlag.load() & READ_COUNT_MASK);
			if (_lockFlag.compare_exchange_strong(expected, expected + 1)) {
				return;
			}
		}
		const int64 endTick = ::GetTickCount64();
		if (endTick - beginTick >= ACQUIRE_TIMEOUT_TICK) {
			CRASH("Lock Timeout 발생");
		}
		this_thread::yield();
	}
}

void Lock::ReadUnlock(const char* name) {
#if _DEBUG
	GDeadLockProfiler->PopLock(name);
#endif

	if ((_lockFlag.fetch_sub(1) & READ_COUNT_MASK) == 0) {
		CRASH("Unlock 이중 발생")
	}
}