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

	void OnClientConnected(size_t ClientID)
	{
		std::cout << "Client Connected! id: " << ClientID << std::endl;
	}

	void ClientMessageHandler(std::string message, size_t ClientID)
	{
		// Important. This function is run in a separate thread. 
		// If you want to write data to class variables, you should use mutexes or other algorithms for thread-safe code.
		std::cout << message << std::endl;
		bool ShouldShutdown = false;
		std::vector<std::string> InterpretedMessage = EN::Split(message);

		if (message.find("send file") != -1ull)
		{
			EN::RecvFile(ClientSockets[ClientID], ShouldShutdown, EN::DownloadStatus);
			return;
		}
		
		if (message.find("get file") != -1ull)
		{
			if (EN::IsFileExist(InterpretedMessage[2]))
			{
				std::cout << "Sending file" << std::endl;
				SendToClient(ClientID, "ok");
				EN::SendFile(ClientSockets[ClientID], InterpretedMessage[2], ShouldShutdown, EN::DownloadStatus);
			}
			else
			{
				std::cout << "No file with this name" << std::endl;
				SendToClient(ClientID, "bad");
			}
			return;
		}

		if (message.find("continue download") != -1ull)
		{
			if (EN::IsFileExist(InterpretedMessage[2]))
			{
				SendToClient(ClientID, "ok");
				EN::SendFile(ClientSockets[ClientID], InterpretedMessage[2], ShouldShutdown, EN::DownloadStatus, std::stoll(InterpretedMessage[3]));
			}
			else
				EN::TCP_Send(ClientSockets[ClientID], "bad");
			return;
		}
	}

	void OnClientDisconnect(size_t ClientID)
	{
		std::cout << "Client disconnected! ID: " << ClientID << std::endl;
	}
};


int main()
{
	std::cout << "Write server ip or/and port. Format: ip:port. Example: 192.168.1.85:1234. \nIf you dont write port it will be default value: 1111" << std::endl;
	std::string ip;
	getline(std::cin, ip);
	auto vec = EN::Split(ip, ":");
	
	int port;
	if (vec.size() == 2)
		port = std::atoi(vec[1].c_str());
	else port = 1111; // Default value 

	// No incorrect port check
	MyServer A(vec[0], port);
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