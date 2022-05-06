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
	class EN_TCP_Client
	{
	private:

		// Default port
		int Port = 1111;

		// Client ip address string. Default set to localhost
		std::string IpAddres = "127.0.0.1";

		// The server's internal method for processing incoming messages. 
		// Passes the incoming string to method ServerMessageHandler to interpretate incoming message
		void ServerHandler();

	protected:

		// Socket to connect to server
		EN_SOCKET ServerConnectionSocket = INVALID_SOCKET;

		// A function to be defined by the user. It is used for logic after connection
		virtual void AfterConnect() = 0;

		// A function to be defined by the user. It is used to process incoming messages from the server
		virtual void ServerMessageHandler(std::string message) = 0;

		// A function to be defined by the user. Performed before disconnected from the server
		virtual void BeforeDisconnect() = 0;

	public:
		// Default constructor. Initiate winsock api
		EN_TCP_Client();

		// Port getter
		int GetPort() { return Port; }

		// Ip getter
		std::string GetIpAddr() { return IpAddres; }

		// Socket getter
		EN_SOCKET* GetSocket() { return &ServerConnectionSocket; }

		// Function return true if client connected to server
		bool IsConnected();

		// Connect to localhost and default port. Return low-level function connect result.
		// See platform documentation if you want get more information about errors
		bool Connect();

		// Connect to localhost and current port. Return low-level function connect result.
		// See platform documentation if you want get more information about errors
		bool Connect(int port);

		// Connect to server with current ip and port. Return low-level function connect result.
		// See platform documentation if you want get more information about errors
		bool Connect(std::string ipAddr, int port);

		// Method to start server. Starts a thread to process server responses
		// You have to determine BeforeDisconnect() and ServerMessageHandler()
		void Run();

		// Function for sending a message to a connected server
		// As an additional parameter, it takes the delay in executing the code to send the message
		void SendToServer(std::string message, int MessageDelay = 10);

		// This function disconnect client from server
		void Disconnect();

		// Default destructor
		~EN_TCP_Client();
	};
}

