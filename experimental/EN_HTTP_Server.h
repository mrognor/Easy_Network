#pragma once

#include "../EN_TCP_Server.h"
#include "../EN_Functions.h"

#include <map>

namespace EN
{
    bool ReadFile(std::string fileName, std::string& fileString, std::ios_base::openmode openMode = std::ios_base::in);

    bool HTTP_Recv(EN_SOCKET sock, std::string& message);

    bool HTTP_Send(EN_SOCKET sock, const std::string& message);

    void ParseHTTPRequest(const std::string& request, std::map<std::string, std::string>& parsedRequestMap, std::string& requestHeader);

    class EN_HTTP_Server : public EN_TCP_Server
    {
    private:
        std::string WebFilesPath = "";
    public:
        EN_HTTP_Server();

        void OnClientConnected(EN_SOCKET clientSocket);

        void ClientMessageHandler(EN_SOCKET clientSocket, std::string message);

        void OnClientDisconnect(EN_SOCKET clientSocket);

        // Function to set custom path with web server files. By default set to same path as server executable.
        // Dont forget to specify last path symbol "\" on windows and "/" on linux
        void SetWebFilesPath(std::string path);

        // Function to handle url params in GET request.
        virtual void GetUrlParamsHandler(const std::string& urlParams) = 0;

        // Function to handle requests from client
        virtual void HTTPRequestHandler(EN_SOCKET clientSocket, std::map<std::string, std::string> parsedRequestMap, std::string requestHeader) = 0;
    };
}