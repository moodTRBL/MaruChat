#include "pch.h"
#include "SocketUtils.h"

LPFN_CONNECTEX SocketUtils::ConnectEx = nullptr;
LPFN_DISCONNECTEX SocketUtils::DisconnectEx = nullptr;
LPFN_ACCEPTEX SocketUtils::AcceptEx = nullptr;

void SocketUtils::Init() {
	WSADATA wsaData;
	ASSERT_CRASH(WSAStartup(MAKEWORD(2, 2), &wsaData) == 0);

	SOCKET dummySocket = CreateSocket();
	ASSERT_CRASH(BindWindowsFunction(dummySocket, WSAID_CONNECTEX, reinterpret_cast<LPVOID*>(&ConnectEx)));
	ASSERT_CRASH(BindWindowsFunction(dummySocket, WSAID_DISCONNECTEX, reinterpret_cast<LPVOID*>(&DisconnectEx)));
	ASSERT_CRASH(BindWindowsFunction(dummySocket, WSAID_ACCEPTEX, reinterpret_cast<LPVOID*>(&AcceptEx)));
	Close(dummySocket);
}

void SocketUtils::Clear() {
	WSACleanup();
}

SOCKET SocketUtils::CreateSocket() {
	return WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
}

bool SocketUtils::BindWindowsFunction(SOCKET socket, GUID guid, LPVOID* fn) {
	DWORD bytes = 0;
	return SOCKET_ERROR !=
	WSAIoctl(socket, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid), fn, sizeof(*fn), &bytes, NULL, NULL);
}

void SocketUtils::Close(SOCKET& socket) {
	if (socket != INVALID_SOCKET) {
		closesocket(socket);
	}
	socket = INVALID_SOCKET;
}

bool SocketUtils::Bind(SOCKET socket, NetAddress address) {
	int32 code = bind(socket, reinterpret_cast<const SOCKADDR*>(&address.GetSockAddr()), sizeof(SOCKADDR_IN));
	if (code == SOCKET_ERROR) {
		int error = WSAGetLastError();
		cout << error << endl;
	}
	return SOCKET_ERROR != code;
}

bool SocketUtils::BindAnyAddress(SOCKET socket, uint16 port) {
	SOCKADDR_IN myAddress;
	myAddress.sin_family = AF_INET;
	myAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	myAddress.sin_port = htons(port);
	return SOCKET_ERROR != bind(socket, reinterpret_cast<const SOCKADDR*>(&myAddress), sizeof(myAddress));
}

bool SocketUtils::Listen(SOCKET socket, int32 backlog) {
	return SOCKET_ERROR != listen(socket, backlog);
}

bool SocketUtils::SetLinger(SOCKET socket, uint16 onoff, uint16 linger)
{
	LINGER option;
	option.l_onoff = onoff;
	option.l_linger = linger;
	return SetSocketOpt(socket, SOL_SOCKET, SO_LINGER, option);
}

bool SocketUtils::SetReuseAddress(SOCKET socket, bool flag)
{
	return SetSocketOpt(socket, SOL_SOCKET, SO_REUSEADDR, flag);
}

bool SocketUtils::SetRecvBufferSize(SOCKET socket, int32 size)
{
	return SetSocketOpt(socket, SOL_SOCKET, SO_RCVBUF, size);
}

bool SocketUtils::SetSendBufferSize(SOCKET socket, int32 size)
{
	return SetSocketOpt(socket, SOL_SOCKET, SO_SNDBUF, size);
}

bool SocketUtils::SetTcpNoDelay(SOCKET socket, bool flag)
{
	return SetSocketOpt(socket, SOL_SOCKET, TCP_NODELAY, flag);
}

bool SocketUtils::SetUpdateAcceptSocket(SOCKET socket, SOCKET listenSocket)
{
	return SetSocketOpt(socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, listenSocket);
}
