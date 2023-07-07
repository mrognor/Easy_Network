#include <iostream>
#include "EN_TCP_Client.h"

class MyClient : public EN::EN_TCP_Client
{
public:
	MyClient()
	{
		IsRunMessageHadlerThread = false;
	}

	// A function to be defined by the user. It is used for logic after connection
	virtual void OnConnect() override {}

	// A function to be defined by the user. It is used to process incoming messages from the server
	virtual void ServerMessageHandler(std::string message) override {}

	// A function to be defined by the user. Performed after disconnected from the server
	virtual void OnDisconnect() override {}
};

int main()
{
	MyClient A;

	std::cout << "Write server ip or/and port. Format: ip:port. Example: 192.168.1.85:1234." << std::endl;
	std::cout << "If you dont write ip it will be default value: 127.0.0.1" << std::endl;
	std::cout << "If you dont write port it will be default value: 1111" << std::endl;

	std::string addr, ip;
	int port;

	getline(std::cin, addr);
	auto vec = EN::Split(addr, ":");
	
	if (addr.empty())
		ip = "127.0.0.1";

	if (vec.size() == 2)
		port = std::atoi(vec[1].c_str());
	else port = 1111; // Default value 

	// No incorrect port check
	if (A.Connect(ip, port) == false)
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

		if (message.find("send file") == 0ull)
		{
			if (EN::IsFileExist(IntrepretedMessage[2]))
			{
				std::atomic_bool isStop(false);
				std::atomic_int transferingSpeed(0);
				A.SendToServer(message);
				EN::SendFile(A.GetSocket(), IntrepretedMessage[2], isStop, transferingSpeed, EN::DownloadStatus, 0);
			}
			else
				std::cout << "No file: " << IntrepretedMessage[2] << " on this directory" << std::endl;
			continue;
		}

		if (message.find("get file") == 0ull)
		{
			std::cout << "Getting file " << IntrepretedMessage[2] << std::endl;

			if (EN::IsFileExist(IntrepretedMessage[2] + ".tmp"))
			{
				uint64_t FileSize = EN::GetFileSize(IntrepretedMessage[2] + ".tmp");

				message = "continue download " + IntrepretedMessage[2] + " " + std::to_string(FileSize);

				A.SendToServer(message);

				std::string responce;
				A.WaitMessage(responce);

				if (responce == "ok")
				{
					std::atomic_bool isStop(false);
					
					if (EN::RecvFile(A.GetSocket(), isStop, EN::DownloadStatus))
						std::cout << "File: " << IntrepretedMessage[2] << " downloaded" << std::endl;
					else
						std::cout << "File: " << IntrepretedMessage[2] << " dont downloaded" << std::endl;
				}
				else
					std::cout << "No file: " << IntrepretedMessage[2] << " on server" << std::endl;
				
			}
			else
			{
				A.SendToServer(message);
				
				std::string responce;
				A.WaitMessage(responce);

				if (responce == "ok")
				{
					std::atomic_bool isStop(false);

					if (EN::RecvFile(A.GetSocket(), isStop, EN::DownloadStatus))
						std::cout << "File: " << IntrepretedMessage[2] << " downloaded" << std::endl;
					else
						std::cout << "File: " << IntrepretedMessage[2] << " dont downloaded" << std::endl;
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

	return 0;
}