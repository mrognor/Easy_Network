#pragma once

#if defined WIN32 || defined _WIN64 

#include <WS2tcpip.h>
#include <Winsock2.h>
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
#include "EN_ThreadCrossWalk.h"


namespace EN
{
	/// Base tcp server class
	class EN_TCP_Server
	{
	private:
		// Boolean variable that displays the status of the server 
		std::atomic_bool IsShutdown;

        // Vector with options to be set after client connected
        std::vector<SocketOption> CreateSocketsOption;

        // Socket to accept incoming clients
        EN_SOCKET ServerListenSocket = INVALID_SOCKET;

		// Mutex to prevent errors while shutdown before run
		std::mutex ShutdownMutex;

		// Thread cross walk to synchronize access to ClientSockets between different threads
		EN_ThreadCrossWalk CrossWalk;

		// A method that processes messages from clients. Sends a message to the function ClientMessageHandler().
		void ClientHandler(size_t ClientID);
	protected:

		/// Server port. Default set to 1111
		int Port = 1111;

		/**
			\brief Server ip address string. 
			
			Default set to empty string. Equals INADDR_ANY Which means that the server can accept connections from any computer network.
			For example. The computer can be connected to a local network and to a VPN network. 
			Then it will have 2 network interfaces and it will be possible to connect from both networks. 
			Use this variable only if you want to accept connections from one specific network.
		*/
		std::string IpAddress = "";

		/// vector of sockets of connected clients
		std::vector<EN_SOCKET> ClientSockets;
		
		/**
			\brief The method that is executed when the client connects to the server

			\warning Must be defined by the user
		*/
		virtual void OnClientConnected(size_t ClientID) = 0;

		/**
			\brief Method that processes incoming messages

			\warning Must be defined by the user
		*/
		virtual void ClientMessageHandler(std::string message, size_t ClientID) = 0;

		/**
			\brief Method that runs after the client is disconnected
			
			\warning Must be defined by the user
		*/
		virtual void OnClientDisconnect(size_t ClientID) = 0;

	public:
		EN_TCP_Server();
		
		/// Port getter
		int GetPort();

		/// Ip getter
		std::string GetIpAddr();

		/// The function returns the number of connected devices
		size_t GetConnectionsCount();

		/**
			\brief Method to start server. Blocking call.
			
			Place this method in try block to catch errors.
			Throws socket errors. To get information about the error, use the documentation of your operating system. 
			For Windows, errors go through WSAGetLastError, for Linux, errors go through errno.  
			All errors with the description are duplicated in the log system.
		*/
		void Run();

		/**
			\brief Method that disconnects the client from the server

			\param[in] ClientID The number of the client to be disconnect
		*/
		void DisconnectClient(size_t ClientID);

		/// Method that stops the server
		void Shutdown();
		
		/**
			\brief Method that send message to client

			\param[in] ClientID The number of the client 
			\param[in] message The message to be sent to the client 

			\return Returns true in case of success, false if it was disconnection  
		*/
		bool SendToClient(size_t ClientId, std::string message);

        /**
			\brief Method that wait new incoming message from client

            \warning Since the ClientMessageHandler runs in a separate thread, the call to the WaitMessage method must be in the same thread.  
            This is necessary so that there is no waiting for a new message in different threads, which leads to undefined behavior.
            Note that you still can use this function on client connection because ClientMessageHandler invokes after OnClientConnected.
            
            \param[in] ClientID The number of the client 
			\param[in] message The string to store incoming message
            \return Returns true in case of success, false if it was disconnection 
		*/
        bool WaitMessage(size_t ClientId, std::string& message);

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
        void SetSocketOption(size_t ClientID, int level, int optionName, int optionValue);

        /**
            \brief The method sets options for client socket

            \param[in] ClientID The number of the client 
            \param[in] socketOptions This parameter takes a predefined structure to specify a package of socket options at once. 
            The list of all predefined structures is in EN_SocketOptions.h. 
            You can create your own sets of options using define or by creating structure objects
        */
        void SetSocketOption(size_t ClientID, PredefinedSocketOptions socketOptions);

		virtual ~EN_TCP_Server();
	};
}
