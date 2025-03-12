#pragma once
#include "Server.h"
#include <vector>
#include <list>
#include "Entity.h"

#define GAME_RESTART_DELAY 3.0

enum MatchState
{
	None = -1,
	Start,
	InProgress,
	End
};

class Game
{
	Server server;
	DataBuffer SendBuffer, ReceiveBuffer;

	std::vector<Entity*> entities;
	std::list<Player> players;
	std::list<Bullet> bullets;

	MatchState matchState = InProgress;
	double restartTime;

public:
	void Init();

	void Update();
	void Draw();

	void Clear();

private:
	void NetUpdate();
	void EntityUpdate();
	void CollisionCheck();
	void UpdateMatchState();

	void CreatePlayerEntity(int id);
	void CreateBulletEntity(int id, Player* creatingPlayer, raylibOverlay::Vector2 targetPos);
	Player* FindPlayer(int id);
	void RestartGame();

	void RemoveDeadEntity();
};

