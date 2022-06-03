#include "EN_RAU_Client.h"

namespace EN
{
	// TCP client
	EN_RAU_TCP_Client::EN_RAU_TCP_Client(EN_RAU_Client* rau_Client)
	{
		RAU_Client = rau_Client;
	}

	void EN_RAU_TCP_Client::AfterConnect()
	{
		SendToServer("UDP IP HERE");
		RAU_Client->AfterConnect();
	}

	void EN_RAU_TCP_Client::ServerMessageHandler(std::string message)
	{
		if (RAU_Client->ServerThreadID != -1)
			RAU_Client->ServerMessageHandler(message);
		else RAU_Client->ServerThreadID = std::atoi(message.c_str());
	}

	void EN_RAU_TCP_Client::BeforeDisconnect()
	{
		RAU_Client->BeforeDisconnect();
	}

	// UDP client
	EN_RAU_UDP_Client::EN_RAU_UDP_Client(EN_RAU_Client* rau_Client)
	{
		RAU_Client = rau_Client;
		ServerPort = rau_Client->ServerPort + 1;
		ServerIpAddres = rau_Client->ServerIpAddress;
	}

	void EN_RAU_UDP_Client::ServerMessageHandler(std::string message)
	{
		std::cout << "Message: " << message << std::endl;
	}
	
	// RAU client
	EN_RAU_Client::EN_RAU_Client()
	{
		TCP_Client = new EN_RAU_TCP_Client(this);
		UDP_Client = new EN_RAU_UDP_Client(this);
	}

	bool EN_RAU_Client::IsConnected()
	{
		return TCP_Client->IsConnected();
	}

	bool EN_RAU_Client::Connect()
	{
		return TCP_Client->Connect();
	}

	bool EN_RAU_Client::Connect(int port)
	{
		ServerPort = port;
		return TCP_Client->Connect(port);
	}

	bool EN_RAU_Client::Connect(std::string ipAddr, int port)
	{
		ServerIpAddress = ipAddr;
		ServerPort = port;
		return TCP_Client->Connect(ipAddr, port);
	}

	void EN_RAU_Client::Run()
	{
		std::thread TCP_Thread([this]() {TCP_Client->Run(); });
		TCP_Thread.detach();
		UDP_Client->Run();
	}

	void EN_RAU_Client::SendToServer(std::string message, bool IsReliable, int MessageDelay)
	{
		if (IsReliable)
			TCP_Client->SendToServer(message, MessageDelay);
		else
			UDP_Client->SendToServer(std::to_string(ServerThreadID) + " " + message, MessageDelay);
	}

	void EN_RAU_Client::Disconnect()
	{
		TCP_Client->Disconnect();
		UDP_Client->Close();
	}

	EN_RAU_Client::~EN_RAU_Client()
	{
		delete TCP_Client;
		delete UDP_Client;
	}
}