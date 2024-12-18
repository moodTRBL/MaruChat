#include "pch.h"
#include "Session.h"
#include "SocketUtils.h"
#include "Service.h"
#include "RecvBuffer.h"

Session::Session() : _recvBuffer(BUFFER_SIZE) {
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
	OnDisconnect();
	GetService()->ReleaseSession(GetSessionRef());
	RegisterDisconnect();
}

void Session::Send(SendBufferRef sendBuffer) {
	WRITE_LOCK;
	_sendQueue.push(sendBuffer);
	if (_sendRegistered.exchange(true) == false) {
		RegisterSend();
	}

	/*SendEvent* sendEvent = xnew<SendEvent>();
	sendEvent->owner = shared_from_this();
	sendEvent->buffer.resize(len);
	memcpy(sendEvent->buffer.data(), buffer, len);

	RegisterSend(sendEvent);*/
}

bool Session::Connect() {
	return RegisterConnect();
}

HANDLE Session::GetHandle() {
	return reinterpret_cast<HANDLE>(_socket);
}

void Session::Dispatch(IocpEvent* iocpEvent, int32 numOfBytes) {
	switch (iocpEvent->eventType) {
	case EventType::CONNECT:
		ProcessConnect();
		break;
	case EventType::DISCONNECT:
		ProcessDisconnect();
		break;
	case EventType::RECV:
		ProcessRecv(numOfBytes);
		break;
	case EventType::SEND:
		ProcessSend(numOfBytes);
		break;

	default:
		break;
	}
}

bool Session::RegisterConnect() {
	if (IsConnected()) {
		return false;
	}
	if (GetService()->GetServiceType() != ServiceType::CLIENT) {
		return false;
	}
	if (SocketUtils::SetReuseAddress(_socket, true) == false) {
		return false;
	}
	if (SocketUtils::BindAnyAddress(_socket, 0) == false) {
		return false;
	}

	_connectEvent.Init();
	_connectEvent.owner = shared_from_this();
	DWORD numOfbytes = 0;
	SOCKADDR_IN sockAddr = GetService()->GetNetAddress().GetSockAddr();
	if (SocketUtils::ConnectEx(_socket, reinterpret_cast<SOCKADDR*>(&sockAddr), sizeof(sockAddr),
		nullptr, 0, &numOfbytes, &_connectEvent) == false) {
		int errorCode = WSAGetLastError();
		if (errorCode != WSA_IO_PENDING) {
			_connectEvent.owner = nullptr;
			return false;
		}
	}
	return true;
}

bool Session::RegisterDisconnect() {
	_disconnectEvent.Init();
	_disconnectEvent.owner = shared_from_this();

	if (SocketUtils::DisconnectEx(_socket, &_disconnectEvent, TF_REUSE_SOCKET, 0) == false) {
		int32 errorCode = WSAGetLastError();
		if (errorCode != WSA_IO_PENDING) {
			HandleError(errorCode);
			_disconnectEvent.owner = nullptr;
			return false;
		}
	}
	return true;
}

void Session::RegisterRecv() {
	if (IsConnected() == false) {
		return;
	}
	_recvEvent.Init();
	_recvEvent.owner = shared_from_this();

	WSABUF wsaBuf;
	wsaBuf.buf = reinterpret_cast<char*>(_recvBuffer.WritePos());
	wsaBuf.len = _recvBuffer.FreeSize();

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

void Session::RegisterSend() {
	if (IsConnected() == false) {
		return;
	}
	_sendEvent.Init();
	_sendEvent.owner = shared_from_this();

	{
		WRITE_LOCK;
		int32 writeSize = 0;
		while (_sendQueue.empty() == false) {
			SendBufferRef sendBuffer = _sendQueue.front();
			writeSize += sendBuffer->WriteSize();
			_sendQueue.pop();
			_sendEvent.sendBuffers.push_back(sendBuffer);
		}
	}

	kvector<WSABUF> wsaBufs;
	wsaBufs.reserve(_sendEvent.sendBuffers.size());
	for (SendBufferRef sendBuffer : _sendEvent.sendBuffers) {
		WSABUF wsaBuf;
		wsaBuf.buf = reinterpret_cast<char*>(sendBuffer->Buffer());
		wsaBuf.len = static_cast<LONG>(sendBuffer->WriteSize());
		wsaBufs.push_back(wsaBuf);
	}

	DWORD numOfBytes = 0;
	if (WSASend(_socket, wsaBufs.data(), static_cast<DWORD>(wsaBufs.size()), &numOfBytes, 0, &_sendEvent, nullptr) == SOCKET_ERROR) {
		int32 errorCode = WSAGetLastError();
		if (errorCode != WSA_IO_PENDING) {
			HandleError(errorCode);
			_sendEvent.owner = nullptr;
			_sendEvent.sendBuffers.clear();
			_sendRegistered.store(false);
		}
	}
}

void Session::ProcessConnect() {
	_connectEvent.owner = nullptr;
	_connected.store(true);
	GetService()->AddSession(GetSessionRef());
	OnConnect();
	RegisterRecv();
}

void Session::ProcessDisconnect() {
	_disconnectEvent.owner = nullptr;
}

void Session::ProcessRecv(int32 numOfbytes) {
	_recvEvent.owner = nullptr;
	if (numOfbytes == 0) {
		Disconnect(L"recv len is 0");
		return;
	}
	if (_recvBuffer.OnWrite(numOfbytes) == false) {
		Disconnect(L"onwrite overflow");
		return;
	}
	int32 dataSize = _recvBuffer.DataSize();
	int32 processLen = OnRecv(_recvBuffer.ReadPos(), dataSize);
	if (processLen < 0 || dataSize < processLen || _recvBuffer.OnRead(processLen) == false) {
		Disconnect(L"onread overflow");
		return;
	}
	_recvBuffer.Clean();
	RegisterRecv();
}

void Session::ProcessSend(int32 numOfbytes) {
	_sendEvent.owner = nullptr;
	_sendEvent.sendBuffers.clear();

	if (numOfbytes == 0) {
		Disconnect(L"send 0");
	}
	OnSend(numOfbytes);

	WRITE_LOCK;
	if (_sendQueue.empty()) {
		_sendRegistered.store(false);
	}
	else {
		RegisterSend();
	}
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
