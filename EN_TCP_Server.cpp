#include "EN_TCP_Server.h"

namespace EN
{
	EN_TCP_Server::EN_TCP_Server()
	{
		IsShutdown.store(false);
	}

	int EN_TCP_Server::GetPort() 
	{ 
		return Port;
	}

	std::string EN_TCP_Server::GetIpAddr() 
	{ 
		return IpAddress;
	}

	size_t EN_TCP_Server::GetConnectionsCount()
	{ 
		CrossWalk.CarStartCrossRoad();
		size_t res = ClientSockets.size();
		CrossWalk.CarStopCrossRoad();
		return res;
	}

	void EN_TCP_Server::Run()
	{
		ShutdownMutex.lock();

		// Configure ip address
		sockaddr_in ServerAddress;
		ServerAddress.sin_family = AF_INET;
		ServerAddress.sin_port = htons(Port);

		// Set ip address
		if (IpAddress.empty())
			ServerAddress.sin_addr.s_addr = htonl(INADDR_ANY);
		else
			inet_pton(AF_INET, IpAddress.c_str(), &ServerAddress.sin_addr);

		// Create socket to listen incoming connections
		ServerListenSocket = socket(AF_INET, SOCK_STREAM, 0);

		if (ServerListenSocket == INVALID_SOCKET)
		{
            LOG(Error, "Error: cannot create socket: " + std::to_string(GetSocketErrorCode()) + " " + EN::GetSocketErrorString());
			throw (std::runtime_error(std::to_string(GetSocketErrorCode())));
		}

		int OperationRes;
		OperationRes = bind(ServerListenSocket, (sockaddr*)&ServerAddress, sizeof(ServerAddress));

		if (OperationRes == SOCKET_ERROR)
		{
            LOG(Error, "Error: cannot bind socket: " + std::to_string(GetSocketErrorCode()) + " " + EN::GetSocketErrorString());
			throw (std::runtime_error(std::to_string(GetSocketErrorCode())));
		}

		OperationRes = listen(ServerListenSocket, SOMAXCONN);

		if (OperationRes == SOCKET_ERROR)
		{
            LOG(Error, "Error: cannot listen socket: " + std::to_string(GetSocketErrorCode()) + " " + EN::GetSocketErrorString());
			throw (std::runtime_error(std::to_string(GetSocketErrorCode())));
		}

		EN_SOCKET IncomingConnection;
		ShutdownMutex.unlock();

		// Handle new connections
		while (true)
		{
            int sizeofaddr = sizeof(ServerAddress);
			#if defined WIN32 || defined _WIN64
			IncomingConnection = accept(ServerListenSocket, (sockaddr*)&ServerAddress, &sizeofaddr);
			#else
			IncomingConnection = accept(ServerListenSocket, (sockaddr*)&ServerAddress, (socklen_t*)&sizeofaddr);
			#endif

			// Shutdown server
			if (IsShutdown.load() == true)
			{
				CrossWalk.PedestrianStartCrossRoad();
				for (EN_SOCKET sock : ClientSockets)
					CloseSocket(sock);
				CrossWalk.PedestrianStopCrossRoad();

				CloseSocket(IncomingConnection);
				CloseSocket(ServerListenSocket);
				ServerListenSocket = INVALID_SOCKET;
				break;
			}

			if (IncomingConnection == INVALID_SOCKET)
			{
				int acceptError = GetSocketErrorCode();
                LOG(Error, "Error: Client connection failure: " + std::to_string(acceptError) + " " + EN::GetSocketErrorString(acceptError));
                LOG(Hint, "Accept on server listen socket return invalid socket. It may occur by invalid server ip address");
				
				// Disconnect all connected clients
				CrossWalk.PedestrianStartCrossRoad();
				for (EN_SOCKET sock : ClientSockets)
					CloseSocket(sock);
				CrossWalk.PedestrianStopCrossRoad();

				CloseSocket(IncomingConnection);
				CloseSocket(ServerListenSocket);
				ServerListenSocket = INVALID_SOCKET;

				// Wait while all clients disconnect
				while (true)
				{
					CrossWalk.PedestrianStartCrossRoad();
					if (ClientSockets.size() == 0)
						break;
					CrossWalk.PedestrianStopCrossRoad();
				}
				CrossWalk.PedestrianStopCrossRoad();

				throw (std::runtime_error(std::to_string(acceptError)));
			}
			else
			{
                for (const auto& it : CreateSocketsOption)
                    EN::SetSocketOption(IncomingConnection, it.Level, it.OptionName, it.OptionValue);

				CrossWalk.PedestrianStartCrossRoad();

				ClientSockets.push_back(IncomingConnection);
				std::thread ClientHandlerThread([this, IncomingConnection]() { this->ClientHandler(IncomingConnection); });
				ClientHandlerThread.detach();

				CrossWalk.PedestrianStopCrossRoad();
			}
		}

		// Wait while all clients disconnect
		while (true)
		{
			CrossWalk.PedestrianStartCrossRoad();
			if (ClientSockets.size() == 0)
				break;
			CrossWalk.PedestrianStopCrossRoad();
		}
		CrossWalk.PedestrianStopCrossRoad();
	}

	void EN_TCP_Server::ClientHandler(EN_SOCKET clientSocket)
	{
		OnClientConnected(clientSocket);

		std::string message;
		bool ConnectionStatus = true;

		while (true)
		{
			ConnectionStatus = EN::TCP_Recv(clientSocket, message);

			if (ConnectionStatus == false)
			{
				OnClientDisconnect(clientSocket);
				break;
			}

			ClientMessageHandler(clientSocket, message);
		}

        CloseSocket(clientSocket);

		CrossWalk.PedestrianStartCrossRoad();

		for (auto sock = ClientSockets.begin(); sock != ClientSockets.end(); ++sock)
		{
			if (*sock == clientSocket)
			{
				ClientSockets.erase(sock);
				break;
			}
		}

		CrossWalk.PedestrianStopCrossRoad();
	}

	void EN_TCP_Server::DisconnectClient(EN_SOCKET clientSocket)
	{
		CloseSocket(clientSocket);
	}

	void EN_TCP_Server::Shutdown()
	{
		IsShutdown.store(true);
		
		// Check what server successfully started
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

	bool EN_TCP_Server::SendToClient(EN_SOCKET clientSocket, std::string message)
	{
		return EN::TCP_Send(clientSocket, message);
	}

    bool EN_TCP_Server::WaitMessage(EN_SOCKET clientSocket, std::string& message)
    {
		return EN::TCP_Recv(clientSocket, message);;
    }

	void EN_TCP_Server::LockClientSockets()
	{
		CrossWalk.CarStartCrossRoad();
	}

	void EN_TCP_Server::UnlockClientSockets()
	{
		CrossWalk.CarStopCrossRoad();
	}

    void EN_TCP_Server::SetAcceptSocketOption(int level, int optionName, int optionValue)
    {
        EN::SetSocketOption(ServerListenSocket, level, optionName, optionValue);
    }

    void EN_TCP_Server::SetAcceptSocketOption(PredefinedSocketOptions socketOptions)
    {
        for (size_t i = 0; i < socketOptions.Levels.size(); ++i)
            EN::SetSocketOption(ServerListenSocket, socketOptions.Levels[i], socketOptions.OptionNames[i], socketOptions.OptionValues[i]);
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
        for (size_t i = 0; i < socketOptions.Levels.size(); ++i)
        {
            SocketOption createOptions;
            createOptions.Level = socketOptions.Levels[i];
            createOptions.OptionName = socketOptions.OptionNames[i];
            createOptions.OptionValue = socketOptions.OptionValues[i];
            CreateSocketsOption.push_back(createOptions);
        }
    }

    void EN_TCP_Server::SetSocketOption(EN_SOCKET clientSocket, int level, int optionName, int optionValue)
    {
        EN::SetSocketOption(clientSocket, level, optionName, optionValue);
    }

    void EN_TCP_Server::SetSocketOption(EN_SOCKET clientSocket, PredefinedSocketOptions socketOptions)
    {
        for (size_t i = 0; i < socketOptions.Levels.size(); ++i)
            	EN::SetSocketOption(clientSocket, socketOptions.Levels[i], socketOptions.OptionNames[i], socketOptions.OptionValues[i]);
    }

	EN_TCP_Server::~EN_TCP_Server() {}
}