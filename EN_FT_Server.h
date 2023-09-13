#pragma once

#include "EN_TCP_Server.h"

namespace EN
{
    class EN_FT_Server_Eternal : public EN::EN_FT_Server
    {
    public:
        virtual void OnClientConnected(EN_SOCKET clientSocket) override {}

        virtual void ClientMessageHandler(EN_SOCKET clientSocket, std::string message) override {}

        virtual void OnClientDisconnect(EN_SOCKET clientSocket) override {}
    };

    class EN_FT_Server : public EN::EN_TCP_Server
    {
    };
}