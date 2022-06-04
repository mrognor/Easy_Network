#include "EN_RAU_Client.h"

namespace EN
{
	// TCP client
	EN_RAU_TCP_Client::EN_RAU_TCP_Client(EN_RAU_Client* rau_Client)
	{
		RAU_Client = rau_Client;
	}

	void EN_RAU_TCP_Client::ServerMessageHandler(std::string message)
	{
		if (RAU_Client->ServerThreadID != -1)
		{
			if (RAU_Client->IsServerGetUDPAddress == false)
			{
				RAU_Client->IsServerGetUDPAddress = true;
				RAU_Client->AfterConnect();
			}
			else
				RAU_Client->ServerMessageHandler(message);
		}
		else 
			RAU_Client->ServerThreadID = std::atoi(message.c_str());
	}

	void EN_RAU_TCP_Client::BeforeDisconnect()
	{
		RAU_Client->BeforeDisconnect();
	}

	void EN_RAU_UDP_Client::SetIpAndPort(std::string Ip, int port)
	{
		ServerPort = port + 1;
		ServerIpAddres = Ip;
	}

	// UDP client
	EN_RAU_UDP_Client::EN_RAU_UDP_Client(EN_RAU_Client* rau_Client)
	{
		RAU_Client = rau_Client;
	}

	void EN_RAU_UDP_Client::ServerMessageHandler(std::string message)
	{
		RAU_Client->ServerMessageHandler(message);
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
		UDP_Client->SetIpAndPort("127.0.0.1", port);
		ServerPort = port;
		return TCP_Client->Connect(port);
	}

	bool EN_RAU_Client::Connect(std::string ipAddr, int port)
	{
		UDP_Client->SetIpAndPort(ipAddr, port);
		ServerIpAddress = ipAddr;
		ServerPort = port;
		return TCP_Client->Connect(ipAddr, port);
	}

	void EN_RAU_Client::Run()
	{
		std::thread TCP_Thread([this]() { TCP_Client->Run(); });
		TCP_Thread.detach();
		UDP_Client->Run();

		Sleep(300);

		while (IsServerGetUDPAddress != true)
		{
			UDP_Client->SendToServer(std::to_string(ServerThreadID));
			Sleep(1000);
		}
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
		IsServerGetUDPAddress = true;
		TCP_Client->Disconnect();
		UDP_Client->Close();
	}

	EN_RAU_Client::~EN_RAU_Client()
	{
		delete TCP_Client;
		delete UDP_Client;
	}
}