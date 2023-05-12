#include "EN_TCP_Client.h"

namespace EN
{
	EN_TCP_Client::EN_TCP_Client()
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

	bool EN_TCP_Client::IsConnected()
	{
		if (ServerConnectionSocket != INVALID_SOCKET)
			return true;
		else return false;
	}

	bool EN_TCP_Client::Connect()
	{
		return Connect(ServerIpAddress, ServerPort);
	}

	bool EN_TCP_Client::Connect(int port)
	{
		return Connect(ServerIpAddress, port);
	}

	bool EN_TCP_Client::Connect(std::string ipAddr, int port)
	{
		ServerIpAddress = GetIpByURL(ipAddr);
		if (ServerIpAddress == "")
			ServerIpAddress = ipAddr;

		ServerPort = port;

		sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		
		// Set ip address
		inet_pton(AF_INET, ServerIpAddress.c_str(), &addr.sin_addr);

		ServerConnectionSocket = socket(AF_INET, SOCK_STREAM, 0);

		if (ServerConnectionSocket == INVALID_SOCKET)
        {
            LOG(Error, "Error at socket: " + std::to_string(GETSOCKETERRNO()));
        }
		// Winsock operation int result
		int OperationRes;
        OperationRes = connect(ServerConnectionSocket, (sockaddr*)&addr, sizeof(addr));
            
        if (OperationRes == 0)
			AfterConnect();
        else
        {
			ServerConnectionSocket = INVALID_SOCKET;
            LOG(Warning, "Error: failed connect to server");
            return false;
        }

		std::thread ServerHandlerThread([this]() { this->ServerHandler(); });
		ServerHandlerThread.detach();
		return true;
	}

	void EN_TCP_Client::ServerHandler()
	{
		bool IsServerConnected = true;
		std::string message;

		while (true)
		{
			IsServerConnected = TCP_Recv(ServerConnectionSocket, message);

			// Means what server was disconnected
			if (IsServerConnected == false)
			{
				CloseSocket(ServerConnectionSocket);
				// Check if disconnect was from client side and BeforeDisconnect() was alredy invoked
				if (!IsClientDisconnect)
				{
					ServerConnectionSocket = INVALID_SOCKET;
					BeforeDisconnect();
					IsClientDisconnect = false;
				}
				return;
			}

			ServerMessageHandler(message);
		}

	}

	void EN_TCP_Client::SendToServer(std::string message, int MessageDelay)
	{
		if (ServerConnectionSocket != INVALID_SOCKET)
			TCP_Send(ServerConnectionSocket, message);
		else LOG(Error, "Error: the server is not connected");
	}

	void EN_TCP_Client::Disconnect()
	{
		BeforeDisconnect();
		IsClientDisconnect = true;
		CloseSocket(ServerConnectionSocket);
		ServerConnectionSocket = INVALID_SOCKET;
	}

    void EN_TCP_Client::SetSocketOption(int level, int optionName, int optionValue)
    {
        setsockopt(ServerConnectionSocket, level, optionName, (const char*)&optionValue, sizeof(optionValue));
    }

    void EN_TCP_Client::SetSocketOption(PredefinedSocketOptions socketOptions)
    {        
        for (int i = 0; i < socketOptions.Levels.size(); ++i)
            setsockopt(ServerConnectionSocket, socketOptions.Levels[i], socketOptions.OptionNames[i], (const char*)&socketOptions.OptionValues[i], sizeof(socketOptions.OptionValues[i]));
    }

	EN_TCP_Client::~EN_TCP_Client()
	{
		#if defined WIN32 || defined _WIN64
		WSACleanup();
		#endif
		
		if (ServerConnectionSocket != INVALID_SOCKET)
		{
            LOG(Error, "Error: You forgot to disconnect from the server. Use method Disconnect() to do this");
			exit(1);
		}
	}
}
