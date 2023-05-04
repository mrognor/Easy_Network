#include "EN_TCP_Server.h"

namespace EN
{
	EN_TCP_Server::EN_TCP_Server()
	{
	}

	void EN_TCP_Server::Run()
	{
		ShutdownMutex.lock();

		// Configure ip address
		sockaddr_in ServerAddress;
		int sizeofaddr = sizeof(ServerAddress);
		ServerAddress.sin_family = AF_INET;
		ServerAddress.sin_port = htons(Port);
		
		// Set ip address
		inet_pton(AF_INET, IpAddress.c_str(), &ServerAddress.sin_addr);

		// Create socket to listen incoming connections
		ServerListenSocket = socket(AF_INET, SOCK_STREAM, 0);

		if (ServerListenSocket == INVALID_SOCKET)
		{
			std::cerr << "Error: cannot create socket" << std::endl;
		}

		int OperationRes;
		OperationRes = bind(ServerListenSocket, (sockaddr*)&ServerAddress, sizeof(ServerAddress));

		if (OperationRes == SOCKET_ERROR)
		{
			std::cerr << "Error: cannot bind socket" << std::endl;
		}

		OperationRes = listen(ServerListenSocket, SOMAXCONN);

		if (OperationRes == SOCKET_ERROR)
		{
			std::cerr << "Error: cannot listen socket" << std::endl;
		}

		EN_SOCKET IncomingConnection;
		ShutdownMutex.unlock();

		// Handle new connections
		while (true)
		{
			#if defined WIN32 || defined _WIN64
			IncomingConnection = accept(ServerListenSocket, (sockaddr*)&ServerAddress, &sizeofaddr);
			#else
			IncomingConnection = accept(ServerListenSocket, (sockaddr*)&ServerAddress, (socklen_t*)&sizeofaddr);
			#endif

			// Shutdown server
			if (IsShutdown == true)
			{
				for (EN_SOCKET sock : ClientSockets)
					CloseSocket(sock);
				
				CloseSocket(IncomingConnection);
				CloseSocket(ServerListenSocket);
				ServerListenSocket = INVALID_SOCKET;
				break;
			}

			if (IncomingConnection == INVALID_SOCKET)
			{
				std::cerr << "Error: Client connection failure." << std::endl;
				std::cerr << "Tip: check server ip" << std::endl;
				break;
			}
			else
			{
                for (const auto& it : CreateSocketsOption)
                    setsockopt(IncomingConnection, it.Level, it.OptionName, (const char *)&it.OptionValue, sizeof(it.OptionValue));

				bool WasReusedSocket = false;
				for (int i = 0; i < ClientSockets.size(); i++)
				{
					if (ClientSockets[i] == INVALID_SOCKET)
					{
						std::thread ClientHandlerThread([this, i]() { this->ClientHandler(i); });
						ClientHandlerThread.detach();
						ClientSockets[i] = IncomingConnection;
						WasReusedSocket = true;
						break;
					}
				}
				if (WasReusedSocket == false)
				{
					ClientSockets.push_back(IncomingConnection);

					std::thread ClientHandlerThread([this]() { this->ClientHandler(ClientSockets.size() - 1); });
					ClientHandlerThread.detach();
				}
			}
		}
	}

	void EN_TCP_Server::ClientHandler(int ClientID)
	{
		OnClientConnected(ClientID);

		std::string message;
		bool ConnectionStatus = true;

		while (true)
		{
			ConnectionStatus = EN::TCP_Recv(ClientSockets[ClientID], message);

			if (ConnectionStatus == false)
			{
				OnClientDisconnect(ClientID);
				break;
			}

			ClientMessageHandler(message, ClientID);
		}

		CloseSocket(ClientID);
		ClientSockets[ClientID] = INVALID_SOCKET;
		if (ClientID == ClientSockets.size() - 1)
			ClientSockets.pop_back();
		return;
	}

	void EN_TCP_Server::DisconnectClient(int ClientID)
	{
		CloseSocket(ClientSockets[ClientID]);
	}

	void EN_TCP_Server::Shutdown()
	{
		IsShutdown = true;

		while (true)
		{
			ShutdownMutex.lock();
			if (ServerListenSocket != INVALID_SOCKET)
			{
				ShutdownMutex.unlock();
				break;
			}
			ShutdownMutex.unlock();
		}

		CloseSocket(ServerListenSocket);
	}

	void EN_TCP_Server::SendToClient(int ClientId, std::string message, int MessageDelay)
	{
		if (ClientId < ClientSockets.size() && ClientSockets[ClientId] != INVALID_SOCKET)
			EN::TCP_Send(ClientSockets[ClientId], message, MessageDelay);
	}

    void EN_TCP_Server::SetAcceptSocketOption(int level, int optionName, int optionValue)
    {
        setsockopt(ServerListenSocket, level, optionName, (const char*)&optionValue, sizeof(optionValue));
    }

    void EN_TCP_Server::SetAcceptSocketOption(PredefinedSocketOptions socketOptions)
    {
        for (int i = 0; i < socketOptions.Levels.size(); ++i)
        {
            setsockopt(ServerListenSocket, socketOptions.Levels[i], socketOptions.OptionNames[i], (const char*)&socketOptions.OptionValues[i], sizeof(socketOptions.OptionValues[i]));
        }
    }

    void EN_TCP_Server::AddOnSocketCreateOption(int level, int optionName, int optionValue)
    {
        SocketOption createOptions;
        createOptions.Level = level;
        createOptions.OptionName = optionName;
        createOptions.OptionValue = optionValue;
        CreateSocketsOption.push_back(createOptions);
    }

    void EN_TCP_Server::AddOnSocketCreateOption(PredefinedSocketOptions socketOptions)
    {
        for (int i = 0; i < socketOptions.Levels.size(); ++i)
        {
            SocketOption createOptions;
            createOptions.Level = socketOptions.Levels[i];
            createOptions.OptionName = socketOptions.OptionNames[i];
            createOptions.OptionValue = socketOptions.OptionValues[i];
            CreateSocketsOption.push_back(createOptions);
        }
    }

    void EN_TCP_Server::SetSocketOption(int ClientID, int level, int optionName, int optionValue)
    {
        setsockopt(ClientSockets[ClientID], level, optionName, (const char*)&optionValue, sizeof(optionValue));
    }

    void EN_TCP_Server::SetSocketOption(int ClientID, PredefinedSocketOptions socketOptions)
    {
        for (int i = 0; i < socketOptions.Levels.size(); ++i)
        {
            setsockopt(ClientSockets[ClientID], socketOptions.Levels[i], socketOptions.OptionNames[i], (const char*)&socketOptions.OptionValues[i], sizeof(socketOptions.OptionValues[i]));
        }
    }
}