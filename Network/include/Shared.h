#pragma once
#include <vector>
#include "raylibOverlay.h"

enum PacketType
{
	ConnectionPacket,
	PayloadPacket,
	DisconnectingPacket
};

enum ConnectionState
{
	NoConnection = -1,
	Connecting,
	WaitPayload,
	Connected,
	Disconnecting,
	Disconnected,
	Timedout
};

enum Command
{
	Undefined = -1,
	CreatePlayer,
	UpdatePosition,
	UpdateRotation,
	MovementInput,
	RotationInput,
	AttackInput,
	CreateBullet,
	DestroyBullet,
	PlayerAlive,
	PlayerDied,
	ShowWinner,
	DestroyAllEntity,
	DestroyPlayer
};

struct DataHeader
{
	Command command;
	int id;

	DataHeader()
	{
		command = Undefined;
		id = -1;
	}

	DataHeader(Command command, int id)
	{
		this->command = command;
		this->id = id;
	}
};

struct DataBuffer
{
	std::vector<DataHeader> headers;
	std::vector<raylibOverlay::Vector2> vector2Data;
	std::vector<raylibOverlay::Color> colorData;
	std::vector<float> floatData;

	double periodTime;

	void ClearBuffer()
	{
		headers.clear();
		vector2Data.clear();
		colorData.clear();
		floatData.clear();
	}
};