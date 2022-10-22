#include "EN_TCP_Server.h"

namespace EN
{
	EN_TCP_Server::EN_TCP_Server()
	{
		#if defined WIN32 || defined _WIN64
		//WSAStartup
		WSAData wsaData;
		if (WSAStartup(MAKEWORD(2, 1), &wsaData) != 0)
		{
			std::cerr << "Error: Library initialization failure." << std::endl;
			exit(1);
		}
		#endif
	}

	int EN_TCP_Server::RecvFromClient(int ClientSocketID, std::string& message)
	{
		int msg_size;
		int ConnectionStatus = recv(ClientSockets[ClientSocketID], (char*)&msg_size, sizeof(int), MSG_WAITALL);

		if (ConnectionStatus <= 0)
		{
			message = "";

			#if defined WIN32 || defined _WIN64
			closesocket(ClientSockets[ClientSocketID]);
			#else 
			close(ClientSockets[ClientSocketID]);
			#endif

			return ConnectionStatus;
		}

		if (msg_size <= 0)
		{
			message = "";
			return true;
		}

		char* msg = new char[msg_size + 1];
		msg[msg_size] = '\0';

		ConnectionStatus = recv(ClientSockets[ClientSocketID], msg, msg_size, MSG_WAITALL);

		if (ConnectionStatus <= 0)
		{
			message = "";

			#if defined WIN32 || defined _WIN64
			closesocket(ClientSockets[ClientSocketID]);
			#else 
			close(ClientSockets[ClientSocketID]);
			#endif
			delete[] msg;
			return ConnectionStatus;
		}

		message = msg;
		delete[] msg;
		return ConnectionStatus;
	}

	void EN_TCP_Server::Run()
	{
		// Configure ip address
		sockaddr_in ServerAddress;
		int sizeofaddr = sizeof(ServerAddress);
		ServerAddress.sin_family = AF_INET;
		ServerAddress.sin_port = htons(Port);
		
		// Set ip address
		inet_pton(AF_INET, IpAddress.c_str(), &ServerAddress.sin_addr);

		// Start socket to listen incoming connections
		EN_SOCKET ServerListenSocket = socket(AF_INET, SOCK_STREAM, NULL);
		
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
				{
					#if defined WIN32 || defined _WIN64
					closesocket(sock);
					#else
					close(sock);
					#endif
				}
				#if defined WIN32 || defined _WIN64
				closesocket(IncomingConnection); 
				closesocket(ServerListenSocket);
				WSACleanup();
				#else
				close(IncomingConnection);
				close(ServerListenSocket);
				#endif

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

		#if defined WIN32 || defined _WIN64
		closesocket(ClientID);
		#else
		close(ClientID);
		#endif

		ClientSockets[ClientID] = INVALID_SOCKET;
		if (ClientID == ClientSockets.size() - 1)
			ClientSockets.pop_back();
		return;
	}

	void EN_TCP_Server::DisconnectClient(int ClientID)
	{
		#if defined WIN32 || defined _WIN64
		closesocket(ClientSockets[ClientID]);
		#else
		shutdown(ClientSockets[ClientID], 2);
		close(ClientSockets[ClientID]);
		#endif
	}

	void EN_TCP_Server::Shutdown()
	{
		IsShutdown = true;

		sockaddr_in CurrentServerAddress;
		int sizeofaddr = sizeof(CurrentServerAddress);
		CurrentServerAddress.sin_family = AF_INET;
		CurrentServerAddress.sin_port = htons(Port);
		
		// Set ip address
		inet_pton(AF_INET, IpAddress.c_str(), &CurrentServerAddress.sin_addr);

		EN_SOCKET ServerConnectionSocket = socket(AF_INET, SOCK_STREAM, NULL);

		if (ServerConnectionSocket == INVALID_SOCKET)
		{
			std::cerr << "Error at socket " << std::endl;
		}

		int OperationRes = connect(ServerConnectionSocket, (sockaddr*)&CurrentServerAddress, sizeof(CurrentServerAddress));

		if (OperationRes != 0)
			std::cerr << "Error: failed connect to server." << std::endl;
		
		#if defined WIN32 || defined _WIN64
		closesocket(ServerConnectionSocket);
		WSACleanup();
		#else
		close(ServerConnectionSocket);
		#endif		
	}

	void EN_TCP_Server::SendToClient(int ClientId, std::string message, int MessageDelay)
	{
		if (ClientId < ClientSockets.size() && ClientSockets[ClientId] != INVALID_SOCKET)
			EN::TCP_Send(ClientSockets[ClientId], message, MessageDelay);
	}
}
