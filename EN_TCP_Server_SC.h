#pragma once

#include "EN_TCP_Server.h"

namespace EN
{
    template <class T>
    struct TCP_SessionContext
    {
        T data;
    };

    class EN_TCP_Server_SC : public EN::EN_TCP_Server
    {
    private:
		virtual void OnClientConnected(EN_SOCKET clientSocket) {};

		/**
			\brief Method that processes incoming messages

			\warning Must be defined by the user
		*/
		virtual void ClientMessageHandler(EN_SOCKET clientSocket, std::string message);

		/**
			\brief Method that runs after the client is disconnected
			
			\warning Must be defined by the user
		*/
		virtual void OnClientDisconnect(EN_SOCKET clientSocket);
    
    public:

    };
}