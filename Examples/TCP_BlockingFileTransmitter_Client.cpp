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
		ip = "127.0.0.1"; // Default value
	else
		ip = vec[0];
	
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
	std::string responce;
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
		std::vector<std::string> InterpretedMessage = EN::Split(message);

		if (InterpretedMessage[0] == "send")
		{
			if (EN::IsFileExist(InterpretedMessage[1]))
			{
				A.SendToServer("was " + InterpretedMessage[1]);
				A.WaitMessage(responce);

				std::vector<std::string> parsedResponce = EN::Split(responce);

				if (parsedResponce[0] == "ok")
				{
					LOG(EN::LogLevels::Info, "Continue the previous sending");
					EN::SendFile(A.GetSocket(), InterpretedMessage[1], isStop, transferingSpeed, std::stoll(parsedResponce[1]), transmissionStatus);
				}
				else
				{
					LOG(EN::LogLevels::Info, "Starting to send the file");
					EN::SendFile(A.GetSocket(), InterpretedMessage[1], isStop, transferingSpeed, 0, transmissionStatus);
				}
			}
			else
				LOG(EN::LogLevels::Info, "The file is not in the directory");
			continue;
		}

		if (InterpretedMessage[0] == "get")
		{
			if (EN::IsFileExist(InterpretedMessage[1] + ".tmp"))
			{
				LOG(EN::LogLevels::Info, "Continue the previous file receiving");
				A.SendToServer("continue " + InterpretedMessage[1] + " " + std::to_string(EN::GetFileSize(InterpretedMessage[1] + ".tmp")));
				A.WaitMessage(responce);

				if (responce == "ok")
				{					
					if (EN::RecvFile(A.GetSocket(), isStop, transmissionStatus))
					{
						LOG(EN::LogLevels::Info, "The file was received");
					}
					else
						LOG(EN::LogLevels::Info, "The file was not received");
				}
				else
				{
					LOG(EN::LogLevels::Info, "The file is not on the server");
				}
			}
			else
			{
				LOG(EN::LogLevels::Info, "Starting the file receiving");

				A.SendToServer("get " + InterpretedMessage[1]);
				A.WaitMessage(responce);
				
				if (responce == "ok")
				{					
					if (EN::RecvFile(A.GetSocket(), isStop, transmissionStatus))
					{
						LOG(EN::LogLevels::Info, "The file was received");
					}
					else
						LOG(EN::LogLevels::Info, "The file was not received");
				}
				else
					LOG(EN::LogLevels::Info, "The file is not on the server");
			}
		}
	}
	A.Disconnect();

	return 0;
}