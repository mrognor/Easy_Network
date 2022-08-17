#pragma once

#include "EN_Functions.h"
#include "EN_TCP_Server.h"
#include "EN_UDP_Server.h"
#include <mutex>
#include <condition_variable>
#include <queue>

namespace EN
{
	// EN_RAU_Server class definition
	class EN_RAU_Server;

	/// \cond HIDDEN_SYMBOLS
	class EN_RAU_TCP_Server : public EN_TCP_Server
	{
	public:
		friend EN_RAU_Server;
		EN_RAU_Server* RAU_Server;

		EN_RAU_TCP_Server(EN_RAU_Server* rau_Server);

		void OnClientConnected(int ClientID);

		void ClientMessageHandler(std::string message, int ClientID);

		void OnClientDisconnect(int ClientID);
	};

	class EN_RAU_UDP_Server : public EN_UDP_Server
	{
	public:
		friend EN_RAU_Server;
		EN_RAU_Server* RAU_Server;
		
		EN_RAU_UDP_Server(EN_RAU_Server* rau_Server);

		// Third parametr in milliseconds
		void ClientMessageHandler(std::string message, std::string ClientSocketAddr, long long TimeSincePackageArrived) {}

		// Function work between putting message in buffer. Return true if you want to put message in buffer
		bool InstantClientMessageHandler(std::string message, std::string ClientSocketAddr, long long TimeWhenPackageArrived);
	};
	/// \endcond

	
	/// Server class with connection and the possibility of reliable and unreliable sending
	class EN_RAU_Server 
	{
	private:
		// RAU_TCP_Server and RAU_UDP_Server pointers
		friend EN_RAU_TCP_Server;
		friend EN_RAU_UDP_Server;
		EN_RAU_TCP_Server* TCP_Server;
		EN_RAU_UDP_Server* UDP_Server;

		// Vector of queues for storing incoming messages
		std::vector<std::queue<std::string>*> VectorQueuesMessages;
		// Vector of clients udp addresses
		std::vector<std::string> UDPIpAddresses;
		// Vector of condition variavles
		std::vector<std::condition_variable*> VectorCondVars;
		// Vector of bools to stop client handler threads
		std::vector<bool> KillThreads;

		// Varibale to shutdown server
		bool IsShutdown = false;

		// Incoming message handler
		void ThreadQueueHandler(int ClientID);

	protected:
		/// Server port. By default set to 1111
		int Port = 1111;

		/// Server ip address. By default set to localhost
		std::string IpAddress = "127.0.0.1";

		/// Max size of unreliable message 
		int MaxUnreliableMessageSize = 64;

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
		EN_RAU_Server();

		/// Port getter
		int GetPort() { return Port; }

		/// Ip getter
		std::string GetIpAddr() { return IpAddress; }

		/// The function returns the number of connected devices
		int GetConnectionsCount() { return TCP_Server->GetConnectionsCount(); }

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
			\param[in] IsReliable if the parameter is set to true, the message is guaranteed to reach. 
			if the parameter is set to false before the message delivery is not guaranteed
		*/
		void SendToClient(int ClientId, std::string message, bool IsReliable = true, int MessageDelay = 10);

		~EN_RAU_Server();
	};
}