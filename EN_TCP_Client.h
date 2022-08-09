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
	/// Base tcp client class
	class EN_TCP_Client
	{
	private:

		// Default port
		int ServerPort = 1111;

		// Server ip address string. Default set to localhost
		std::string ServerIpAddress = "127.0.0.1";

		// Socket to connect to server. Library wrapper for your operating system socket
		EN_SOCKET ServerConnectionSocket = INVALID_SOCKET;

		// The server's internal method for processing incoming messages. 
		// Passes the incoming string to method ServerMessageHandler to interpretate incoming message
		void ServerHandler();

	protected:

		/**
			\brief This function is called after connecting to the server
			
			\warning Must be defined by the user
		*/
		virtual void AfterConnect() = 0;

		/**
			\brief The function processes all incoming messages
			
			
			\warning Must be defined by the user
		*/
		virtual void ServerMessageHandler(std::string message) = 0;

		/**
			\brief The function is called before disconnecting from the server.

			Important! If disconnection occurs from the server side or connection lost, the IsConnected() function returns false
			\warning Must be defined by the user
		*/
		virtual void BeforeDisconnect() = 0;

	public:
		/// Default constructor. On Windows initiate winsock api
		EN_TCP_Client();

		/// Server port getter
		int GetServerPort() { return ServerPort; }

		/// Server ip getter
		std::string GetServerIpAddress() { return ServerIpAddress; }

		/// Socket getter
		EN_SOCKET GetSocket() { return ServerConnectionSocket; }

		/// Function return true if client connected to server
		bool IsConnected();

		/**
			\brief Connect to localhost and default port. 
			\return Returns true in case of success, false otherwise
		*/ 
		bool Connect();

		/**
			\brief Connect to localhost and current port.
			\param[in] port port to connect
			\return Returns true in case of success, false otherwise
		*/
		bool Connect(int port);

		/**
			\brief Connect to current ip and current port.
			\param[in] ip server ip address
			\param[in] port port to connect
			\return Returns true in case of success, false otherwise
		*/
		bool Connect(std::string ipAddr, int port);

		/**
			\brief Method to start server.

			Starts a thread to process server responses
			Important! You have to determine AfterConnect(), ServerMessageHandler() and BeforeDisconnect()
		*/
		void Run();

		/**
			\brief Function for sending a message to a connected server.
			\param[in] message the message string to send to the server
			\param[in] MessageDelay Optional parameter. the time that will pass after sending the message to the server
		*/
		void SendToServer(std::string message, int MessageDelay = 10);

		/// This function disconnect client from server
		void Disconnect();

		// Default destructor
		~EN_TCP_Client();
	};
}

