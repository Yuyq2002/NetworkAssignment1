#include "Server.h"
#include "IDGenerator.h"
#include <string>

void Server::Init()
{
	network.InitWSA();
	network.CreateSocket();
	network.BindSocket(AF_INET, SERVER_PORT);
	network.SetNonBlocking(1);
}

bool Server::Transmit(const DataBuffer& sendBuffer)
{
	if (connections.empty()) return false;
	if (raylibOverlay::SecondsPassedSinceTime(lastSendTime) < sendDelay) return false;

	for (auto& connection : connections)
	{
		byte_stream stream;

		switch (connection.GetConnectionState())
		{
		case Connecting:
			WriteConnectionPacket(stream);
			break;
		case Connected:
			WritePayloadPacket(stream, sendBuffer);
			break;
		case Disconnecting:
			WriteDisconnectingPacket(stream);
			break;
		}

		periodTimeStart = raylibOverlay::GetTime();
		connection.AddAmountOfByteSent(stream.m_size);
		connection.IncreasePacketSent();
		connection.AddAmountOfMessageSent((int)sendBuffer.headers.size());
		connection.SetCanCalculateRTT(true);
		connection.SetLastSend();

		network.SendData(connection.GetAddress(), stream);

		lastSendTime = raylibOverlay::GetTime();
	}

	return true;
}

void Server::Receive(DataBuffer& receiveBuffer)
{
	byte_stream stream;
	sockaddr_in remote;

	if (!network.RecvData(remote, stream)) return;

	byte_stream_reader reader(stream);

	PacketType packetType;
	reader.serialize_enum(packetType);

	bool validPacket = true;

	switch (packetType)
	{
	case ConnectionPacket:
		HandleConnectionPacket(reader, remote, receiveBuffer);
		break;
	case PayloadPacket:
		HandlePayloadPacket(reader, remote, receiveBuffer);
		break;
	case DisconnectingPacket:
		HandleDisconnectingPacket(reader, remote, receiveBuffer);
		break;
	default:
		validPacket = false;
		break;
	}

	if (validPacket)
	{
		Connection* source = FindConnection(remote);

		if (source == nullptr) return;

		source->SetLastRecv();
		source->CalculateRTT();
		source->AddAmountOfByteReceived(stream.m_size);
		source->IncreasePacketReceived();
		source->AddAmountOfMessageReceived((int)receiveBuffer.headers.size());
	}
}

void Server::WriteConnectionPacket(byte_stream& stream)
{
	byte_stream_writer writer(stream);

	writer.serialize_enum(ConnectionPacket);
	writer.serialize_enum(Connecting);
}

void Server::HandleConnectionPacket(byte_stream_reader& reader, sockaddr_in& remote, DataBuffer& dataBuffer)
{
	ConnectionState connectionState;
	reader.serialize_enum(connectionState);

	switch (connectionState)
	{
	case Connecting:
	{
		if (FindConnection(remote) != nullptr) break;

		Connection newConnection;
		newConnection.SetAddress(remote);
		newConnection.SetConnectioinState(Connecting);

		connections.push_back(newConnection);
		break;
	}
	case WaitPayload:
	{
		Connection* connection = FindConnection(remote);

		if (connection == nullptr) break;
		if (connection->GetConnectionState() == Connected) break;

		int newID = IDGenerator::GenerateID();

		connection->SetConnectioinState(Connected);
		connection->SetID(newID);

		dataBuffer.headers.push_back(DataHeader(CreatePlayer, newID));

		break;
	}
	}
}

void Server::WritePayloadPacket(byte_stream& stream, const DataBuffer& sendBuffer)
{
	byte_stream_writer writer(stream);

	std::vector<raylibOverlay::Vector2>::const_iterator vector2DataIter = sendBuffer.vector2Data.begin();
	std::vector<raylibOverlay::Color>::const_iterator colorDataIter = sendBuffer.colorData.begin();
	std::vector<float>::const_iterator floatDataIter = sendBuffer.floatData.begin();

	writer.serialize_enum(PayloadPacket);
	writer.serialize(raylibOverlay::GetTime() - periodTimeStart);

	for (const auto& header : sendBuffer.headers)
	{
		writer.serialize_enum(header.command);
		writer.serialize(header.id);

		switch (header.command)
		{
		case CreatePlayer:
			writer.serialize_vector_2(*vector2DataIter);
			writer.serialize_color(*colorDataIter);

			vector2DataIter++;
			colorDataIter++;
			break;
		case UpdatePosition:
			writer.serialize_vector_2(*vector2DataIter);

			vector2DataIter++;
			break;
		case UpdateRotation:
			writer.serialize(*floatDataIter);

			floatDataIter++;
		}
	}
}

void Server::HandlePayloadPacket(byte_stream_reader& reader, sockaddr_in& remote, DataBuffer& receiveBuffer)
{
	Connection* connection = FindConnection(remote);

	if (connection == nullptr) return;

	int connectionID = connection->GetID();

	while (true)
	{
		DataHeader receiveHeader;
		raylibOverlay::Vector2 receiveVector2;
		raylibOverlay::Color receiveColor;

		if (!reader.serialize_enum(receiveHeader.command)) break;
		receiveHeader.id = connectionID;

		switch (receiveHeader.command)
		{
		case MovementInput:
			reader.serialize_vector_2(receiveVector2);

			receiveBuffer.headers.push_back(receiveHeader);
			receiveBuffer.vector2Data.push_back(receiveVector2);
			break;

		case AttackInput:
			reader.serialize_vector_2(receiveVector2);

			receiveBuffer.headers.push_back(receiveHeader);
			receiveBuffer.vector2Data.push_back(receiveVector2);
			break;
		case RotationInput:
			reader.serialize_vector_2(receiveVector2);

			receiveBuffer.headers.push_back(receiveHeader);
			receiveBuffer.vector2Data.push_back(receiveVector2);
			break;
		}
	}
}

void Server::WriteDisconnectingPacket(byte_stream& stream)
{
	byte_stream_writer writer(stream);

	writer.serialize_enum(DisconnectingPacket);

	writer.serialize_enum(Disconnecting);
}

void Server::HandleDisconnectingPacket(byte_stream_reader& reader, sockaddr_in& remote, DataBuffer& receiveBuffer)
{
	Connection* connection = FindConnection(remote);

	if (connection == nullptr) return;

	int connectionID = connection->GetID();

	ConnectionState state;

	reader.serialize_enum(state);

	switch (state)
	{
	case Disconnecting:
		if (connection->GetConnectionState() == Disconnecting) break;
		connection->SetConnectioinState(state);
		receiveBuffer.headers.push_back(DataHeader(DestroyPlayer, connectionID));
		break;
	case Disconnected:
		RemoveConnection(remote);
		break;
	}

}

void Server::ReplicateNewPlayerToClients(Player& newPlayer, std::vector<Entity*>& allEntities)
{
	for (auto& connection : connections)
	{
		byte_stream stream;
		byte_stream_writer writer(stream);

		writer.serialize_enum(PayloadPacket);
		writer.serialize(raylibOverlay::GetTime() - periodTimeStart);

		int numOfMessage = 0;
		
		if (connection.GetID() == newPlayer.GetID())
		{
			writer.serialize_enum(PlayerAlive);
			writer.serialize(newPlayer.GetID());

			for (auto& entity : allEntities)
			{
				writer.serialize_enum(CreatePlayer);
				writer.serialize(entity->GetID());
				writer.serialize_vector_2(entity->GetPosition());
				writer.serialize_color(entity->GetColor());
				numOfMessage++;
			}
		}
		else
		{
			writer.serialize_enum(CreatePlayer);
			writer.serialize(newPlayer.GetID());
			writer.serialize_vector_2(newPlayer.GetPosition());
			writer.serialize_color(newPlayer.GetColor());
			numOfMessage++;
		}

		connection.AddAmountOfByteSent(stream.m_size);
		connection.IncreasePacketSent();
		connection.AddAmountOfMessageSent(numOfMessage);

		network.SendData(connection.GetAddress(), stream);
	}
}

void Server::ReplicateEntityToClients(Entity* entity)
{
	byte_stream stream;
	byte_stream_writer writer(stream);

	writer.serialize_enum(PayloadPacket);
	writer.serialize(raylibOverlay::GetTime() - periodTimeStart);
	writer.serialize_enum(CreateBullet);
	writer.serialize(entity->GetID());
	writer.serialize_vector_2(entity->GetPosition());
	writer.serialize_color(entity->GetColor());

	for (auto& connection : connections)
	{
		connection.AddAmountOfByteSent(stream.m_size);
		connection.IncreasePacketSent();
		connection.AddAmountOfMessageSent(1);

		network.SendData(connection.GetAddress(), stream);
	}
}

void Server::DestroyReplicatedPlayer(Player* playerToDestroy)
{
	byte_stream stream;
	byte_stream_writer writer(stream);

	writer.serialize_enum(PayloadPacket);
	writer.serialize(raylibOverlay::GetTime() - periodTimeStart);

	writer.serialize_enum(DestroyPlayer);
	writer.serialize(playerToDestroy->GetID());

	for (auto& connection : connections)
	{
		connection.AddAmountOfByteSent(stream.m_size);
		connection.IncreasePacketSent();
		connection.AddAmountOfMessageSent(1);

		network.SendData(connection.GetAddress(), stream);
	}
}

void Server::DisablePlayerInput(Player& player)
{
	Connection* connection = FindConnection(player.GetID());

	if (connection != nullptr)
	{
		byte_stream stream;
		byte_stream_writer writer(stream);

		writer.serialize_enum(PayloadPacket);
		writer.serialize(raylibOverlay::GetTime() - periodTimeStart);

		writer.serialize_enum(PlayerDied);
		writer.serialize(player.GetID());

		connection->AddAmountOfByteSent(stream.m_size);
		connection->IncreasePacketSent();
		connection->AddAmountOfMessageSent(1);

		network.SendData(connection->GetAddress(), stream);
	}
}

void Server::EnablePlayerInput(Player& player)
{
	Connection* connection = FindConnection(player.GetID());

	if (connection != nullptr)
	{
		byte_stream stream;
		byte_stream_writer writer(stream);

		writer.serialize_enum(PayloadPacket);
		writer.serialize(raylibOverlay::GetTime() - periodTimeStart);

		writer.serialize_enum(PlayerAlive);
		writer.serialize(player.GetID());

		connection->AddAmountOfByteSent(stream.m_size);
		connection->IncreasePacketSent();
		connection->AddAmountOfMessageSent(1);

		network.SendData(connection->GetAddress(), stream);
	}
}

void Server::SendWinEvent(Player& player)
{
	Connection* connection = FindConnection(player.GetID());

	if (connection != nullptr)
	{
		byte_stream stream;
		byte_stream_writer writer(stream);

		writer.serialize_enum(PayloadPacket);
		writer.serialize(raylibOverlay::GetTime() - periodTimeStart);

		writer.serialize_enum(ShowWinner);
		writer.serialize(player.GetID());

		connection->AddAmountOfByteSent(stream.m_size);
		connection->IncreasePacketSent();
		connection->AddAmountOfMessageSent(1);

		network.SendData(connection->GetAddress(), stream);
	}
}

void Server::CheckTimeout(DataBuffer& receiveBuffer)
{
	for (auto& connection : connections)
	{
		if (connection.isTimedout())
		{
			if (connection.GetConnectionState() != Disconnecting || connection.GetConnectionState() != Disconnected)
			{
				receiveBuffer.headers.push_back(DataHeader(DestroyPlayer, connection.GetID()));
				SendTimeoutEvent(connection);
				RemoveConnection(connection.GetAddress());
			}
		}
	}
}

void Server::DrawConnectionInfo(raylibOverlay::Vector2 position)
{

	std::string displayText = "";
	raylibOverlay::Color black = { 0, 0, 0, 255 };
	raylibOverlay::Vector2 textPos = position;
	int size = 20;

	float averageRTT = 0;
	float averageOutKbPS = 0, averageInKbPS = 0;
	float averageInPPS = 0, averageOutPPS = 0;
	float averageTotalByteIn = 0, averageTotalByteOut = 0;
	float averageTotalPacketIn = 0, averageTotalPacketOut = 0;
	float averageTotalMessageIn = 0, averageTotalMessageOut = 0;

	for (auto& connection : connections)
	{
		averageRTT += connection.GetRTT();
		averageInKbPS += connection.GetKbReceivedPerSecond();
		averageOutKbPS += connection.GetKbSentPerSecond();
		averageInPPS += connection.GetPacketReceivedPerSecond();
		averageOutPPS += connection.GetPacketSentPerSecond();
		averageTotalByteIn += connection.GetTotalByteReceived();
		averageTotalByteOut += connection.GetTotalByteSent();
		averageTotalPacketIn += connection.GetTotalPacketReceived();
		averageTotalPacketOut += connection.GetTotalPacketSent();
		averageTotalMessageIn += connection.GetTotalMessageReceived();
		averageTotalMessageOut += connection.GetTotalMessageSent();
	}

	averageRTT /= connections.size();
	averageInKbPS /= connections.size();
	averageOutKbPS /= connections.size();
	averageInPPS /= connections.size();
	averageOutPPS /= connections.size();
	averageTotalByteIn /= connections.size();
	averageTotalByteOut /= connections.size();
	averageTotalPacketIn /= connections.size();
	averageTotalPacketOut /= connections.size();
	averageTotalMessageIn /= connections.size();
	averageTotalMessageOut /= connections.size();

	displayText = "Number of Clients: " + std::to_string(connections.size());
	raylibOverlay::DrawTextV(displayText.c_str(), textPos, size, black);

	textPos.y += 20;
	displayText = "Average RTT(s): " + std::to_string(averageRTT);
	raylibOverlay::DrawTextV(displayText.c_str(), textPos, size, black);

	textPos.y += 20;
	displayText = "Average Kb per second out: " + std::to_string(averageOutKbPS);
	raylibOverlay::DrawTextV(displayText.c_str(), textPos, size, black);

	textPos.y += 20;
	displayText = "Average Kb per second in: " + std::to_string(averageInKbPS);
	raylibOverlay::DrawTextV(displayText.c_str(), textPos, size, black);

	textPos.y += 20;
	displayText = "Average Packet per second out: " + std::to_string(averageOutPPS);
	raylibOverlay::DrawTextV(displayText.c_str(), textPos, size, black);

	textPos.y += 20;
	displayText = "Average Packet per second in: " + std::to_string(averageInPPS);
	raylibOverlay::DrawTextV(displayText.c_str(), textPos, size, black);

	textPos.y += 20;
	displayText = "Average Total byte sent: " + std::to_string(averageTotalByteOut);
	raylibOverlay::DrawTextV(displayText.c_str(), textPos, size, black);

	textPos.y += 20;
	displayText = "Average Total byte received: " + std::to_string(averageTotalByteIn);
	raylibOverlay::DrawTextV(displayText.c_str(), textPos, size, black);

	textPos.y += 20;
	displayText = "Average Total packet sent: " + std::to_string(averageTotalPacketOut);
	raylibOverlay::DrawTextV(displayText.c_str(), textPos, size, black);

	textPos.y += 20;
	displayText = "Average Total packet received: " + std::to_string(averageTotalPacketIn);
	raylibOverlay::DrawTextV(displayText.c_str(), textPos, size, black);

	textPos.y += 20;
	displayText = "Average Total message sent: " + std::to_string(averageTotalMessageOut);
	raylibOverlay::DrawTextV(displayText.c_str(), textPos, size, black);

	textPos.y += 20;
	displayText = "Average Total message received: " + std::to_string(averageTotalMessageIn);
	raylibOverlay::DrawTextV(displayText.c_str(), textPos, size, black);
}

void Server::SendTimeoutEvent(Connection& connection)
{
	byte_stream stream;
	byte_stream_writer writer(stream);

	writer.serialize_enum(DisconnectingPacket);

	writer.serialize_enum(Timedout);

	network.SendData(connection.GetAddress(), stream);
}

void Server::Clear()
{
	network.Clear();
}

Connection* Server::FindConnection(sockaddr_in address)
{
	for (auto &connection : connections)
	{
		sockaddr_in connectionAddress = connection.GetAddress();
		if (connectionAddress.sin_addr.S_un.S_addr == address.sin_addr.S_un.S_addr &&
			connectionAddress.sin_port == address.sin_port)
		{
			return &connection;
		}
	}

	return nullptr;
}

Connection* Server::FindConnection(int id)
{
	for (auto& connection : connections)
	{
		if (connection.GetID() == id)
		{
			return &connection;
		}
	}

	return nullptr;
}

void Server::RemoveConnection(sockaddr_in address)
{
	for (auto connectionIter = connections.begin(); connectionIter != connections.end(); connectionIter++)
	{
		sockaddr_in connectionAddress = connectionIter->GetAddress();

		if (connectionAddress.sin_addr.S_un.S_addr == address.sin_addr.S_un.S_addr &&
			connectionAddress.sin_port == address.sin_port)
		{
			connections.erase(connectionIter);
			return;
		}
	}
}