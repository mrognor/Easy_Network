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

		void OnClientConnected(size_t ClientID);

		void ClientMessageHandler(std::string message, size_t ClientID);

		void OnClientDisconnect(size_t ClientID);
	};

	class EN_RAU_UDP_Server : public EN_UDP_Server
	{
	public:
		friend EN_RAU_Server;
		EN_RAU_Server* RAU_Server;
		
		EN_RAU_UDP_Server(EN_RAU_Server* rau_Server);

		// Third parametr in milliseconds
		void ClientMessageHandler(std::string message, std::string ClientSocketAddr, long long TimeSincePackageArrived);

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

		/**
			\brief The method that is executed when the client connects to the server

			\warning Must be defined by the user
		*/
		virtual void OnClientConnected(size_t ClientID) = 0;

		/**
			\brief Method that processes incoming messages

			\warning Must be defined by the user
		*/
		virtual void ClientMessageHandler(std::string message, size_t ClientID) = 0;

		/**
			\brief Method that runs after the client is disconnected
			
			\warning Must be defined by the user
		*/
		virtual void OnClientDisconnect(size_t ClientID) = 0;

	public:
		EN_RAU_Server();

		/// Port getter
		int GetPort();

		/// Ip getter
		std::string GetIpAddr();

		/// The function returns the number of connected devices
		size_t GetConnectionsCount();

		/// Method to start server.
		void Run();

		/**
			\brief Method that disconnects the client from the server

			\param[in] ClientID The number of the client to be disconnect
		*/
		void DisconnectClient(size_t ClientID);

		/// Method that stops the server
		void Shutdown();

		/**
			\brief Method that send message to client

			\param[in] ClientID The number of the client
			\param[in] message The message to be sent to the client
			\param[in] IsReliable if the parameter is set to true, the message is guaranteed to reach. If the parameter is set to false the message delivery is not guaranteed
		*/
		void SendToClient(size_t ClientId, std::string message, bool IsReliable = true);

		/**
           \brief The method sets options for accept socket. Works with tcp(reliable)
           Accept socket accepts incoming connections from clients.

            \param[in] level The level at which the option is defined (for example, SOL_SOCKET).
			\param[in] optionName The socket option for which the value is to be set (for example, SO_BROADCAST). 
            The optionName parameter must be a socket option defined within the specified level, or behavior is undefined.
			\param[in] optionValue The value for the requested option is specified.
        */
        void SetTCPAcceptSocketOption(int level, int optionName, int optionValue);

        /**
            \brief The method sets options for accept socket. Works with tcp(reliable)
            Accept socket accepts incoming connections from clients.

            \param[in] socketOptions This parameter takes a predefined structure to specify a package of socket options at once. 
            The list of all predefined structures is in EN_SocketOptions.h. 
            You can create your own sets of options using define or by creating structure objects
        */
        void SetTCPAcceptSocketOption(PredefinedSocketOptions socketOptions);

        /**
           \brief The method sets options for all sockets that will connect after its call. Works with tcp(reliable)

            It makes sense to call this method before starting connecting clients, 
            since it does not affect previously created sockets.

            \param[in] level The level at which the option is defined (for example, SOL_SOCKET).
			\param[in] optionName The socket option for which the value is to be set (for example, SO_BROADCAST). 
            The optionName parameter must be a socket option defined within the specified level, or behavior is undefined.
			\param[in] optionValue The value for the requested option is specified.
        */
        void AddOnTCPSocketCreateOption(int level, int optionName, int optionValue);

        /**
           \brief The method sets options for all sockets that will connect after its call. Works with tcp(reliable)

            It makes sense to call this method before starting connecting clients, 
            since it does not affect previously created sockets

            \param[in] socketOptions This parameter takes a predefined structure to specify a package of socket options at once. 
            The list of all predefined structures is in EN_SocketOptions.h. 
            You can create your own sets of options using define or by creating structure objects 
        */
        void AddOnTCPSocketCreateOption(PredefinedSocketOptions socketOptions);

        /**
           \brief The method sets options for client socket. Works with tcp(reliable)

            \param[in] ClientID The number of the client 
            \param[in] level The level at which the option is defined (for example, SOL_SOCKET).
			\param[in] optionName The socket option for which the value is to be set (for example, SO_BROADCAST). 
            The optionName parameter must be a socket option defined within the specified level, or behavior is undefined.
			\param[in] optionValue The value for the requested option is specified.
        */
        void SetTCPSocketOption(size_t ClientID, int level, int optionName, int optionValue);

        /**
            \brief The method sets options for client socket. Works with tcp(reliable)

            \param[in] ClientID The number of the client 
            \param[in] socketOptions This parameter takes a predefined structure to specify a package of socket options at once. 
            The list of all predefined structures is in EN_SocketOptions.h. 
            You can create your own sets of options using define or by creating structure objects
        */
        void SetTCPSocketOption(size_t ClientID, PredefinedSocketOptions socketOptions);

		/**
           \brief The method sets options for client socket. Works with udp(unreliable)

            \param[in] level The level at which the option is defined (for example, SOL_SOCKET).
			\param[in] optionName The socket option for which the value is to be set (for example, SO_BROADCAST). 
            The optionName parameter must be a socket option defined within the specified level, or behavior is undefined.
			\param[in] optionValue The value for the requested option is specified.
        */
        void SetUDPSocketOption(int level, int optionName, int optionValue);

        /**
           \brief The method sets options for client socket. Works with udp(unreliable)

            \param[in] socketOptions This parameter takes a predefined structure to specify a package of socket options at once. 
            The list of all predefined structures is in EN_SocketOptions.h. 
            You can create your own sets of options using define or by creating structure objects
        */
        void SetUDPSocketOption(PredefinedSocketOptions socketOptions);
		
		virtual ~EN_RAU_Server();
	};
}