#include "EN_TCP_Client.h"

namespace EN
{
	EN_TCP_Client::EN_TCP_Client()
	{
		ServerConnectionSocket = socket(AF_INET, SOCK_STREAM, 0);

		if (ServerConnectionSocket == INVALID_SOCKET)
        {
            LOG(LogLevels::Error, "Error at socket: " + std::to_string(GetSocketErrorCode()) + " " + EN::GetSocketErrorString());
        }
	}

	int EN_TCP_Client::GetServerPort() 
	{ 
		return ServerPort; 
	}

	std::string EN_TCP_Client::GetServerIpAddress()
	{ 
		return ServerIpAddress; 
	}

	EN_SOCKET EN_TCP_Client::GetSocket() 
	{ 
		return ServerConnectionSocket; 
	}

	bool EN_TCP_Client::IsConnected()
	{
		if (ServerConnectionSocket != INVALID_SOCKET)
			return true;
		else return false;
	}

	bool EN_TCP_Client::Connect()
	{
		return Connect(ServerIpAddress, ServerPort);
	}

	bool EN_TCP_Client::Connect(int port)
	{
		return Connect(ServerIpAddress, port);
	}

	bool EN_TCP_Client::Connect(std::string ipAddr, int port)
	{
		ServerIpAddress = GetIpByURL(ipAddr);
		if (ServerIpAddress == "")
			ServerIpAddress = ipAddr;

		ServerPort = port;

		sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		
		// Set ip address
		inet_pton(AF_INET, ServerIpAddress.c_str(), &addr.sin_addr);

		// Winsock operation int result
		int OperationRes;
        OperationRes = connect(ServerConnectionSocket, (sockaddr*)&addr, sizeof(addr));
            
        if (OperationRes == 0)
			OnConnect();
        else
        {
			ServerConnectionSocket = INVALID_SOCKET;
            LOG(Warning, "Error: failed connect to server");
            return false;
        }

		ServerHandlerMtx.lock();
		if (ServerHandlerThread.joinable())
			ServerHandlerThread.join();
		ServerHandlerMtx.unlock();

		if (IsRunMessageHadlerThread)
			ServerHandlerThread = std::thread([this]() { this->ServerHandler(); });
			
		return true;
	}

	void EN_TCP_Client::ServerHandler()
	{
		bool IsServerConnected = true;
		std::string message;

		while (true)
		{
			IsServerConnected = TCP_Recv(ServerConnectionSocket, message);

			// Means what server was disconnected
			if (IsServerConnected == false)
			{
				CloseSocket(ServerConnectionSocket);
				OnDisconnect();
				ServerConnectionSocket = INVALID_SOCKET;
				return;
			}

			ServerMessageHandler(message);
		}
	}

	bool EN_TCP_Client::SendToServer(std::string message)
	{
		return TCP_Send(ServerConnectionSocket, message);
	}

    bool EN_TCP_Client::WaitMessage(std::string& message)
    {
        return EN::TCP_Recv(ServerConnectionSocket, message);
    }

	void EN_TCP_Client::Disconnect()
	{
		CloseSocket(ServerConnectionSocket);
		if (ServerHandlerThread.get_id() != std::this_thread::get_id())
		{
			ServerHandlerMtx.lock();
			if (ServerHandlerThread.joinable())
				ServerHandlerThread.join();
			ServerHandlerMtx.unlock();
		}
	}

    void EN_TCP_Client::SetSocketOption(int level, int optionName, int optionValue)
    {
		EN::SetSocketOption(ServerConnectionSocket, level, optionName, optionValue);
    }

    void EN_TCP_Client::SetSocketOption(PredefinedSocketOptions socketOptions)
    {        
        for (size_t i = 0; i < socketOptions.Levels.size(); ++i)
			EN::SetSocketOption(ServerConnectionSocket, socketOptions.Levels[i], socketOptions.OptionNames[i], socketOptions.OptionValues[i]);
    }

	EN_TCP_Client::~EN_TCP_Client()
	{	
		if (ServerConnectionSocket != INVALID_SOCKET)
            LOG(Error, "Error: You forgot to disconnect from the server. Use method Disconnect() to do this");

		ServerHandlerMtx.lock();
		if (ServerHandlerThread.joinable())
			ServerHandlerThread.join();
		ServerHandlerMtx.unlock();
	}
}
