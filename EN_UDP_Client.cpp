#include "EN_UDP_Client.h"

namespace EN
{
	EN_UDP_Client::EN_UDP_Client()
	{
		#if defined WIN32 || defined _WIN64
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

	void EN_UDP_Client::Run()
	{
		if ((ServerConnectionSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR)
		{
			std::cerr << "Failed to create socket" << std::endl;
			exit(1);
		}

		ServerSockAddr.sin_family = AF_INET;
		ServerSockAddr.sin_port = htons(ServerPort);

		// Set ip address
		inet_pton(AF_INET, ServerIpAddress.c_str(), &ServerSockAddr.sin_addr);

		// Get port from os
		sendto(ServerConnectionSocket, "", MaxMessageSize, 0, (sockaddr*)&ServerSockAddr, sizeof(ServerSockAddr));

		std::thread ServerHandlerThread([this]() { this->ServerHandler(); });
		ServerHandlerThread.detach();
	}

	void EN_UDP_Client::ServerHandler()
	{
		char* Message = new char[MaxMessageSize];
		memset(Message, '\0', MaxMessageSize);

		int sizeofaddr = sizeof(ServerSockAddr);
		int OperationRes;
		while (true)
		{
			#if defined WIN32 || defined _WIN64
			OperationRes = recvfrom(ServerConnectionSocket, Message, MaxMessageSize, 0, (sockaddr*)&ServerSockAddr, &sizeofaddr);
			#else
			OperationRes = recvfrom(ServerConnectionSocket, Message, MaxMessageSize, 0, (sockaddr*)&ServerSockAddr, (socklen_t*)&sizeofaddr);
			#endif
			
			if (OperationRes > 0)
			{
				ServerMessageHandler(Message);
			}
			if (OperationRes == 0)
			{
				break;
			}
		}

		delete[] Message;
	}

	void EN_UDP_Client::SendToServer(std::string message, int MessageDelay)
	{
		if (sendto(ServerConnectionSocket, message.c_str(), MaxMessageSize, 0, (sockaddr*)&ServerSockAddr, sizeof(ServerSockAddr)) == SOCKET_ERROR)
		{
			std::cerr << "Error: Failed to send to server" << std::endl;
		}
		
		EN::Delay(MessageDelay);
	}

	void EN_UDP_Client::Stop()
	{
		CloseSocket(ServerConnectionSocket);
	}

	EN_UDP_Client::~EN_UDP_Client()
	{
		#if defined WIN32 || defined _WIN64
		WSACleanup();
		#endif
	}
}
