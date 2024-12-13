#include "pch.h"
#include "OperationServer.h"

OperationServer::OperationServer() {
}

OperationServer::~OperationServer() {
}

kvector<ChatRoom*> OperationServer::GetChatRooms() {
	return _rooms;
}