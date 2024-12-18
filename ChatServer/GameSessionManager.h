#pragma once

class GameSession;

using GameSessionRef = shared_ptr<GameSession>;

class GameSessionManager {
public:


private:
	USE_LOCK;
	kset<GameSessionRef> _sessions;
};

