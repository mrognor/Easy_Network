#pragma once

#if defined WIN32 || defined _WIN64 

#include <WS2tcpip.h>
#include <Winsock2.h>
#define TCP_KEEPIDLE 3
#define TCP_KEEPCNT 16
#define TCP_KEEPINTVL 17

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
#include "EN_SocketOptions.h"
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

        // Vector with options to be set after client connected
        std::vector<SocketOption> CreateSocketsOption;

        // Socket to accept incoming clients
        EN_SOCKET ServerListenSocket = INVALID_SOCKET;

		// Mutex to prevent errors while shutdown before run
		std::mutex ShutdownMutex;
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

		/// The function returns whether the server is running
		bool GetIsShutdown() { return IsShutdown; }

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

        /**
           \brief The method sets options for accept socket.
           Accept socket accepts incoming connections from clients

            \param[in] level The level at which the option is defined (for example, SOL_SOCKET).
			\param[in] optionName The socket option for which the value is to be set (for example, SO_BROADCAST). 
            The optionName parameter must be a socket option defined within the specified level, or behavior is undefined.
			\param[in] optionValue The value for the requested option is specified.
        */
        void SetAcceptSocketOption(int level, int optionName, int optionValue);

        /**
            \brief The method sets options for accept socket.
            Accept socket accepts incoming connections from clients

            \param[in] socketOptions This parameter takes a predefined structure to specify a package of socket options at once. 
            The list of all predefined structures is in EN_SocketOptions.h. 
            You can create your own sets of options using define or by creating structure objects
        */
        void SetAcceptSocketOption(PredefinedSocketOptions socketOptions);

        /**
           \brief The method sets options for all sockets that will connect after its call

            It makes sense to call this method before starting connecting clients, 
            since it does not affect previously created sockets

            \param[in] level The level at which the option is defined (for example, SOL_SOCKET).
			\param[in] optionName The socket option for which the value is to be set (for example, SO_BROADCAST). 
            The optionName parameter must be a socket option defined within the specified level, or behavior is undefined.
			\param[in] optionValue The value for the requested option is specified.
        */
        void AddOnSocketCreateOption(int level, int optionName, int optionValue);

        /**
           \brief The method sets options for all sockets that will connect after its call

            It makes sense to call this method before starting connecting clients, 
            since it does not affect previously created sockets

            \param[in] socketOptions This parameter takes a predefined structure to specify a package of socket options at once. 
            The list of all predefined structures is in EN_SocketOptions.h. 
            You can create your own sets of options using define or by creating structure objects 
        */
        void AddOnSocketCreateOption(PredefinedSocketOptions socketOptions);

        /**
           \brief The method sets options for client socket

            \param[in] ClientID The number of the client 
            \param[in] level The level at which the option is defined (for example, SOL_SOCKET).
			\param[in] optionName The socket option for which the value is to be set (for example, SO_BROADCAST). 
            The optionName parameter must be a socket option defined within the specified level, or behavior is undefined.
			\param[in] optionValue The value for the requested option is specified.
        */
        void SetSocketOption(int ClientID, int level, int optionName, int optionValue);

        /**
            \brief The method sets options for client socket

            \param[in] ClientID The number of the client 
            \param[in] socketOptions This parameter takes a predefined structure to specify a package of socket options at once. 
            The list of all predefined structures is in EN_SocketOptions.h. 
            You can create your own sets of options using define or by creating structure objects
        */
        void SetSocketOption(int ClientID, PredefinedSocketOptions socketOptions);
	};
}
