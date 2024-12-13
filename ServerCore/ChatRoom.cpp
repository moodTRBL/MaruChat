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
        cout << "�ο��� ���� �� �����Դϴ�" << endl;
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

    cout << LNickname << "���� �����Ͽ����ϴ�. -> " << LThreadId << endl;
    cout << "�����ο�: " << _sessionCount << endl;
    return true;
}

void ChatRoom::QuitRoom() {
    WRITE_LOCK;
    int64 time = _idToEntertime[LThreadId];
    _sessions.erase({ time,LThreadId });
    _idToEntertime.erase(LThreadId);
    _nicknameToId[LNickname];
    _sessionCount--;

    cout << LNickname << "���� �����Ͽ����ϴ�." << endl;

    if (_sessionCount == 0) {
        krelease(this);
        return;
    }

    //���� ����� ������ ��� ���� ������ ���� ���� ����
    if (LThreadId == _admin) {
        auto it = _sessions.begin();
        _admin = (*it).first;
        cout << LNickname << "���� ���ο� ������ �Ǿ����ϴ�." << endl;
    }
}

//���θ� ���𰡴�
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

    cout << LNickname << "���� ������߽��ϴ�." << endl;
    return true;
}

//��й�ȣ ����� ä�ù� ������ ������ ���� �������� ����Ͽ� ���� �ɾ����
bool ChatRoom::ChangePassword(kstring newPassword) {
    if (LThreadId != _admin) {
        return false;
    }
    LockGuard guard(_lock);
    _password = newPassword;
}