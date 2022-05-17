#include "EN_TCP_Client.h"

namespace EN
{
	EN_TCP_Client::EN_TCP_Client()
	{
		#ifdef WIN32
		//WSAStartup
		WSAData wsaData;
		// Winsock operation int result
		int OperationRes;
		OperationRes = WSAStartup(MAKEWORD(2, 1), &wsaData);

		if (OperationRes != 0)
		{
			std::cerr << "WSAStartup failed: " << OperationRes << std::endl;
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
		return Connect(ServerIpAddres, Port);
	}

	bool EN_TCP_Client::Connect(int port)
	{
		return Connect(ServerIpAddres, port);
	}

	bool EN_TCP_Client::Connect(std::string ipAddr, int port)
	{
		ServerIpAddres = ipAddr;
		Port = port;

		sockaddr_in addr;
		int sizeofaddr = sizeof(addr);
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		
		// Set ip address
		inet_pton(AF_INET, ipAddr.c_str(), &addr.sin_addr);

		ServerConnectionSocket = socket(AF_INET, SOCK_STREAM, NULL);

		if (ServerConnectionSocket == INVALID_SOCKET)
		{
			std::cerr << "Error at socket" << std::endl;
		}

		// Winsock operation int result
		int OperationRes;
		OperationRes = connect(ServerConnectionSocket, (sockaddr*)&addr, sizeof(addr));

		if (OperationRes == 0)
		{
			AfterConnect();
			return true;
		}
		else
		{
			std::cerr << "Error: failed connect to server." << std::endl;
			return false;
		}
	}

	void EN_TCP_Client::Run()
	{
		std::thread ServerHandlerThread([this]() { this->ServerHandler(); });
		ServerHandlerThread.detach();
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
				Disconnect();
				return;
			}

			ServerMessageHandler(message);
		}

	}

	void EN_TCP_Client::SendToServer(std::string message, int MessageDelay)
	{
		if (ServerConnectionSocket != INVALID_SOCKET)
			TCP_Send(ServerConnectionSocket, message);
		else std::cerr << "Error: the server is not connected" << std::endl;
	}

	void EN_TCP_Client::Disconnect()
	{
		BeforeDisconnect();

		#ifdef WIN32
		closesocket(ServerConnectionSocket);
		WSACleanup();
		#else 
		close(ServerConnectionSocket);
		#endif
		ServerConnectionSocket = INVALID_SOCKET;
	}

	EN_TCP_Client::~EN_TCP_Client()
	{
		if (ServerConnectionSocket != INVALID_SOCKET)
		{
			std::cerr << "Error: You forgot to disconnect from the server. Use method Disconnect() to do this" << std::endl;
			exit(1);
		}
	}
}
