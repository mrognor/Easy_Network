#include "EN_UDP_Server.h"


namespace EN
{
	void EN_UDP_Server::ThreadListHandler(int ThreadID)
	{
		std::mutex mtx;
		std::unique_lock<std::mutex> unique_lock_mutex(mtx);
		while (true)
		{
			while (!QueueMessageVec[ThreadID]->empty())
			{
				if (QueueMessageVec[ThreadID]->front() == "")
					return;

				std::chrono::milliseconds elapsed_seconds = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - QueueTimeVec[ThreadID]->front());

				auto f1 = QueueMessageVec[ThreadID]->front();
				auto f2 = QueueAddrVec[ThreadID]->front();

				switch (ServerBuferType)
				{
				case (Stack):

					Mutexes[ThreadID]->lock();
					QueueMessageVec[ThreadID]->pop_front();
					QueueAddrVec[ThreadID]->pop_front();
					QueueTimeVec[ThreadID]->pop_front();
					Mutexes[ThreadID]->unlock();

					ClientMessageHandler(f1, f2, elapsed_seconds.count());
					break;

				case (Queue):
					ClientMessageHandler(f1, f2, elapsed_seconds.count());

					Mutexes[ThreadID]->lock();
					QueueMessageVec[ThreadID]->pop_front();
					QueueAddrVec[ThreadID]->pop_front();
					QueueTimeVec[ThreadID]->pop_front();
					Mutexes[ThreadID]->unlock();
					break;
				}
			}
			CondVarVec[ThreadID]->wait(unique_lock_mutex);
		}
	}

	EN_UDP_Server::EN_UDP_Server()
	{
		#ifdef WIN32
		//WSAStartup
		WSAData wsaData;
		if (WSAStartup(MAKEWORD(2, 1), &wsaData) != 0)
		{
			std::cerr << "Error: Library initialization failure." << std::endl;
			exit(1);
		}
		#endif
	}

	void EN_UDP_Server::Run()
	{
		//Create a socket
		if ((ServerSocket = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
		{
			std::cerr << "Could not create socket" << std::endl;
		}

		//Prepare the sockaddr_in structure
		ServerAddress.sin_family = AF_INET;
		ServerAddress.sin_port = htons(Port);

		// Set ip address
		inet_pton(AF_INET, IpAddress.c_str(), &ServerAddress.sin_addr);

		//Bind
		if (bind(ServerSocket, (sockaddr*)&ServerAddress, sizeof(ServerAddress)) == SOCKET_ERROR)
		{
			std::cerr << "Bind failed" << std::endl;
		}

		char* IncomingMessageBuffer = new char[MaxMessageSize];

		sockaddr_in ClientAddress;

		int sizeofaddr = sizeof(ClientAddress);

		QueueMessageVec = new std::list<std::string>*[ThreadAmount];
		QueueAddrVec = new std::list<sockaddr_in>*[ThreadAmount];
		QueueTimeVec = new std::list<EN_TimePoint>*[ThreadAmount];
		CondVarVec = new std::condition_variable*[ThreadAmount];
		ThreadVec = new std::thread[ThreadAmount];
		Mutexes = new std::mutex*[ThreadAmount];
		
		for (int i = 0; i < ThreadAmount; i++)
		{
			QueueMessageVec[i] = new std::list<std::string>;
			QueueAddrVec[i] = new std::list<sockaddr_in>;
			QueueTimeVec[i] = new std::list<EN_TimePoint>;
			CondVarVec[i] = new std::condition_variable;
			Mutexes[i] = new std::mutex;

			ThreadVec[i] = std::thread([this, i]() {this->ThreadListHandler(i); });
			ThreadVec[i].detach();
		}

		//keep listening for data
		while (true)
		{
			if (IsShutdown)
				break;
			
			//clear the buffer by filling null, it might have previously received data
			memset(IncomingMessageBuffer, '\0', MaxMessageSize);

			#ifdef WIN32
			//try to receive some data, this is a blocking call
			recvfrom(ServerSocket, IncomingMessageBuffer, MaxMessageSize, 0, (sockaddr*)&ClientAddress, &sizeofaddr);
			#else
			//try to receive some data, this is a blocking call
			recvfrom(ServerSocket, IncomingMessageBuffer, MaxMessageSize, 0, (sockaddr*)&ClientAddress, (socklen_t*)&sizeofaddr);
			#endif

			if (std::string(IncomingMessageBuffer) != "")
			{
				if (InstantClientMessageHandler(IncomingMessageBuffer, ClientAddress, 0) == false)
					continue;

				int IndexMinQueue = 0;
				for (int i = 1; i < ThreadAmount; i++)
				{
					if (QueueMessageVec[i]->size() < QueueMessageVec[IndexMinQueue]->size())
						IndexMinQueue = i;
				}

				switch (ServerBuferType)
				{
				case Queue:
					Mutexes[IndexMinQueue]->lock();
					QueueMessageVec[IndexMinQueue]->push_back(IncomingMessageBuffer);
					QueueAddrVec[IndexMinQueue]->push_back(ClientAddress);
					QueueTimeVec[IndexMinQueue]->push_back(std::chrono::system_clock::now());
					Mutexes[IndexMinQueue]->unlock();

					CondVarVec[IndexMinQueue]->notify_one();
					break;

				case Stack:
					Mutexes[IndexMinQueue]->lock();
					QueueMessageVec[IndexMinQueue]->push_front(IncomingMessageBuffer);
					QueueAddrVec[IndexMinQueue]->push_front(ClientAddress);
					QueueTimeVec[IndexMinQueue]->push_front(std::chrono::system_clock::now());
					
					if (QueueMessageVec[IndexMinQueue]->size() > MaxStackBuffSize)
					{
						QueueMessageVec[IndexMinQueue]->pop_back();
						QueueAddrVec[IndexMinQueue]->pop_back();
						QueueTimeVec[IndexMinQueue]->pop_back();
					}

					Mutexes[IndexMinQueue]->unlock();
					CondVarVec[IndexMinQueue]->notify_one();
					break;
				}
				
			}
		}	

		for (int i = 0; i < ThreadAmount; i++)
		{
			ThreadVec[i].join();
		}

		for (int i = 0; i < ThreadAmount; i++)
		{
			delete QueueMessageVec[i];
			delete QueueAddrVec[i];
			delete CondVarVec[i];
			delete QueueTimeVec[i];
			delete Mutexes[i];
		}

		delete[] QueueMessageVec;
		delete[] QueueAddrVec;
		delete[] CondVarVec;
		delete[] ThreadVec;
		delete[] QueueTimeVec;
		delete[] Mutexes;

		delete[] IncomingMessageBuffer;
		#ifdef WIN32
		closesocket(ServerSocket);
		#else
		close(ServerSocket);
		#endif		
	}

	void EN_UDP_Server::Shutdown()
	{
		IsShutdown = true;

		if (sendto(ServerSocket, "", MaxMessageSize, 0, (sockaddr*)&ServerAddress, sizeof(ServerAddress)) == SOCKET_ERROR)
		{
			std::cerr << "sendto failed" << std::endl;
		}

		switch (ServerBuferType)
		{
		case Queue:
			for (int i = 0; i < ThreadAmount; i++)
			{
				Mutexes[i]->lock();
				QueueMessageVec[i]->push_back("");
				Mutexes[i]->unlock();
			}
			break;

		case Stack:
			for (int i = 0; i < ThreadAmount; i++)
			{
				Mutexes[i]->lock();
				QueueMessageVec[i]->push_front("");
				Mutexes[i]->unlock();
			}
			break;
		}
	}

	void EN_UDP_Server::SendToClient(std::string message, UDP_Address ClientSocketAddr)
	{
		//now reply the client with the same data
		if (sendto(ServerSocket, message.c_str(), MaxMessageSize, 0, (sockaddr*)&ClientSocketAddr, sizeof(ClientSocketAddr)) == SOCKET_ERROR)
		{
			std::cerr << "sendto failed" << std::endl;
		}
	}

	EN_UDP_Server::~EN_UDP_Server()
	{
		#ifdef WIN32
		WSACleanup();
		#endif
	}
}
