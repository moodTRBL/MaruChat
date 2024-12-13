#pragma once

class ChatRoom;

class OperationServer {
public:
	OperationServer();
	~OperationServer();

	kvector<ChatRoom*> GetChatRooms();

private:
	kvector<ChatRoom*> _rooms;
};

