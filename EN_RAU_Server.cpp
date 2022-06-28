#include "EN_RAU_Server.h"

namespace EN
{
	// TCP server
	EN_RAU_TCP_Server::EN_RAU_TCP_Server(EN_RAU_Server* rau_Server)
	{
		RAU_Server = rau_Server;
	}

	void EN_RAU_TCP_Server::OnClientConnected(int ClientID)
	{
		SendToClient(ClientID, std::to_string(ClientID));

		auto f1 = new std::queue<std::string>;
		auto f2 = new std::condition_variable;

		if (ClientID >= RAU_Server->UDPIpAddresses.size())
		{
			RAU_Server->VectorQueuesMessages.push_back(f1);

			RAU_Server->UDPIpAddresses.push_back("none");

			RAU_Server->VectorCondVars.push_back(f2);

			RAU_Server->KillThreads.push_back(false);
		}
		else
		{
			RAU_Server->VectorQueuesMessages[ClientID] = f1;

			RAU_Server->UDPIpAddresses[ClientID] = "none";

			RAU_Server->VectorCondVars[ClientID] = f2;
			
			RAU_Server->KillThreads[ClientID] = false;
		}

		std::thread QueueThreadHandler([this, ClientID]() {this->RAU_Server->ThreadQueueHandler(ClientID); });
		QueueThreadHandler.detach();
	}

	void EN_RAU_TCP_Server::ClientMessageHandler(std::string message, int ClientID)
	{
		RAU_Server->VectorQueuesMessages[ClientID]->push(message);
		RAU_Server->VectorCondVars[ClientID]->notify_all();
	}

	void EN_RAU_TCP_Server::OnClientDisconnect(int ClientID)
	{
		RAU_Server->OnClientDisconnect(ClientID);
		RAU_Server->KillThreads[ClientID] = true;
		RAU_Server->VectorCondVars[ClientID]->notify_all();

		Delay(300);

		delete RAU_Server->VectorQueuesMessages[ClientID];
		delete RAU_Server->VectorCondVars[ClientID];


		if (ClientID == RAU_Server->UDPIpAddresses.size() - 1)
		{
			RAU_Server->KillThreads.pop_back();

			RAU_Server->VectorQueuesMessages.pop_back();

			RAU_Server->UDPIpAddresses.pop_back();

			RAU_Server->VectorCondVars.pop_back();
		}
		else
		{
			RAU_Server->KillThreads[ClientID] = false;

			RAU_Server->VectorQueuesMessages[ClientID] = nullptr;

			RAU_Server->UDPIpAddresses[ClientID] = "none";

			RAU_Server->VectorCondVars[ClientID] = nullptr;
		}
	}

	// UDP server
	EN_RAU_UDP_Server::EN_RAU_UDP_Server(EN_RAU_Server* rau_Server)
	{
		RAU_Server = rau_Server;
	}

	bool EN_RAU_UDP_Server::InstantClientMessageHandler(std::string message, std::string ClientSocketAddr, long long TimeWhenPackageArrived)
	{
		int ThreadID = std::atoi(message.substr(0, message.find(" ")).c_str());

		if (RAU_Server->UDPIpAddresses[ThreadID] == "none")
		{
			RAU_Server->UDPIpAddresses[ThreadID] = ClientSocketAddr;
			RAU_Server->TCP_Server->SendToClient(ThreadID, " ");
			RAU_Server->OnClientConnected(ThreadID);
		}
		else
		{
			RAU_Server->VectorQueuesMessages[ThreadID]->push(message.substr(message.find(" ") + 1));
			RAU_Server->VectorCondVars[ThreadID]->notify_all();
		}
		return false;
	}

	// RAU server
	EN_RAU_Server::EN_RAU_Server()
	{
		TCP_Server = new EN_RAU_TCP_Server(this);
		UDP_Server = new EN_RAU_UDP_Server(this);
	}

	void EN_RAU_Server::ThreadQueueHandler(int ClientID)
	{
		std::mutex mtx;
		std::unique_lock<std::mutex> unique_lock_mutex(mtx);

		while (true)
		{
			while (!VectorQueuesMessages[ClientID]->empty())
			{
				ClientMessageHandler(VectorQueuesMessages[ClientID]->front(), ClientID);
				VectorQueuesMessages[ClientID]->pop();
			}

			if (IsShutdown || KillThreads[ClientID] == true)
				break;

			VectorCondVars[ClientID]->wait(unique_lock_mutex);

			if (IsShutdown || KillThreads[ClientID] == true)
				break;
		}
	}

	void EN_RAU_Server::Run()
	{
		// Setup ip and port on tcp server
		TCP_Server->IpAddress = IpAddress;
		TCP_Server->Port = Port;
		
		// Setup ip and port on udp server
		UDP_Server->IpAddress = IpAddress;
		UDP_Server->Port = Port + 1;
		UDP_Server->MaxMessageSize = MaxUnreliableMessageSize + 10;
		UDP_Server->ThreadAmount = 1;
		UDP_Server->ServerBuferType = EN::Queue;

		std::thread UDP_Thread([this]() {this->UDP_Server->Run(); });
		UDP_Thread.detach();
		TCP_Server->Run();
	}

	void EN_RAU_Server::DisconnectClient(int ClientID)
	{
		TCP_Server->DisconnectClient(ClientID);
	}

	void EN_RAU_Server::Shutdown()
	{
		TCP_Server->Shutdown();
		UDP_Server->Shutdown();
		IsShutdown = true;
		for (int i = 0; i < VectorCondVars.size(); i++)
		{
			VectorCondVars[i]->notify_all();
		}
	}

	void EN_RAU_Server::SendToClient(int ClientId, std::string message, bool IsReliable)
	{
		if (UDPIpAddresses[ClientId] != "none")
		{
			if (IsReliable)
				TCP_Server->SendToClient(ClientId, message);
			else UDP_Server->SendToClient(UDPIpAddresses[ClientId], message);
		}
	}

	EN_RAU_Server::~EN_RAU_Server()
	{
		for (int i = 0; i < VectorQueuesMessages.size(); i++)
		{
			delete VectorQueuesMessages[i];
			delete VectorCondVars[i];
		}

		delete TCP_Server;
		delete UDP_Server;
	}
}