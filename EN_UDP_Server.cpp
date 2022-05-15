#include "EN_UDP_Server.h"


namespace EN
{
	void ThreadListHandler(EN_UDP_Server* server, std::list<std::string>* QueueMsg, std::list<sockaddr_in>* QueueAddr, std::list<EN_TimePoint>* QueueTime, std::condition_variable* cv, EN_UDP_ServerBuferType buffType)
	{
		std::mutex mtx;
		std::unique_lock<std::mutex> unique_lock_mutex(mtx);
		while (true)
		{
			while (!QueueMsg->empty())
			{
				if (QueueMsg->front() == "")
					return;

				std::chrono::duration<double> elapsed_seconds = std::chrono::system_clock::now() - QueueTime->front();
				auto f1 = QueueMsg->front();
				auto f2 = QueueAddr->front();

				QueueMsg->pop_front();
				QueueAddr->pop_front();
				QueueTime->pop_front();

				server->Call(f1, f2, elapsed_seconds.count());
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

		//Create a socket
		if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
		{
			printf("Could not create socket : %d", WSAGetLastError());
		}
		printf("Socket created.\n");

		//Prepare the sockaddr_in structure
		server.sin_family = AF_INET;
		server.sin_port = htons(Port);

		// Set ip address
		inet_pton(AF_INET, IpAddress.c_str(), &server.sin_addr);

		//Bind
		if (bind(s, (sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
		{
			printf("Bind failed with error code : %d", WSAGetLastError());
		}
	}

	void EN_UDP_Server::Run()
	{
		int slen, recv_len;
		char* buf = new char[MaxMessageSize];

		sockaddr_in si_other;

		slen = sizeof(si_other);

		QueueMessageVec = new std::list<std::string>*[ThreadAmount];
		QueueAddrVec = new std::list<sockaddr_in>*[ThreadAmount];
		QueueTimeVec = new std::list<EN_TimePoint>*[ThreadAmount];
		CondVarVec = new std::condition_variable*[ThreadAmount];
		std::thread* ThreadVec = new std::thread[ThreadAmount];

		for (int i = 0; i < ThreadAmount; i++)
		{
			QueueMessageVec[i] = new std::list<std::string>;
			QueueAddrVec[i] = new std::list<sockaddr_in>;
			QueueTimeVec[i] = new std::list<EN_TimePoint>;
			CondVarVec[i] = new std::condition_variable;

			ThreadVec[i] = std::thread(ThreadListHandler, this, QueueMessageVec[i], QueueAddrVec[i], QueueTimeVec[i], CondVarVec[i], ServerBuferType);
		}

		//keep listening for data
		while (true)
		{
			if (IsShutdown)
				break;
			
			//clear the buffer by filling null, it might have previously received data
			memset(buf, '\0', MaxMessageSize);

			//try to receive some data, this is a blocking call
			if ((recv_len = recvfrom(s, buf, MaxMessageSize, 0, (sockaddr*)&si_other, &slen)) == SOCKET_ERROR)
			{
				printf("recvfrom() failed with error code : %d", WSAGetLastError());
				// Get error if client disconnected but packages still goes down
			}

			if (std::string(buf) != "")
			{
				ImportantClientMessageHandler(buf, si_other, 0);

				int IndexMinQueue = 0;
				for (int i = 1; i < ThreadAmount; i++)
				{
					if (QueueMessageVec[i]->size() < QueueMessageVec[IndexMinQueue]->size())
						IndexMinQueue = i;
				}

				switch (ServerBuferType)
				{
				case Queue:
					QueueMessageVec[IndexMinQueue]->push_back(buf);
					QueueAddrVec[IndexMinQueue]->push_back(si_other);
					QueueTimeVec[IndexMinQueue]->push_back(std::chrono::system_clock::now());
					CondVarVec[IndexMinQueue]->notify_one();
					break;

				case Stack:
					QueueMessageVec[IndexMinQueue]->push_front(buf);
					QueueAddrVec[IndexMinQueue]->push_front(si_other);
					QueueTimeVec[IndexMinQueue]->push_front(std::chrono::system_clock::now());
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
		}

		delete[] QueueMessageVec;
		delete[] QueueAddrVec;
		delete[] CondVarVec;
		delete[] ThreadVec;
		delete[] QueueTimeVec;
		delete[] buf;
		#ifdef WIN32
		closesocket(s);
		#else
		close(s);
		#endif		
	}

	void EN_UDP_Server::Shutdown()
	{
		IsShutdown = true;

		if (sendto(s, "", MaxMessageSize, 0, (sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
		{
			printf("sendto() failed with error code : %d", WSAGetLastError());
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

	void EN_UDP_Server::SendToClient(std::string msg, sockaddr_in ClientSocketAddr)
	{
		//now reply the client with the same data
		if (sendto(s, msg.c_str(), MaxMessageSize, 0, (sockaddr*)&ClientSocketAddr, sizeof(ClientSocketAddr)) == SOCKET_ERROR)
		{
			printf("sendto() failed with error code : %d", WSAGetLastError());
		}
	}

	void EN_UDP_Server::FlushThread(int i)
	{
		//if (i == -1)
			//for (int j = 0; j < ThreadAmount; j++)
				//QueueMessageVec[j]->push_front("");

		//else
			//QueueMessageVec[i]->push_front("");
	}

	EN_UDP_Server::~EN_UDP_Server()
	{
		#ifdef WIN32
		WSACleanup();
		#endif
	}
}
