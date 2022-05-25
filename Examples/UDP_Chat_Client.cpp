#include <iostream>
#include "../EN_UDP_Client.h"
#include <string>

class MyClient : public EN::EN_UDP_Client
{
public:
	MyClient()
	{
		// ServerIpAddres = "37.140.195.195"; //Default set to localhost
		// Port = <put int here> to set port. Default port is 1111
		MaxMessageSize = 512;
	}

	void ServerMessageHandler(std::string message)
	{
		std::cout << "Message: " << message << std::endl;
	}

};


int main()
{
	MyClient A;
	A.Run();

	std::string msg;
	while (true)
	{
		getline(std::cin, msg);
		if (msg == "exit()")
			break;
		
		if (msg == "stress test")
		{
			for (int i = 0; i < 100; i++)
				A.SendToServer("No bitches?");
		}
		A.SendToServer(msg);
	}

	A.Close();

	system("pause");
}
