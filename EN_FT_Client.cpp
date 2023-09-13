#include "EN_FT_Client.h"

namespace EN
{
    void EN_FT_EternalClient::OnConnect()
	{
		LOG(EN::LogLevels::Info, "File transmitter client connected to server!");
	}

	void EN_FT_EternalClient::ServerMessageHandler(std::string message)
	{
		LOG(EN::LogLevels::Info, message);
	}

	void EN_FT_EternalClient::OnDisconnect()
	{
		LOG(EN::LogLevels::Info, "File transmitter client disconnected from server!");
	}

	bool EN_FT_Client::Connect(std::string ipAddr, int tcpPort, int ftPort)
    {
        if (!EN_TCP_Client::Connect(ipAddr, tcpPort))
            return false;
        if (!EternalFTCient.Connect(ipAddr, ftPort))
            return false;
        return true;
    }
}