#pragma once

#define USE_MANY_LOCK(count) Lock _locks[count];
#define USE_LOCK USE_MANY_LOCK(1);
#define READ_LOCK_IDX(idx)		ReadLockGuard readLockGuard_##idx(_locks[idx], typeid(this).name());
#define READ_LOCK				READ_LOCK_IDX(0);
#define WRITE_LOCK_IDX(idx)		WriteLockGuard writeLockGuard_##idx(_locks[idx], typeid(this).name());
#define WRITE_LOCK				WRITE_LOCK_IDX(0);

#define OUT

#ifdef _DEBUG
#define kalloc(size) PoolAllocator::Alloc(size)
#define krelease(ptr) PoolAllocator::Release(ptr);
#else
#define kalloc(size) PoolAllocator::Alloc(size)
#define krelease(ptr) PoolAllocator::Release(ptr);
#endif

#define CRASH(cause)							\
{												\
	uint32* crash = nullptr;					\
	__analysis_assume(crash != nullptr);		\
	*crash = 0xDEADBEEF;						\
}								

#define ASSERT_CRASH(expr)						\
{												\
	if (!(expr)) {								\
		CRASH("ASSERT CRASH");					\
		__analysis_assume(expr);				\
	}											\
}												