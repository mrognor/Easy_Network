#include "EN_RAU_Client.h"

namespace EN
{
	// TCP client
	EN_RAU_TCP_Client::EN_RAU_TCP_Client(EN_RAU_Client* rau_Client)
	{
		RAU_Client = rau_Client;
	}

	void EN_RAU_TCP_Client::OnConnect() {};

	void EN_RAU_TCP_Client::ServerMessageHandler(std::string message)
	{
		if (RAU_Client->ClientId != -1)
		{
			if (RAU_Client->IsServerGetUDPAddress == false)
			{
				RAU_Client->IsServerGetUDPAddress = true;
				RAU_Client->OnConnect();
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

	void EN_RAU_TCP_Client::OnDisconnect()
	{
		RAU_Client->OnDisconnect();
		RAU_Client->UDP_Client->Stop();

		Delay<std::chrono::milliseconds>(300);

		RAU_Client->IsServerGetUDPAddress = true;
		RAU_Client->IsShutdown = true;

		RAU_Client->CondVar.notify_all();

		Delay<std::chrono::milliseconds>(300);

		RAU_Client->IsServerGetUDPAddress = false;
		RAU_Client->IsShutdown = false;
		RAU_Client->ClientId = -1;
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

	// RAU client
	EN_RAU_Client::EN_RAU_Client()
	{
		TCP_Client = new EN_RAU_TCP_Client(this);
		UDP_Client = new EN_RAU_UDP_Client(this);
	}

	int EN_RAU_Client::GetServerPort() { return ServerPort; }

	std::string EN_RAU_Client::GetServerIpAddress() { return ServerIpAddress; }

	bool EN_RAU_Client::IsConnected()
	{
		return TCP_Client->IsConnected();
	}

	bool EN_RAU_Client::Connect()
	{
		return Connect("127.0.0.1", 1111);
	}

	bool EN_RAU_Client::Connect(int port)
	{
		return Connect("127.0.0.1", port);
	}

	bool EN_RAU_Client::Connect(std::string ipAddr, int port)
	{
		ServerIpAddress = ipAddr;
		ServerPort = port;

		UDP_Client->ServerIpAddress = ServerIpAddress;
		UDP_Client->ServerPort = ServerPort + 1;
		UDP_Client->Run();
		
		std::thread QueueHandlerThread([this]() {this->QueueMessageHandler(); });
		QueueHandlerThread.detach();

		if (!TCP_Client->Connect(ipAddr, port)) 
		{
            LOG(Warning, "Failed to connect to server");
			return false;
		}

		while (IsServerGetUDPAddress != true)
			UDP_Client->SendToServer(std::to_string(ClientId));

		return true;
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

	void EN_RAU_Client::SendToServer(std::string message, bool IsReliable)
	{
		if (IsReliable)
			TCP_Client->SendToServer(message);
		else
			UDP_Client->SendToServer(std::to_string(ClientId) + " " + message);
	}

	void EN_RAU_Client::Disconnect()
	{
		TCP_Client->Disconnect();
		UDP_Client->Stop();
	}

	void EN_RAU_Client::SetTCPSocketOption(int level, int optionName, int optionValue)
	{
		TCP_Client->SetSocketOption(level, optionName, optionValue);
	}

	void EN_RAU_Client::SetTCPSocketOption(PredefinedSocketOptions socketOptions)
	{
		TCP_Client->SetSocketOption(socketOptions);
	}

	void EN_RAU_Client::SetUDPSocketOption(int level, int optionName, int optionValue)
	{
		UDP_Client->SetSocketOption(level, optionName, optionValue);
	}

    void EN_RAU_Client::SetUDPSocketOption(PredefinedSocketOptions socketOptions)
	{
		UDP_Client->SetSocketOption(socketOptions);
	}

	EN_RAU_Client::~EN_RAU_Client()
	{
		delete TCP_Client;
		delete UDP_Client;
	}
}