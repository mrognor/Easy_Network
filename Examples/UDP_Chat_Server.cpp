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
		ServerBuferType = EN::Stack;
	}

	void ClientMessageHandler(std::string message, sockaddr_in ClientSocketAddr, double TimeSincePackageArrived)
	{
		//if (TimeSincePackageArrived > 7)
		//	return;

		char str[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &(ClientSocketAddr.sin_addr), str, INET_ADDRSTRLEN);

		std::cout << "Message: " << message << " Ip: " << str <<
			" Port: " << ntohs(ClientSocketAddr.sin_port) << " Time: " << TimeSincePackageArrived << std::endl;

		if (message == "f")
			Shutdown();

		SendToClient(message, ClientSocketAddr);
	}

};


int main()
{
	MyServer A;
	A.Run();

	system("pause");
}
