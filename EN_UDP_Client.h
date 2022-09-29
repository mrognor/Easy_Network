#pragma once

#if defined WIN32 || defined _WIN64 

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
#define SOCKET_ERROR -1
#endif

#include <iostream>
#include <string>
#include <thread>

#include "EN_Functions.h"

namespace EN
{
	/// Base udp client class
	class EN_UDP_Client
	{
	private:
		// Server address
		sockaddr_in ServerSockAddr;

		/// Socket to connect to server
		EN_SOCKET ServerConnectionSocket = INVALID_SOCKET;
		
		// The server's internal method for processing incoming messages. 
		// Passes the incoming string to method ServerMessageHandler to interpretate incoming message
		void ServerHandler();

	protected:

		/// Default port. Default set to 1111
		int ServerPort = 1111;

		/// Maximum number of characters in a message
		int MaxMessageSize = 256;

		/// Server ip address string. Default set to localhost
		std::string ServerIpAddress = "127.0.0.1";

		/// It is used to process incoming messages from the server
		/// \warning Must be defined by the user
		virtual void ServerMessageHandler(std::string message) = 0;

	public:
		// Default constructor. Initiate winsock api
		EN_UDP_Client();

		/// Server port getter
		int GetPort() { return ServerPort; }

		/// Ip getter
		std::string GetIpAddr() { return ServerIpAddress; }

		/// Socket getter
		EN_SOCKET GetSocket() { return ServerConnectionSocket; }

		/// Method to start server. Starts a thread to process server responses
		void Run();

		/**
			\brief Function for sending a message to a connected server

			\param[in] message string to send to server
			\param[in] MessageDelay Additional parameter, it takes the delay in executing the code to send the message
		*/
		void SendToServer(std::string message, int MessageDelay = 10);

		/// Function to stop client
		void Stop();

		// Default destructor
		~EN_UDP_Client();
	};
}

