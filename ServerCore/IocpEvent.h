#pragma once

class Session;

enum class EventType : uint8 {
	CONNECT,
	ACCEPT,
	RECV,
	SEND,
	DISCONNECT
};

class IocpEvent : public OVERLAPPED {
public:
	IocpEvent(EventType type);
	
	void Init();

public:
	EventType eventType;
	IocpObjectRef owner;
};

class ConnectEvent : public IocpEvent {
public:
	ConnectEvent() : IocpEvent(EventType::CONNECT) {}
};

class AcceptEvent : public IocpEvent {
public:
	AcceptEvent() : IocpEvent(EventType::ACCEPT) {}

public:
	SessionRef session = nullptr;
};

class RecvEvent : public IocpEvent {
public:
	RecvEvent() : IocpEvent(EventType::RECV) {}
};

class SendEvent : public IocpEvent {
public:
	SendEvent() : IocpEvent(EventType::SEND) {}
	vector<BYTE> buffer;
};
