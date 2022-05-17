#include "EN_UDP_Client.h"

namespace EN
{
	EN_UDP_Client::EN_UDP_Client()
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

		if ((ServerConnectionSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR)
		{
			std::cerr << "Failed to create socket" << std::endl;
			exit(1);
		}

		ServerSockAddr.sin_family = AF_INET;
		ServerSockAddr.sin_port = htons(Port);

		// Set ip address
		inet_pton(AF_INET, ServerIpAddres.c_str(), &ServerSockAddr.sin_addr);
	}

	void EN_UDP_Client::Run()
	{
		std::thread ServerHandlerThread([this]() { this->ServerHandler(); });
		ServerHandlerThread.detach();
	}

	void EN_UDP_Client::ServerHandler()
	{
		// Get port from os
		sendto(ServerConnectionSocket, "", MaxMessageSize, 0, (sockaddr*)&ServerSockAddr, sizeof(ServerSockAddr));

		char* Message = new char[MaxMessageSize];
		memset(Message, '\0', MaxMessageSize);

		int sizeofaddr = sizeof(ServerSockAddr);
		int OperationRes;
		while (true)
		{
			OperationRes = recvfrom(ServerConnectionSocket, Message, MaxMessageSize, 0, (sockaddr*)&ServerSockAddr, &sizeofaddr);

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
			std::cerr << "Failed to send" << std::endl;
		}
		
		#ifdef WIN32
		Sleep(MessageDelay);
		#else
		usleep(MessageDelay);
		#endif
	}

	void EN_UDP_Client::Close()
	{
		#ifdef WIN32
		closesocket(ServerConnectionSocket);
		#else
		close(ServerConnectionSocket);
		#endif
	}

	EN_UDP_Client::~EN_UDP_Client()
	{
		#ifdef WIN32
		WSACleanup();

		#endif
	}
}
