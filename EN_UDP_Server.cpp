#include "EN_UDP_Server.h"


namespace EN
{
	void EN_UDP_Server::ThreadListHandler(int ThreadID)
	{
		while (true)
		{
			if (IsShutdown)
				return;

			while (!QueueMessageVec[ThreadID]->empty())
			{
				std::chrono::milliseconds elapsed_seconds = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - QueueTimeVec[ThreadID]->front());

				std::string TopMessage = QueueMessageVec[ThreadID]->front();
				std::string sourceAddress = QueueAddrVec[ThreadID]->front();

				switch (ServerBuferType)
				{
				case (Stack):

					Mutexes[ThreadID]->lock();
					QueueMessageVec[ThreadID]->pop_front();
					QueueAddrVec[ThreadID]->pop_front();
					QueueTimeVec[ThreadID]->pop_front();
					Mutexes[ThreadID]->unlock();

					ClientMessageHandler(TopMessage, sourceAddress, elapsed_seconds.count());
					break;

				case (Queue):
					ClientMessageHandler(TopMessage, sourceAddress, elapsed_seconds.count());

					Mutexes[ThreadID]->lock();
					QueueMessageVec[ThreadID]->pop_front();
					QueueAddrVec[ThreadID]->pop_front();
					QueueTimeVec[ThreadID]->pop_front();
					Mutexes[ThreadID]->unlock();
					break;
				}
			}

			GateVec[ThreadID]->Close();
		}
	}

	EN_UDP_Server::EN_UDP_Server()
	{
		#if defined WIN32 || defined _WIN64
		//WSAStartup
		WSAData wsaData;
        // Winsock operation int result
		int OperationRes;
		OperationRes = WSAStartup(MAKEWORD(2, 1), &wsaData);
		if (OperationRes != 0)
		{
			LOG(Error, "WSAStartup failed: " + OperationRes);
			exit(1);
		}
		#endif
	}

	void EN_UDP_Server::Run()
	{
		ShutdownMutex.lock();
		//Create a socket
		if ((UDP_ServerSocket = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
        {
            LOG(Error, "Could not create socket");
        }

		// Server address
		sockaddr_in ServerAddress;

		//Prepare the sockaddr_in structure
		ServerAddress.sin_family = AF_INET;
		ServerAddress.sin_port = htons(Port);

		// Set ip address
		inet_pton(AF_INET, IpAddress.c_str(), &ServerAddress.sin_addr);

		// Bind
		if (bind(UDP_ServerSocket, (sockaddr*)&ServerAddress, sizeof(ServerAddress)) == SOCKET_ERROR)
        {
            LOG(Error, "Bind failed");
        }

		QueueMessageVec = new std::list<std::string>*[ThreadAmount];
		QueueAddrVec = new std::list<std::string>*[ThreadAmount];
		QueueTimeVec = new std::list<std::chrono::system_clock::time_point>*[ThreadAmount];
		GateVec = new EN::EN_Gate*[ThreadAmount];
		ThreadVec = new std::thread[ThreadAmount];
		Mutexes = new std::mutex*[ThreadAmount];
		
		for (int i = 0; i < ThreadAmount; i++)
		{
			QueueMessageVec[i] = new std::list<std::string>;
			QueueAddrVec[i] = new std::list<std::string>;
			QueueTimeVec[i] = new std::list<std::chrono::system_clock::time_point>;
			GateVec[i] = new EN::EN_Gate;
			Mutexes[i] = new std::mutex;

			ThreadVec[i] = std::thread([this, i]() {this->ThreadListHandler(i); });
		}

		ShutdownMutex.unlock();

		std::string message, clientAddress;
		// Keep listening for data
		while (true)
		{					
			EN::UDP_Recv(UDP_ServerSocket, clientAddress, message);
			
			if (IsShutdown)
				break;

			if (InstantClientMessageHandler(message, clientAddress, 0) == false)
				continue;

			int IndexMinQueue = 0;
			for (int i = 1; i < ThreadAmount; i++)
				if (QueueMessageVec[i]->size() < QueueMessageVec[IndexMinQueue]->size())
					IndexMinQueue = i;
			
			switch (ServerBuferType)
			{
			case Queue:
				Mutexes[IndexMinQueue]->lock();
				QueueMessageVec[IndexMinQueue]->push_back(message);
				QueueAddrVec[IndexMinQueue]->push_back(clientAddress);
				QueueTimeVec[IndexMinQueue]->push_back(std::chrono::system_clock::now());
				Mutexes[IndexMinQueue]->unlock();

				GateVec[IndexMinQueue]->Open();
				break;

			case Stack:
				Mutexes[IndexMinQueue]->lock();
				QueueMessageVec[IndexMinQueue]->push_front(message);
				QueueAddrVec[IndexMinQueue]->push_front(clientAddress);
				QueueTimeVec[IndexMinQueue]->push_front(std::chrono::system_clock::now());
				
				if (QueueMessageVec[IndexMinQueue]->size() > MaxStackBuffSize)
				{
					QueueMessageVec[IndexMinQueue]->pop_back();
					QueueAddrVec[IndexMinQueue]->pop_back();
					QueueTimeVec[IndexMinQueue]->pop_back();
				}

				Mutexes[IndexMinQueue]->unlock();
				GateVec[IndexMinQueue]->Open();
				break;
			}
		}	

		for (int i = 0; i < ThreadAmount; i++)
			ThreadVec[i].join();

		for (int i = 0; i < ThreadAmount; i++)
		{
			delete QueueMessageVec[i];
			delete QueueAddrVec[i];
			delete GateVec[i];
			delete QueueTimeVec[i];
			delete Mutexes[i];
		}

		delete[] QueueMessageVec;
		delete[] QueueAddrVec;
		delete[] GateVec;
		delete[] ThreadVec;
		delete[] QueueTimeVec;
		delete[] Mutexes;

		CloseSocket(UDP_ServerSocket);	
	}

	void EN_UDP_Server::Shutdown()
	{
		IsShutdown = true;

		// Check what server successfully started
		while (true)
		{
			ShutdownMutex.lock();
			if (UDP_ServerSocket != INVALID_SOCKET)
			{
				ShutdownMutex.unlock();
				break;
			}
			ShutdownMutex.unlock();
		}

		CloseSocket(UDP_ServerSocket);	

		for (int i = 0; i < ThreadAmount; i++)
			GateVec[i]->Open();
	}

	void EN_UDP_Server::SendToClient(std::string ClientIpAddress, std::string message, int messageDelay)
	{
		EN::UDP_Send(UDP_ServerSocket, ClientIpAddress, message, messageDelay);
	}

	EN_UDP_Server::~EN_UDP_Server()
	{
		#if defined WIN32 || defined _WIN64
		WSACleanup();
		#endif
	}
}
