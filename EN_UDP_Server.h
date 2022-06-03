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
#define SOCKET_ERROR -1

#endif

#include "EN_Functions.h"
#include <thread>
#include <mutex>
#include <queue>
#include <vector>
#include <string>
#include <condition_variable>
#include <chrono>
#include <stack>
#include <list>

typedef std::chrono::system_clock::time_point EN_TimePoint;

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
		bool IsShutdown = false;

		// Server address
		sockaddr_in ServerAddress;

		// Server socket
		EN_SOCKET UDP_ServerSocket;

		// Array of pointer
		std::list<std::string>** QueueMessageVec;
		std::list<sockaddr_in>** QueueAddrVec;
		std::list<EN_TimePoint>** QueueTimeVec;
		std::condition_variable** CondVarVec;
		std::mutex** Mutexes;
		std::thread* ThreadVec;

		// Functions to handle incoming message buffer
		void ThreadListHandler(int ThreadID);

	protected:

		/// Server port. Default set to 1111
		int Port = 1111;

		/// Maximum number of characters in a message
		int MaxMessageSize = 256;

		/// Server ip address string. Default set to localhost
		std::string IpAddress = "127.0.0.1";

		/// The number of threads in which incoming messages will be processed
		int ThreadAmount = 2;

		/**
			\brief The maximum size of the incoming message stack. 

			Used only if ServerBuferType set to EN::Stack
		*/
		int MaxStackBuffSize = 16;

		/// Incoming message buffer type
		EN_UDP_ServerBuferType ServerBuferType = Queue;

		/**
			\brief Method that processes incoming messages

			This method processes a message from the buffer
			Get message, UDP client address and time since message come to server
			\warning Must be defined by the user
		*/
		virtual void ClientMessageHandler(std::string message, std::string ClientSocketAddr, long long TimeWhenPackageArrived) = 0;

		/**
			\brief Method that processes incoming messages

			This method processes messages before they are placed in the buffer
			Get message, UDP client address and time since message come to server
			Return true if you want to put message into bufer, false otherwise
			\warning Must be defined by the user
		*/
		virtual bool InstantClientMessageHandler(std::string message, std::string ClientSocketAddr, long long TimeWhenPackageArrived) = 0;

	public:

		// Default constructor. Port: 1111. Ip address: 127.0.0.1(localhost)
		EN_UDP_Server();

		/// Port getter
		int GetPort() { return Port; }

		/// Ip getter
		std::string GetIpAddr() { return IpAddress; }

		/// Method to start server.
		void Run();

		/// Method that stops the server
		void Shutdown();

		/**
			\brief function for sending a message to the client

			\param[in] message string to send to server
			\param[in] ClientSocketAddr string with server address. Format: 127.0.0.1:1111
		*/
		void SendToClient(std::string message, std::string ClientSocketAddr);

		~EN_UDP_Server();
	};
}
