#include "../EN_TCP_Server.h"
#include "../EN_ThreadBarrier.h"
#include "map"

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

		void OnClientDisconnect(EN_SOCKET clientSocket);
	public:
		EN_FT_Server_Internal_FileTransmitter(EN_FT_Server* fT_Server);
	};

	class EN_FT_Server_Internal_MessageTransmitter : public EN::EN_TCP_Server
	{
	private:
		EN_FT_Server* FT_Server;
	protected:
		void OnClientConnected(EN_SOCKET clientSocket);

		void ClientMessageHandler(EN_SOCKET clientSocket, std::string message);

		void OnClientDisconnect(EN_SOCKET clientSocket);
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

		// Thread crosswalk to work with MtToFtSocketsMap
		EN::EN_ThreadCrossWalk MtToFtSocketsCW;
		// Map with message transmitter socket as key and file transmitter socket as value
		std::map<EN_SOCKET, EN_SOCKET> MtToFtSocketsMap;

		// Thread crosswalk to work with FtSocketToBarrierMap
		EN::EN_ThreadCrossWalk FtSocketToBarrierCW;
		// Map with file transmitter socket as key and thread barrier as value
		std::map<EN_SOCKET, EN::EN_ThreadBarrier*> FtSocketToBarrierMap;

		// Thread crosswalk to work with MtSocketToBarrierMap
		EN::EN_ThreadCrossWalk MtSocketToBarrierCW;
		// Map with message transmitter socket as key and thread barrier as value
		std::map<EN_SOCKET, EN::EN_ThreadBarrier*> MtSocketToBarrierMap;
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
	
		void OnClientConnected(EN_SOCKET clientSocket)
		{
			EN_SOCKET ftSock;
			MtToFtSocketsCW.CarStartCrossRoad();
			auto f = MtToFtSocketsMap.find(clientSocket);
			if (f != MtToFtSocketsMap.end())
				ftSock = f->second;
			MtToFtSocketsCW.CarStopCrossRoad();

			std::cout << "Connected! Sock map: MT=" << std::to_string(clientSocket) + " FT=" + std::to_string(ftSock) << std::endl; 
			SendToClient(clientSocket, "Hello from FT server");
		}

		void ClientMessageHandler(EN_SOCKET clientSocket, std::string message)
		{
			std::cout << "Message from client: " << std::to_string(clientSocket) << " Message: " << message << std::endl;
		}

		void OnClientDisconnect(EN_SOCKET clientSocket)
		{
			EN_SOCKET ftSock;
			MtToFtSocketsCW.CarStartCrossRoad();
			auto f = MtToFtSocketsMap.find(clientSocket);
			if (f != MtToFtSocketsMap.end())
				ftSock = f->second;
			MtToFtSocketsCW.CarStopCrossRoad();

			std::cout << "Disconnected! Sock map: MT=" << std::to_string(clientSocket) + " FT=" + std::to_string(ftSock) << std::endl; 
		}
	
		bool SendToClient(EN_SOCKET clientSocket, std::string message)
		{
			return MessageTransmitter.SendToClient(clientSocket, message);
		}

		void MulticastSend(std::string message)
		{
			MessageTransmitter.MulticastSend(message);
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
		// Add pair with client socket and barrier to map
		EN_ThreadBarrier* socketsBarrier = new EN_ThreadBarrier;
		FT_Server->FtSocketToBarrierCW.PedestrianStartCrossRoad();
		FT_Server->FtSocketToBarrierMap.emplace(clientSocket, socketsBarrier);
		FT_Server->FtSocketToBarrierCW.PedestrianStopCrossRoad();

		SendToClient(clientSocket, "FtSockDesc " + std::to_string(clientSocket));
		
		LOG(LogLevels::Info, "Internal file transmitter connected! Socket descriptor: " + std::to_string(clientSocket));
		socketsBarrier->Wait(2); // Wait A
	}

	void EN_FT_Server_Internal_FileTransmitter::OnClientDisconnect(EN_SOCKET clientSocket)
	{
		EN_ThreadBarrier* barr;
		// Remove pair with file transmitter socket and barrier from map
		FT_Server->FtSocketToBarrierCW.PedestrianStartCrossRoad();
		auto f = FT_Server->FtSocketToBarrierMap.find(clientSocket);
		if (f != FT_Server->FtSocketToBarrierMap.end())
		{
			barr = f->second;
			FT_Server->FtSocketToBarrierMap.erase(f);
			FT_Server->FtSocketToBarrierCW.PedestrianStopCrossRoad();
			barr->Wait(2); // Wait B
			delete barr;
		}
		else
		{
			LOG(LogLevels::Warning, "No thread barrier on FT server. Possibly memmory leak!");
			FT_Server->FtSocketToBarrierCW.PedestrianStopCrossRoad();
		}
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

		WaitMessage(clientSocket, ftSockDesc);
		
		auto vec = Split(ftSockDesc);

		int ftSock;
		if (StringToInt(ftSockDesc, ftSock))

		// Add pair message socket to file transmitter socket to map
		FT_Server->MtToFtSocketsCW.PedestrianStartCrossRoad();
		FT_Server->MtToFtSocketsMap.emplace(clientSocket, (EN_SOCKET)ftSock);
		FT_Server->MtToFtSocketsCW.PedestrianStopCrossRoad();

		EN_ThreadBarrier* barr;
		// Find required thread barrier
		FT_Server->FtSocketToBarrierCW.CarStartCrossRoad();
		auto f = FT_Server->FtSocketToBarrierMap.find((EN_SOCKET)ftSock);

		// Check if it is thread barrier on map
		if (f != FT_Server->FtSocketToBarrierMap.end())
		{
			// Save ptr to thread barrier
			barr = f->second;
			FT_Server->FtSocketToBarrierCW.CarStopCrossRoad();

			// Add thread barrier with message transmitter key to map
			FT_Server->MtSocketToBarrierCW.PedestrianStartCrossRoad();
			FT_Server->MtSocketToBarrierMap.emplace(clientSocket, barr);
			FT_Server->MtSocketToBarrierCW.PedestrianStopCrossRoad();
		}
		else
		{
			LOG(LogLevels::Error, "Not find client barrier!");
			FT_Server->FtSocketToBarrierCW.CarStopCrossRoad();
			throw("Not find client barrier!");
		}

		if (vec[0] != "FtSockDesc")
		{
			LOG(LogLevels::Warning, "Not FT client connected! Force disconnected it! Socket descriptor: " + std::to_string(clientSocket));
			barr->Wait(2);
			FT_Server->Disconnect(clientSocket);
			return;
		}

		barr->Wait(2); // Wait A
		FT_Server->OnClientConnected(clientSocket);
	}

	void EN_FT_Server_Internal_MessageTransmitter::ClientMessageHandler(EN_SOCKET clientSocket, std::string message)
	{
		FT_Server->ClientMessageHandler(clientSocket, message);
	}

	void EN_FT_Server_Internal_MessageTransmitter::OnClientDisconnect(EN_SOCKET clientSocket)
	{
		EN_ThreadBarrier* barr;
		// Remove pair with message transmitter socket and barrier from map
		FT_Server->MtSocketToBarrierCW.PedestrianStartCrossRoad();
		auto f = FT_Server->MtSocketToBarrierMap.find(clientSocket);
		if (f != FT_Server->MtSocketToBarrierMap.end())
		{
			barr = f->second;
			FT_Server->MtSocketToBarrierMap.erase(f);	
			FT_Server->MtSocketToBarrierCW.PedestrianStopCrossRoad();
			FT_Server->OnClientDisconnect(clientSocket);
			barr->Wait(2); // Wait B
		}
		
		// Remove message transmitter to file transmitter pair from map
		FT_Server->MtToFtSocketsCW.PedestrianStartCrossRoad();
		FT_Server->MtToFtSocketsMap.erase(clientSocket);
		FT_Server->MtToFtSocketsCW.PedestrianStopCrossRoad();
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

		A.MulticastSend(message);
	}

	th.join();
}