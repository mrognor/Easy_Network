#include <iostream>
#include "../EN_TCP_Server.h"

class MyServer : public EN::EN_TCP_Server
{
public:
	MyServer(std::string ip)
	{
		IpAddress = ip; //Default set to localhost
		// Port = <put int here> to set port. Default port is 1111
	}

	void OnClientConnected(int ClientID)
	{
		std::cout << "Client Connected! id: " << ClientID << std::endl;
	}

	void ClientMessageHandler(std::string message, int ClientID)
	{
		// Important. This function is run in a separate thread. 
		// If you want to write data to class variables, you should use mutexes or other algorithms for thread-safe code.
		std::cout << message << std::endl;
		bool f = false;
		std::vector<std::string> InterpretedMessage = EN::Split(message);

		if (message.find("send file") != -1)
		{
			EN::RecvFile(ClientSockets[ClientID], f, EN::DownloadStatus);
			return;
		}

		if (message.find("get file") != -1)
		{
			if (EN::IsFileExist(InterpretedMessage[2]))
			{
				EN::Send(ClientSockets[ClientID], "ok");
				EN::SendFile(ClientSockets[ClientID], InterpretedMessage[2], f, EN::DownloadStatus);
			}
			else
				EN::Send(ClientSockets[ClientID], "bad");
			return;
		}
	}

	void OnClientDisconnect(int ClientID)
	{
		std::cout << "Client disconnected! ID: " << ClientID << std::endl;
	}
};


int main()
{
	std::cout << "Write server ip" << std::endl;
	std::string ip;
	getline(std::cin, ip);
	MyServer A(ip);
	A.Run();


	// Uncomment this code to make server standart console input.
	// Using this you can write logic to kick clients or shutdown server
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
	system("pause");
}
