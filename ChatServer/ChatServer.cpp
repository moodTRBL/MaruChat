#include "pch.h"
#include "ThreadManager.h"
#include "Allocator.h"
#include "Memory.h"
#include "ChatRoom.h"

#include "SocketUtils.h"
#include "Listener.h"
#include "Service.h"
#include "Session.h"
#include "GameSession.h"

class GameSession2 : public Session {
public:
	~GameSession2() {
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

int main() {
	ServerServiceRef service = MakeShared<ServerService>(
		NetAddress(L"127.0.0.1", 7777), MakeShared<IocpCore>(), MakeShared<GameSession>, 100
	);
	service->Start();

	for (int32 i = 0; i < 5; i++) {
		GThreadManager->Launch([=]()
			{
				while (true) {
					service->GetIocpCore()->Dispatch();
				}
			});
	}

	return 0;
}