#include "EN_UDP_Server.h"


namespace EN
{
	void ThreadListHandler(EN_UDP_Server* server, std::list<std::string>* QueueMsg, std::list<sockaddr_in>* QueueAddr, std::list<EN_TimePoint>* QueueTime, std::condition_variable* cv, EN_UDP_ServerBuferType buffType)
	{
		std::mutex mtx, mtx1;
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

				mtx1.lock();
				QueueMsg->pop_front();
				QueueAddr->pop_front();
				QueueTime->pop_front();
				mtx1.unlock();

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
			std::cerr << "Could not create socket" << std::endl;
		}

		//Prepare the sockaddr_in structure
		server.sin_family = AF_INET;
		server.sin_port = htons(Port);

		// Set ip address
		inet_pton(AF_INET, IpAddress.c_str(), &server.sin_addr);

		//Bind
		if (bind(s, (sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
		{
			std::cerr << "Bind failed" << std::endl;
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
		std::mutex mtx;
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

			#ifdef WIN32
			//try to receive some data, this is a blocking call
			recv_len = recvfrom(s, buf, MaxMessageSize, 0, (sockaddr*)&si_other, &slen);
			#else
			//try to receive some data, this is a blocking call
			recv_len = recvfrom(s, buf, MaxMessageSize, 0, (sockaddr*)&si_other, (socklen_t*)&slen);
			#endif

			if (std::string(buf) != "")
			{
				if (ImportantClientMessageHandler(buf, si_other, 0) == false)
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
					mtx.lock();
					QueueMessageVec[IndexMinQueue]->push_back(buf);
					QueueAddrVec[IndexMinQueue]->push_back(si_other);
					QueueTimeVec[IndexMinQueue]->push_back(std::chrono::system_clock::now());
					CondVarVec[IndexMinQueue]->notify_one();
					mtx.unlock();
					break;

				case Stack:
					mtx.lock();
					QueueMessageVec[IndexMinQueue]->push_front(buf);
					QueueAddrVec[IndexMinQueue]->push_front(si_other);
					QueueTimeVec[IndexMinQueue]->push_front(std::chrono::system_clock::now());
					CondVarVec[IndexMinQueue]->notify_one();
					mtx.unlock();
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

	void EN_UDP_Server::SendToClient(std::string msg, sockaddr_in ClientSocketAddr)
	{
		//now reply the client with the same data
		if (sendto(s, msg.c_str(), MaxMessageSize, 0, (sockaddr*)&ClientSocketAddr, sizeof(ClientSocketAddr)) == SOCKET_ERROR)
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
