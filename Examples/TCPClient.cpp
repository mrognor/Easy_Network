#include <iostream>
#include "../EN_TCP_Client.h"


class MyClient : public EN::EN_TCP_Client
{
public:
	bool IsConnected = false;

	MyClient(){}

	// A function to be defined by the user. It is used for logic after connection
	void AfterConnect()
	{
		IsConnected = true; 
	}

	// A function to be defined by the user. It is used to process incoming messages from the server
	void ServerMessageHandler(std::string message)
	{
		std::cout << message << std::endl;
	}

	// A function to be defined by the user. Performed before disconnected from the server
	void BeforeDisconnect()
	{
		if (IsConnected)
		{
			SendToServer("Goodbye");
			std::cout << "Server disconnected" << std::endl;
		}
	}
};



int main()
{
	MyClient A;

	A.Connect();
	A.Run();

	std::string message;

	while (true)
	{
		getline(std::cin, message);

		if (message == "f")
			return 0;

		A.SendToServer(message);
	}
	A.Disconnect();
	return 0;
}
