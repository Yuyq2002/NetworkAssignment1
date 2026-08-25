#include "Game.h"
#include "Game.h"
#include "IDGenerator.h"

void Game::Init()
{
	server.Init();
}

void Game::Update()
{
	for(int i = 0; i < 4; i++)
	{
		server.Receive(ReceiveBuffer);
	}

	NetUpdate();

	EntityUpdate();

	CollisionCheck();

	UpdateMatchState();

	RemoveDeadEntity();

	server.CheckTimeout(ReceiveBuffer);

	{
		if (server.Transmit(SendBuffer))
		{
			SendBuffer.ClearBuffer();
		}
	}
}

void Game::Draw()
{
	for (auto& entity : entities)
	{
		entity->Draw();
	}

	raylibOverlay::DrawFPS();

	if (raylibOverlay::IsKeyDown(90))
	{
		server.DrawConnectionInfo({ 100, 20 });
	}
}

void Game::NetUpdate()
{
	std::vector<raylibOverlay::Vector2>::iterator vector2DataIter = ReceiveBuffer.vector2Data.begin();
	std::vector<raylibOverlay::Color>::iterator colorDataIter = ReceiveBuffer.colorData.begin();

	for (auto header : ReceiveBuffer.headers)
	{
		switch (header.command)
		{
		case CreatePlayer:
			CreatePlayerEntity(header.id);
			break;
		case MovementInput:	
		{
			Player* playerToUpdate = FindPlayer(header.id);

			if (playerToUpdate != nullptr)
			{
				playerToUpdate->ProcessMovementInput(*vector2DataIter);

				SendBuffer.headers.push_back(DataHeader(UpdatePosition, playerToUpdate->GetID()));
				SendBuffer.vector2Data.push_back(playerToUpdate->GetPosition());
			}

			vector2DataIter++;
			break;
		}
		case AttackInput:
		{
			Player* playerCreatingBullet = FindPlayer(header.id);

			if (playerCreatingBullet != nullptr)
			{
				if (playerCreatingBullet->ProcessAttackInput() == true)
				{
					CreateBulletEntity(IDGenerator::GenerateID(), playerCreatingBullet, *vector2DataIter);
				}
			}

			vector2DataIter++;
			break;
		}
		case RotationInput:
		{
			Player* playerToUpdate = FindPlayer(header.id);

			if (playerToUpdate != nullptr)
			{
				playerToUpdate->ProcessRotationInput(*vector2DataIter);

				SendBuffer.headers.push_back(DataHeader(UpdateRotation, playerToUpdate->GetID()));
				SendBuffer.floatData.push_back(playerToUpdate->GetRotation());
			}
			vector2DataIter++;
			break;
		}
		case DestroyPlayer:
			Player* playerToDestroy = FindPlayer(header.id);

			if (playerToDestroy != nullptr)
			{
				playerToDestroy->SetIsDead(true);

				server.DestroyReplicatedPlayer(playerToDestroy);
			}

			break;
		}
	}

	ReceiveBuffer.ClearBuffer();
}

void Game::EntityUpdate()
{
	for (auto& entity : entities)
	{
		entity->Update();
	}

	for (auto& bullet : bullets)
	{
		SendBuffer.headers.push_back(DataHeader(UpdatePosition, bullet.GetID()));
		SendBuffer.vector2Data.push_back(bullet.GetPosition());
	}

	for (auto& player : players)
	{
		if (player.GetEnabled())
		{
			if (player.CheckHealth() == false)
			{
				player.SetEnabled(false);
				server.DisablePlayerInput(player);
			}
		}
	}
}

void Game::CollisionCheck()
{
	for (auto& player : players)
	{
		for (auto& bullet : bullets)
		{
			if (raylibOverlay::CheckCollisionCircles(player.GetPosition(), 38, bullet.GetPosition(), 10))
			{
				player.TakeDamage();
				bullet.SetIsDead(true);
			}
		}
	}

	for (auto& bullet : bullets)
	{
		raylibOverlay::Vector2 bulletPosition = bullet.GetPosition();

		if (bulletPosition.x < 0 ||
			bulletPosition.y < 0 ||
			bulletPosition.x > raylibOverlay::GetScreenWidth() ||
			bulletPosition.y > raylibOverlay::GetScreenHeight())
		{
			bullet.SetIsDead(true);
		}
	}
}

void Game::UpdateMatchState()
{
	switch (matchState)
	{
	case Start:
		if (players.size() < 2) return;
		matchState = InProgress;
		RestartGame();
		break;
	case InProgress:
	{
		if (players.empty()) return;
		int playersAlive = 0;
		Player* alivePlayer = nullptr;

		for (auto& player : players)
		{
			if (player.GetEnabled() == true)
			{
				playersAlive++;
				alivePlayer = &player;
			}
		}

		if (playersAlive == 1)
		{
			server.SendWinEvent(*alivePlayer);
			restartTime = raylibOverlay::GetTime() + GAME_RESTART_DELAY;
			matchState = End;
		}
		else if (playersAlive == 0)
		{
			restartTime = raylibOverlay::GetTime() + GAME_RESTART_DELAY;
			matchState = End;
		}
		break;
	}
	case End:
		if (raylibOverlay::GetTime() > restartTime)
		{
			matchState = Start;
		}
		break;
	}
}

void Game::CreatePlayerEntity(int id)
{
	Player newPlayer;
	raylibOverlay::Vector2 newPosition = { float(rand() % 800), float(rand() % 450) };
	raylibOverlay::Color newColor = { unsigned char(rand() % 256), unsigned char(rand() % 256), unsigned char(rand() % 256), 255 };
	newPlayer.Init(id, newPosition, newColor);
	
	players.push_back(newPlayer);
	entities.push_back(&players.back());

	server.ReplicateNewPlayerToClients(newPlayer, entities);
}

void Game::CreateBulletEntity(int id, Player* creatingPlayer, raylibOverlay::Vector2 targetPos)
{
	Bullet newBullet;
	raylibOverlay::Vector2 newPosition = creatingPlayer->GetPosition();
	raylibOverlay::Color newColor = creatingPlayer->GetColor();
	raylibOverlay::Vector2 newDirection = raylibOverlay::Vector2Subtract(targetPos, newPosition);
	newDirection = raylibOverlay::Vector2Normalize(newDirection);

	newPosition.x += newDirection.x * 50;
	newPosition.y += newDirection.y * 50;

	newBullet.Init(id, newPosition, newDirection, newColor);
	
	bullets.push_back(newBullet);
	entities.push_back(&bullets.back());

	server.ReplicateEntityToClients(&newBullet);
}

Player* Game::FindPlayer(int id)
{
	for (auto& player : players)
	{
		if (player.GetID() == id)
		{
			return &player;
		}
	}

	return nullptr;
}

void Game::RestartGame()
{
	for (auto& player : players)
	{
		player.ResetHealth();
		player.SetEnabled(true);
		server.EnablePlayerInput(player);
	}
}

void Game::RemoveDeadEntity()
{
	auto entityIterator = entities.begin();
	while (true)
	{
		if (entityIterator == entities.end()) break;

		if ((*entityIterator)->GetIsDead())
		{
			SendBuffer.headers.push_back(DataHeader(DestroyBullet, (*entityIterator)->GetID()));
			entityIterator = entities.erase(entityIterator);
		}
		else
		{
			entityIterator++;
		}
	}

	bullets.remove_if([](Bullet& a) { return a.GetIsDead(); });
}

void Game::Clear()
{
	server.Clear();
}