#include "Game.h"

void Game::Init()
{
	client.Init();
	playerTexture = raylibOverlay::LoadTexture("../Assets/tank.png");
}

void Game::Update()
{
	if (raylibOverlay::IsKeyPressed(67))
	{
		client.InitConnection();
	}

	if (raylibOverlay::IsKeyPressed(88))
	{
		client.InitDisconnection();
	}

	//if (raylibOverlay::IsKeyPressed(82))
	for(int i = 0; i < 2; i++)
	{
		client.Receive(ReceiveBuffer);
	}

	NetUpdate();

	EntityUpdate();

	CheckInput();

	RemoveDeadEntity();

	//if (!raylibOverlay::IsKeyDown(83))
	{
		if (client.Transmit(SendBuffer))
		{
			SendBuffer.ClearBuffer();
		}
	}
}

void Game::Draw()
{
	for (auto& entity : entities)
	{
		entity.Draw();
	}

	raylibOverlay::DrawFPS();

	if (matchOutcome == Win)
	{
		WinScreen();
	}

	if (matchOutcome == Lose)
	{
		LoseScreen();
	}

	if (raylibOverlay::IsKeyDown(90))
	{
		client.DrawConnectionInfo({ 100, 20 });
	}
}

void Game::Clear()
{
	client.Clear();
}

void Game::NetUpdate()
{
	std::vector<raylibOverlay::Vector2>::iterator vector2DataIter = ReceiveBuffer.vector2Data.begin();
	std::vector<raylibOverlay::Color>::iterator colorDataIter = ReceiveBuffer.colorData.begin();
	std::vector<float>::iterator floatDataIter = ReceiveBuffer.floatData.begin();

	for (auto header : ReceiveBuffer.headers)
	{
		switch (header.command)
		{
		case CreatePlayer:
			CreateEntity(header.id, *vector2DataIter, *colorDataIter, &playerTexture);
			vector2DataIter++;
			colorDataIter++;
			break;
		case CreateBullet:
			CreateEntity(header.id, *vector2DataIter, *colorDataIter, nullptr);
			vector2DataIter++;
			colorDataIter++;
			break;
		case UpdatePosition:
		{
			Entity* entityToUpdate = FindEntity(header.id);
			if (entityToUpdate != nullptr)
			{
				entityToUpdate->AddPosition(*vector2DataIter, ReceiveBuffer.periodTime);

				vector2DataIter++;
			}
			break;
		}
		case UpdateRotation:
		{
			Entity* entityToUpdate = FindEntity(header.id);
			if (entityToUpdate != nullptr)
			{
				entityToUpdate->AddRotation(*floatDataIter, ReceiveBuffer.periodTime);

				floatDataIter++;
			}
			break;
		}
		case PlayerAlive:
		{
			controller.SetEnabled(true);

			Entity* player = FindEntity(header.id);

			if (player != nullptr)
			{
				player->SetEnabled(true);
				matchOutcome = Undetermined;
			}
			break;
		}
		case PlayerDied:
		{
			controller.SetEnabled(false);

			Entity* player = FindEntity(header.id);

			if (player != nullptr)
			{
				player->SetEnabled(false);
				matchOutcome = Lose;
			}
			break;
		}
		case ShowWinner:
			controller.SetEnabled(false);

			matchOutcome = Win;
			break;
		case DestroyBullet:
		{
			Entity* bulletToDestroy = FindEntity(header.id);

			bulletToDestroy->SetIsDead(true);
			break;
		}
		case DestroyAllEntity:
			RemoveAllEntity();
			matchOutcome = Undetermined;
			break;
		}
	}

	ReceiveBuffer.ClearBuffer();
}

void Game::EntityUpdate()
{
	for (auto& entity : entities)
	{
		entity.Update();
	}
}

void Game::CheckInput()
{
	if (controller.InputMovement())
	{
		SendBuffer.headers.push_back(DataHeader(MovementInput, -1));
		SendBuffer.vector2Data.push_back(controller.GetInput());
	}

	if (controller.InputAttack())
	{
		SendBuffer.headers.push_back(DataHeader(AttackInput, -1));
		SendBuffer.vector2Data.push_back(raylibOverlay::GetMousePosition());
	}

	if (controller.InputMouseMove())
	{
		SendBuffer.headers.push_back(DataHeader(RotationInput, -1));
		SendBuffer.vector2Data.push_back(raylibOverlay::GetMousePosition());
	}
}

void Game::CreateEntity(int id, raylibOverlay::Vector2 position, raylibOverlay::Color color, raylibOverlay::Texture* texture)
{
	Entity newEntity;
	newEntity.Init(id, position, color, texture);

	entities.push_back(newEntity);
}

Entity* Game::FindEntity(int id)
{
	for (auto& entity : entities)
	{
		if (entity.GetID() == id)
		{
			return &entity;
		}
	}

	return nullptr;
}

void Game::WinScreen()
{
	raylibOverlay::DrawTextV("YOU WIN", { 50, 150 }, 70, { 0, 255, 0, 255 });
}

void Game::LoseScreen()
{
	raylibOverlay::DrawTextV("YOU LOSE", { 50, 150 }, 70, { 255, 0, 0, 255 });
}

void Game::RemoveDeadEntity()
{
	auto entityIterator = entities.begin();
	while (true)
	{
		if (entityIterator == entities.end()) break;

		if ((*entityIterator).GetIsDead())
		{
			entityIterator = entities.erase(entityIterator);
		}
		else
		{
			entityIterator++;
		}
	}
}

void Game::RemoveAllEntity()
{
	entities.clear();
	SendBuffer.ClearBuffer();
	ReceiveBuffer.ClearBuffer();
}