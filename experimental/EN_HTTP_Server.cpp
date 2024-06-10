#include "EN_HTTP_Server.h"

namespace EN
{
    bool HTTP_Recv(EN_SOCKET sock, std::string& message)
    {
        char firstLastChar = 0; 
        char secondLastChar = 0;
        char thirdLastChar = 0;
        char thouthLastChar = 0;

        message.clear();

        while (true)
        {
            int res = recv(sock, &firstLastChar, 1, MSG_WAITALL);
            
            if (res != 1) return false;

            message += firstLastChar;

            if (message.length() > 4 && thouthLastChar == '\r' && thirdLastChar == '\n' && secondLastChar == '\r' && firstLastChar == '\n')
            {
                message.pop_back();
                message.pop_back();
                message.pop_back();
                message.pop_back();
                return true;
            }

            thouthLastChar = thirdLastChar;
            thirdLastChar = secondLastChar;
            secondLastChar = firstLastChar;
        }
    }

    bool HTTP_Send(EN_SOCKET sock, const std::string& message)
    {
        int res = send(sock, message.c_str(), message.length(), 0);
        return ((size_t)res == message.length());
    }

    void ParseHTTPRequest(const std::string& request, std::map<std::string, std::string>& parsedRequestMap, std::string& requestHeader)
    {
        std::vector<std::string> parsedRequestVec = EN::Split(request, "\r\n");
        requestHeader = parsedRequestVec[0];
        
        for (std::string& requestLine : parsedRequestVec)
        {
            size_t paramDividerPos = requestLine.find(":");
            std::string paramName = requestLine.substr(0, paramDividerPos);
            std::string paramVal = requestLine.substr(paramDividerPos + 2);
            parsedRequestMap.insert(std::pair<std::string, std::string>(paramName, paramVal));
        }
    }

    void EN_HTTP_Server::SendResponce(const EN_SOCKET& socket, const std::string& responce, const std::string& fileName)
    {
        SendToClient(socket, responce);
        
        // Open file
        std::ifstream file(fileName, std::ios::binary | std::ios_base::ate);

        // Save file size
        uint64_t fileSize = file.tellg();
        file.seekg(0);

        // Arrays to read 4kb from file and to save 4 kb to file
        char fileDataChunk[CHUNK_SIZE];

        // Counter for file reading
        std::size_t counter = 0;

        // Checking whether the file is larger than the size of the file processing chunks
        if (fileSize > CHUNK_SIZE)
        {
            // Processing the part of the file that is a multiple of the chunk size
            for(; counter < fileSize - CHUNK_SIZE; counter += CHUNK_SIZE)
            {
                // Read chunk from input file
                file.read(fileDataChunk, CHUNK_SIZE);

                send(socket, fileDataChunk, CHUNK_SIZE, 0);
            }
        }

        // Calculating the remaining bytes in the file
        counter = fileSize - counter;

        // Read last bytes from input file
        file.read(fileDataChunk, counter);
        send(socket, fileDataChunk, counter, 0);
    }

    EN_HTTP_Server::EN_HTTP_Server()
    {
        SetTCPSendFunction(HTTP_Send);
        SetTCPRecvFunction(HTTP_Recv);
        Port = 80;
    }

    void EN_HTTP_Server::OnClientConnected(EN_SOCKET clientSocket)
    {
        LOG(EN::LogLevels::Info, "HTTP client connected! Socket descriptor: " + std::to_string(clientSocket));
    }

    void EN_HTTP_Server::ClientMessageHandler(EN_SOCKET clientSocket, std::string message)
    {
        LOG(EN::LogLevels::Info, "From: " + std::to_string(clientSocket) + " Message: " + message);

        std::map<std::string, std::string> parsedRequestMap;
        std::string requestHeader;
        ParseHTTPRequest(message, parsedRequestMap, requestHeader);

        std::vector<std::string> parsedRequestHeaderVec = EN::Split(requestHeader);

        if (parsedRequestHeaderVec[0] == "GET")
        {
            auto splittedFileRequest = EN::Split(parsedRequestHeaderVec[1], "?");
            std::string requestFileName;

            if (splittedFileRequest[0] == "/")
                requestFileName = WebFilesPath + "index.html";
            else
                requestFileName = splittedFileRequest[0].substr(1);

            // Checking whether it is possible to send a file to a GET request
            if (EN::FindAllOccurrences(requestFileName, ".").size() != 1 && splittedFileRequest[0] != "/")
            {
                HTTPRequestHandler(clientSocket, parsedRequestMap, requestHeader, "");
                return;
            }

            if (splittedFileRequest.size() > 1)
                GetUrlParamsHandler(splittedFileRequest[1]);

            std::string responce;
            
            // Checking that the requested files are allowed to be given away
            if (splittedFileRequest[0] != "/")
            {
                if (WebFilesPath == "")
                    requestFileName = GetRunningDirectory() + requestFileName;
                else 
                    requestFileName = WebFilesPath + requestFileName;
            }

            uint64_t fileSize = EN::GetFileSize(requestFileName);

            if (fileSize != 0)
            {
                // Value of Sec-Fetch-Dest field
                auto findRes = parsedRequestMap.find("Sec-Fetch-Dest");

                std::string requestedDataType;
                if (findRes == parsedRequestMap.end())
                {
                    findRes = parsedRequestMap.find("sec-fetch-dest");
                    if (findRes != parsedRequestMap.end())
                        requestedDataType = findRes->second;
                }
                else requestedDataType = findRes->second;

                responce = "HTTP/1.1 200 OK\r\n";
                   
                if (requestedDataType == "document" || requestedDataType == "")
                    responce += "content-type: text/html\r\n";

                if (requestedDataType == "image")
                {
                    if (requestFileName.substr(requestFileName.rfind(".") + 1) == "svg")
                        responce += "content-type: image/svg+xml\r\n";
                    if (requestFileName.substr(requestFileName.rfind(".") + 1) == "png")
                        responce += "content-type: image/png\r\n";
                    if (requestFileName.substr(requestFileName.rfind(".") + 1) == "jpg")
                        responce += "content-type: image/jpg\r\n";
                }

                if (requestedDataType == "style")
                    if (requestFileName.substr(requestFileName.rfind(".") + 1) == "css")
                        responce += "content-type: text/css\r\n";

                if (requestedDataType == "script")
                    if (requestFileName.substr(requestFileName.rfind(".") + 1) == "js")
                        responce += "content-type: text/javascript\r\n";

                if (requestedDataType == "font")
                {
                    if (requestFileName.substr(requestFileName.rfind(".") + 1) == "ttf")
                        responce += "content-type: application/x-font-ttf\r\n";

                    if (requestFileName.substr(requestFileName.rfind(".") + 1) == "otf")
                        responce += "content-type: application/x-font-opentype\r\n";
                }

                responce += "content-length: " + std::to_string(fileSize) + "\r\n";
                responce += "connection: closed\r\n\r\n";
            }
            else
            {
                fileSize = EN::GetFileSize(WebFilesPath + "404.html");
                if (fileSize != 0)
                {

                    responce = "HTTP/1.1 404 Not Found\r\n";
                    responce += "content-type: text/html\r\n";
                    responce += "content-length: " + std::to_string(fileSize) + "\r\n";
                    responce += "connection: closed\r\n\r\n";
                    requestFileName = WebFilesPath + "404.html";
                }
                else
                {
                    LOG(EN::LogLevels::Hint, "Cannot find files. This is only examples hint. Check directory with your files");
                }
            }

            SendResponce(clientSocket, responce, requestFileName);
            return;
        }

        if (parsedRequestHeaderVec[0] == "POST")
        {
            auto requestPole = parsedRequestMap.find("Content-Length");
            if (requestPole != parsedRequestMap.end())
            {
                unsigned long long contentLength;
                if (StringToInt(requestPole->second, contentLength) && contentLength < GetMaxTcpMessageSize())
                {
                    char* content = new char[contentLength];
                    recv(clientSocket, content, contentLength, MSG_WAITALL);

                    HTTPRequestHandler(clientSocket, parsedRequestMap, requestHeader, std::string(content, contentLength));
                    delete[] content;
                    return;
                }
            }
        }

        // If the server does not know what to do with the message, it will be passed on HTTPRequestHandler
        HTTPRequestHandler(clientSocket, parsedRequestMap, requestHeader, "");
    }

    void EN_HTTP_Server::SetWebFilesPath(std::string path)
    {
        WebFilesPath = path;
    }

    void EN_HTTP_Server::OnClientDisconnect(EN_SOCKET clientSocket)
    {
        LOG(EN::LogLevels::Info, "Client disconnected! Socket descriptor: " + std::to_string(clientSocket));
    }
}