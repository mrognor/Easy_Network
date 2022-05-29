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
	enum EN_UDP_ServerBuferType
	{
		Queue, Stack
	};

	class EN_UDP_Server
	{
	protected:

		// Default port
		int Port = 1111;

		int MaxMessageSize = 256;

		// Server ip address string. Default set to localhost
		std::string IpAddress = "127.0.0.1";

		EN_SOCKET s;

		int ThreadAmount = 2;

		int MaxStackBuffSize = 16;

		// Method that processes incoming messages
		virtual void ClientMessageHandler(std::string message, std::string ClientSocketAddr, long long TimeWhenPackageArrived) = 0;

		// Method that processes incoming messages
		virtual bool ImportantClientMessageHandler(std::string message, std::string ClientSocketAddr, long long TimeWhenPackageArrived) = 0;

		bool IsShutdown = false;

		sockaddr_in server;

		EN_UDP_ServerBuferType ServerBuferType = Queue;
		
		std::list<std::string>** QueueMessageVec;
	public:

		bool* WhichThreadFlushing;

		void SetServerBuferType(EN_UDP_ServerBuferType type) { ServerBuferType = type; }

		void Call(std::string message, std::string ClientSocketAddr, long long TimeSincePackageArrived) { ClientMessageHandler(message, ClientSocketAddr, TimeSincePackageArrived); }

		// Default constructor. Port: 1111. Ip address: 127.0.0.1(localhost)
		EN_UDP_Server();

		// Port getter
		int GetPort() { return Port; }

		// Ip getter
		std::string GetIpAddr() { return IpAddress; }

		// Method to start server.
		void Run();

		// Method that stops the server
		void Shutdown();

		void SendToClient(std::string msg, std::string ClientSocketAddr);

		~EN_UDP_Server();
	};
}
