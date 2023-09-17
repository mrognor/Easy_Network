#include "../EN_TCP_Server.h"

class EN_FT_Server;

class EN_FT_Server_Internal_FileTransmitter : public EN::EN_TCP_Server
{
private:
	EN_FT_Server* FT_Server;
protected:
	virtual void OnClientConnected(EN_SOCKET clientSocket) override { std::cout << "FT conn" << std::endl; }

	virtual void ClientMessageHandler(EN_SOCKET clientSocket, std::string message) override {}

	virtual void OnClientDisconnect(EN_SOCKET clientSocket) override { std::cout << "FT disconn" << std::endl; }
public:
	EN_FT_Server_Internal_FileTransmitter(EN_FT_Server* fT_Server);
};

class EN_FT_Server_Internal_MessageTransmitter : public EN::EN_TCP_Server
{
private:
	EN_FT_Server* FT_Server;
protected:
	virtual void OnClientConnected(EN_SOCKET clientSocket) override { std::cout << "MSG conn" << std::endl; }

	virtual void ClientMessageHandler(EN_SOCKET clientSocket, std::string message) override {}

	virtual void OnClientDisconnect(EN_SOCKET clientSocket) override { std::cout << "MSG disconn" << std::endl; }
public:
	EN_FT_Server_Internal_MessageTransmitter(EN_FT_Server* fT_Server);
};

class EN_FT_Server
{
private:
	friend EN_FT_Server_Internal_FileTransmitter;
	friend EN_FT_Server_Internal_MessageTransmitter;

	std::string IpAddress;
	int FileTransmitterPort = 1111;
	int MessageTransmitterPort = 1112;

	EN_FT_Server_Internal_FileTransmitter FileTransmitter;
	EN_FT_Server_Internal_MessageTransmitter MessageTransmitter;

	std::thread FileTransmitterRunThread;
public:
	EN_FT_Server() : FileTransmitter(this), MessageTransmitter(this) {}

	void Run()
    {
        FileTransmitterRunThread = std::thread ([&]()
        {
            FileTransmitter.Run();
        });

        MessageTransmitter.Run();
    }

    void Shutdown()
    {
        FileTransmitter.Shutdown();
        MessageTransmitter.Shutdown();
        FileTransmitterRunThread.join();
    }
};

EN_FT_Server_Internal_FileTransmitter::EN_FT_Server_Internal_FileTransmitter(EN_FT_Server* fT_Server)
{
	FT_Server = fT_Server;
	IpAddress = FT_Server->IpAddress;
	Port = FT_Server->FileTransmitterPort;
}

EN_FT_Server_Internal_MessageTransmitter::EN_FT_Server_Internal_MessageTransmitter(EN_FT_Server* fT_Server)
{
	FT_Server = fT_Server;
	IpAddress = FT_Server->IpAddress;
	Port = FT_Server->MessageTransmitterPort;
}

int main()
{
	EN_FT_Server A;

	std::thread th([&A]() 
	{
		try 
		{
			A.Run(); 
		}
		catch (std::runtime_error& err)
		{
			LOG(EN::LogLevels::Error, "Run throw error with error code: " + std::string(err.what()));
		}
	});
	
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

	th.join();
}