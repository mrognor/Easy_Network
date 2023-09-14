#include "EN_FT_Server.h"

namespace EN
{        
    void EN_FT_Server_Eternal::SetIpAndPort(std::string ipAddress, int port)
    {
        IpAddress = ipAddress;
        Port = port;
    }

    void EN_FT_Server_Eternal::OnClientConnected(EN_SOCKET clientSocket) 
    {
        LOG(LogLevels::Info, "FT eternal client connected. Socket descriptor: " + std::to_string(clientSocket));
    }

    void EN_FT_Server_Eternal::ClientMessageHandler(EN_SOCKET clientSocket, std::string message) 
    {
        LOG(LogLevels::Info, "Message from FT eternal client. Socket descriptor: " + std::to_string(clientSocket) + " " + message);
    }

    void EN_FT_Server_Eternal::OnClientDisconnect(EN_SOCKET clientSocket) 
    {
        LOG(LogLevels::Info, "FT eternal client disconnected. Socket descriptor: " + std::to_string(clientSocket));
    }

    EN_FT_Server::EN_FT_Server()
    {
        FTServerEternal.SetIpAndPort(IpAddress, FTEternalServerPort);
    }

    void EN_FT_Server::Run()
    {
        FTServerRunThread = std::thread ([&]()
        {
            FTServerEternal.Run(); 
        });

        EN_TCP_Server::Run();
    }

    void EN_FT_Server::Shutdown()
    {
        FTServerEternal.Shutdown();
        EN_TCP_Server::Shutdown();
        FTServerRunThread.join();
    }
}