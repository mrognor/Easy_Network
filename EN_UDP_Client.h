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
		// Socket to connect to server
		EN_SOCKET ServerConnectionSocket = INVALID_SOCKET;
		
		// The server's internal method for processing incoming messages. 
		// Passes the incoming string to method ServerMessageHandler to interpretate incoming message
		void ServerHandler();

		// Vector with socket options
		std::vector<SocketOption> SocketOptions;

		std::mutex ClientMtx;

		// A pointer to a function for sending messages. Allows you to use custom network protocols. Send message to socket
		void (*UDP_Send)(EN_SOCKET sock, std::string destinationAddress, const std::string& message) = EN::Default_UDP_Send;
		
		// A pointer to a function for recv messages. Allows you to use custom network protocols. Recv message from socket
		bool (*UDP_Recv)(EN_SOCKET sock, std::string& sourceAddress, std::string& message) = EN::Default_UDP_Recv;
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

        /**
           \brief The method sets custom send function. Allow you use your own protocol

            \param[in] UDPSendFunction This parameter is a pointer to a function for sending messages to the socket. 
			The function accepts the socket of the connected client, where you want to send 
			the message and the message itself

			\warning If you want to use your protocol, use only one send call. 
			This is necessary because the send function is thread-safe, 
			but if you send one message to 2 send calls, then if 2 threads write to the same socket, 
			then the data of two different messages may be mixed and you will receive errors
        */
		void SetUDPSendFunction(void (*UDPSendFunction)(EN_SOCKET sock, std::string destinationAddress, const std::string& message));

        /**
           \brief The method sets custom recv function. Allow you use your own protocol

            \param[in] UDPRecvFunction This parameter is a pointer to a function for receiving messages from the socket. 
			The function accepts the socket of the connected client, where you want to recv from 
			the message and the message itself
        */
		void SetUDPRecvFunction(bool (*UDPRecvFunction)(EN_SOCKET sock, std::string& sourceAddress, std::string& message));

		virtual ~EN_UDP_Client();
	};
}

