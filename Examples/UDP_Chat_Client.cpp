#include "EN_UDP_Client.h"

class MyClient : public EN::EN_UDP_Client
{
public:
	MyClient()
	{
		// ServerIpAddress = "178.21.11.82"; Default set to localhost
		// ServerPort = <put int here> to set port. Default port is 1111
	}

	virtual void ServerMessageHandler(std::string message) override
	{
		LOG(EN::LogLevels::Info, "Message: " + message);
	}

};


int main()
{
	MyClient A;
	// Start client
	A.Run();

	std::string msg;
	while (true)
	{
		// Read line from standart input
		getline(std::cin, msg);

		// Exit from while loop
		if (msg == "exit()")
			break;
		
		A.SendToServer(msg);
	}

	// Stop cient
	A.Stop();
}
