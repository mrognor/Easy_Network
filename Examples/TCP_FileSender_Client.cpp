#include <iostream>
#include "..\EN_TCP_FileSender.h"

int main()
{
	std::cout << "Write server ip" << std::endl;
	std::string ip;
	getline(std::cin, ip);

	EN_TCP_FileSender A;
	A.Connect(ip, 1111);

	std::string message;

	while (true)
	{
		getline(std::cin, message);
			
		if (message.find("send file") != -1)
		{
			A.SendToServer("send file");
			A.SendFileToServer(EN::Split(message)[2], EN::DownloadStatus);
			continue;
		}

		A.SendToServer(message);
	}
	A.Disconnect();

	system("pause");
	return 0;
}
