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

namespace EN
{
	class EN_UDP_Server
	{
	protected:

		// Default port
		int Port = 1111;

		int MaxMessageSize = 256;

		// Server ip address string. Default set to localhost
		std::string IpAddress = "127.0.0.1";

		EN_SOCKET s;

		int ThreadAmount = 8;

		// Method that processes incoming messages
		virtual void ClientMessageHandler(std::string message, sockaddr_in ClientSocketAddr) = 0;

		bool IsShutdown = false;

		sockaddr_in server;

	public:

		void Call(std::string message, sockaddr_in ClientSocketAddr) { ClientMessageHandler(message, ClientSocketAddr); }

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

		void SendToClient(std::string msg, sockaddr_in ClientSocketAddr);

		~EN_UDP_Server();
	};
}
