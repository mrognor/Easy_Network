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
		size_t res;
		CrossWalk.CarStartCrossRoad();
		res = ClientSockets.size();
		CrossWalk.CarStopCrossRoad();
		return res;
	}

	void EN_TCP_Server::Run()
	{
		ShutdownMutex.lock();

		if (IsShutdown.load())
		{
			ShutdownMutex.unlock();
			IsShutdown.store(false);
			return;
		}

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
			ShutdownMutex.unlock();
			throw (std::runtime_error(std::to_string(GetSocketErrorCode())));
		}

		int OperationRes;
		OperationRes = bind(ServerListenSocket, (sockaddr*)&ServerAddress, sizeof(ServerAddress));

		if (OperationRes == SOCKET_ERROR)
		{
            LOG(Error, "Error: cannot bind socket: " + std::to_string(GetSocketErrorCode()) + " " + EN::GetSocketErrorString());
			ShutdownMutex.unlock();
			throw (std::runtime_error(std::to_string(GetSocketErrorCode())));
		}

		OperationRes = listen(ServerListenSocket, SOMAXCONN);

		if (OperationRes == SOCKET_ERROR)
		{
            LOG(Error, "Error: cannot listen socket: " + std::to_string(GetSocketErrorCode()) + " " + EN::GetSocketErrorString());
			ShutdownMutex.unlock();
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

				ShutdownMutex.lock();
				CloseSocket(IncomingConnection);
				CloseSocket(ServerListenSocket);
				ServerListenSocket = INVALID_SOCKET;
				ShutdownMutex.unlock();
				break;
			}

			if (IncomingConnection == INVALID_SOCKET)
			{
				int acceptError = GetSocketErrorCode();
                LOG(Error, "Error: Client connection failure: " + std::to_string(acceptError) + " " + EN::GetSocketErrorString(acceptError));
                LOG(Hint, "Accept on server listen socket return invalid socket. It may occur by invalid server ip address");
				
				// Disconnect all connected clients
				DisconnectAllConnectedClients();

				ShutdownMutex.lock();
				CloseSocket(IncomingConnection);
				CloseSocket(ServerListenSocket);
				ServerListenSocket = INVALID_SOCKET;
				ShutdownMutex.unlock();

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
		IsShutdown.store(false);
	}

	void EN_TCP_Server::ClientHandler(EN_SOCKET clientSocket)
	{
		OnClientConnected(clientSocket);

		std::string message;
		bool ConnectionStatus;

		while (true)
		{
			ConnectionStatus = TCP_Recv(clientSocket, message);

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
		ShutdownMutex.lock();
		if (ServerListenSocket != INVALID_SOCKET)
			CloseSocket(ServerListenSocket);
		ShutdownMutex.unlock();
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
		
        // Variable to store sending result
        // -2 - no client with this socket id on server
        // -1 - failed to send to client. For example if socket closed at sending time
        // 0 - sending succeeded
		int resCode = -2;

		if (ClientSockets.find(clientSocket) != ClientSockets.end())
		{
			if(!TCP_Send(clientSocket, message))
                resCode = -1;
            else 
                resCode = 0;
		}
		
		CrossWalk.CarStopCrossRoad();

		if (resCode == -2)
		{
			LOG(LogLevels::Warning, "You are trying to send to non client socket. Socket descriptor: " + std::to_string(clientSocket));
            LOG(LogLevels::Hint, "Check that you are dont forget to lock sockets. This means that no one is already connected to the called socket" + std::to_string(clientSocket));
            return false;
		}
        
        if (resCode == -1)
		{
			LOG(LogLevels::Warning, "Failed to send data to socket. Socket descriptor: " + std::to_string(clientSocket));
            return false;
        }

		return true;
	}

	void EN_TCP_Server::MulticastSend(std::string message)
	{
		CrossWalk.CarStartCrossRoad();

		for (const EN_SOCKET it : ClientSockets)
			TCP_Send(it, message);

		CrossWalk.CarStopCrossRoad();
	}

    bool EN_TCP_Server::WaitMessage(EN_SOCKET clientSocket, std::string& message)
    {
		// Thread safety because this method should be called only inside handler thread
		return TCP_Recv(clientSocket, message);
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

	void EN_TCP_Server::SetTCPSendFunction(bool (*TCPSendFunction)(EN_SOCKET, const std::string&))
	{
		ShutdownMutex.lock();
		if (ServerListenSocket == INVALID_SOCKET)
			TCP_Send = TCPSendFunction;
		else
			LOG(LogLevels::Error, "Try to set TCP send function while server alredy running");
		ShutdownMutex.unlock();
	}

	void EN_TCP_Server::SetTCPRecvFunction(bool (*TCPRecvFunction)(EN_SOCKET, std::string&))
	{
		ShutdownMutex.lock();
		if (ServerListenSocket == INVALID_SOCKET)
			TCP_Recv = TCPRecvFunction;
		else
			LOG(LogLevels::Error, "Try to set TCP send function while server alredy running");
		ShutdownMutex.unlock();
	}

	EN_TCP_Server::~EN_TCP_Server() {}
}