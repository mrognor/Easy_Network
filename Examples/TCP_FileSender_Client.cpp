#include <iostream>
#include "../EN_TCP_FileSender.h"

int main()
{
	std::cout << "Write server ip" << std::endl;
	std::string ip;
	getline(std::cin, ip);

	EN::EN_TCP_FileSender A;

	if (A.Connect(ip, 1111) == false)
	{
		std::cout << "Failed to connect" << std::endl;
		return 0;
	}

	std::cout << "Connected to server" << std::endl;

	if (A.IsConnected())
		A.SendToServer("FileSender client connected");
	else
	{
		std::cout << "Server disconnected" << std::endl;
		return 0;
	}


	std::string message;

	while (true)
	{
		getline(std::cin, message);

		if (A.IsConnected() == false)
		{
			std::cout << "Server disconnected" << std::endl;
			break;
		}
		std::vector<std::string> IntrepretedMessage = EN::Split(message);

		if (message.find("send file") != -1)
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

		if (message.find("get file") != -1)
		{
			std::cout << "Getting file " << IntrepretedMessage[2] << std::endl;
			
			if (EN::IsFileExist(IntrepretedMessage[2] + ".tmp"))
			{
				// Open file
				std::ifstream SendingFile(IntrepretedMessage[2] + ".tmp", std::ios::binary);

				// Find file size in bytes
				SendingFile.seekg(0, std::ios::end);
				uint64_t FileSize = SendingFile.tellg();
				SendingFile.close();
				rename((IntrepretedMessage[2] + ".tmp").c_str(), IntrepretedMessage[2].c_str());
				message = "continue download " + IntrepretedMessage[2] + " " + std::to_string(FileSize);

				A.SendToServer(message);

				std::string responce;
				A.RecvMessageFromServer(responce);

				if (responce == "ok")
				{
					if (A.ContinueRecvFileFromServer(EN::DownloadStatus))
						std::cout << "File: " << IntrepretedMessage[2] << " downloaded" << std::endl;
					else
					{
						std::cout << "File: " << IntrepretedMessage[2] << " dont downloaded" << std::endl;
						rename(IntrepretedMessage[2].c_str(), (IntrepretedMessage[2] + ".tmp").c_str());
					}
				}
				else
				{
					std::cout << "No file: " << IntrepretedMessage[2] << " on server" << std::endl;
				}
			}
			else
			{
				A.SendToServer(message);

				std::string responce;
				A.RecvMessageFromServer(responce);

				if (responce == "ok")
				{
					if (A.RecvFileFromServer(EN::DownloadStatus))
						std::cout << "File: " << IntrepretedMessage[2] << " downloaded" << std::endl;
					else
					{
						std::cout << "File: " << IntrepretedMessage[2] << " dont downloaded" << std::endl;
						rename(IntrepretedMessage[2].c_str(), (IntrepretedMessage[2] + ".tmp").c_str());
					}
				}
				else
				{
					std::cout << "No file: " << IntrepretedMessage[2] << " on server" << std::endl;
				}
			}
			continue;
		}
	}
	A.Disconnect();

	system("pause");
	return 0;
}