#include <iostream>
#include "include/EN_TCP_Server.h"
#include <fstream>

class MyServer : public EN::EN_TCP_Server
{
public:
	MyServer()
	{
		//IpAddress = "192.168.1.69";
		// IpAddress = IP to set real computer ip address
		// IpAddress = LOCALHOST to set localhost computer ip
		// Port = <put int here> to set port. Default port is 1111
	}

	void OnClientConnected(int ClientID)
	{
		std::cout << "Client Connected! id: " << ClientID << std::endl;
		EN::Send(ClientSockets[ClientID], "Welcome. You are connected to server.");
	}

	void ClientMessageHandler(std::string message, int ClientID)
	{
		std::cout << message << std::endl;
		std::string exec_message = message + " > tmp.txt";
		system(exec_message.c_str());
		std::ifstream TmpFile("tmp.txt");
		
		std::string send_message;
		while(getline(TmpFile, send_message))
		{
			EN::Send(ClientSockets[ClientID], send_message);
		}
		TmpFile.close();
		system("rm tmp.txt");
	}

	void OnClientDisconnect(int ClientID)
	{
		std::cout << "Client disconnected! ID: " << ClientID << std::endl;
	}
};


int main()
{
	MyServer A;
	A.Run();
	/*
	MyServer A;

	std::thread th([&A]() { A.Run(); });
	th.detach();

	std::string message;

	while (true)
	{
		getline(std::cin, message);

		if (message == "f")
		{
			A.Shutdown();
			break;
		}
	}
	*/
}
