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
        std::vector<SocketOption> SocketOptionsAfterConnection;

		// Vector with options to be set on accept socket
        std::vector<SocketOption> AcceptSocketOptions;

        // Socket to accept incoming clients
        EN_SOCKET ServerListenSocket = INVALID_SOCKET;

		// Mutex to prevent errors while shutdown before run
		std::mutex ShutdownMutex;

		// Thread cross walk to synchronize access to ClientSockets between different threads
		EN_ThreadCrossWalk CrossWalk;

		// A method that processes messages from clients. Sends a message to the function ClientMessageHandler().
		void ClientHandler(EN_SOCKET clientSocket);

		// A pointer to a function for sending messages. Allows you to use custom network protocols. Send message to socket
		bool (*TCP_Send)(EN_SOCKET sock, const std::string& message) = EN::Default_TCP_Send;
		
		// A pointer to a function for recv messages. Allows you to use custom network protocols. Recv message from socket
		bool (*TCP_Recv)(EN_SOCKET sock, std::string& message) = EN::Default_TCP_Recv;
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

		/**
			\brief Set of sockets of connected clients

			\warning Dont modify set because it is not thread safety
			You can only work with set elements, for this use the methods LockClientSockets and UnlockClientSockets
		*/
		std::set<EN_SOCKET> ClientSockets;
		
		/**
			\brief The method that is executed when the client connects to the server

			\warning Must be defined by the user
		*/
		virtual void OnClientConnected(EN_SOCKET clientSocket) = 0;

		/**
			\brief Method that processes incoming messages

			\warning Must be defined by the user
		*/
		virtual void ClientMessageHandler(EN_SOCKET clientSocket, std::string message) = 0;

		/**
			\brief Method that runs after the client is disconnected
			
			\warning Must be defined by the user
		*/
		virtual void OnClientDisconnect(EN_SOCKET clientSocket) = 0;

	public:
		EN_TCP_Server();
		
		/// Port getter
		int GetPort();

		/// Ip getter
		std::string GetIpAddr();

		/**
			\brief The function returns the number of connected devices

			\warning Use the function only inside methods LockClientSockets and UnlockClientSockets
			This is necessary because you can write logic depending on the number of connected clients, 
			and after receiving the number, one of the clients will disconnect.
		*/
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

			\param[in] clientSocket The number of the client to be disconnect
		*/
		void DisconnectClient(EN_SOCKET clientSocket);

		/// Method that stops the server
		void Shutdown();
		
		/// Method that disconnect all connected clients
		void DisconnectAllConnectedClients();

		/// Method to wait while all clients disconnect
		void WaitWhileAllClientsDisconnect();

		/**
			\brief Method that send message to client

			\param[in] clientSocket The number of the client 
			\param[in] message The message to be sent to the client 

			\return Returns true in case of success, false if it was disconnection  
		*/
		bool SendToClient(EN_SOCKET clientSocket, std::string message);

		/**
			\brief Method that send message to all connected clients

			\param[in] message The message to be sent to the client 
		*/
		void MulticastSend(std::string message);

        /**
			\brief Method that wait new incoming message from client

            \warning Since the ClientMessageHandler runs in a separate thread, the call to the WaitMessage method must be in the same thread.  
            This is necessary so that there is no waiting for a new message in different threads, which leads to undefined behavior.
            Note that you still can use this function on client connection because ClientMessageHandler invokes after OnClientConnected.
            
            \param[in] clientSocket The number of the client 
			\param[in] message The string to store incoming message
            \return Returns true in case of success, false if it was disconnection 
		*/
        bool WaitMessage(EN_SOCKET clientSocket, std::string& message);

		/**
			\brief Function to lock client sockets list

			This function is necessary to synchronize multiple threads when working with a list of connected clients. 
			It will not allow other threads to change the list, while the ability to send messages and read them from 
			the list items remains. Equals ThreadCrossWalk.CarStartCrossRoad();
		 */
		void LockClientSockets();

		/**
			\brief Function to unlock client sockets list

			This function is necessary to synchronize multiple threads when working with a list of connected clients. 
			It will not allow other threads to change the list, while the ability to send messages and read them from 
			the list items remains. Equals ThreadCrossWalk.CarStartCrossRoad();
		 */
		void UnlockClientSockets();
		
        /**
           \brief The method sets options for accept socket.
           Accept socket accepts incoming connections from clients

            \param[in] level The level at which the option is defined (for example, SOL_SOCKET).
			\param[in] optionName The socket option for which the value is to be set (for example, SO_BROADCAST). 
            The optionName parameter must be a socket option defined within the specified level, or behavior is undefined.
			\param[in] optionValue The value for the requested option is specified.
        */
        void AddAcceptSocketOption(int level, int optionName, int optionValue);

        /**
            \brief The method sets options for accept socket.
            Accept socket accepts incoming connections from clients

            \param[in] socketOptions This parameter takes a predefined structure to specify a package of socket options at once. 
            The list of all predefined structures is in EN_SocketOptions.h. 
            You can create your own sets of options using define or by creating structure objects
        */
        void AddAcceptSocketOption(PredefinedSocketOptions socketOptions);

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

            \param[in] clientSocket The descriptor of the client 
            \param[in] level The level at which the option is defined (for example, SOL_SOCKET).
			\param[in] optionName The socket option for which the value is to be set (for example, SO_BROADCAST). 
            The optionName parameter must be a socket option defined within the specified level, or behavior is undefined.
			\param[in] optionValue The value for the requested option is specified.
        */
        void SetSocketOption(EN_SOCKET clientSocket, int level, int optionName, int optionValue);

        /**
            \brief The method sets options for client socket

            \param[in] clientSocket The descriptor of the client 
            \param[in] socketOptions This parameter takes a predefined structure to specify a package of socket options at once. 
            The list of all predefined structures is in EN_SocketOptions.h. 
            You can create your own sets of options using define or by creating structure objects
        */
        void SetSocketOption(EN_SOCKET clientSocket, PredefinedSocketOptions socketOptions);

        /**
           \brief The method sets custom send function. Allow you use your own protocol

            \param[in] TCPSendFunction This parameter is a pointer to a function for sending messages to the socket. 
			The function accepts the socket of the connected client, where you want to send 
			the message and the message itself

			\warning If you want to use your protocol, use only one send call. 
			This is necessary because the send function is thread-safe, 
			but if you send one message to 2 send calls, then if 2 threads write to the same socket, 
			then the data of two different messages may be mixed and you will receive errors
        */
		void SetTCPSendFunction(bool (*TCPSendFunction)(EN_SOCKET, const std::string&));

        /**
           \brief The method sets custom recv function. Allow you use your own protocol

            \param[in] TCPRecvFunction This parameter is a pointer to a function for receiving messages from the socket. 
			The function accepts the socket of the connected client, where you want to recv from 
			the message and the message itself
        */
		void SetTCPRecvFunction(bool (*TCPRecvFunction)(EN_SOCKET, std::string&));

		virtual ~EN_TCP_Server();
	};
}
