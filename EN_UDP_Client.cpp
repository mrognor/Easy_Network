#include "EN_UDP_Client.h"

namespace EN
{
	EN_UDP_Client::EN_UDP_Client()
	{
		#if defined WIN32 || defined _WIN64
		//WSAStartup
		WSAData wsaData;
		// Winsock operation int result
		int OperationRes;
		OperationRes = WSAStartup(MAKEWORD(2, 1), &wsaData);

		if (OperationRes != 0)
		{
            LOG(Error, "WSAStartup failed: " + OperationRes);
			exit(1);
		}
		#endif
	}

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
		EN::UDP_Send(ServerConnectionSocket, "0.0.0.0:0", "", 0);

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

	void EN_UDP_Client::SendToServer(std::string message, int MessageDelay)
	{
		EN::UDP_Send(ServerConnectionSocket, ServerIpAddress + ":" + std::to_string(ServerPort), message, MessageDelay);
	}

	void EN_UDP_Client::Stop()
	{
		CloseSocket(ServerConnectionSocket);
	}

	EN_UDP_Client::~EN_UDP_Client()
	{
		#if defined WIN32 || defined _WIN64
		WSACleanup();
		#endif
	}
}
