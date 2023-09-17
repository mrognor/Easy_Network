#include  "../EN_TCP_Client.h"

class EN_FT_Client;

class EN_FT_Client_Internal_FileTransmitter : public EN::EN_TCP_Client
{
private:
	EN_FT_Client* FT_Client;
protected:
	virtual void OnConnect() override { std::cout << "FT conn" << std::endl; }

	virtual void ServerMessageHandler(std::string message) override {}

	virtual void OnDisconnect() override { std::cout << "FT disconn" << std::endl; }
public:
	EN_FT_Client_Internal_FileTransmitter(EN_FT_Client* fT_Client);
};

class EN_FT_Client_Internal_MessageTransmitter : public EN::EN_TCP_Client
{
private:
	EN_FT_Client* FT_Client;
protected:
	virtual void OnConnect() override { std::cout << "MSG conn" << std::endl; }

	virtual void ServerMessageHandler(std::string message) override {}

	virtual void OnDisconnect() override { std::cout << "MSG disconn" << std::endl; }
public:
	EN_FT_Client_Internal_MessageTransmitter(EN_FT_Client* fT_Client);
};

class EN_FT_Client
{
private:
	friend EN_FT_Client_Internal_MessageTransmitter;
	friend EN_FT_Client_Internal_FileTransmitter;

	EN_FT_Client_Internal_MessageTransmitter MessageTransmitter;
	EN_FT_Client_Internal_FileTransmitter FileTransmitter;
public:
	EN_FT_Client() : MessageTransmitter(this), FileTransmitter(this) {}

	bool Connect(std::string serverIpAddress = "127.0.0.1", int messageTransmitterPort = 1111, int fileTransmitterPort = 1112)
	{
		if (!MessageTransmitter.Connect(serverIpAddress, messageTransmitterPort))
			return false;

		if (!FileTransmitter.Connect(serverIpAddress, fileTransmitterPort))
		{
			MessageTransmitter.Disconnect();
			return false;
		}

		return true;
	}

	void Disconnect()
	{
		MessageTransmitter.Disconnect();
		FileTransmitter.Disconnect();
	}

	bool IsConnected()
	{
		return (MessageTransmitter.IsConnected() || FileTransmitter.IsConnected());
	}
};

EN_FT_Client_Internal_FileTransmitter::EN_FT_Client_Internal_FileTransmitter(EN_FT_Client* fT_Client)
{
	FT_Client = fT_Client;
}

EN_FT_Client_Internal_MessageTransmitter::EN_FT_Client_Internal_MessageTransmitter(EN_FT_Client* fT_Client)
{
	FT_Client = fT_Client;
}

int main()
{
    EN_FT_Client A;

	// Check if connection success
	if (A.Connect() == false)
	{
		LOG(EN::LogLevels::Info, "Failed to connect");
		return 0;
	}

	std::string message;

	while (true)
	{
		// Get line from standart input
		getline(std::cin, message);

		// Stop while loop 
		if (message == "f")
			break;

		// Disconnect to client
		if (message == "d")
		{
			A.Disconnect();
			continue;
		}

		// Reconnect to client
		if (message == "r")
			A.Connect();

		// Check if we still connected
		//if (A.IsConnected())
		//{
			// A.SendToServer(message);
		//}
		//else 
		//	break;
	}

	// Disconnect client from server if still connected
	if (A.IsConnected())
		A.Disconnect();

	return 0;
}