#include <iostream>
#include "../EN_TCP_Client.h"


class MyClient : public EN::EN_TCP_Client
{
public:
	MyClient(){}

	// A function to be defined by the user. It is used for logic after connection
	void AfterConnect() 
	{
		char myIP[16];
		unsigned int myPort;
		sockaddr_in my_addr;
		socklen_t len = sizeof(my_addr);
		getsockname(*GetSocket(), (struct sockaddr*)&my_addr, &len);
		inet_ntop(AF_INET, &my_addr.sin_addr, myIP, sizeof(myIP));
		myPort = ntohs(my_addr.sin_port);

		printf("Local ip address: %s\n", myIP);
		printf("Local port : %u\n", myPort);
	}

	// A function to be defined by the user. It is used to process incoming messages from the server
	void ServerMessageHandler(std::string message)
	{
		std::cout << message << std::endl;
	}

	// A function to be defined by the user. Performed before disconnected from the server
	void BeforeDisconnect()
	{
		if (IsConnected())
		{
			SendToServer("Goodbye");
			std::cout << "Server disconnected" << std::endl;
		}
	}
};



int main()
{
	MyClient A;

	if (A.Connect() == false)
	{
		std::cout << "Failed to connect" << std::endl;
		return 0;
	}

	A.Run();

	std::string message;

	while (true)
	{
		getline(std::cin, message);

		if (message == "f")
			break;

		if (A.IsConnected())
			A.SendToServer(message);
		else break;
	}

	A.Disconnect();

	return 0;
}
