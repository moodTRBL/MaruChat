#pragma once
#include "IocpCore.h"
#include "IocpEvent.h"
#include "NetAddress.h"

class Service;

class Session : public IocpObject {
	friend class Listener;
	friend class IocpCore;
	friend class Service;
public:
	Session();
	virtual ~Session();

	void SetNetAddress(NetAddress address) { _address = address; }
	NetAddress GetAddress() { return _address; }
	SOCKET GetSocket() { return _socket; }
	bool IsConnected() { return _connected; }
	SessionRef GetSessionRef() { return static_pointer_cast<Session>(shared_from_this()); }

	void Disconnect(const WCHAR* cause);
	void Send(BYTE* buffer, int32 len);

	shared_ptr<Service> GetService() { return _service.lock(); }
	void SetService(shared_ptr<Service> serivce) { _service = serivce; }

	BYTE _recvBuffer[1000];

private:
	virtual HANDLE GetHandle() override;
	virtual void Dispatch(class IocpEvent* iocpEvent, int32 numOfBytes = 0) override;

	void RegisterConnect();
	void RegisterRecv();
	void RegisterSend(SendEvent* sendEvent);

	void ProcessConnect();
	void ProcessRecv(int32 numOfbytes);
	void ProcessSend(SendEvent* sendEvent, int32 numOfbytes);

	void HandleError(int32 errorCode);

	virtual void OnConnect() {}
	virtual int32 OnRecv(BYTE* buffer, int32 len) { return len; }
	virtual void OnSend(int32 len) {}
	virtual void OnDisconnected() {}

private:
	weak_ptr<Service> _service;
	SOCKET _socket = INVALID_SOCKET;
	NetAddress _address = {};
	atomic<bool> _connected = false;
	
	USE_LOCK;
	RecvEvent _recvEvent;
};

