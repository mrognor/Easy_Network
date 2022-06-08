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
		if (RAU_Client->ClientId != -1)
		{
			if (RAU_Client->IsServerGetUDPAddress == false)
			{
				RAU_Client->IsServerGetUDPAddress = true;
				RAU_Client->AfterConnect();
			}
			else
			{
				RAU_Client->Mtx.lock();
				RAU_Client->Messages.push(message);
				RAU_Client->Mtx.unlock();
				RAU_Client->CondVar.notify_all();
			}
		}
		else 
			RAU_Client->ClientId = std::atoi(message.c_str());
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
		RAU_Client->Mtx.lock();
		RAU_Client->Messages.push(message);
		RAU_Client->Mtx.unlock();
		RAU_Client->CondVar.notify_all();
	}
	
	void EN_RAU_Client::QueueMessageHandler()
	{
		std::mutex mtx;
		std::unique_lock<std::mutex> unique_lock_mutex(mtx);

		while (true)
		{
			while (!Messages.empty())
			{
				ServerMessageHandler(Messages.front());
				Mtx.lock();
				Messages.pop();
				Mtx.unlock();
			}
			
			if (IsShutdown)
				break;

			CondVar.wait(unique_lock_mutex);

			if (IsShutdown)
				break;
		}
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
		TCP_Client->Run(); 
		UDP_Client->Run();

		Delay(300);

		while (IsServerGetUDPAddress != true)
		{
			UDP_Client->SendToServer(std::to_string(ClientId));
			
			Delay(1000);
		}

		std::thread QueueHandlerThread([this]() {this->QueueMessageHandler(); });
		QueueHandlerThread.detach();
	}

	void EN_RAU_Client::SendToServer(std::string message, bool IsReliable, int MessageDelay)
	{
		if (IsReliable)
			TCP_Client->SendToServer(message, MessageDelay);
		else
			UDP_Client->SendToServer(std::to_string(ClientId) + " " + message, MessageDelay);
	}

	void EN_RAU_Client::Disconnect()
	{
		TCP_Client->Disconnect();
		UDP_Client->Close();
		
		Delay(300);
		
		IsServerGetUDPAddress = true;
		IsShutdown = true;

		CondVar.notify_all();
	}

	EN_RAU_Client::~EN_RAU_Client()
	{
		delete TCP_Client;
		delete UDP_Client;
	}
}