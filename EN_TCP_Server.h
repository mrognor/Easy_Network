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

#include "EN_Functions.h"
#include "thread"


namespace EN
{
	/// Base tcp server class
	class EN_TCP_Server
	{
	private:

		// Boolean variable that displays the status of the server 
		bool IsShutdown = false;

		// A method that processes messages from clients. Sends a message to the function ClientMessageHandler().
		void ClientHandler(int ClientID);

		// The method that receives messages from the client. The first parameter is the socket index in the vector
		int RecvFromClient(int ClientSocketID, std::string& message);

	protected:

		/// Server port. Default set to 1111
		int Port = 1111;

		/// Server ip address string. Default set to localhost
		std::string IpAddress = "127.0.0.1";

		/// vector of sockets of connected clients
		std::vector<EN_SOCKET> ClientSockets;
		
		/**
			\brief The method that is executed when the client connects to the server

			\warning Must be defined by the user
		*/
		virtual void OnClientConnected(int ClientID) = 0;

		/**
			\brief Method that processes incoming messages

			\warning Must be defined by the user
		*/
		virtual void ClientMessageHandler(std::string message, int ClientID) = 0;

		/**
			\brief Method that runs after the client is disconnected
			
			\warning Must be defined by the user
		*/
		virtual void OnClientDisconnect(int ClientID) = 0;

	public:

		EN_TCP_Server();

		/// Port getter
		int GetPort() { return Port; }

		/// Ip getter
		std::string GetIpAddr() { return IpAddress; }

		/// The function returns the number of connected devices
		int GetConnectionsCount() { return ClientSockets.size(); }

		/// Method to start server.
		void Run();

		/**
			\brief Method that disconnects the client from the server

			\param[in] ClientID The number of the client to be disconnect
		*/
		void DisconnectClient(int ClientID);

		/// Method that stops the server
		void Shutdown();
		
		/**
			\brief Method that send message to client

			\param[in] ClientID The number of the client 
			\param[in] message The message to be sent to the client 
			\param[in] MessageDelay The delay after sending message
		*/
		void SendToClient(int ClientId, std::string message, int MessageDelay = 10);
	};
}
