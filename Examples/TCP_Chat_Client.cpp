#include <iostream>
#include "EN_TCP_Client.h"


class MyClient : public EN::EN_TCP_Client
{
public:
	MyClient(){}

	// A function to be defined by the user. It is used for logic after connection
	void OnConnect() {}

	// A function to be defined by the user. It is used to process incoming messages from the server
	void ServerMessageHandler(std::string message)
	{
		std::cout << message << std::endl;
	}

	// A function to be defined by the user. Performed after disconnected from the server
	void OnDisconnect()
	{
		std::cout << "Server disconnected." << std::endl;
	}
};



int main()
{	
	MyClient A;

	// Check if connection success
	if (A.Connect() == false)
	{
		std::cout << "Failed to connect" << std::endl;
		return 0;
	}

	std::string message;

	while (true)
	{
		// Get line from standart input
		getline(std::cin, message);

		// Stop while loop 
		if (message == "f")
			break;

		// Check if we still connected
		if (A.IsConnected())
			A.SendToServer(message);

		else break;
	}
	
	// Disconnect client from server if still connected
	if (A.IsConnected())
		A.Disconnect();

	return 0;
}
