#pragma once

#if defined WIN32 || defined _WIN64 

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
#include "EN_SocketOptions.h"

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

		// Variable for understanding the server or client is disconnected
		bool IsClientDisconnect = false;

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
			\brief Function for sending a message to a connected server.
			\param[in] message the message string to send to the server
			\param[in] MessageDelay Optional parameter. the time that will pass after sending the message to the server
		*/
		void SendToServer(std::string message, int MessageDelay = 10);

        /**
			\brief Method that wait new incoming message from client

            \warning Since the ServerMessageHandler runs in a separate thread, the call to the WaitMessage method must be in the same thread.  
            This is necessary so that there is no waiting for a new message in different threads, which leads to undefined behavior.
            Note that you still can use this function on client connection because ServerMessageHandler invokes after AfterConnect.

			\param[in] message The string to store incoming message
            \return Returns true in case of success, false if it was disconnection  
		*/
        bool WaitMessage(std::string& message);
        
		/// This function disconnect client from server
		void Disconnect();

        /**
           \brief The method sets options for client socket

            \param[in] level The level at which the option is defined (for example, SOL_SOCKET).
			\param[in] optionName The socket option for which the value is to be set (for example, SO_BROADCAST). 
            The optionName parameter must be a socket option defined within the specified level, or behavior is undefined.
			\param[in] optionValue The value for the requested option is specified.
        */
        void SetSocketOption(int level, int optionName, int optionValue);

        /**
           \brief The method sets options for client socket

            \param[in] socketOptions This parameter takes a predefined structure to specify a package of socket options at once. 
            The list of all predefined structures is in EN_SocketOptions.h. 
            You can create your own sets of options using define or by creating structure objects
        */
        void SetSocketOption(PredefinedSocketOptions socketOptions);

		// Default destructor
		~EN_TCP_Client();
	};
}

