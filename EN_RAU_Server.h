#pragma once

#include "EN_Functions.h"
#include "EN_TCP_Server.h"
#include "EN_UDP_Server.h"
#include <mutex>
#include <condition_variable>
#include <queue>

namespace EN
{
	class EN_RAU_Server;

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

	/// Server class with connection and the possibility of reliable and unreliable sending
	class EN_RAU_Server 
	{
	private:
		friend EN_RAU_TCP_Server;
		friend EN_RAU_UDP_Server;
		EN_RAU_TCP_Server* TCP_Server;
		EN_RAU_UDP_Server* UDP_Server;
		std::vector<std::queue<std::string>*> VectorQueuesMessages;
		std::vector<std::string> UDPIpAddresses;
		std::vector<std::condition_variable*> VectorCondVars;
		std::vector<bool> KillThreads;

		bool IsShutdown = false;

		void ThreadQueueHandler(int ClientID);

	protected:
		int Port = 1111;

		std::string IpAddress = "127.0.0.1";

		int MaxUnreliableMessageSize = 64;

		virtual void OnClientConnected(int ClientID) = 0;

		virtual void ClientMessageHandler(std::string message, int ClientID) = 0;

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
		void SendToClient(int ClientId, std::string message, bool IsReliable = true);

		~EN_RAU_Server();
	};
}