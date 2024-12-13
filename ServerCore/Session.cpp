#include "pch.h"
#include "Session.h"
#include "SocketUtils.h"
#include "Service.h"

Session::Session() {
	_socket = SocketUtils::CreateSocket();
}

Session::~Session() {
	SocketUtils::Close(_socket);
}

void Session::Disconnect(const WCHAR* cause) {
	if (_connected.exchange(false) == false) {
		return;
	}
	wcout << "disconnect: " << cause << endl;
	OnDisconnected();
	SocketUtils::Close(_socket);
	GetService()->ReleaseSession(GetSessionRef());
}

void Session::Send(BYTE* buffer, int32 len) {
	SendEvent* sendEvent = xnew<SendEvent>();
	sendEvent->owner = shared_from_this();
	sendEvent->buffer.resize(len);
	memcpy(sendEvent->buffer.data(), buffer, len);

	RegisterSend(sendEvent);
}

HANDLE Session::GetHandle() {
	return reinterpret_cast<HANDLE>(_socket);
}

void Session::Dispatch(IocpEvent* iocpEvent, int32 numOfBytes) {
	switch (iocpEvent->eventType) {
	case EventType::ACCEPT:
		ProcessConnect();
		break;
	case EventType::RECV:
		ProcessRecv(numOfBytes);
		break;
	case EventType::SEND:
		ProcessSend(static_cast<SendEvent*>(iocpEvent), numOfBytes);
		break;
	default:
		break;
	}
}

void Session::RegisterConnect() {
	
}

void Session::RegisterRecv() {
	if (IsConnected() == false) {
		return;
	}
	_recvEvent.Init();
	_recvEvent.owner = shared_from_this();

	WSABUF wsaBuf;
	wsaBuf.buf = reinterpret_cast<char*>(_recvBuffer);
	wsaBuf.len = len32(_recvBuffer);

	DWORD numOfBytes = 0;
	DWORD flag = 0;
	if (WSARecv(_socket, &wsaBuf, 1, &numOfBytes, &flag, &_recvEvent, nullptr) == SOCKET_ERROR) {
		int32 errorCode = WSAGetLastError();
		if (errorCode != WSA_IO_PENDING) {
			HandleError(errorCode);
			_recvEvent.owner = nullptr;
		}
	}
}

void Session::RegisterSend(SendEvent* sendEvent) {
	if (IsConnected() == false) {
		return;
	}
	WSABUF wsaBuf;
	wsaBuf.buf = (char*)sendEvent->buffer.data();
	wsaBuf.len = (ULONG)sendEvent->buffer.size();

	DWORD numOfBytes = 0;
	if (WSASend(_socket, &wsaBuf, 1, &numOfBytes, 0, sendEvent, nullptr)==SOCKET_ERROR) {
		int32 errorCode = WSAGetLastError();
		if (errorCode != WSA_IO_PENDING) {
			HandleError(errorCode);
			sendEvent->owner = nullptr;
			xdelete(sendEvent);
		}
	}
}

void Session::ProcessConnect() {
	_connected.store(true);
	GetService()->AddSession(GetSessionRef());
	OnConnect();
	RegisterRecv();
}

void Session::ProcessRecv(int32 numOfbytes) {
	_recvEvent.owner = nullptr;
	if (numOfbytes == 0) {
		Disconnect(L"recv len is 0");
		return;
	}

	OnRecv(_recvBuffer, numOfbytes);
	RegisterRecv();
}

void Session::ProcessSend(SendEvent* sendEvent, int32 numOfbytes) {
	sendEvent->owner = nullptr;
	xdelete(sendEvent);
	if (numOfbytes == 0) {
		Disconnect(L"send 0");
	}
	OnSend(numOfbytes);
}

void Session::HandleError(int32 errorCode) {
	switch (errorCode) {
	case WSAECONNRESET:
	case WSAECONNABORTED:
		Disconnect(L"HandleError");
		break;
	default:
		cout << "Handle Error : " << errorCode << endl;
		break;
	}
}
