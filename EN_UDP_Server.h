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

#include "EN_Functions.h"
#include "EN_ThreadGate.h"
#include "EN_SocketOptions.h"

namespace EN
{
	/// UDP server buffer types
	enum EN_UDP_ServerBuferType
	{
		Queue, ///< Queue. First in last out
		Stack ///< Stack. First in first out
	};

	/// Base udp server class
	class EN_UDP_Server
	{
	private:
		// Variable to shutdown server
		std::atomic_bool IsShutdown;

		// Server socket
		EN_SOCKET UDP_ServerSocket = INVALID_SOCKET;

		// Vector with socket options
		std::vector<SocketOption> SocketOptions;

		// Array of pointers
		std::list<std::string>** QueueMessageVec;
		std::list<std::string>** QueueAddrVec;
		std::list<std::chrono::system_clock::time_point>** QueueTimeVec;
		EN::EN_ThreadGate** GateVec;
		std::mutex** Mutexes;
		std::thread* ThreadVec;

		// Mutex to prevent errors while shutdown before run
		std::mutex ShutdownMutex;

		// Functions to handle incoming message buffer
		void ThreadListHandler(int ThreadID);

		// A pointer to a function for sending messages. Allows you to use custom network protocols. Send message to socket
		void (*UDP_Send)(EN_SOCKET sock, std::string destinationAddress, const std::string& message) = EN::Default_UDP_Send;
		
		// A pointer to a function for recv messages. Allows you to use custom network protocols. Recv message from socket
		bool (*UDP_Recv)(EN_SOCKET sock, std::string& sourceAddress, std::string& message) = EN::Default_UDP_Recv;
	protected:
		/// Server port. Default set to 1111
		int Port = 1111;

		/// Server ip address string. Default set to localhost
		std::string IpAddress = "127.0.0.1";

		/// The number of threads in which incoming messages will be processed
		int ThreadAmount = 2;

		/**
			\brief The maximum size of the incoming message stack. 

			Used only if ServerBuferType set to EN::Stack
		*/
		size_t MaxStackBuffSize = 16;

		/// Incoming message buffer type
		EN_UDP_ServerBuferType ServerBuferType = Queue;

		/**
			\brief Method that processes incoming messages

			This method processes a message from the buffer
			Get message, UDP client address and time in milliseconds since message come to server
			\warning Must be defined by the user
		*/
		virtual void ClientMessageHandler(std::string message, std::string ClientIpAddress, long long TimeWhenPackageArrived) = 0;

		/**
			\brief Method that processes incoming messages

			This method processes messages before they are placed in the buffer
			Get message, UDP client address and time in milliseconds since message come to server
			Return true if you want to put message into bufer, false otherwise
			\warning Must be defined by the user
		*/
		virtual bool InstantClientMessageHandler(std::string message, std::string ClientIpAddress, long long TimeWhenPackageArrived) = 0;

	public:
		EN_UDP_Server();
		
		/// Port getter
		int GetPort();

		/// Ip getter
		std::string GetIpAddr();

		/**
			\brief Method to start server. Blocking call.
			
			Place this method in try block to catch errors.
			Throws socket errors. To get information about the error, use the documentation of your operating system. 
			For Windows, errors go through WSAGetLastError, for Linux, errors go through errno.  
			All errors with the description are duplicated in the log system.
		*/
		void Run();

		/// Method that stops the server
		void Shutdown();

		/**
			\brief function for sending a message to the client

			\param[in] message string to send to server
			\param[in] ClientSocketAddr string with server address. Format: 127.0.0.1:1111
		*/
		void SendToClient(std::string ClientIpAddress, std::string message);

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

		virtual ~EN_UDP_Server();
	};
}
