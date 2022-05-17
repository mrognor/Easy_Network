#include <iostream>
#include "../EN_UDP_Server.h"

class MyServer : public EN::EN_UDP_Server
{
public:
	MyServer()
	{
		// IpAddress = "192.168.1.69"; Default set to localhost
		// Port = <put int here> to set port. Default port is 1111
		// MaxMessageSize = <put int here> to set max message size. Default set to 256.
		// You have to set synchronizied it with client
		// ServerBuferType queue or stack
		MaxMessageSize = 512;
		ThreadAmount = 1;
		ServerBuferType = EN::Queue;
	}
	// Third parametr in milliseconds
	void ClientMessageHandler(std::string message, sockaddr_in ClientSocketAddr, long long TimeSincePackageArrived)
	{
		if (TimeSincePackageArrived > 700)
			return;
			
		#ifdef WIN32
		Sleep(1000);
		#else
		usleep(1000);
		#endif

		char str[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &(ClientSocketAddr.sin_addr), str, INET_ADDRSTRLEN);

		std::cout << "Message: " << message << " Ip: " << str <<
			" Port: " << ntohs(ClientSocketAddr.sin_port) << " Time: " << TimeSincePackageArrived << std::endl;

		if (message == "f")
			Shutdown();

		SendToClient(message, ClientSocketAddr);
	}

	// Function work between putting message in buffer. Return true if you want to put message in thread
	bool ImportantClientMessageHandler(std::string message, sockaddr_in ClientSocketAddr, long long TimeWhenPackageArrived)
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
