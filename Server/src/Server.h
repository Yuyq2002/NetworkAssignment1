#pragma once
#include "Network.h"
#include "Connection.h"
#include <vector>
#include "Entity.h"

class Server
{
	Network network;
	std::vector<Connection> connections;
	double lastSendTime, sendDelay = 0.100;
	double periodTimeStart;

public:
	void Init();

	bool Transmit(const DataBuffer &dataBuffer);
	void Receive(DataBuffer &dataBuffer);

	void WriteConnectionPacket(byte_stream& stream);
	void HandleConnectionPacket(byte_stream_reader& reader, sockaddr_in& remote, DataBuffer& receiveBuffer);

	void WritePayloadPacket(byte_stream& stream, const DataBuffer& sendBuffer);
	void HandlePayloadPacket(byte_stream_reader& reader, sockaddr_in& remote, DataBuffer& receiveBuffer);

	void WriteDisconnectingPacket(byte_stream& stream);
	void HandleDisconnectingPacket(byte_stream_reader& reader, sockaddr_in& remote, DataBuffer& receiveBuffer);

	void ReplicateNewPlayerToClients(Player& newPlayer, std::vector<Entity*>& allEntities);
	void ReplicateEntityToClients(Entity* entity);

	void DestroyReplicatedPlayer(Player* playerToDestroy);

	void DisablePlayerInput(Player& player);
	void EnablePlayerInput(Player& player);

	void SendWinEvent(Player& player);

	void CheckTimeout(DataBuffer& receiveBuffer);
	void SendTimeoutEvent(Connection& connection);

	void DrawConnectionInfo(raylibOverlay::Vector2 position);

	void Clear();

private:
	Connection* FindConnection(sockaddr_in address); 
	Connection* FindConnection(int id);
	void RemoveConnection(sockaddr_in address);
};

