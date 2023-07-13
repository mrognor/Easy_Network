#include <iostream>
#include "EN_TCP_Server.h"

class MyServer : public EN::EN_TCP_Server
{
public:
	MyServer()
	{
		// IpAddress = "192.168.1.64"; // Default set to localhost. Read description for this variable before use it.
		// Port = <put int here> to set port. Default port is 1111
	}

	virtual void OnClientConnected(EN_SOCKET clientSocket) override
	{
		LOG(EN::LogLevels::Info, "Client connected! Socket descriptor: " + std::to_string(clientSocket));
		SendToClient(clientSocket, "Welcome. You are connected to server.");
	}

	virtual void ClientMessageHandler(EN_SOCKET clientSocket, std::string message) override
	{
		// Important. This function is run in a separate thread. 
		// If you want to write data to class variables, you should use mutexes or other algorithms for thread-safe code.
		LOG(EN::LogLevels::Info, "From: " + std::to_string(clientSocket) + " Message: " + message);

		// Disconnect client
		if (message == "d")
			DisconnectClient(clientSocket); 

		// Shutdown server
		if (message == "F")
		{
			LOG(EN::LogLevels::Info, "Server was shutdown by client. Enter any key to end programm");
			Shutdown(); 
		}

		// Send incoming message to all different clients
		// Blocking the client sockets list is necessary because while the cycle is going through, 
		// one of the clients may disconnect and UB will occur. If the client disconnects during the lock, 
		// its socket will not be released until the unlock occurs, and the Send method for this client returns false
		LockClientSockets();
		for (EN_SOCKET sock : ClientSockets)
		{
			if (sock != clientSocket)
				SendToClient(sock, message);
		}
		UnlockClientSockets();
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
