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

	LOG(EN::LogLevels::Info, "Write server ip or/and port. Format: ip:port. Example: 192.168.1.85:1234.");
	LOG(EN::LogLevels::Info, "If you dont write ip it will be default value: 127.0.0.1");
	LOG(EN::LogLevels::Info, "If you dont write port it will be default value: 1111");

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
		LOG(EN::LogLevels::Warning, "Failed to connect");
		return 0;
	}

	LOG(EN::LogLevels::Info, "Connected to server");

	if (!A.SendToServer("FileSender client connected"))
	{
		LOG(EN::LogLevels::Info, "Server disconnected");
		return 0;
	}

	std::string message;
	
	std::atomic_bool isStop(false);
	std::atomic_int transferingSpeed(0);
	EN::EN_FileTransmissionStatus transmissionStatus;
	transmissionStatus.SetProgressFunction(EN::DefaultDownloadStatusFunction);

	while (true)
	{
		getline(std::cin, message);

		if (A.IsConnected() == false)
		{
			LOG(EN::LogLevels::Warning, "Server disconnected");
			break;
		}
		std::vector<std::string> IntrepretedMessage = EN::Split(message);

		if (message.find("send file") == 0ull)
		{
			if (EN::IsFileExist(IntrepretedMessage[2]))
			{
				A.SendToServer(message);
				EN::SendFile(A.GetSocket(), IntrepretedMessage[2], isStop, transferingSpeed, 0, transmissionStatus);
			}
			else
				LOG(EN::LogLevels::Info, "No file: " + IntrepretedMessage[2] + " on this directory");
			continue;
		}

		if (message.find("get file") == 0ull)
		{
			LOG(EN::LogLevels::Info, "Getting file " + IntrepretedMessage[2]);

			if (EN::IsFileExist(IntrepretedMessage[2] + ".tmp"))
			{
				uint64_t FileSize = EN::GetFileSize(IntrepretedMessage[2] + ".tmp");

				message = "continue download " + IntrepretedMessage[2] + " " + std::to_string(FileSize);

				A.SendToServer(message);

				std::string responce;
				A.WaitMessage(responce);

				if (responce == "ok")
				{					
					if (EN::RecvFile(A.GetSocket(), isStop, transmissionStatus))
					{
						LOG(EN::LogLevels::Info, "File: " + IntrepretedMessage[2] + " downloaded");
					}
					else
					{
						LOG(EN::LogLevels::Info, "File: " + IntrepretedMessage[2] + " dont downloaded");
					}
				}
				else
				{
					LOG(EN::LogLevels::Info, "No file: " + IntrepretedMessage[2] + " on server");
				}
			}
			else
			{
				A.SendToServer(message);
				
				std::string responce;
				A.WaitMessage(responce);

				if (responce == "ok")
				{
					if (EN::RecvFile(A.GetSocket(), isStop, transmissionStatus))
					{
						LOG(EN::LogLevels::Info, "File: " + IntrepretedMessage[2] + " downloaded");
					}
					else
					{
						LOG(EN::LogLevels::Info, "File: " + IntrepretedMessage[2] + " dont downloaded");
					}
				}
				else
				{
					LOG(EN::LogLevels::Info, "No file: " + IntrepretedMessage[2] + " on server");
				}
			}
			continue;
		}
	}
	A.Disconnect();

	return 0;
}