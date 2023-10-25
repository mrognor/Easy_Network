#pragma once

#include "EN_TCP_Server.h"

namespace EN
{
	/**
		\brief Template class for storing different information during a single client session

		An object of this class will be created after the client is connected, 
		it will be created in the client processing thread and will be available 
		in all three methods of working with the connected client, 
		i.e. with OnClientConnected, ClientMessageHandler and OnClientDisconnect. 
		As the type of this class, you need to specify the type or class that stores all 
		the necessary variables to describe the session. Data is accessed via the "->" operator. 
		Inside EN_TCP_Server_SC, each object of this class is used only in its own thread, 
		so it is thread-safe, and you should not try to use it in different threads.
	*/
    template <class T>
    struct TCP_SessionContext_SC
    {
		/// Template variable to store session data
        T data;

        T* operator-> ()
        {
            return &data;
        }
    };

	/**
		\brief Tcp server with session context

        This class adds the ability to store data within a single client session.
        The type of data to store is specified in the class template specification. 
        The user class will be wrapped in TCP_SessionContext_SC, data access inside it is implemented through the "->" operator.

        An object of the TCP_SessionContext_SC class will be created after the client is connected, 
		it will be created in the client processing thread and will be available 
		in all three methods of working with the connected client, 
		i.e. with OnClientConnected, ClientMessageHandler and OnClientDisconnect. 
        Each object of this class is used only in its own thread, 
		so it is thread-safe, and you should not try to use it in different threads.
	*/
    template <class T>
    class EN_TCP_Server_SC : public EN::EN_TCP_Server
    {
    private:
        // Definition of a purely virtual parent function, for further redefinition of this function with the addition of new parameters
		virtual void OnClientConnected(EN_SOCKET clientSocket) {};

        // Definition of a purely virtual parent function, for further redefinition of this function with the addition of new parameters
		virtual void ClientMessageHandler(EN_SOCKET clientSocket, std::string message) {};

        // Definition of a purely virtual parent function, for further redefinition of this function with the addition of new parameters
		virtual void OnClientDisconnect(EN_SOCKET clientSocket) {};
    
        // A method that processes messages from clients. Sends a message to the function ClientMessageHandler().
        virtual void ClientHandler(EN_SOCKET clientSocket) override
        {
            OnClientConnected(clientSocket, SessionContext);

            std::string message;
            bool ConnectionStatus;

            while (true)
            {
                ConnectionStatus = TCP_Recv(clientSocket, message);

                if (ConnectionStatus == false)
                {
                    OnClientDisconnect(clientSocket, SessionContext);
                    break;
                }

                ClientMessageHandler(clientSocket, message, SessionContext);
            }

            CloseSocket(clientSocket);

            CrossWalk.PedestrianStartCrossRoad();

            ClientSockets.erase(clientSocket);

            CrossWalk.PedestrianStopCrossRoad();
        }

    protected:
        /// Variable to store session context data
        TCP_SessionContext_SC<T> SessionContext;

		/**
			\brief The method that is executed when the client connects to the server

			\param[in] clientSocket Socket of the connected client
            \param[in] sessionContext A reference to a variable that stores the context of the connected client's session

			\warning Must be defined by the user
		*/
    	virtual void OnClientConnected(EN_SOCKET clientSocket, TCP_SessionContext_SC<T>& sessionContext) = 0;

		/**
			\brief Method that processes incoming messages

			\param[in] clientSocket The socket of the client from which the message came
			\param[in] message Message from the client
            \param[in] sessionContext A reference to a variable that stores the context of the connected client's session

			\warning Must be defined by the user
		*/
		virtual void ClientMessageHandler(EN_SOCKET clientSocket, std::string message, TCP_SessionContext_SC<T>& sessionContext) = 0;

		/**
			\brief Method that runs after the client is disconnected
			
			\param[in] clientSocket Socket of the disconnected client
            \param[in] sessionContext A reference to a variable that stores the context of the connected client's session

			\warning Must be defined by the user
		*/
		virtual void OnClientDisconnect(EN_SOCKET clientSocket, TCP_SessionContext_SC<T>& sessionContext) = 0;
    };
}