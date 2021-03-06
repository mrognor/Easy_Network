#include "../EN_RAU_Client.h"

class MyClient : public EN::EN_RAU_Client
{
public:
	MyClient() { MaxUnreliableMessageSize = 512; }

	// A function to be defined by the user. It is used for logic after connection
	void AfterConnect() { SendToServer("Hallo"); }

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
		exit(1);
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

	int count = 0;

	while (true)
	{
		getline(std::cin, message);

		if (message == "f")
			break;

		if (A.IsConnected())
		{
			if (count % 2 == 0)
				A.SendToServer("TCP " + message, true);
			else A.SendToServer("UDP " + message, false);
			count++;
		}
		else break;
	}

	A.Disconnect();

	return 0;
}
