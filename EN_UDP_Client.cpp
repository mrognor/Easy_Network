#include "EN_UDP_Client.h"

namespace EN
{
	int EN_UDP_Client::GetPort() 
	{ 
		return ServerPort; 
	}

	std::string EN_UDP_Client::GetIpAddr() 
	{ 
		return ServerIpAddress; 
	}

	EN_SOCKET EN_UDP_Client::GetSocket() 
	{ 
		return ServerConnectionSocket; 
	}

	bool EN_UDP_Client::Run()
	{
		ClientMtx.lock();
		// Server address
		sockaddr_in ServerSockAddr;

		ServerSockAddr.sin_family = AF_INET;
		ServerSockAddr.sin_port = htons(ServerPort);

		ServerConnectionSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

		if (ServerConnectionSocket == INVALID_SOCKET)
		{
			ClientMtx.unlock();
            LOG(Error, "Failed to create socket");
			return false;
		}

		// Set ip address
		inet_pton(AF_INET, ServerIpAddress.c_str(), &ServerSockAddr.sin_addr);

		for (SocketOption& opt : SocketOptions)
			EN::SetSocketOption(ServerConnectionSocket, opt.Level, opt.OptionName, opt.OptionValue);

		// Get port from os
		UDP_Send(ServerConnectionSocket, "0.0.0.0:0", "");

		std::thread ServerHandlerThread([this]() { this->ServerHandler(); });
		ServerHandlerThread.detach();
		ClientMtx.unlock();
		return true;
	}

	void EN_UDP_Client::ServerHandler()
	{
		std::string message, ipAddress;
		bool operationRes;
		while (true)
		{
			operationRes = UDP_Recv(ServerConnectionSocket, ipAddress, message);
			
			if (!operationRes)
				break;

			ServerMessageHandler(message);
		}
	}

	void EN_UDP_Client::SendToServer(std::string message)
	{
		UDP_Send(ServerConnectionSocket, ServerIpAddress + ":" + std::to_string(ServerPort), message);
	}

	void EN_UDP_Client::Stop()
	{
		CloseSocket(ServerConnectionSocket);
	}

	void EN_UDP_Client::SetSocketOption(int level, int optionName, int optionValue)
    {
		ClientMtx.lock();
		if (ServerConnectionSocket == INVALID_SOCKET)
        	EN::SetSocketOption(ServerConnectionSocket, level, optionName, optionValue);
		else
			SocketOptions.push_back(SocketOption(level, optionName, optionValue));
		ClientMtx.unlock();
    }

    void EN_UDP_Client::SetSocketOption(PredefinedSocketOptions socketOptions)
    {    
		ClientMtx.lock();
		if (ServerConnectionSocket == INVALID_SOCKET)
        	for (size_t i = 0; i < socketOptions.Levels.size(); ++i)
            	EN::SetSocketOption(ServerConnectionSocket, socketOptions.Levels[i], socketOptions.OptionNames[i], socketOptions.OptionValues[i]);
		else
			for (size_t i = 0; i < socketOptions.Levels.size(); ++i)
				SocketOptions.push_back(SocketOption(socketOptions.Levels[i], socketOptions.OptionNames[i], socketOptions.OptionValues[i]));
		ClientMtx.unlock();
	}

	void EN_UDP_Client::SetUDPSendFunction(void (*UDPSendFunction)(EN_SOCKET sock, std::string destinationAddress, const std::string& message))
	{
		UDP_Send = UDPSendFunction;
	}

	void EN_UDP_Client::SetUDPRecvFunction(bool (*UDPRecvFunction)(EN_SOCKET sock, std::string& sourceAddress, std::string& message))
	{
		UDP_Recv = UDPRecvFunction;
	}

	EN_UDP_Client::~EN_UDP_Client() {}
}
