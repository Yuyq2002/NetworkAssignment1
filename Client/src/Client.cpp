#include "Client.h"
#include <string>

void Client::Init()
{
	//Initialize network
	network.InitWSA();
	network.CreateSocket();
	network.BindSocket(AF_INET);
	network.SetNonBlocking(1);
	network.SetBroadcast(1);

	//Initialize connection
	broadcast.sin_family = AF_INET;
	broadcast.sin_port = htons(SERVER_PORT);
	broadcast.sin_addr.S_un.S_addr = INADDR_BROADCAST;
	connection.SetAddress(broadcast);
}

void Client::InitConnection()
{
	if (connection.GetConnectionState() == Connected) return;
	connection.SetConnectioinState(Connecting);
	connection.ResetInfo();
}

void Client::InitDisconnection()
{
	if (connection.GetConnectionState() == Disconnecting || connection.GetConnectionState() == Disconnected)
	{
		return;
	}

	connection.SetConnectioinState(Disconnecting);
}

bool Client::Transmit(const DataBuffer& sendBuffer)
{
	if (connection.GetConnectionState() == NoConnection) return false;
	if (raylibOverlay::SecondsPassedSinceTime(lastSendTime) < sendDelay) return false;

	byte_stream stream;

	switch (connection.GetConnectionState())
	{
	case Connecting:
		WriteConnectionPacket(stream);
		break;
	case WaitPayload:
		WriteConnectionPacket(stream);
		break;
	case Connected:
		WritePayloadPacket(stream, sendBuffer);
		break;
	case Disconnecting:
		WriteDisconnectingPacket(stream);
		break;
	case Disconnected:
		WriteDisconnectingPacket(stream);
		break;
	}

	connection.AddAmountOfByteSent(stream.m_size);
	connection.IncreasePacketSent();
	connection.AddAmountOfMessageSent((int)sendBuffer.headers.size());
	connection.SetLastSend();

	network.SendData(connection.GetAddress(), stream);

	if (connection.GetConnectionState() == Disconnected)
	{
		connection.SetConnectioinState(NoConnection);
		connection.SetAddress(broadcast);
	}

	return true;
}

void Client::Receive(DataBuffer& receiveBuffer)
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
		HandleConnectionPacket(reader, remote);
		break;
	case PayloadPacket:
		HandlePayloadPacket(reader, receiveBuffer);
		break;
	case DisconnectingPacket:
		HandleDisconnectingPacket(reader, receiveBuffer);
		break;
	default:
		validPacket = false;
		break;
	}

	if (validPacket) {
		Connection* source = &connection;

		if (source == nullptr) return;

		//source->CalculateRTT(sendClockTime, raylibOverlay::GetClockTime());
		source->SetLastRecv();
		source->SetCanCalculateRTT(true);
		source->CalculateRTT();
		source->AddAmountOfByteReceived(stream.m_size);
		source->IncreasePacketReceived();
		source->AddAmountOfMessageReceived((int)receiveBuffer.headers.size());
	}
}

void Client::WriteConnectionPacket(byte_stream& stream)
{
	byte_stream_writer writer(stream);

	writer.serialize_enum(ConnectionPacket);
	writer.serialize_enum(connection.GetConnectionState());
}

void Client::HandleConnectionPacket(byte_stream_reader& reader, sockaddr_in& remote)
{
	ConnectionState connectionState;
	reader.serialize_enum(connectionState);

	if (connectionState == Connecting)
	{
		connection.SetAddress(remote);
		connection.SetConnectioinState(WaitPayload);
	}
}

void Client::WritePayloadPacket(byte_stream& stream, const DataBuffer& sendBuffer)
{
	byte_stream_writer writer(stream);

	std::vector<raylibOverlay::Vector2>::const_iterator vector2DataIter = sendBuffer.vector2Data.begin();
	std::vector<raylibOverlay::Color>::const_iterator colorDataIter = sendBuffer.colorData.begin();

	writer.serialize_enum(PayloadPacket);

	for (const auto& header : sendBuffer.headers)
	{
		writer.serialize_enum(header.command);

		switch (header.command)
		{
		case MovementInput:
			writer.serialize_vector_2(*vector2DataIter);

			vector2DataIter++;
			break;
		case AttackInput:
			writer.serialize_vector_2(*vector2DataIter);

			vector2DataIter++;
			break;
		case RotationInput:
			writer.serialize_vector_2(*vector2DataIter);

			vector2DataIter++;
			break;
		}
	}
}

void Client::HandlePayloadPacket(byte_stream_reader& reader, DataBuffer& receiveBuffer)
{
	if (connection.GetConnectionState() == WaitPayload) connection.SetConnectioinState(Connected);

	reader.serialize(receiveBuffer.periodTime);

	while (true)
	{
		DataHeader receiveHeader;
		raylibOverlay::Vector2 receiveVector2;
		raylibOverlay::Color receiveColor;
		float receiveFloat;

		if(!reader.serialize_enum(receiveHeader.command)) break;
		
		reader.serialize(receiveHeader.id);

		receiveBuffer.headers.push_back(receiveHeader);

		switch (receiveHeader.command)
		{
		case CreatePlayer:
			reader.serialize_vector_2(receiveVector2);
			reader.serialize_color(receiveColor);

			receiveBuffer.vector2Data.push_back(receiveVector2);
			receiveBuffer.colorData.push_back(receiveColor);
			break;
		case CreateBullet:
			reader.serialize_vector_2(receiveVector2);
			reader.serialize_color(receiveColor);

			receiveBuffer.vector2Data.push_back(receiveVector2);
			receiveBuffer.colorData.push_back(receiveColor);
			break;
		case UpdatePosition:
			reader.serialize_vector_2(receiveVector2);

			receiveBuffer.vector2Data.push_back(receiveVector2);
			break;
		case UpdateRotation:
			reader.serialize(receiveFloat);

			receiveBuffer.floatData.push_back(receiveFloat);
		}
	}
}

void Client::WriteDisconnectingPacket(byte_stream& stream) {
	byte_stream_writer writer(stream);

	writer.serialize_enum(DisconnectingPacket);

	writer.serialize_enum(connection.GetConnectionState());
}

void Client::HandleDisconnectingPacket(byte_stream_reader& reader, DataBuffer& receiveBuffer) {
	ConnectionState connectionState;
	reader.serialize_enum(connectionState);
	if (connectionState == Disconnecting) {
		//Disconnection storage free-up
		receiveBuffer.headers.push_back(DataHeader(DestroyAllEntity, -1));
		connection.SetConnectioinState(Disconnected);
	}

	if (connectionState == Timedout)
	{
		receiveBuffer.headers.push_back(DataHeader(DestroyAllEntity, -1));
		connection.SetAddress(broadcast);
		connection.SetConnectioinState(Timedout);
	}
}

void Client::DrawConnectionInfo(raylibOverlay::Vector2 position)
{
	std::string displayText = "";
	raylibOverlay::Color black = { 0, 0, 0, 255 };
	raylibOverlay::Vector2 textPos = position;
	int size = 20;

	displayText = "Average RTT(s): " + std::to_string(connection.GetRTT());
	raylibOverlay::DrawTextV(displayText.c_str(), textPos, size, black);

	textPos.y += 20;
	displayText = "Average Kb per second out: " + std::to_string(connection.GetKbSentPerSecond());
	raylibOverlay::DrawTextV(displayText.c_str(), textPos, size, black);

	textPos.y += 20;
	displayText = "Average Kb per second in: " + std::to_string(connection.GetKbReceivedPerSecond());
	raylibOverlay::DrawTextV(displayText.c_str(), textPos, size, black);

	textPos.y += 20;
	displayText = "Average Packet per second out: " + std::to_string(connection.GetPacketSentPerSecond());
	raylibOverlay::DrawTextV(displayText.c_str(), textPos, size, black);

	textPos.y += 20;
	displayText = "Average Packet per second in: " + std::to_string(connection.GetPacketReceivedPerSecond());
	raylibOverlay::DrawTextV(displayText.c_str(), textPos, size, black);

	textPos.y += 20;
	displayText = "Average Total byte sent: " + std::to_string(connection.GetTotalByteSent());
	raylibOverlay::DrawTextV(displayText.c_str(), textPos, size, black);

	textPos.y += 20;
	displayText = "Average Total byte received: " + std::to_string(connection.GetTotalByteReceived());
	raylibOverlay::DrawTextV(displayText.c_str(), textPos, size, black);

	textPos.y += 20;
	displayText = "Average Total packet sent: " + std::to_string(connection.GetTotalPacketSent());
	raylibOverlay::DrawTextV(displayText.c_str(), textPos, size, black);

	textPos.y += 20;
	displayText = "Average Total packet received: " + std::to_string(connection.GetTotalPacketReceived());
	raylibOverlay::DrawTextV(displayText.c_str(), textPos, size, black);

	textPos.y += 20;
	displayText = "Average Total message sent: " + std::to_string(connection.GetTotalMessageSent());
	raylibOverlay::DrawTextV(displayText.c_str(), textPos, size, black);

	textPos.y += 20;
	displayText = "Average Total message received: " + std::to_string(connection.GetTotalMessageReceived());
	raylibOverlay::DrawTextV(displayText.c_str(), textPos, size, black);
}

void Client::Clear()
{
	network.Clear();
}
