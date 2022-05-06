#include <iostream>
#include "../EN_TCP_FileSender.h"

int main()
{
	std::cout << "Write server ip" << std::endl;
	std::string ip;
	getline(std::cin, ip);

	EN_TCP_FileSender A;
	
	if (A.Connect(ip, 1111) == false)
	{
		std::cout << "Failed to connect" << std::endl;
		return 0;
	}

	if(A.IsConnected())
		A.SendToServer("FileSender client connected");

	std::cout << "Connected to server" << std::endl;

	std::string message;

	while (true)
	{
		getline(std::cin, message);

		if (A.IsConnected() == false)
			break;

		std::vector<std::string> IntrepretedMessage = EN::Split(message);

		if (message.find("send file") != -1 )
		{
			if (EN::IsFileExist(IntrepretedMessage[2]))
			{
				A.SendToServer(message);
				A.SendFileToServer(IntrepretedMessage[2], EN::DownloadStatus);
			}
			else 
				std::cout << "No file: " << IntrepretedMessage[2] << " on this directory" << std::endl;
			continue;
		}

		if (message.find("get file") != -1 )
		{
			std::cout << "Getting file " << IntrepretedMessage[2] << std::endl;
			std::string responce;
			A.SendToServer(message);
			A.RecvMessageFromServer(responce);
			if (responce == "ok")
			{
				A.RecvFileFromServer();
				std::cout << "File: " << IntrepretedMessage[2] << " downloaded" << std::endl;
			}
			else
			{
				std::cout << "No file: " << IntrepretedMessage[2] << " on server" << std::endl;
			}

			continue;
		}
	}
	A.Disconnect();

	system("pause");
	return 0;
}
