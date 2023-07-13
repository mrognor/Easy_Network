#include "EN_TCP_Client.h"


class MyClient : public EN::EN_TCP_Client
{
public:
	MyClient()
	{
		// IsRunMessageHadlerThread = true; // Variable to disable starting thread with message handler
	}

	// A function to be defined by the user. It is used for logic after connection
	virtual void OnConnect() override
	{
		LOG(EN::LogLevels::Info, "Server connected.");
	}

	// A function to be defined by the user. It is used to process incoming messages from the server
	virtual void ServerMessageHandler(std::string message) override
	{
		LOG(EN::LogLevels::Info, message);
	}

	// A function to be defined by the user. Performed after disconnected from the server
	virtual void OnDisconnect() override
	{
		LOG(EN::LogLevels::Info, "Server disconnected.");
	}
};


int main()
{
	MyClient A;

	// Check if connection success
	if (A.Connect() == false)
	{
		LOG(EN::LogLevels::Info, "Failed to connect");
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

		// Disconnect to client
		if (message == "d")
		{
			A.Disconnect();
			continue;
		}

		// Reconnect to client
		if (message == "r")
			A.Connect();

		// Check if we still connected
		if (A.IsConnected())
			A.SendToServer(message);
		else 
			break;
	}

	// Disconnect client from server if still connected
	if (A.IsConnected())
		A.Disconnect();

	return 0;
}
