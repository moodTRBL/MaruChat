#include "pch.h"
//
//#include <thread>
//#include <functional>

#include "ThreadManager.h"
#include "CoreTLS.h"
#include "CoreGlobal.h"

ThreadManager::ThreadManager() {
	InitTLS();
}

ThreadManager::~ThreadManager() {
	Join();
}

void ThreadManager::Launch(function<void(void)> callback) {
	LockGuard guard(_lock);
	_threads.push_back(thread([=]() {
			InitTLS();
			callback();
			DestoryTLS();
		}));
}

void ThreadManager::Join() {
	for (thread& t : _threads) {
		if (t.joinable()) {
			t.join();
		}
	}
	_threads.clear();
}

void ThreadManager::InitTLS() {
	static Atomic<uint32> SThreadId = 1;
	LThreadId = SThreadId.fetch_add(1);
}

void ThreadManager::DestoryTLS() {

}
