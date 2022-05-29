#include "EN_UDP_Server.h"


namespace EN
{
	void ThreadListHandler(EN_UDP_Server* ServerAddress, std::mutex* ThreadMutex, std::list<std::string>* QueueMsg, std::list<sockaddr_in>* QueueAddr, std::list<EN_TimePoint>* QueueTime, std::condition_variable* cv, EN_UDP_ServerBuferType buffType)
	{
		std::mutex mtx;
		std::unique_lock<std::mutex> unique_lock_mutex(mtx);
		while (true)
		{
			while (!QueueMsg->empty())
			{
				if (QueueMsg->front() == "")
					return;

				std::chrono::milliseconds elapsed_seconds = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - QueueTime->front());
				
				auto f1 = QueueMsg->front();
				auto f2 = QueueAddr->front();
				
				char str[INET_ADDRSTRLEN];
				inet_ntop(AF_INET, &(f2.sin_addr), str, INET_ADDRSTRLEN);
				std::string ClientAddress = str;
				ClientAddress += ":";
				ClientAddress += std::to_string(ntohs(f2.sin_port));
				
				switch (buffType)
				{
				case (Stack):

					ThreadMutex->lock();
					QueueMsg->pop_front();
					QueueAddr->pop_front();
					QueueTime->pop_front();
					ThreadMutex->unlock();

					ServerAddress->Call(f1, ClientAddress, elapsed_seconds.count());
					break;

				case (Queue):
					ServerAddress->Call(f1, ClientAddress, elapsed_seconds.count());

					ThreadMutex->lock();
					QueueMsg->pop_front();
					QueueAddr->pop_front();
					QueueTime->pop_front();
					ThreadMutex->unlock();
					break;
				}
			}
			cv->wait(unique_lock_mutex);
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

		int slen, recv_len;
		char* buf = new char[MaxMessageSize];

		sockaddr_in si_other;

		slen = sizeof(si_other);

		QueueMessageVec = new std::list<std::string>*[ThreadAmount];
		std::list<sockaddr_in>**  QueueAddrVec = new std::list<sockaddr_in>*[ThreadAmount];
		std::list<EN_TimePoint>** QueueTimeVec = new std::list<EN_TimePoint>*[ThreadAmount];
		std::condition_variable** CondVarVec = new std::condition_variable*[ThreadAmount];
		std::thread* ThreadVec = new std::thread[ThreadAmount];
		std::mutex** Mutexes = new std::mutex*[ThreadAmount];
		
		for (int i = 0; i < ThreadAmount; i++)
		{
			QueueMessageVec[i] = new std::list<std::string>;
			QueueAddrVec[i] = new std::list<sockaddr_in>;
			QueueTimeVec[i] = new std::list<EN_TimePoint>;
			CondVarVec[i] = new std::condition_variable;
			Mutexes[i] = new std::mutex;

			ThreadVec[i] = std::thread(ThreadListHandler, this, Mutexes[i], QueueMessageVec[i], QueueAddrVec[i], QueueTimeVec[i], CondVarVec[i], ServerBuferType);
		}

		//keep listening for data
		while (true)
		{
			if (IsShutdown)
				break;
			
			//clear the buffer by filling null, it might have previously received data
			memset(buf, '\0', MaxMessageSize);

			#ifdef WIN32
			//try to receive some data, this is a blocking call
			recv_len = recvfrom(ServerSocket, buf, MaxMessageSize, 0, (sockaddr*)&si_other, &slen);
			#else
			//try to receive some data, this is a blocking call
			recv_len = recvfrom(ServerSocket, buf, MaxMessageSize, 0, (sockaddr*)&si_other, (socklen_t*)&slen);
			#endif

			if (std::string(buf) != "")
			{
				char str[INET_ADDRSTRLEN];
				inet_ntop(AF_INET, &(si_other.sin_addr), str, INET_ADDRSTRLEN);
				std::string ClientAddress = str; 
				ClientAddress += ":"; 
				ClientAddress += ntohs(si_other.sin_port);

				if (ImportantClientMessageHandler(buf, ClientAddress, 0) == false)
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
					QueueMessageVec[IndexMinQueue]->push_back(buf);
					QueueAddrVec[IndexMinQueue]->push_back(si_other);
					QueueTimeVec[IndexMinQueue]->push_back(std::chrono::system_clock::now());
					Mutexes[IndexMinQueue]->unlock();

					CondVarVec[IndexMinQueue]->notify_one();
					break;

				case Stack:
					Mutexes[IndexMinQueue]->lock();
					QueueMessageVec[IndexMinQueue]->push_front(buf);
					QueueAddrVec[IndexMinQueue]->push_front(si_other);
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

		delete[] buf;
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
			for(int i = 0; i < ThreadAmount; i++)
				QueueMessageVec[i]->push_back("");
			break;

		case Stack:
			for (int i = 0; i < ThreadAmount; i++)
				QueueMessageVec[i]->push_front("");
			break;
		}
	}

	void EN_UDP_Server::SendToClient(std::string msg, std::string ClientSocketAddr)
	{
		//Prepare the sockaddr_in structure
		sockaddr_in client;
		client.sin_family = AF_INET;
		client.sin_port = htons(std::atoi((ClientSocketAddr.substr(ClientSocketAddr.find(":")).c_str())));

		// Set ip address
		inet_pton(AF_INET, (ClientSocketAddr.substr(0, ClientSocketAddr.find(":"))).c_str(), &client.sin_addr);

		//now reply the client with the same data
		if (sendto(ServerSocket, msg.c_str(), MaxMessageSize, 0, (sockaddr*)&client, sizeof(ClientSocketAddr)) == SOCKET_ERROR)
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
