#include "pch.h"
#include "CoreGlobal.h"
#include "ThreadManager.h"
#include "DeadLockProfiler.h"
#include "Memory.h"
#include "SocketUtils.h"

ThreadManager* GThreadManager = nullptr;
DeadLockProfiler* GDeadLockProfiler = nullptr;
Memory* GMemory = nullptr;

class CoreGlobal {
public:
	CoreGlobal() {
		GThreadManager = new ThreadManager();
		GDeadLockProfiler = new DeadLockProfiler();
		GMemory = new Memory();
		SocketUtils::Init();
	};

	~CoreGlobal() {
		delete GThreadManager;
		delete GDeadLockProfiler;
		delete GMemory;
		SocketUtils::Clear();
	};
} GCoreGlobal;