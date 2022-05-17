#pragma once

#ifdef WIN32

#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <WS2tcpip.h>
typedef SOCKET EN_SOCKET;

#else

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
typedef int EN_SOCKET;
#define INVALID_SOCKET -1

#endif

#include <iostream>
#include <string>
#include <thread>

#include "EN_Functions.h"

namespace EN
{
	// Easy network class
	class EN_UDP_Client
	{
	private:

		// Default port
		int Port = 1111;

		// Server ip address string. Default set to localhost
		std::string ServerIpAddres = "127.0.0.1";

		// The server's internal method for processing incoming messages. 
		// Passes the incoming string to method ServerMessageHandler to interpretate incoming message
		void ServerHandler();

	protected:

		int MaxMessageSize = 256;

		// Socket to connect to server
		EN_SOCKET ServerConnectionSocket = INVALID_SOCKET;

		sockaddr_in ServerSockAddr;

		// A function to be defined by the user. It is used to process incoming messages from the server
		virtual void ServerMessageHandler(std::string message) = 0;

	public:
		// Default constructor. Initiate winsock api
		EN_UDP_Client();

		// Port getter
		int GetPort() { return Port; }

		// Ip getter
		std::string GetIpAddr() { return ServerIpAddres; }

		// Socket getter
		EN_SOCKET* GetSocket() { return &ServerConnectionSocket; }

		// Method to start server. Starts a thread to process server responses
		// You have to determine BeforeDisconnect() and ServerMessageHandler()
		void Run();

		// Function for sending a message to a connected server
		// As an additional parameter, it takes the delay in executing the code to send the message
		void SendToServer(std::string message, int MessageDelay = 10);

		void Close();

		// Default destructor
		~EN_UDP_Client();
	};
}

