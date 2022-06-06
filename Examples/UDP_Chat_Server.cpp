#include <iostream>
#include "../EN_UDP_Server.h"

class MyServer : public EN::EN_UDP_Server
{
public:
	MyServer()
	{
		// IpAddress = "192.168.1.64"; // Default set to localhost
		// Port = <put int here> to set port. Default port is 1111
		// MaxMessageSize = <put int here> to set max message size. Default set to 256.
		// You have to set synchronizied it with client
		// ServerBuferType queue or stack
		MaxMessageSize = 512;
		ThreadAmount = 1;
		ServerBuferType = EN::Queue;
		MaxStackBuffSize = 4;
	}

	// Third parametr in milliseconds
	void ClientMessageHandler(std::string message, std::string ClientSocketAddr, long long TimeSincePackageArrived)
	{
		//if (TimeSincePackageArrived > 700)
		//	return;
		
		EN::Delay(5000);

		if (message == "f")
			Shutdown();
		
		std::cout << message << std::endl;

		SendToClient(message, ClientSocketAddr);
	}

	// Function work between putting message in buffer. Return true if you want to put message in buffer
	bool InstantClientMessageHandler(std::string message, std::string ClientSocketAddr, long long TimeWhenPackageArrived)
	{
		std::cout << "Important! " << message << std::endl;
		if (message == "oleg")
			return false;
		return true;
	}
};


int main()
{
	MyServer A;
	A.Run();

	system("pause");
}
