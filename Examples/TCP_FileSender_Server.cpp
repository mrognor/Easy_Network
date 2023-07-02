#include <iostream>
#include "EN_TCP_Server.h"

class MyServer : public EN::EN_TCP_Server
{
public:
	MyServer(std::string ip, int port)
	{
		IpAddress = ip; // Default set to localhost
		Port = port; // Default port is 1111
	}

	void OnClientConnected(EN_SOCKET clientSocket)
	{
		std::cout << "Client connected! Socket descriptor: " << clientSocket << std::endl;
	}

	void ClientMessageHandler(EN_SOCKET clientSocket, std::string message)
	{
		// Important. This function is run in a separate thread. 
		// If you want to write data to class variables, you should use mutexes or other algorithms for thread-safe code.
		std::cout << message << std::endl;
		std::atomic_bool ShouldShutdown(false);
		std::vector<std::string> InterpretedMessage = EN::Split(message);

		if (message.find("send file") != -1ull)
		{
			EN::RecvFile(clientSocket, ShouldShutdown, EN::DownloadStatus);
			return;
		}
		
		if (message.find("get file") != -1ull)
		{
			if (EN::IsFileExist(InterpretedMessage[2]))
			{
				std::cout << "Sending file" << std::endl;
				SendToClient(clientSocket, "ok");
				EN::SendFile(clientSocket, InterpretedMessage[2], ShouldShutdown, EN::DownloadStatus);
			}
			else
			{
				std::cout << "No file with this name" << std::endl;
				SendToClient(clientSocket, "bad");
			}
			return;
		}

		if (message.find("continue download") != -1ull)
		{
			if (EN::IsFileExist(InterpretedMessage[2]))
			{
				SendToClient(clientSocket, "ok");
				EN::SendFile(clientSocket, InterpretedMessage[2], ShouldShutdown, EN::DownloadStatus, std::stoll(InterpretedMessage[3]));
			}
			else
				EN::TCP_Send(clientSocket, "bad");
			return;
		}
	}

	void OnClientDisconnect(EN_SOCKET clientSocket)
	{
		std::cout << "Client disconnected! Socket descriptor: " << clientSocket << std::endl;
	}
};


int main()
{
	std::cout << "Write server ip or/and port. Format: ip:port. Example: 192.168.1.85:1234." << std::endl;
	std::cout << "If you dont write ip it will be default value: 127.0.0.1" << std::endl;
	std::cout << "If you dont write port it will be default value: 1111" << std::endl;
	
	std::string addr, ip;
	int port;

	getline(std::cin, addr);
	auto vec = EN::Split(addr, ":");
	
	if (addr.empty())
		ip = "127.0.0.1";

	if (vec.size() == 2)
		port = std::atoi(vec[1].c_str());
	else port = 1111; // Default value 

	// No incorrect port check
	MyServer A(ip, port);
	try 
	{
		A.Run(); 
	}
	catch (std::runtime_error& err)
	{
		LOG(EN::LogLevels::Error, "Run throw error with error code: " + std::string(err.what()));
	}

	// Uncomment this code to make server standart console input.
	// Using this you can write logic to kick clients or shutdown server
	/*
	MyServer A;
	std::thread th([&A]() 
	{ 
		try 
		{
			A.Run(); 
		}
		catch (std::runtime_error err)
		{
			LOG(EN::LogLevels::Error, "Run throw error with error code: " + std::string(err.what()));
		}
	});

	th.detach();
	std::string message;
	while (true)
	{
		getline(std::cin, message);
		if (message == "f")
		{
			A.Shutdown();
			break;
		}
	}
	*/
}