#include "EN_UDP_Client.h"

namespace EN
{
	int EN_UDP_Client::GetPort() { return ServerPort; }

	std::string EN_UDP_Client::GetIpAddr() { return ServerIpAddress; }

	EN_SOCKET EN_UDP_Client::GetSocket() { return ServerConnectionSocket; }

	bool EN_UDP_Client::Run()
	{
		// Server address
		sockaddr_in ServerSockAddr;

		ServerSockAddr.sin_family = AF_INET;
		ServerSockAddr.sin_port = htons(ServerPort);

		ServerConnectionSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

		if (ServerConnectionSocket == INVALID_SOCKET)
		{
            LOG(Error, "Failed to create socket");
			return false;
		}

		// Set ip address
		inet_pton(AF_INET, ServerIpAddress.c_str(), &ServerSockAddr.sin_addr);

		// Get port from os
		EN::UDP_Send(ServerConnectionSocket, "0.0.0.0:0", "");

		std::thread ServerHandlerThread([this]() { this->ServerHandler(); });
		ServerHandlerThread.detach();
		return true;
	}

	void EN_UDP_Client::ServerHandler()
	{
		std::string message, ipAddress;
		bool OperationRes;
		while (true)
		{
			OperationRes = EN::UDP_Recv(ServerConnectionSocket, ipAddress, message);
			
			if (OperationRes)
				ServerMessageHandler(message);
			else
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

	void EN_UDP_Client::SetSocketOption(int level, int optionName, int optionValue)
    {
        EN::SetSocketOption(ServerConnectionSocket, level, optionName, optionValue);
    }

    void EN_UDP_Client::SetSocketOption(PredefinedSocketOptions socketOptions)
    {        
        for (size_t i = 0; i < socketOptions.Levels.size(); ++i)
            EN::SetSocketOption(ServerConnectionSocket, socketOptions.Levels[i], socketOptions.OptionNames[i], socketOptions.OptionValues[i]);
    }

	EN_UDP_Client::~EN_UDP_Client() {}
}
