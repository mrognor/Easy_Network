#include "EN_UDP_Server.h"

namespace EN
{
	void ThreadQueueHandler(EN_UDP_Server* server, std::queue<std::string>* Queue, std::queue<sockaddr_in>* QueueAddr, std::condition_variable* cv)
	{
		std::mutex mtx;
		std::unique_lock<std::mutex> unique_lock_mutex(mtx);
		while (true)
		{
			while (!Queue->empty())
			{
				server->Call(Queue->front(), QueueAddr->front());
				//std::cout << Queue->front() << std::endl;
				Queue->pop();
				QueueAddr->pop();
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
			exit(EXIT_FAILURE);
		}
	}

	void EN_UDP_Server::Run()
	{
		int slen, recv_len;
		char* buf = new char[MaxMessageSize];

		sockaddr_in si_other;

		slen = sizeof(si_other);

		std::queue<std::string>** QueueVec = new std::queue<std::string>*[ThreadAmount];
		std::queue<sockaddr_in>** QueueAddrVec = new std::queue<sockaddr_in>*[ThreadAmount];
		std::condition_variable** CondVarVec = new std::condition_variable * [ThreadAmount];
		std::thread* ThreadVec = new std::thread[ThreadAmount];


		for (int i = 0; i < ThreadAmount; i++)
		{
			sockaddr_in addr;
			//Prepare the sockaddr_in structure
			addr.sin_family = AF_INET;
			addr.sin_port = htons(Port);

			// Set ip address
			inet_pton(AF_INET, IpAddress.c_str(), &addr.sin_addr);

			QueueVec[i] = new std::queue<std::string>;
			CondVarVec[i] = new std::condition_variable;
			QueueAddrVec[i] = new std::queue<sockaddr_in>;;
			ThreadVec[i] = std::thread(ThreadQueueHandler, this, QueueVec[i], QueueAddrVec[i], CondVarVec[i]);
			ThreadVec[i].detach();
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
				exit(EXIT_FAILURE);
			}

			if (std::string(buf) != "")
			{
				int IndexMinQueue = 0;
				for (int i = 1; i < ThreadAmount; i++)
				{
					if (QueueVec[i]->size() < QueueVec[IndexMinQueue]->size())
						IndexMinQueue = i;
				}

				QueueVec[IndexMinQueue]->push(buf);
				QueueAddrVec[IndexMinQueue]->push(si_other);
				CondVarVec[IndexMinQueue]->notify_one();
			}
		}	

		for (int i = 0; i < ThreadAmount; i++)
		{
			delete QueueVec[i];
			delete QueueAddrVec[i];
			delete CondVarVec[i];
		}

		delete[] QueueVec;
		delete[] QueueAddrVec;
		delete[] CondVarVec;
		delete[] ThreadVec;

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
	}

	void EN_UDP_Server::SendToClient(std::string msg, sockaddr_in ClientSocketAddr)
	{
		//now reply the client with the same data
		if (sendto(s, msg.c_str(), MaxMessageSize, 0, (sockaddr*)&ClientSocketAddr, sizeof(ClientSocketAddr)) == SOCKET_ERROR)
		{
			printf("sendto() failed with error code : %d", WSAGetLastError());
			exit(EXIT_FAILURE);
		}
	}

	EN_UDP_Server::~EN_UDP_Server()
	{
		#ifdef WIN32
		WSACleanup();
		#endif
	}
}
