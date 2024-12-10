#include "pch.h"
#include "DeadLockProfiler.h"

void DeadLockProfiler::PushLock(const char* name) {
	LockGuard guard(_lock);

	int32 lockId = 0;
	auto findIt = _nameToId.find(name);
	if (findIt == _nameToId.end()) {
		lockId = static_cast<int32>(_nameToId.size());
		_nameToId[name] = lockId;
		_idToName[lockId] = name;
	}
	else {
		lockId = findIt->second;
	}

	if (_lockStack.empty() == false) {
		const int32 preId = _lockStack.top();
		if (preId != lockId) {
			set<int32>& history = _lockHistory[preId];
			if (history.find(lockId) == history.end()) {
				history.insert(lockId);
				CheckCycle();
			}
		}
	}
	_lockStack.push(lockId);
}

void DeadLockProfiler::PopLock(const char* name) {
	LockGuard guard(_lock);
	if (_lockStack.empty()) {
		CRASH("Poplock 중복")
	}
	_lockStack.pop();
}

void DeadLockProfiler::CheckCycle() {
	const int32 lockCount = static_cast<int32>(_nameToId.size());
	_discoveredOrder = vector<int32>(lockCount, -1);
	_discoveredCount = 0;
	_finished = vector<bool>(lockCount, false);
	_parent = vector<int32>(lockCount, -1);
	for (int32 i = 0; i < lockCount; i++) {
		DFS(i);
	}
	_discoveredOrder.clear();
	_finished.clear();
	_parent.clear();
}

void DeadLockProfiler::DFS(int32 index) {
	if (_discoveredOrder[index] != -1) {
		return;
	}

	_discoveredOrder[index] = _discoveredCount++;
	auto findIt = _lockHistory.find(index);
	if (findIt == _lockHistory.end()) {
		_finished[index] = true;
		return;
	}

	set<int32>& nextSet = findIt->second;
	for (int32 nex : nextSet) {
		if (_discoveredOrder[nex] == -1) {
			_parent[nex] = index;
			DFS(nex);
			continue;
		}
		if (_discoveredOrder[index] < _discoveredOrder[nex]) {
			continue;
		}
		if (_finished[nex] == false) {
			int32 now = index;
			while (now != nex) {
				cout << _idToName[_parent[now]] << " ";
				cout << "-> ";
				cout << _idToName[now] << endl;
				now = _parent[now];
			}
			CRASH("Dead Lock 감지")
		}
	}
	_finished[index] = true;
}
