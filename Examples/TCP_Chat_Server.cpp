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

	void OnClientConnected(size_t ClientID)
	{
		std::cout << "Client connected! Id: " << ClientID << std::endl;
		SendToClient(ClientID, "Welcome. You are connected to server.");
	}

	void ClientMessageHandler(std::string message, size_t ClientID)
	{
		// Important. This function is run in a separate thread. 
		// If you want to write data to class variables, you should use mutexes or other algorithms for thread-safe code.
		std::cout << message << std::endl;

		// Disconnect client
		if (message == "d")
			DisconnectClient(ClientID); 

		// Shutdown server
		if (message == "F")
		{
			std::cout << "Server was shutdown by client. Enter any key to end programm" << std::endl;
			Shutdown(); 
		}

		// Send incoming message to all different clients 
		for (size_t j = 0; j < GetConnectionsCount(); ++j)
		{
			if (j != ClientID)
				SendToClient(j, message);
		}
	}

	void OnClientDisconnect(size_t ClientID)
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
		
		A.SendToClient(0, message);
	}

	th.join();
}
