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

		// Thread to handle incoming from server messages
		std::thread ServerHandlerThread;
		
		// Mutex to synchronize server handler logic
		std::mutex ServerHandlerMtx;

		// Mutex to synchronize socket logic
		std::mutex SocketMtx;

		// Vector with options to be set on client socket
        std::vector<SocketOption> SocketOptions;

		// The server's internal method for processing incoming messages. 
		// Passes the incoming string to method ServerMessageHandler to interpretate incoming message
		void ServerHandler();

		// This method will wait until thread handler return
		void WaitForServerHandlerEnd();

		// A pointer to a function for sending messages. Allows you to use custom network protocols. Send message to socket
		bool (*TCP_Send)(EN_SOCKET sock, const std::string& message) = EN::Default_TCP_Send;
		
		// A pointer to a function for recv messages. Allows you to use custom network protocols. Recv message from socket
		bool (*TCP_Recv)(EN_SOCKET sock, std::string& message) = EN::Default_TCP_Recv;
	protected:
		/**
			Variable for disabling the incoming message handler. 
			If it is true, then all incoming messages will be processed inside the ServerMessageHandler function, 
			after connecting to the server, the OnConnect method will be called, 
			and after disconnecting from the server, the onDisconnect method will be called. 
			If this variable is false, then the OnConnect, ServerMessageHandler, 
			onDisconnect methods will be called in another thread. 
			Use it if you don't need to process messages in another thread. 
			In this mode, the WaitMessage method can be used in the main thread.
		*/
		bool IsRunMessageHadlerThread = true;

		/**
			\brief This function is called after connecting to the server
			
			\warning Must be defined by the user
		*/
		virtual void OnConnect() = 0;

		/**
			\brief The function processes all incoming messages
			
			
			\warning Must be defined by the user
		*/
		virtual void ServerMessageHandler(std::string message) = 0;

		/**
			\brief The function is called after disconnecting from the server.

			\warning Must be defined by the user
		*/
		virtual void OnDisconnect() = 0;

	public:	
		/// Server port getter
		int GetServerPort();

		/// Server ip getter
		std::string GetServerIpAddress();

		/// Socket getter
		EN_SOCKET GetSocket();

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
		*/
		bool SendToServer(std::string message);

        /**
			\brief Method that wait new incoming message from client

            \warning Since the ServerMessageHandler runs in a separate thread, the call to the WaitMessage method must be in the same thread.  
            This is necessary so that there is no waiting for a new message in different threads, which leads to undefined behavior.
            Note that you still can use this function on OnConnect and OnDisconnect methods.
			If the variable IsRunMessageHadlerThread is false, then this method can also be used in the main thread

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

		// Default destructor
		virtual ~EN_TCP_Client();
	};
}

