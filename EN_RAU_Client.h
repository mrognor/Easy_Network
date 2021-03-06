#pragma once

#include "EN_Functions.h"
#include "EN_TCP_Client.h"
#include "EN_UDP_Client.h"
#include <mutex>
#include <condition_variable>
#include <queue>

namespace EN
{
	// EN_RAU_Client class definition
	class EN_RAU_Client;

	/// \cond HIDDEN_SYMBOLS
	class EN_RAU_TCP_Client : public EN_TCP_Client
	{
	public:
		friend EN_RAU_Client;
		EN_RAU_Client* RAU_Client;

		EN_RAU_TCP_Client(EN_RAU_Client* rau_Client);

		// A function to be defined by the user. It is used for logic after connection
		void AfterConnect() {};

		// A function to be defined by the user. It is used to process incoming messages from the server
		void ServerMessageHandler(std::string message);

		// A function to be defined by the user. Performed before disconnected from the server
		void BeforeDisconnect();
	};

	class EN_RAU_UDP_Client : public EN_UDP_Client
	{
	public:
		friend EN_RAU_Client;
		EN_RAU_Client* RAU_Client;

		EN_RAU_UDP_Client(EN_RAU_Client* rau_Client);
		
		void ServerMessageHandler(std::string message);
	};
	/// \endcond
	
	
	/// Client class with connection and the possibility of reliable and unreliable sending and receiveng
	class EN_RAU_Client 
	{
	private:
		// RAU_TCP_Client and RAU_UDP_Client pointers
		friend EN_RAU_TCP_Client;
		friend EN_RAU_UDP_Client;
		EN_RAU_TCP_Client* TCP_Client;
		EN_RAU_UDP_Client* UDP_Client;

		// Client number on the server
		int ClientId = -1;

		// Boolean variable for tracking the connection status. 
		// When the server receives the client's IP address, 
		// it will send a message via tsp and this variable will become true
		bool IsServerGetUDPAddress = false;

		// Variable to shutdown server
		bool IsShutdown = false;

		// Incoming messages buffer
		std::queue<std::string> Messages;
		// Condition variable to wake up server messahe handle thread
		std::condition_variable CondVar;
		// Mutex to make code thread-safety
		std::mutex Mtx;

		// Incoming messages handler
		void QueueMessageHandler();

		// Default port
		int ServerPort = 1111;

		// Server ip address string. Default set to localhost
		std::string ServerIpAddress = "127.0.0.1";

	protected:

		/// Max size of unreliable message 
		int MaxUnreliableMessageSize = 64;

		/**
			\brief This function is called after connecting to the server
			
			\warning Must be defined by the user
		*/
		virtual void AfterConnect() = 0; 

		/**
			\brief The function processes all incoming messages
			
			
			\warning Must be defined by the user
		*/
		virtual void ServerMessageHandler(std::string message) = 0;

		/**
			\brief The function is called before disconnecting from the server.

			Important! If disconnection occurs from the server side or connection lost, the IsConnected() function returns false
			\warning Must be defined by the user
		*/
		virtual void BeforeDisconnect() = 0;

	public:
		EN_RAU_Client();

		/// Server port getter
		int GetServerPort() { return ServerPort; }

		/// Server ip getter
		std::string GetServerIpAddress() { return ServerIpAddress; }

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
			\brief Method to start server.

			Starts a thread to process server responses
			Important! You have to determine AfterConnect(), ServerMessageHandler() and BeforeDisconnect()
		*/
		void Run();

		/**
			\brief Function for sending a message to a connected server.
			\param[in] message the message string to send to the server
			\param[in] MessageDelay Optional parameter. the time that will pass after sending the message to the server
		*/
		void SendToServer(std::string message, bool IsReliable = true, int MessageDelay = 10);

		/// This function disconnect client from server
		void Disconnect();

		~EN_RAU_Client();
	};
}