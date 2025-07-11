#include "pch.h"
#include "Memory.h"
#include "MemoryPool.h"

Memory::Memory() {
	int32 size = 0;
	int32 index = 0;
	for (size = 32; size < 1024; size += 32) {
		MemoryPool* pool = new MemoryPool(size);
		_pools.push_back(pool);
		while (index <= size) {
			_poolTable[index++] = pool;
		}
	}
	for (; size < 2048; size += 128) {
		MemoryPool* pool = new MemoryPool(size);
		_pools.push_back(pool);
		while (index <= size) {
			_poolTable[index++] = pool;
		}
	}
	for (; size <= 4098; size += 256) {
		MemoryPool* pool = new MemoryPool(size);
		_pools.push_back(pool);
		while (index <= size) {
			_poolTable[index++] = pool;
		}
	}
}

Memory::~Memory() {
	for (MemoryPool* pool : _pools) {
		delete pool;
	}
	_pools.clear();

}

void* Memory::Allocate(int32 size) {
	MemoryHeader* header = nullptr;
	const int32 allocSize = size + sizeof(MemoryHeader);
	if (allocSize > MAX_ALLOC_SIZE) {
		header = reinterpret_cast<MemoryHeader*>(_aligned_malloc(allocSize, SLIST_ALIGNMENT));
	}
	else {
		header = _poolTable[allocSize]->Pop();
	}
	return MemoryHeader::AttachHeader(header, allocSize);
}

void Memory::Release(void* ptr) {
	MemoryHeader* header = MemoryHeader::DetachHeader(ptr);
	const int32 allocSize = header->allocSize;
	if (allocSize > MAX_ALLOC_SIZE) {
		_aligned_free(header);
	}
	else {
		_poolTable[allocSize]->Push(header);
	}
}