#pragma once
#include "IocpCore.h"
#include "NetAddress.h"

class AcceptEvent;
class ServerService;

class Listener : public IocpObject {
public:
	Listener() = default;
	~Listener();

	bool StartAccept(ServerServiceRef service);
	void CloseSocket();

	virtual HANDLE GetHandle() override;
	virtual void Dispatch(class IocpEvent* iocpEvent, int32 numOfBytes = 0) override;

private:
	void RegisterAccept(AcceptEvent* acceptEvent);
	void ProcessAccept(AcceptEvent* acceptEvent);

private:
	SOCKET _socket = INVALID_SOCKET;
	kvector<AcceptEvent*> _acceptEvents;
	ServerServiceRef _service;
};

