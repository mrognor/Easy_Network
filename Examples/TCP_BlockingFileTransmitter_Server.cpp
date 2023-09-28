#include "EN_TCP_Server.h"

class MyServer : public EN::EN_TCP_Server
{
public:
	MyServer(std::string ip, int port)
	{
		IpAddress = ip; // Default set to localhost
		Port = port; // Default port is 1111
	}

	virtual void OnClientConnected(EN_SOCKET clientSocket) override
	{
		LOG(EN::LogLevels::Info, "Client connected! Socket descriptor: " + std::to_string(clientSocket));
	}

	virtual void ClientMessageHandler(EN_SOCKET clientSocket, std::string message) override
	{
		// Important. This function is run in a separate thread. 
		// If you want to write data to class variables, you should use mutexes or other algorithms for thread-safe code.
		LOG(EN::LogLevels::Info, message);
		std::atomic_bool ShouldShutdown(false);
		std::atomic_int transferingSpeed(0);
		EN::EN_FileTransmissionStatus transmissionStatus;
		transmissionStatus.SetProgressFunction(EN::DefaultDownloadStatusFunction);

		std::vector<std::string> InterpretedMessage = EN::Split(message);

		if (InterpretedMessage[0] == "was")
		{
			if (EN::IsFileExist(InterpretedMessage[1] + ".tmp"))
			{
				LOG(EN::LogLevels::Info, "Continue the previous file receiving");
				SendToClient(clientSocket, "ok " + std::to_string(EN::GetFileSize(InterpretedMessage[1] + ".tmp")));
			}
			else
			{
				LOG(EN::LogLevels::Info, "Starting the file receiving");
				SendToClient(clientSocket, "bad");
			}

			EN::RecvFile(clientSocket, ShouldShutdown, transmissionStatus);
			return;
		}
		
		if (InterpretedMessage[0] == "get")
		{
			if (EN::IsFileExist(InterpretedMessage[1]))
			{
				LOG(EN::LogLevels::Info, "Starting to send the file");
				SendToClient(clientSocket, "ok");
				EN::SendFile(clientSocket, InterpretedMessage[1], ShouldShutdown, transferingSpeed, 0, transmissionStatus);
			}
			else
			{
				LOG(EN::LogLevels::Info, "The file is not on the server");
				SendToClient(clientSocket, "bad");
			}

			return;
		}

		if (InterpretedMessage[0] == "continue")
		{
			if (EN::IsFileExist(InterpretedMessage[1]))
			{
				LOG(EN::LogLevels::Info, "Continue the previous sending");
				SendToClient(clientSocket, "ok");
				std::uint64_t sendingSize;
				EN::StringToUnsignedLongLong(InterpretedMessage[2], sendingSize);
				EN::SendFile(clientSocket, InterpretedMessage[1], ShouldShutdown, transferingSpeed, sendingSize, transmissionStatus);
			}
			else
			{
				LOG(EN::LogLevels::Info, "The file is not on the server");
				SendToClient(clientSocket, "bad");
			}
			return;
		}
	}

	virtual void OnClientDisconnect(EN_SOCKET clientSocket) override
	{
		LOG(EN::LogLevels::Info, "Client disconnected! Socket descriptor: " + std::to_string(clientSocket));
	}
};


int main()
{
	LOG(EN::LogLevels::Info, "Write server ip or/and port. Format: ip:port. Example: 192.168.1.85:1234.");
	LOG(EN::LogLevels::Info, "If you dont write ip it will be all available pc ips from all networks");
	LOG(EN::LogLevels::Info, "If you dont write port it will be default value: 1111");
	
	std::string addr, ip;
	int port;

	getline(std::cin, addr);
	auto vec = EN::Split(addr, ":");
	
	if (!addr.empty())
		ip = vec[0];

	if (vec.size() == 2)
		EN::StringToInt(vec[1], port);
	else port = 1111; // Default value 

	// No incorrect port check
	MyServer A(ip, port);
	try 
	{
		A.Run(); 
	}
	catch (std::runtime_error& err)
	{
		LOG(EN::LogLevels::Error, "Run throw error with error code: " + std::string(err.what()));
	}

	// Uncomment this code to make server standart console input.
	// Using this you can write logic to kick clients or shutdown server
	/*
	MyServer A;
	std::thread th([&A]() 
	{ 
		try 
		{
			A.Run(); 
		}
		catch (std::runtime_error err)
		{
			LOG(EN::LogLevels::Error, "Run throw error with error code: " + std::string(err.what()));
		}
	});

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