#include "../EN_TCP_Server.h"

namespace EN
{
	class EN_FT_Server;

	class EN_FT_Server_Internal_FileTransmitter : public EN::EN_TCP_Server
	{
	private:
		EN_FT_Server* FT_Server;
	protected:
		void OnClientConnected(EN_SOCKET clientSocket);

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
		void OnClientConnected(EN_SOCKET clientSocket);

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
		int MessageTransmitterPort = 1111;
		int FileTransmitterPort = 1112;

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

		void Disconnect(EN_SOCKET socketToDisconnect)
		{

		}
	};

	EN_FT_Server_Internal_FileTransmitter::EN_FT_Server_Internal_FileTransmitter(EN_FT_Server* fT_Server)
	{
		FT_Server = fT_Server;
		IpAddress = FT_Server->IpAddress;
		Port = FT_Server->FileTransmitterPort;
	}

	void EN_FT_Server_Internal_FileTransmitter::OnClientConnected(EN_SOCKET clientSocket) 
	{
		LOG(LogLevels::Info, "Internal file transmitter connected! Socket descriptor: " + std::to_string(clientSocket));
		SendToClient(clientSocket, "FtSockDesc " + std::to_string(clientSocket));
	}

	EN_FT_Server_Internal_MessageTransmitter::EN_FT_Server_Internal_MessageTransmitter(EN_FT_Server* fT_Server)
	{
		FT_Server = fT_Server;
		IpAddress = FT_Server->IpAddress;
		Port = FT_Server->MessageTransmitterPort;
	}

	void EN_FT_Server_Internal_MessageTransmitter::OnClientConnected(EN_SOCKET clientSocket) 
	{ 
		LOG(LogLevels::Info, "Internal message transmitter connected! Socket descriptor: " + std::to_string(clientSocket));
		std::string ftSockDesc;
		if (!WaitMessage(clientSocket, ftSockDesc))
			return;
		
		auto vec = Split(ftSockDesc);
		if (vec[0] != "FtSockDesc")
		{
			FT_Server->Disconnect(clientSocket);
			return;
		}
		
		std::cout << "Sock map: " << std::to_string(clientSocket) << " " << vec[1] << std::endl;
	}
}

int main()
{
	EN::EN_FT_Server A;

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