#pragma once
#include "Session.h"

class GameSession : public Session {
public:
	~GameSession() {
		cout << "delete client sesion" << endl;
	}

	virtual int32 OnRecv(BYTE* buffer, int32 len) override {
		cout << "on recv len : " << len << endl;
		SendBufferRef sendBuffer = MakeShared<SendBuffer>(4096);
		sendBuffer->CopyData(buffer, len);
		Send(sendBuffer);
		return len;
	}

	virtual void OnSend(int32 len) override {
		cout << "on send len : " << len << endl;
	}
};

