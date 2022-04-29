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
#define SOCKET_ERROR -1

#endif

#include "EN_Functions.h"
#include "thread"


namespace EN
{
	class EN_TCP_Server
	{
	private:

		// Boolean variable that displays the status of the server 
		bool IsShutdown = false;

		// A method that processes messages from clients. Sends a message to the function ClientMessageHandler().
		void ClientHandler(int ClientID);

		// The method that receives messages from the server. The first parameter is the socket index in the vector
		int RecvFromClient(int ClientSocketID, std::string& message);

	protected:

		// Default port
		int Port = 1111;

		// Server ip address string. Default set to localhost
		std::string IpAddress = "127.0.0.1";

		// Connected clients
		std::vector<EN_SOCKET> ClientSockets;

		// The method that is executed when the client connects to the server
		virtual void OnClientConnected(int ClientID) = 0;

		// Method that processes incoming messages
		virtual void ClientMessageHandler(std::string message, int ClientID) = 0;

		// Method that runs after the client is disconnected
		virtual void OnClientDisconnect(int ClientID) = 0;

	public:

		// Default constructor. Port: 1111. Ip address: 127.0.0.1(localhost)
		EN_TCP_Server();

		// Port getter
		int GetPort() { return Port; }

		// Ip getter
		std::string GetIpAddr() { return IpAddress; }

		// Method to start server.
		void Run();

		// Method that disconnects the client from the server
		void DisconnectClient(int ClientID);

		// Method that stops the server
		void Shutdown();
	};
}
