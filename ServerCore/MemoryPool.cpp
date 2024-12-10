#include "pch.h"
#include "MemoryPool.h"

MemoryPool::MemoryPool(int32 allocSize) : _allocSize(allocSize) {
	InitializeSListHead(&_header);
}

MemoryPool::~MemoryPool() {
	while (MemoryHeader* header = static_cast<MemoryHeader*>(InterlockedPopEntrySList(&_header))) {
		_aligned_free(header);
	}
}

void MemoryPool::Push(MemoryHeader* header) {
	header->allocSize = 0;
	InterlockedPushEntrySList(&_header, static_cast<PSLIST_ENTRY>(header));
	_allocCount.fetch_sub(1);
}

MemoryHeader* MemoryPool::Pop() {
	MemoryHeader* header = static_cast<MemoryHeader*>(InterlockedPopEntrySList(&_header));
	
	if (header == nullptr) {
		header = reinterpret_cast<MemoryHeader*>(_aligned_malloc(_allocSize, SLIST_ALIGNMENT));
	}
	_allocCount.fetch_add(1);
	return header;
}
