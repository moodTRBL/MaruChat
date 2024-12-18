#pragma once
#include "IocpCore.h"
#include "IocpEvent.h"
#include "NetAddress.h"
#include "RecvBuffer.h"

class Service;

class Session : public IocpObject {
	friend class Listener;
	friend class IocpCore;
	friend class Service;

	enum {
		BUFFER_SIZE = 0x10000
	};

public:
	Session();
	virtual ~Session();

	void SetNetAddress(NetAddress address) { _address = address; }
	NetAddress GetAddress() { return _address; }
	SOCKET GetSocket() { return _socket; }
	bool IsConnected() { return _connected; }
	SessionRef GetSessionRef() { return static_pointer_cast<Session>(shared_from_this()); }

	void Disconnect(const WCHAR* cause);
	void Send(SendBufferRef sendBuffer);
	bool Connect();

	shared_ptr<Service> GetService() { return _service.lock(); }
	void SetService(shared_ptr<Service> serivce) { _service = serivce; }

private:
	virtual HANDLE GetHandle() override;
	virtual void Dispatch(class IocpEvent* iocpEvent, int32 numOfBytes = 0) override;

	bool RegisterConnect();
	bool RegisterDisconnect();
	void RegisterRecv();
	void RegisterSend();

	void ProcessConnect();
	void ProcessDisconnect();
	void ProcessRecv(int32 numOfbytes);
	void ProcessSend(int32 numOfbytes);

	void HandleError(int32 errorCode);

	virtual void OnConnect() {}
	virtual int32 OnRecv(BYTE* buffer, int32 len) { return len; }
	virtual void OnSend(int32 len) {}
	virtual void OnDisconnect() {}

private:
	weak_ptr<Service> _service;
	SOCKET _socket = INVALID_SOCKET;
	NetAddress _address = {};
	atomic<bool> _connected = false;
	kqueue<SendBufferRef> _sendQueue;
	atomic<bool> _sendRegistered = false;	
	
	USE_LOCK;
	RecvEvent _recvEvent;
	ConnectEvent _connectEvent;
	DisconnectEvent _disconnectEvent;
	SendEvent _sendEvent;

	RecvBuffer _recvBuffer;
};

