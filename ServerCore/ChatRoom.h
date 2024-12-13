#pragma once

class ChatRoom {
public:
	ChatRoom(uint32 maxSession, kstring password);
	~ChatRoom();

	bool EnterRoom(kstring password);
	void QuitRoom();
	bool ExpelSession(kstring nickname);
	bool ChangePassword(kstring newPassword);

private:
	//추후 세션을 넣을 예정
	USE_LOCK;
	uint32 _admin;
	kset<pair<int64, uint32>> _sessions;
	uint32 _maxSession;
	uint32 _sessionCount = 0;
	kstring _password;
	kmap<kstring, uint32> _nicknameToId;
	kmap<uint32, int64> _idToEntertime;
	mutex _lock;

};

