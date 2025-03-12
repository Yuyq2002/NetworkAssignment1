#pragma once
#include "Client.h"
#include "Entity.h"
#include "Controller.h"

enum MatchOutcome
{
	Undetermined,
	Win,
	Lose
};

class Game
{
	Client client;
	DataBuffer SendBuffer, ReceiveBuffer;

	std::vector<Entity> entities;
	Controller controller;

	raylibOverlay::Texture playerTexture;

	MatchOutcome matchOutcome= Undetermined;

public:
	void Init();

	void Update();
	void Draw();

	void Clear();

private:
	void NetUpdate();

	void EntityUpdate();

	void CheckInput();

	void CreateEntity(int id, raylibOverlay::Vector2 position, raylibOverlay::Color color, raylibOverlay::Texture* texture);
	Entity* FindEntity(int id);

	void WinScreen();
	void LoseScreen();

	void RemoveDeadEntity();
	void RemoveAllEntity();
};

