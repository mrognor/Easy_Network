#include "EN_UDP_Client.h"

namespace EN
{
	int EN_UDP_Client::GetPort() { return ServerPort; }

	std::string EN_UDP_Client::GetIpAddr() { return ServerIpAddress; }

	EN_SOCKET EN_UDP_Client::GetSocket() { return ServerConnectionSocket; }

	void EN_UDP_Client::Run()
	{
		if ((ServerConnectionSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR)
		{
            LOG(Error, "Failed to create socket");
			exit(1);
		}

		// Server address
		sockaddr_in ServerSockAddr;

		ServerSockAddr.sin_family = AF_INET;
		ServerSockAddr.sin_port = htons(ServerPort);

		// Set ip address
		inet_pton(AF_INET, ServerIpAddress.c_str(), &ServerSockAddr.sin_addr);

		// Get port from os
		EN::UDP_Send(ServerConnectionSocket, "0.0.0.0:0", "");

		std::thread ServerHandlerThread([this]() { this->ServerHandler(); });
		ServerHandlerThread.detach();
	}

	void EN_UDP_Client::ServerHandler()
	{
		std::string message, ipAddress;
		int OperationRes;
		while (true)
		{
			OperationRes = EN::UDP_Recv(ServerConnectionSocket, ipAddress, message);
			
			if (OperationRes > 0)
				ServerMessageHandler(message);
			
			if (OperationRes == 0)
				break;
		}
	}

	void EN_UDP_Client::SendToServer(std::string message)
	{
		EN::UDP_Send(ServerConnectionSocket, ServerIpAddress + ":" + std::to_string(ServerPort), message);
	}

	void EN_UDP_Client::Stop()
	{
		CloseSocket(ServerConnectionSocket);
	}
}
