#include "EN_TCP_Client.h"

namespace EN
{
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
		SocketMtx.lock();
		EN_SOCKET tmpSock = ServerConnectionSocket;
		SocketMtx.unlock();
		return tmpSock; 
	}

	bool EN_TCP_Client::IsConnected()
	{
		SocketMtx.lock();
		bool isConnected = (ServerConnectionSocket != INVALID_SOCKET);
		SocketMtx.unlock();
		return isConnected;
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
		
		SocketMtx.lock();
		
		if (ServerConnectionSocket != INVALID_SOCKET)
		{
			SocketMtx.unlock();
			LOG(LogLevels::Warning, "You are trying to connect but you are alredy connected");
			return false;
		}

		ServerConnectionSocket = socket(AF_INET, SOCK_STREAM, 0);

		if (ServerConnectionSocket == INVALID_SOCKET)
        {
			SocketMtx.unlock();
            LOG(LogLevels::Error, "Error at socket: " + std::to_string(GetSocketErrorCode()) + " " + EN::GetSocketErrorString());
			return false;
		}

		// Set ip address
		inet_pton(AF_INET, ServerIpAddress.c_str(), &addr.sin_addr);

        int operationRes = connect(ServerConnectionSocket, (sockaddr*)&addr, sizeof(addr));
        
        if (operationRes != 0)
		{
			ServerConnectionSocket = INVALID_SOCKET;
			SocketMtx.unlock();
            LOG(LogLevels::Warning, "Warning: failed connect to server");
            return false;
        }		

		SocketMtx.unlock();

		// If reconnection we have to join last std::thread
		WaitForServerHandlerEnd();

		for (SocketOption& opt : SocketOptions)
			EN::SetSocketOption(ServerConnectionSocket, opt.Level, opt.OptionName, opt.OptionValue);

		if (IsRunMessageHadlerThread)
			ServerHandlerThread = std::thread([this]() { this->ServerHandler(); });

		return true;
	}

	void EN_TCP_Client::ServerHandler()
	{
		OnConnect();

		bool isServerConnected = true;
		std::string message;

		while (true)
		{
			isServerConnected = TCP_Recv(ServerConnectionSocket, message);

			// Means what server was disconnected
			if (isServerConnected == false)
			{
				OnDisconnect();

				SocketMtx.lock();
				CloseSocket(ServerConnectionSocket);
				ServerConnectionSocket = INVALID_SOCKET;
				SocketMtx.unlock();

				return;
			}

			ServerMessageHandler(message);
		}
	}

	void EN_TCP_Client::WaitForServerHandlerEnd()
	{
		ServerHandlerMtx.lock();
		if (ServerHandlerThread.joinable() && IsRunMessageHadlerThread)
			ServerHandlerThread.join();
		ServerHandlerMtx.unlock();
	}

	bool EN_TCP_Client::SendToServer(std::string message)
	{
		SocketMtx.lock();
		bool sendRes = TCP_Send(ServerConnectionSocket, message);
		SocketMtx.unlock();
		return sendRes;
	}

    bool EN_TCP_Client::WaitMessage(std::string& message)
    {
		// Thread safety because this method should be called only inside handler thread or handler thread even not started
        return TCP_Recv(ServerConnectionSocket, message);
    }

	void EN_TCP_Client::Disconnect(bool isBlocking)
	{
		SocketMtx.lock();
		CloseSocket(ServerConnectionSocket);
		SocketMtx.unlock();
		
		ServerHandlerMtx.lock();

		if (ServerHandlerThread.get_id() != std::this_thread::get_id() && ServerHandlerThread.joinable() && isBlocking)
			ServerHandlerThread.join();

		ServerConnectionSocket = INVALID_SOCKET;

		ServerHandlerMtx.unlock();
	}

    void EN_TCP_Client::SetSocketOption(int level, int optionName, int optionValue)
    {
		SocketOptions.push_back(SocketOption(level, optionName, optionValue));
    }

    void EN_TCP_Client::SetSocketOption(PredefinedSocketOptions socketOptions)
    {        
        for (size_t i = 0; i < socketOptions.Levels.size(); ++i)
			SocketOptions.push_back(SocketOption(socketOptions.Levels[i], socketOptions.OptionNames[i], socketOptions.OptionValues[i]));
    }

	void EN_TCP_Client::SetTCPSendFunction(bool (*TCPSendFunction)(EN_SOCKET, const std::string&))
	{
		TCP_Send = TCPSendFunction;
	}

	void EN_TCP_Client::SetTCPRecvFunction(bool (*TCPRecvFunction)(EN_SOCKET, std::string&))
	{
		TCP_Recv = TCPRecvFunction;
	}

	EN_TCP_Client::~EN_TCP_Client()
	{	
        SocketMtx.lock();
        if (ServerConnectionSocket != INVALID_SOCKET)
        {
            LOG(LogLevels::Error, "Error: You forgot to disconnect from the server. Use method Disconnect() to do this");
        }

		CloseSocket(ServerConnectionSocket);
		SocketMtx.unlock();

		WaitForServerHandlerEnd();
	}
}
