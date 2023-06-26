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

		for (const auto& it : AcceptSocketOptions)
                    EN::SetSocketOption(ServerListenSocket, it.Level, it.OptionName, it.OptionValue);
		
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
				// Disconnect all connected clients
				DisconnectAllConnectedClients();

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
				DisconnectAllConnectedClients();

				CloseSocket(IncomingConnection);
				CloseSocket(ServerListenSocket);
				ServerListenSocket = INVALID_SOCKET;

				// Wait while all clients disconnect
				WaitWhileAllClientsDisconnect();

				throw (std::runtime_error(std::to_string(acceptError)));
			}
			else
			{
                for (const auto& it : SocketOptionsAfterConnection)
                    EN::SetSocketOption(IncomingConnection, it.Level, it.OptionName, it.OptionValue);

				CrossWalk.PedestrianStartCrossRoad();

				ClientSockets.insert(IncomingConnection);
				std::thread ClientHandlerThread([this, IncomingConnection]() { this->ClientHandler(IncomingConnection); });
				ClientHandlerThread.detach();

				CrossWalk.PedestrianStopCrossRoad();
			}
		}

		// Wait while all clients disconnect
		WaitWhileAllClientsDisconnect();
	}

	void EN_TCP_Server::ClientHandler(EN_SOCKET clientSocket)
	{
		OnClientConnected(clientSocket);

		std::string message;
		bool ConnectionStatus;

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

		ClientSockets.erase(clientSocket);

		CrossWalk.PedestrianStopCrossRoad();
	}

	void EN_TCP_Server::DisconnectClient(EN_SOCKET clientSocket)
	{
		CrossWalk.CarStartCrossRoad();
		
		bool wasDeletion = false;

		if (ClientSockets.find(clientSocket) != ClientSockets.end())
		{
			CloseSocket(clientSocket);
			wasDeletion = true;
		}

		CrossWalk.CarStopCrossRoad();

		if (!wasDeletion)
		{
			LOG(LogLevels::Warning, "You are trying to close non client socket");
		}
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

		// Wait while all clients disconnect
		WaitWhileAllClientsDisconnect();
	}

	void EN_TCP_Server::DisconnectAllConnectedClients()
	{
		CrossWalk.PedestrianStartCrossRoad();
		for (EN_SOCKET sock : ClientSockets)
			CloseSocket(sock);
		CrossWalk.PedestrianStopCrossRoad();
	}

	void EN_TCP_Server::WaitWhileAllClientsDisconnect()
	{
		while (true)
		{
			CrossWalk.CarStartCrossRoad();
			if (ClientSockets.size() == 0)
				break;
			CrossWalk.CarStopCrossRoad();
		}
		CrossWalk.CarStopCrossRoad();
	}

	bool EN_TCP_Server::SendToClient(EN_SOCKET clientSocket, std::string message)
	{
		CrossWalk.CarStartCrossRoad();
		
		bool wasSend = false;
		bool res = false;

		if (ClientSockets.find(clientSocket) != ClientSockets.end())
		{
			res = EN::TCP_Send(clientSocket, message);
			wasSend = true;
		}
		
		CrossWalk.CarStopCrossRoad();

		if (!wasSend)
		{
			LOG(LogLevels::Warning, "You are trying to send to non client socket");
		}

		return res;
	}

	void EN_TCP_Server::MulticastSend(std::string message)
	{
		CrossWalk.CarStartCrossRoad();

		for (const EN_SOCKET it : ClientSockets)
			EN::TCP_Send(it, message);

		CrossWalk.CarStopCrossRoad();
	}

    bool EN_TCP_Server::WaitMessage(EN_SOCKET clientSocket, std::string& message)
    {
		// Thread safety because this method should be called only inside handler thread
		return EN::TCP_Recv(clientSocket, message);
    }

	void EN_TCP_Server::LockClientSockets()
	{
		CrossWalk.CarStartCrossRoad();
	}

	void EN_TCP_Server::UnlockClientSockets()
	{
		CrossWalk.CarStopCrossRoad();
	}

    void EN_TCP_Server::AddAcceptSocketOption(int level, int optionName, int optionValue)
    {
		AcceptSocketOptions.push_back(SocketOption(level, optionName, optionValue));
    }

    void EN_TCP_Server::AddAcceptSocketOption(PredefinedSocketOptions socketOptions)
    {
        for (size_t i = 0; i < socketOptions.Levels.size(); ++i)
			AcceptSocketOptions.push_back(SocketOption(socketOptions.Levels[i], socketOptions.OptionNames[i], socketOptions.OptionValues[i]));
	}

    void EN_TCP_Server::AddOnSocketCreateOption(int level, int optionName, int optionValue)
    {
        SocketOptionsAfterConnection.push_back(SocketOption(level, optionName, optionValue));
    }

    void EN_TCP_Server::AddOnSocketCreateOption(PredefinedSocketOptions socketOptions)
    {
        for (size_t i = 0; i < socketOptions.Levels.size(); ++i)
            SocketOptionsAfterConnection.push_back(SocketOption(socketOptions.Levels[i], socketOptions.OptionNames[i], socketOptions.OptionValues[i]));
    }

    void EN_TCP_Server::SetSocketOption(EN_SOCKET clientSocket, int level, int optionName, int optionValue)
    {
		CrossWalk.CarStartCrossRoad();

		bool wasOptionSet = false;
		if (ClientSockets.find(clientSocket) != ClientSockets.end())
		{
			EN::SetSocketOption(clientSocket, level, optionName, optionValue);
			wasOptionSet = true;
		}

		CrossWalk.CarStopCrossRoad();

		if (!wasOptionSet)
		{
			LOG(LogLevels::Warning, "You are trying to set socket option on non client socket");
		}
    }

    void EN_TCP_Server::SetSocketOption(EN_SOCKET clientSocket, PredefinedSocketOptions socketOptions)
    {
		CrossWalk.CarStartCrossRoad();

		bool wasOptionSet = false;
		if (ClientSockets.find(clientSocket) != ClientSockets.end())
		{
        	for (size_t i = 0; i < socketOptions.Levels.size(); ++i)
            	EN::SetSocketOption(clientSocket, socketOptions.Levels[i], socketOptions.OptionNames[i], socketOptions.OptionValues[i]);
		}

		CrossWalk.CarStopCrossRoad();

		if (!wasOptionSet)
		{
			LOG(LogLevels::Warning, "You are trying to set socket option on non client socket");
		}
	}

	EN_TCP_Server::~EN_TCP_Server() {}
}