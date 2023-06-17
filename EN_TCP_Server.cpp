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
				for (int i = 0; i < ClientSockets.size(); ++i)
					CloseSocket(ClientSockets[i]);
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

				bool WasReusedSocket = false;

				CrossWalk.PedestrianStartCrossRoad();
				for (int i = 0; i < ClientSockets.size(); ++i)
				{
					if (ClientSockets[i] == INVALID_SOCKET)
					{
						ClientSockets[i] = IncomingConnection;
						std::thread ClientHandlerThread([this, i]() { this->ClientHandler(i); });
						ClientHandlerThread.detach();
						WasReusedSocket = true;
						break;
					}
				}
				if (WasReusedSocket == false)
				{
					ClientSockets.push_back(IncomingConnection);

					int i = (int)ClientSockets.size() - 1;
					std::thread ClientHandlerThread([this, i]() { this->ClientHandler(i); });
					ClientHandlerThread.detach();
				}
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

	void EN_TCP_Server::ClientHandler(int ClientID)
	{
		OnClientConnected(ClientID);

		std::string message;
		bool ConnectionStatus = true;
		EN_SOCKET clientSocket = INVALID_SOCKET;

		CrossWalk.CarStartCrossRoad();
		if (ClientSockets.size() > ClientID)
			clientSocket = ClientSockets[ClientID];
		CrossWalk.CarStopCrossRoad();

		while (true)
		{
			ConnectionStatus = EN::TCP_Recv(clientSocket, message);

			if (ConnectionStatus == false)
			{
				OnClientDisconnect(ClientID);
				break;
			}

			ClientMessageHandler(message, ClientID);
		}

        CloseSocket(clientSocket);

		CrossWalk.PedestrianStartCrossRoad();

		ClientSockets[ClientID] = INVALID_SOCKET;
		if (ClientID == ClientSockets.size() - 1)
		{
			for (int i = (int)ClientSockets.size() - 1; i >= 0; --i)
			{
				if (ClientSockets[i] == INVALID_SOCKET)
					ClientSockets.pop_back();
				else
					break;
			}
		}

		CrossWalk.PedestrianStopCrossRoad();
	}

	void EN_TCP_Server::DisconnectClient(int ClientID)
	{
		CrossWalk.CarStartCrossRoad();
		if (ClientSockets.size() > ClientID)
			CloseSocket(ClientSockets[ClientID]);
		CrossWalk.CarStopCrossRoad();
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

	bool EN_TCP_Server::SendToClient(int ClientId, std::string message)
	{
		bool res = false;
		CrossWalk.CarStartCrossRoad();
		if (ClientSockets.size() > ClientId)
			res = EN::TCP_Send(ClientSockets[ClientId], message);
		CrossWalk.CarStopCrossRoad();
		return res;
	}

    bool EN_TCP_Server::WaitMessage(int ClientId, std::string& message)
    {
		EN_SOCKET clientSocket = INVALID_SOCKET;
		CrossWalk.CarStartCrossRoad();
		if (ClientSockets.size() > ClientId)
			clientSocket = ClientSockets[ClientId];
		CrossWalk.CarStopCrossRoad();

		return EN::TCP_Recv(clientSocket, message);;
    }

    void EN_TCP_Server::SetAcceptSocketOption(int level, int optionName, int optionValue)
    {
        EN::SetSocketOption(ServerListenSocket, level, optionName, optionValue);
    }

    void EN_TCP_Server::SetAcceptSocketOption(PredefinedSocketOptions socketOptions)
    {
        for (int i = 0; i < socketOptions.Levels.size(); ++i)
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
		CrossWalk.CarStartCrossRoad();
		if (ClientSockets.size() > ClientID)
        	EN::SetSocketOption(ClientSockets[ClientID], level, optionName, optionValue);
		CrossWalk.CarStopCrossRoad();
    }

    void EN_TCP_Server::SetSocketOption(int ClientID, PredefinedSocketOptions socketOptions)
    {
		CrossWalk.CarStartCrossRoad();
        for (int i = 0; i < socketOptions.Levels.size(); ++i)
			if (ClientSockets.size() > ClientID)
            	EN::SetSocketOption(ClientSockets[ClientID], socketOptions.Levels[i], socketOptions.OptionNames[i], socketOptions.OptionValues[i]);
		CrossWalk.CarStopCrossRoad();
    }
}