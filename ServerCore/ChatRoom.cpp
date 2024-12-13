#include "pch.h"
#include "ChatRoom.h"

ChatRoom::ChatRoom(uint32 maxSession, kstring password)
    : _maxSession(maxSession), _password(password) {
    int64 time = GetTickCount64();
    _sessions.insert({ time,LThreadId });
    _idToEntertime[LThreadId] = time;
    _nicknameToId[LNickname] = LThreadId;
    _sessionCount++;
    _admin = LThreadId;
}

ChatRoom::~ChatRoom() {

}

bool ChatRoom::EnterRoom(kstring password) {
    LockGuard guard(_lock);
    WRITE_LOCK;
    if (_sessionCount == _maxSession) {
        cout << "인원이 가득 찬 상태입니다" << endl;
        return false;
    }
    if (password != _password) {
        return false;
    }
    int64 time = GetTickCount64();
    _sessions.insert({ time,LThreadId });
    _idToEntertime[LThreadId] = time;
    _nicknameToId[LNickname] = LThreadId;
    _sessionCount++;

    cout << LNickname << "님이 입장하였습니다. -> " << LThreadId << endl;
    cout << "현재인원: " << _sessionCount << endl;
    return true;
}

void ChatRoom::QuitRoom() {
    WRITE_LOCK;
    int64 time = _idToEntertime[LThreadId];
    _sessions.erase({ time,LThreadId });
    _idToEntertime.erase(LThreadId);
    _nicknameToId[LNickname];
    _sessionCount--;

    cout << LNickname << "님이 퇴장하였습니다." << endl;

    if (_sessionCount == 0) {
        krelease(this);
        return;
    }

    //나간 사람이 어드민인 경우 들어온 순서에 따라 어드민 위임
    if (LThreadId == _admin) {
        auto it = _sessions.begin();
        _admin = (*it).first;
        cout << LNickname << "님이 새로운 방장이 되었습니다." << endl;
    }
}

//어드민만 강퇴가능
bool ChatRoom::ExpelSession(kstring nickname) {
    if (LThreadId != _admin) {
        return false;
    }

    WRITE_LOCK;
    uint32 id = _nicknameToId[nickname];
    int64 time = _idToEntertime[id];
    _sessions.erase({ time,id });
    _nicknameToId.erase(nickname);
    _idToEntertime.erase(id);
    _sessionCount--;

    cout << LNickname << "님이 강퇴당했습니다." << endl;
    return true;
}

//비밀번호 변경과 채팅방 들어오는 동작은 같은 세마포를 사용하여 락을 걸어야함
bool ChatRoom::ChangePassword(kstring newPassword) {
    if (LThreadId != _admin) {
        return false;
    }
    LockGuard guard(_lock);
    _password = newPassword;
}