#include "EN_TCP_Server.h"

namespace EN
{
	EN_TCP_Server::EN_TCP_Server()
	{
		//WSAStartup
		WSAData wsaData;
		if (WSAStartup(MAKEWORD(2, 1), &wsaData) != 0)
		{
			std::cerr << "Error: Library initialization failure." << std::endl;
			exit(1);
		}
	}

	int EN_TCP_Server::RecvFromClient(int ClientSocketID, std::string& message)
	{
		int msg_size;
		int ConnectionStatus = recv(ClientSockets[ClientSocketID], (char*)&msg_size, sizeof(int), NULL);

		if (ConnectionStatus <= 0)
		{
			message = "";
			closesocket(ClientSockets[ClientSocketID]);
			return ConnectionStatus;
		}

		char* msg = new char[msg_size + 1];
		msg[msg_size] = '\0';

		ConnectionStatus = recv(ClientSockets[ClientSocketID], msg, msg_size, NULL);

		if (ConnectionStatus <= 0)
		{
			message = "";
			closesocket(ClientSockets[ClientSocketID]);
			return ConnectionStatus;
		}

		message = msg;
		delete[] msg;
		return ConnectionStatus;
	}

	void EN_TCP_Server::Run()
	{
		// Configure ip address
		SOCKADDR_IN ServerAddress;
		int sizeofaddr = sizeof(ServerAddress);
		ServerAddress.sin_family = AF_INET;
		ServerAddress.sin_port = htons(Port);
		
		// Set ip address
		inet_pton(AF_INET, IpAddress.c_str(), &ServerAddress.sin_addr);

		// Start socket to listen incoming connections
		SOCKET ServerListenSocket = socket(AF_INET, SOCK_STREAM, NULL);
		
		if (ServerListenSocket == INVALID_SOCKET)
		{
			std::cerr << "Error: cannot create socket" << std::endl;
		}

		int OperationRes;
		OperationRes = bind(ServerListenSocket, (SOCKADDR*)&ServerAddress, sizeof(ServerAddress));

		if (OperationRes == SOCKET_ERROR)
		{
			std::cerr << "Error: cannot bind socket" << std::endl;
		}

		OperationRes = listen(ServerListenSocket, SOMAXCONN);

		if (OperationRes == SOCKET_ERROR)
		{
			std::cerr << "Error: cannot listen socket" << std::endl;
		}

		SOCKET IncomingConnection;

		// Handle new connections
		while (true)
		{
			IncomingConnection = accept(ServerListenSocket, (SOCKADDR*)&ServerAddress, &sizeofaddr);

			// Shutdown server
			if (IsShutdown == true)
			{
				for (SOCKET sock : ClientSockets)
				{
					closesocket(sock);
				}
				WSACleanup();
				break;
			}

			if (IncomingConnection == INVALID_SOCKET)
			{
				std::cerr << "Error: Client connection failure." << std::endl;
			}
			else
			{
				bool WasReusedSocket = false;
				for (int i = 0; i < ClientSockets.size(); i++)
				{
					if (ClientSockets[i] == INVALID_SOCKET)
					{
						//CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandler, (LPVOID)(i), NULL, NULL);
						std::thread ClientHandlerThread([this, i]() { this->ClientHandler(i); });
						ClientHandlerThread.detach();
						ClientSockets[i] = IncomingConnection;
						WasReusedSocket = true;
						break;
					}
				}
				if (WasReusedSocket == false)
				{
					//CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandler, (LPVOID)(ClientSockets.size()), NULL, NULL);
					std::thread ClientHandlerThread([this]() { this->ClientHandler(ClientSockets.size() - 1); });
					ClientHandlerThread.detach();

					ClientSockets.push_back(IncomingConnection);
				}
			}
		}
	}

	void EN_TCP_Server::ClientHandler(int ClientID)
	{
		OnClientConnected(ClientID);

		std::string message;
		int ConnectionStatus;

		while (true)
		{
			ConnectionStatus = RecvFromClient(ClientID, message);

			if (ConnectionStatus <= 0)
			{
				OnClientDisconnect(ClientID);
				break;
			}

			ClientMessageHandler(message, ClientID);
		}

		closesocket(ClientID);
		ClientSockets[ClientID] = INVALID_SOCKET;
		if (ClientID == ClientSockets.size() - 1)
			ClientSockets.pop_back();
		return;
	}

	void EN_TCP_Server::DisconnectClient(int ClientID)
	{
		closesocket(ClientSockets[ClientID]);
	}

	void EN_TCP_Server::Shutdown()
	{
		IsShutdown = true;

		SOCKADDR_IN addr;
		int sizeofaddr = sizeof(addr);
		addr.sin_family = AF_INET;
		addr.sin_port = htons(Port);
		
		// Set ip address
		inet_pton(AF_INET, IpAddress.c_str(), &addr.sin_addr);

		SOCKET ServerConnectionSocket = socket(AF_INET, SOCK_STREAM, NULL);

		if (ServerConnectionSocket == INVALID_SOCKET)
		{
			std::cerr << "Error at socket() : " << WSAGetLastError() << std::endl;
		}

		int OperationRes = connect(ServerConnectionSocket, (SOCKADDR*)&addr, sizeof(addr));

		if (OperationRes != 0)
			std::cerr << "Error: failed connect to server." << std::endl;
		closesocket(ServerConnectionSocket);
		WSACleanup();
	}
}
