#include "../../EN_TCP_Server.h"
#include "../../EN_Functions.h"
#include <map>

// String with path to directory with server files
std::string ServerPrefix = "bin/";

bool ReadFile(std::string fileName, std::string& fileString, std::ios_base::openmode openMode = std::ios_base::in)
{
	std::ifstream file(fileName, openMode);
	
	if (!file.is_open())
	{
		LOG(EN::LogLevels::Warning, "In function ReadFile: failed to open file. File name: " + fileName);
		return false;
	}

	std::string tmpFileString;
	
	while (std::getline(file, tmpFileString))
		fileString += tmpFileString + "\n";
	
	return true;
}

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

void GetUrlParamsHandler(const std::string& urlParams)
{

}

class MyServer : public EN::EN_TCP_Server
{
public:
	MyServer()
	{
		SetTCPSendFunction(HTTP_Send);
		SetTCPRecvFunction(HTTP_Recv);

		// IpAddress = "192.168.1.64"; // Default set to localhost. Read description for this variable before use it.
		Port = 80;
	}

	virtual void OnClientConnected(EN_SOCKET clientSocket) override
	{
		LOG(EN::LogLevels::Info, "Client connected! Socket descriptor: " + std::to_string(clientSocket));
	}

	virtual void ClientMessageHandler(EN_SOCKET clientSocket, std::string message) override
	{
		LOG(EN::LogLevels::Info, "From: " + std::to_string(clientSocket) + " Message: " + message);

		std::map<std::string, std::string> parsedRequestMap;
		std::string requestHeader;
		ParseHTTPRequest(message, parsedRequestMap, requestHeader);
		std::vector<std::string> parsedRequstHeaderVec = EN::Split(requestHeader);

		// Value of Sec-Fetch-Dest field
		auto findRes = parsedRequestMap.find("Sec-Fetch-Dest");
		std::string requestedDataType;
		if (findRes != parsedRequestMap.end())
			requestedDataType = findRes->second;

		if (parsedRequstHeaderVec[0] == "GET")
		{
			std::string requestFileName = ServerPrefix + "index.html";
			if (parsedRequstHeaderVec[1] != "/")
			{
				auto splittedFileRequest = EN::Split(parsedRequstHeaderVec[1], "?");
				requestFileName = ServerPrefix + splittedFileRequest[0];
				if (splittedFileRequest.size() > 1)
					GetUrlParamsHandler(splittedFileRequest[1]);
			}

			std::string requestFile;
			std::string responce;
			if (EN::IsFileExist(requestFileName))
			{
				responce = "HTTP/1.1 200 OK\r\n";
				if (requestedDataType == "document" || requestedDataType == "")
				{
					responce += "content-type: text/html\r\n";
					ReadFile(requestFileName, requestFile);
				}
				if (requestedDataType == "image")
				{
					if (requestFileName.substr(requestFileName.rfind(".") + 1) == "svg")
						responce += "content-type: image/svg+xml\r\n";
					if (requestFileName.substr(requestFileName.rfind(".") + 1) == "png")
						responce += "content-type: image/png\r\n";
					if (requestFileName.substr(requestFileName.rfind(".") + 1) == "jpg")
						responce += "content-type: image/jpg\r\n";
						
					ReadFile(requestFileName, requestFile, std::ios::binary);
				}
				if (requestedDataType == "style")
				{
					if (requestFileName.substr(requestFileName.rfind(".") + 1) == "css")
						responce += "content-type: text/css\r\n";
					ReadFile(requestFileName, requestFile);
				}
				if (requestedDataType == "script")
				{
					if (requestFileName.substr(requestFileName.rfind(".") + 1) == "js")
						responce += "content-type: text/javascript\r\n";
					ReadFile(requestFileName, requestFile);
				}

				responce += "content-length: " + std::to_string(requestFile.length()) + "\r\n";
				responce += "connection: closed\r\n\r\n";
				responce += requestFile;
			}
			else
			{
				if (ReadFile(ServerPrefix + "404.html", requestFile))
				{
					responce = "HTTP/1.1 404 Not Found\r\n";
					responce += "content-type: text/html\r\n";
					responce += "content-length: " + std::to_string(requestFile.length()) + "\r\n";
					responce += "connection: closed\r\n\r\n";
					responce += requestFile;
				}
				else
				{
					LOG(EN::LogLevels::Hint, "Cannot find files. This is only examples hint. Check directory with your files");
				}
			}

			SendToClient(clientSocket, responce);
		}
	}

	virtual void OnClientDisconnect(EN_SOCKET clientSocket) override
	{
		LOG(EN::LogLevels::Info, "Client disconnected! Socket descriptor: " + std::to_string(clientSocket));
	}
};


int main()
{
	// MyServer A;
	// A.Run();


	// Uncomment this code to make server standart console input.
	// Using this you can write logic to kick clients or shutdown server
	
	MyServer A;

	std::thread th([&A]() 
	{
		try 
		{
			A.Run(); 
		}
		catch (std::runtime_error& err)
		{
			LOG(EN::LogLevels::Error, "Run throw error with error code: " + std::string(err.what()));
		}
	});
	
	std::string message;

	while (true)
	{
		getline(std::cin, message);

		if (message == "f")
		{
			A.Shutdown();
			break;
		}
		
		A.MulticastSend(message);
	}

	th.join();
}
