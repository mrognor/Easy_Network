#include <iostream>
#include "../EN_UDP_Client.h"
#include <string>

class MyClient : public EN::EN_UDP_Client
{
public:
	MyClient()
	{
		// IpAddress = "192.168.1.69"; Default set to localhost
		// Port = <put int here> to set port. Default port is 1111
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
		
		A.SendToServer(msg);
	}

	A.Close();

	system("pause");
}
