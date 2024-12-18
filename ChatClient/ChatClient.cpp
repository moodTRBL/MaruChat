#include "pch.h"
#include <iostream>

#include "Service.h"
#include "Session.h"
#include "CoreGlobal.h"
#include "ThreadManager.h"

char sendData[] = "hello world";

class ServerSession : public Session {
public:
	~ServerSession() {
		cout << "delete server session" << endl;
	}

	virtual void OnConnect() override {
		cout << "connect to server" << endl;
		SendBufferRef sendBuffer = MakeShared<SendBuffer>(4096);
		sendBuffer->CopyData(sendData, sizeof(sendData));
		Send(sendBuffer);
	}

	virtual void OnDisconnect()override {
		cout << "disconnect success" << endl;
	}

	virtual int32 OnRecv(BYTE* buffer, int32 len) override {
		cout << "on recv len : " << len << endl;
		this_thread::sleep_for(1s);
		SendBufferRef sendBuffer = MakeShared<SendBuffer>(4096);
		sendBuffer->CopyData(sendData, sizeof(sendData));
		Send(sendBuffer);
		return len;
	}

	virtual void OnSend(int32 len) override {
		cout << "on send len : " << len << endl;
	}
};

int main() {
	this_thread::sleep_for(1s);

	ClientServiceRef service = MakeShared<ClientService>(
		NetAddress(L"127.0.0.1", 7777), MakeShared<IocpCore>(), MakeShared<ServerSession>, 5
	);
	service->Start();

	for (int32 i = 0; i < 2; i++) {
		GThreadManager->Launch([=]()
			{
				while (true) {
					service->GetIocpCore()->Dispatch();
				}
			});
	}
	GThreadManager->Join();
}