#pragma once

#include "EN_TCP_Client.h"

namespace EN
{
    class EN_FT_EternalClient : public EN::EN_TCP_Client
    {
    protected:
	    void OnConnect();

	    void ServerMessageHandler(std::string message);

	    void OnDisconnect();
    };

    class EN_FT_Client : public EN::EN_TCP_Client
    {
    private:
        EN_FT_EternalClient EternalFTCient;
    public:
        bool Connect(std::string ipAddr = "127.0.0.1", int tcpPort = 1111, int ftPort = 1112);

        void Disconnect();

        void SendTest(std::string test)
        {
            EternalFTCient.SendToServer(test);
        }
    };
}