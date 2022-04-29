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

	int EN_TCP_Client::Connect()
	{
		return Connect(IpAddres, Port);
	}

	int EN_TCP_Client::Connect(int port)
	{
		return Connect(IpAddres, port);
	}

	int EN_TCP_Client::Connect(std::string ipAddr, int port)
	{
		IpAddres = ipAddr;
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

		if (OperationRes != 0)
			std::cerr << "Error: failed connect to server." << std::endl; 
		else
			AfterConnect();
		
		
		return OperationRes;
	}

	void EN_TCP_Client::Run()
	{
		std::thread ServerHandlerThread([this]() { this->ServerHandler(); });
		ServerHandlerThread.detach();
	}

	void EN_TCP_Client::ServerHandler()
	{
		int serverStatus = 1;
		std::string message;

		while (true)
		{
			serverStatus = Recv(ServerConnectionSocket, message);

			// Means what socket closed
			if (serverStatus <= 0)
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
			Send(ServerConnectionSocket, message);
		else std::cerr << "Error: the server is not connected" << std::endl;
	}

	void EN_TCP_Client::Disconnect()
	{
		BeforeDisconnect();

		#ifdef WIN32
		closesocket(ServerConnectionSocket);
		ServerConnectionSocket = INVALID_SOCKET;
		WSACleanup();
		#else 
		close(ServerConnectionSocket);
		ServerConnectionSocket = INVALID_SOCKET;
		#endif
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
