﻿#include <iostream>
#include "EN_TCP_Server.h"


class MyServer : public EN::EN_TCP_Server
{
public:
	MyServer()
	{
		// IpAddress = "192.168.1.64"; //Default set to localhost
		// Port = <put int here> to set port. Default port is 1111
	}

	void OnClientConnected(int ClientID)
	{
		std::cout << "Client connected! Id: " << ClientID << std::endl;
		SendToClient(ClientID, "Welcome. You are connected to server.");
	}

	void ClientMessageHandler(std::string message, int ClientID)
	{
		// Important. This function is run in a separate thread. 
		// If you want to write data to class variables, you should use mutexes or other algorithms for thread-safe code.
		std::cout << message << std::endl;

		// Disconnect client
		if (message == "d")
			DisconnectClient(ClientID); 

		// Shutdown server
		if (message == "F")
			Shutdown(); 

		// Send incoming message to all different clients 
		for (int j = 0; j < GetConnectionsCount(); j++)
		{
			if (j != ClientID)
				SendToClient(j, message);
		}
	}

	void OnClientDisconnect(int ClientID)
	{
		std::cout << "Client disconnected! ID: " << ClientID << std::endl;
	}
};


int main()
{
	// MyServer A;
	// A.Run();


	// Uncomment this code to make server standart console input.
	// Using this you can write logic to kick clients or shutdown server
	
	// MyServer A;

	// std::thread th([&A]() { A.Run(); });
	// th.detach();

	// std::string message;

	// while (true)
	// {
	// 	getline(std::cin, message);
	// 	std::cout << message << std::endl;

	// 	if (message == "f")
	// 	{
	// 		A.Shutdown();
	// 		break;
	// 	}
		
	// 	A.SendToClient(0, message);
	// }
		
	#if defined WIN32 || defined _WIN64
	//WSAStartup
	WSAData wsaData;
	if (WSAStartup(MAKEWORD(2, 1), &wsaData) != 0)
	{
		std::cerr << "Error: Library initialization failure." << std::endl;
		exit(1);
	}
	#endif
	
	MyServer A;
	for (int i = 0; i < 10000; ++i)
	{
		std::thread th([&]() { A.Run(); });
		A.Shutdown();
		th.join();

		std::cout << i << std::endl;
	}

	#if defined WIN32 || defined _WIN64
	//WSAStartup
	WSACleanup();
	#endif
}
