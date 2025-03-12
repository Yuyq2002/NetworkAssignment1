#include "Connection.h"
#include "raylibOverlay.h"
#include <iostream>

Connection::Connection()
{
	connectionInitTime = 0;

	lastRecv = 0;

	RTT = 0;
	totalByteSent = 0;
	totalByteReceived = 0;
	totalPacketSent = 0;
	totalPacketReceived = 0;
	totalMessageSent = 0;
	totalMessageReceived = 0;
}

void Connection::SetAddress(sockaddr_in addr)
{
	address = addr;
}

sockaddr_in& Connection::GetAddress()
{
	return address;
}

void Connection::SetConnectioinState(ConnectionState state_)
{
	this->state = state_;
}

ConnectionState Connection::GetConnectionState()
{
	return state;
}

void Connection::SetLastRecv()
{
	lastRecv = raylibOverlay::GetTime();
}

void Connection::SetLastSend()
{
	lastSend = raylibOverlay::GetTime();
}

bool Connection::isTimedout()
{
	if (raylibOverlay::SecondsPassedSinceTime(lastRecv) >= timeout)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void Connection::SetID(int ID)
{
	this->id = ID;
}

int Connection::GetID()
{
	return id;
}

void Connection::IncreasePacketSent()
{
	totalPacketSent++;
}

void Connection::IncreasePacketReceived()
{
	totalPacketReceived++;
}

void Connection::AddAmountOfByteSent(int byteSent)
{
	totalByteSent += byteSent;
}

void Connection::AddAmountOfByteReceived(int byteReceived)
{
	totalByteReceived += byteReceived;
}

void Connection::AddAmountOfMessageSent(int messageSent)
{
	totalMessageSent += messageSent;
}

void Connection::AddAmountOfMessageReceived(int messageReceived)
{
	totalMessageReceived += messageReceived;
}

void Connection::CalculateRTT()
{
	if (canCalcRTT)
	{
		RTT = (double(lastRecv - lastSend));
		canCalcRTT = false;
	}
}

void Connection::SetCanCalculateRTT(bool canCalc)
{
	canCalcRTT = canCalc;
}

float Connection::GetKbSentPerSecond()
{
	return float((totalByteSent / 1000.0f) / (raylibOverlay::GetTime() - connectionInitTime));
}

float Connection::GetKbReceivedPerSecond()
{
	return float((totalByteReceived / 1000.0f) / (raylibOverlay::GetTime() - connectionInitTime));
}

float Connection::GetPacketSentPerSecond()
{
	return float((totalPacketSent) / (raylibOverlay::GetTime() - connectionInitTime));
}

float Connection::GetPacketReceivedPerSecond()
{
	return float((totalPacketReceived) / (raylibOverlay::GetTime() - connectionInitTime));
}

float Connection::GetRTT()
{
	return (float)RTT;
}

int Connection::GetTotalPacketSent()
{
	return totalPacketSent;
}

int Connection::GetTotalPacketReceived()
{
	return totalPacketReceived;
}

int Connection::GetTotalByteSent()
{
	return totalByteSent;
}

int Connection::GetTotalByteReceived()
{
	return totalByteReceived;
}

int Connection::GetTotalMessageSent()
{
	return totalMessageSent;
}

int Connection::GetTotalMessageReceived()
{
	return totalMessageReceived;
}

void Connection::ResetInfo()
{
	totalByteSent = 0;
	totalByteReceived = 0;
	totalPacketSent = 0;
	totalPacketReceived = 0;
	totalMessageSent = 0; totalMessageReceived = 0;
	connectionInitTime = raylibOverlay::GetTime();
	RTT = 0;
}