#pragma once

#include "../EN_TCP_Server.h"
#include "../EN_Functions.h"

#include <map>

#define CHUNK_SIZE 4096

namespace EN
{
    bool HTTP_Recv(EN_SOCKET sock, std::string& message);

    bool HTTP_Send(EN_SOCKET sock, const std::string& message);

    void ParseHTTPRequest(const std::string& request, std::map<std::string, std::string>& parsedRequestMap, std::string& requestHeader);

    class EN_HTTP_Server : public EN_TCP_Server
    {
    private:
        std::string WebFilesPath = "";

        // Function to send responce with file to client
        void SendResponce(const EN_SOCKET& socket, const std::string& responce, const std::string& fileName);

    public:
        EN_HTTP_Server();

        void OnClientConnected(EN_SOCKET clientSocket);

        void ClientMessageHandler(EN_SOCKET clientSocket, std::string message);

        void OnClientDisconnect(EN_SOCKET clientSocket);

        // Function to set custom path with web server files. By default set to same path as server executable.
        // Dont forget to specify last path symbol "\\" (only one) on windows and "/" on linux
        void SetWebFilesPath(std::string path);

        // Function to handle url params in GET request.
        virtual void GetUrlParamsHandler(const std::string& urlParams) = 0;

        // Function to handle requests from client
        virtual void HTTPRequestHandler(EN_SOCKET clientSocket, std::map<std::string, std::string> parsedRequestMap, std::string requestHeader, std::string requestBody) = 0;
    };
}