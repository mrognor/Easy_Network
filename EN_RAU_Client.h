#pragma once

#include "EN_Functions.h"
#include "EN_TCP_Client.h"
#include "EN_UDP_Client.h"
#include <mutex>
#include <condition_variable>
#include <queue>

namespace EN
{
	class EN_RAU_Client;

	class EN_RAU_TCP_Client : public EN_TCP_Client
	{
	public:
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
		EN_RAU_Client* RAU_Client;

		void SetIpAndPort(std::string Ip, int port);

		EN_RAU_UDP_Client(EN_RAU_Client* rau_Client);

		void ServerMessageHandler(std::string message);
	};

	/// Server class with connection and the possibility of reliable and unreliable sending
	class EN_RAU_Client 
	{
	private:
		friend EN_RAU_TCP_Client;
		friend EN_RAU_UDP_Client;
		EN_RAU_TCP_Client* TCP_Client;
		EN_RAU_UDP_Client* UDP_Client;

		int ServerThreadID = -1;

		bool IsServerGetUDPAddress = false;

		// Default port
		int ServerPort = 1111;

		// Server ip address string. Default set to localhost
		std::string ServerIpAddress = "127.0.0.1";

	protected:

		// A function to be defined by the user. It is used for logic after connection
		virtual void AfterConnect() = 0; 

		// A function to be defined by the user. It is used to process incoming messages from the server
		virtual void ServerMessageHandler(std::string message) = 0;

		// A function to be defined by the user. Performed before disconnected from the server
		virtual void BeforeDisconnect() = 0;

	public:
		/// Default constructor
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

		/// Default destructor
		~EN_RAU_Client();
	};
}