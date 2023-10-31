#pragma once

#include "EN_TCP_Server_SC.h"

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
		Also, this class allows you to change the communication protocol for each individual client while working with him, 
		for this you need to set new values for the parameters TCP_Send and TCP_Recv. 
		Inside EN_TCP_Server_SC, each object of this class is used only in its own thread, 
		so it is thread-safe, and you should not try to use it in different threads.

		\warning Note that if TCP_Send and TCP_Recv have a default value not equal to nullptr, 
		then when creating EN_TCP_Server_SC, the methods EN_TCP_Server_SC::TCP_Send and EN_TCP_Server_SC::TCP_Recv 
		will be set the same as TCP_SessionContext_SC_CUP, regardless of which methods were setted in the childs of 
		the EN_TCP_Server_SC class
	*/
    template <class T>
    struct TCP_SessionContext_SCUP
    {
		/// Template variable to store session data
        T data;

        // A pointer to a function for sending messages. Allows you to use custom network protocols. Send message to socket
		bool (*TCP_Send)(EN_SOCKET sock, const std::string& message) = nullptr;
		
		// A pointer to a function for recv messages. Allows you to use custom network protocols. Recv message from socket
		bool (*TCP_Recv)(EN_SOCKET sock, std::string& message) = nullptr;

        T* operator-> ()
        {
            return &data;
        }
    };

	/**
		TCP server with dynamic protocol changes during the session and session context.
		Полезен только в определенных ситуациях, например для http сервера, так как в нем
		нет пересылки между клиентами, но достаточно часто происходит изменение протоколов. 
		SCUP - session context with custom user protocol
	*/
    template <class T>
    class EN_TCP_Server_SCUP : public EN::EN_TCP_Server_SC<T>
    {    
	private:
		// Definition of a purely virtual parent function, for further redefinition of this function with the addition of new parameters
		virtual void OnClientConnected(EN_SOCKET clientSocket, TCP_SessionContext_SC<T>& sessionContext) {};
	 	
		// Definition of a purely virtual parent function, for further redefinition of this function with the addition of new parameters
		virtual void ClientMessageHandler(EN_SOCKET clientSocket, std::string message, TCP_SessionContext_SC<T>& sessionContext) {};

		// Definition of a purely virtual parent function, for further redefinition of this function with the addition of new parameters
		virtual void OnClientDisconnect(EN_SOCKET clientSocket, TCP_SessionContext_SC<T>& sessionContext) {};

		// Transfer the function to the private section so that it cannot be accessed in child classes
		virtual void SetTCPSendFunction(bool (*TCPSendFunction)(EN_SOCKET, const std::string&)) {};

		// Transfer the function to the private section so that it cannot be accessed in child classes
		virtual void SetTCPRecvFunction(bool (*TCPRecvFunction)(EN_SOCKET, std::string&)) {};
    protected:
		/// Variable to store session context data
        TCP_SessionContext_SCUP<T> SessionContext;

		/**
			\brief The method that is executed when the client connects to the server

			\param[in] clientSocket Socket of the connected client
            \param[in] sessionContext A reference to a variable that stores the context of the connected client's session

			\warning Must be defined by the user
		*/
		virtual void OnClientConnected(EN_SOCKET clientSocket, TCP_SessionContext_SCUP<T>& sessionContext) = 0;

		/**
			\brief Method that processes incoming messages

			\param[in] clientSocket The socket of the client from which the message came
			\param[in] message Message from the client
            \param[in] sessionContext A reference to a variable that stores the context of the connected client's session

			\warning Must be defined by the user
		*/
		virtual void ClientMessageHandler(EN_SOCKET clientSocket, std::string message, TCP_SessionContext_SCUP<T>& sessionContext) = 0;

		/**
			\brief Method that runs after the client is disconnected
			
			\param[in] clientSocket Socket of the disconnected client
            \param[in] sessionContext A reference to a variable that stores the context of the connected client's session

			\warning Must be defined by the user
		*/
		virtual void OnClientDisconnect(EN_SOCKET clientSocket, TCP_SessionContext_SCUP<T>& sessionContext) = 0;

    public:
		EN_TCP_Server_SCUP()
		{
			if (SessionContext.TCP_Send != nullptr)
				this->SetTCPSendFunction(SessionContext.TCP_Send);

			if (SessionContext.TCP_Recv != nullptr)
				this->SetTCPRecvFunction(SessionContext.TCP_Recv);
		}

		/**
			\brief Method that send message to all connected clients

			\param[in] message The message to be sent to the client 

			\warning This method uses the function for sending, which is set in the class.
			Do not use this function if clients with a modified protocol cannot receive messages from the default function.
		*/
		virtual void MulticastSend(std::string message)
		{
			EN_TCP_Server::MulticastSend(message);
		}

        virtual void ClientHandler(EN_SOCKET clientSocket)
        {
            OnClientConnected(clientSocket, SessionContext);

            std::string message;
            bool ConnectionStatus;

            while (true)
            {
                ConnectionStatus = SessionContext.TCP_Recv(clientSocket, message);

                if (ConnectionStatus == false)
                {
                    OnClientDisconnect(clientSocket, SessionContext);
                    break;
                }

                ClientMessageHandler(clientSocket, message, SessionContext);
            }

            CloseSocket(clientSocket);

            this->CrossWalk.PedestrianStartCrossRoad();

            this->ClientSockets.erase(clientSocket);

            this->CrossWalk.PedestrianStopCrossRoad();
        }

		virtual bool SendToClient(EN_SOCKET clientSocket, std::string message, TCP_SessionContext_SCUP<T>& sessionContext)
		{
			this->CrossWalk.CarStartCrossRoad();
			
			// Variable to store sending result
			// -2 - no client with this socket id on server
			// -1 - failed to send to client. For example if socket closed at sending time
			// 0 - sending succeeded
			int resCode = -2;

			if (this->ClientSockets.find(clientSocket) != this->ClientSockets.end())
			{
				if(!sessionContext.TCP_Send(clientSocket, message))
					resCode = -1;
				else 
					resCode = 0;
			}
			
			this->CrossWalk.CarStopCrossRoad();

			if (resCode == -2)
			{
				LOG(LogLevels::Warning, "You are trying to send to non client socket. Socket descriptor: " + std::to_string(clientSocket));
				LOG(LogLevels::Hint, "Check that you are dont forget to lock sockets. This means that no one is already connected to the called socket" + std::to_string(clientSocket));
				return false;
			}
			
			if (resCode == -1)
			{
				LOG(LogLevels::Warning, "Failed to send data to socket. Socket descriptor: " + std::to_string(clientSocket));
				return false;
			}

			return true;
		}

		virtual bool WaitMessage(EN_SOCKET clientSocket, std::string& message, TCP_SessionContext_SCUP<T>& sessionContext)
		{
			// Thread safety because this method should be called only inside handler thread
			return sessionContext.TCP_Recv(clientSocket, message);
    	}
    };
}