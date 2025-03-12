#pragma once
#include <WinSock2.h>
#include "Shared.h"

class Connection
{
	int id = -1;
	sockaddr_in address;
	ConnectionState state = ConnectionState::NoConnection;
	double lastRecv, lastSend, timeout = 5;

	int totalByteSent, totalByteReceived;
	int totalPacketSent, totalPacketReceived;
	int totalMessageSent, totalMessageReceived;
	double connectionInitTime;
	double RTT;
	bool canCalcRTT;

public:
	Connection();
	void SetAddress(sockaddr_in addr);
	sockaddr_in& GetAddress();

	void SetConnectioinState(ConnectionState state_);
	ConnectionState GetConnectionState();

	void SetID(int ID);
	int GetID();

	void SetLastRecv();
	void SetLastSend();
	bool isTimedout();

	void IncreasePacketSent();
	void IncreasePacketReceived();
	void AddAmountOfByteSent(int byteSent);
	void AddAmountOfByteReceived(int byteReceived);
	void AddAmountOfMessageSent(int messageSent);
	void AddAmountOfMessageReceived(int messageReceived);
	void CalculateRTT();
	void SetCanCalculateRTT(bool canCalc);

	float GetKbSentPerSecond();
	float GetKbReceivedPerSecond();
	float GetPacketSentPerSecond();
	float GetPacketReceivedPerSecond();
	float GetRTT();
	int GetTotalPacketSent();
	int GetTotalPacketReceived();
	int GetTotalByteSent();
	int GetTotalByteReceived();
	int GetTotalMessageSent();
	int GetTotalMessageReceived();

	void ResetInfo();
};