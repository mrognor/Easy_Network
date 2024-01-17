#include "../../experimental/EN_HTTP_Server.h"

class MyHttpServer : public EN::EN_HTTP_Server
{
	// Handle file request parametrs
	void GetUrlParamsHandler(const std::string& urlParams) override {}

	// Gets post or get requests
    void HTTPRequestHandler(EN_SOCKET clientSocket, std::map<std::string, std::string> parsedRequestMap, std::string requestHeader, std::string requestBody) 
	{
		std::cout << requestBody << std::endl;
		SendToClient(clientSocket, "HTTP/1.1 200 OK\r\ncontent-length: 0\r\nconnection: closed\r\n\r\n");
	}
};

int main()
{
	// MyServer A;
	// A.Run();


	// Uncomment this code to make server standart console input.
	// Using this you can write logic to kick clients or shutdown server
	
	MyHttpServer A;

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
