#include "../EN_RAU_Client.h"

class MyClient : public EN::EN_RAU_Client
{
public:
	MyClient() 
	{ 
		// MaxUnreliableMessageSize = 512; Default set to 64
		// You have to set synchronizied it with client
	}

	// A function to be defined by the user. It is used for logic after connection
	void AfterConnect() 
	{ 
		SendToServer("Hello"); 
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
			std::cout << "Client disconnected" << std::endl;
		}
		else std::cout << "Server disconnected. Press any key to try to reconnect" << std::endl;
	}
};



int main()
{
	MyClient A;
	
	// Connect to server
	if (A.Connect() == false)
	{
		std::cout << "Failed to connect" << std::endl;
		return 0;
	}

	// Start client
	A.Run();

	std::string message;

	// Bool variable to change message type 
	bool IsTCP = true;

	while (true)
	{
		// Read line from standart input
		getline(std::cin, message);

		// Break from while loop
		if (message == "f")
			break;

		// Check if we still connected
		if (A.IsConnected())
		{
			// Reliable send
			if (IsTCP)
			{
				A.SendToServer("TCP " + message, true);
				IsTCP = false;
			}
			// Unreliable send
			else 
			{
				A.SendToServer("UDP " + message, false);
				IsTCP = true;
			}
		}
		// Break from while loop in case of server disconnect
		else
		{
			bool IsSuccessConnection = false;
			for (int i = 0; i < 25; i++)
			{
				std::cout << "Attempt to reconnect. Attempt: " << i << std::endl;
				if (A.Connect())
				{
					IsSuccessConnection = true;
					A.Run();
					break;
				}
				EN::Delay(20);
			}
			if (!IsSuccessConnection)
				break;
		}
	}

	// Disconnect from server if still connected
	if (A.IsConnected())
		A.Disconnect();

	return 0;
}
