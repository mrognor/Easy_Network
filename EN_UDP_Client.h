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
#define SOCKET_ERROR -1
#endif

#include <iostream>
#include <string>
#include <thread>

#include "EN_Functions.h"
#include "EN_SocketOptions.h"

namespace EN
{
	/// Base udp client class
	class EN_UDP_Client
	{
	private:
		/// Socket to connect to server
		EN_SOCKET ServerConnectionSocket = INVALID_SOCKET;
		
		// The server's internal method for processing incoming messages. 
		// Passes the incoming string to method ServerMessageHandler to interpretate incoming message
		void ServerHandler();

	protected:

		/// Default port. Default set to 1111
		int ServerPort = 1111;

		/// Server ip address string. Default set to localhost
		std::string ServerIpAddress = "127.0.0.1";

		/// It is used to process incoming messages from the server
		/// \warning Must be defined by the user
		virtual void ServerMessageHandler(std::string message) = 0;

	public:
		/// Server port getter
		int GetPort();

		/// Ip getter
		std::string GetIpAddr();

		/// Socket getter
		EN_SOCKET GetSocket();

		/// Method to start server. Starts a thread to process server responses
		/// Will return the true in case of a successful launch otherwise false
		bool Run();

		/**
			\brief Function for sending a message to a connected server

			\param[in] message string to send to server
		*/
		void SendToServer(std::string message);

		/// Function to stop client
		void Stop();

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

		virtual ~EN_UDP_Client();
	};
}

