#include  "../EN_TCP_Client.h"
#include "../EN_ThreadBarrier.h"

namespace EN
{
	class EN_FT_Client;

	class EN_FT_Client_Internal_FileTransmitter : public EN::EN_TCP_Client
	{
	private:
		EN_FT_Client* FT_Client;
	protected:
		void OnConnect();

		virtual void ServerMessageHandler(std::string message) override { std::cout << "FT " << message << std::endl; }

		void OnDisconnect();
	public:
		EN_FT_Client_Internal_FileTransmitter(EN_FT_Client* fT_Client);
	};

	class EN_FT_Client_Internal_MessageTransmitter : public EN::EN_TCP_Client
	{
	private:
		EN_FT_Client* FT_Client;
	protected:
		void OnConnect();

		virtual void ServerMessageHandler(std::string message) override { std::cout << "MT " << message << std::endl; }

		void OnDisconnect();
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
		EN_ThreadBarrier StartEndBarrier;
		bool IsConnectedToFTServer = true;
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
			FileTransmitter.Disconnect(false);
			MessageTransmitter.Disconnect(false);
		}

		bool IsConnected()
		{
			return (MessageTransmitter.IsConnected() || FileTransmitter.IsConnected());
		}

		void OnConnect()
		{
			LOG(LogLevels::Info, "FT_Client connected!");
		}

		void OnDisconnect()
		{
			LOG(LogLevels::Info, "FT_Client disconnected!");
		}
	};

	EN_FT_Client_Internal_FileTransmitter::EN_FT_Client_Internal_FileTransmitter(EN_FT_Client* fT_Client)
	{
		FT_Client = fT_Client;
	}

	void EN_FT_Client_Internal_FileTransmitter::OnConnect() 
	{
		LOG(LogLevels::Info, "Internal file transmitter connected!");
		std::string ftSockDesc;
		// Wait first message with socket descriptor from file server
		WaitMessage(ftSockDesc);

		auto vec = Split(ftSockDesc);

		// Valid first message
		if (vec[0] != "FtSockDesc")
			FT_Client->IsConnectedToFTServer = false; // Set correct server var to false
		else
			FT_Client->MessageTransmitter.SendToServer(ftSockDesc); // Send file server socket descriptor to message server

		// Wait while message client connected
		FT_Client->StartEndBarrier.Wait(2); // Wait A
		// Wait while message client call FT_Client OnConnect function
		FT_Client->StartEndBarrier.Wait(2); // Wait B

		if (!FT_Client->IsConnectedToFTServer)
		{
			LOG(LogLevels::Info, "The wrong beginning of communication. The FT client did not connect to FT server");
			FT_Client->Disconnect();
		}
	}

	void EN_FT_Client_Internal_FileTransmitter::OnDisconnect() 
	{
		FT_Client->MessageTransmitter.Disconnect(false);

		LOG(LogLevels::Info, "Internal file transmitter disconnected!");

		FT_Client->StartEndBarrier.Wait(2);
	}

	EN_FT_Client_Internal_MessageTransmitter::EN_FT_Client_Internal_MessageTransmitter(EN_FT_Client* fT_Client)
	{
		FT_Client = fT_Client;
	}

	void EN_FT_Client_Internal_MessageTransmitter::OnConnect()
	{
		LOG(LogLevels::Info, "Internal message transmitter connected!");

		FT_Client->StartEndBarrier.Wait(2); // Wait A
		if (FT_Client->IsConnectedToFTServer)
			FT_Client->OnConnect();
		FT_Client->StartEndBarrier.Wait(2); // Wait B
	}

	void EN_FT_Client_Internal_MessageTransmitter::OnDisconnect()
	{
		FT_Client->FileTransmitter.Disconnect(false);

		LOG(LogLevels::Info, "Internal message transmitter disconnected!");
		
		FT_Client->StartEndBarrier.Wait(2);
		FT_Client->OnDisconnect();
	}
}


int main()
{
    EN::EN_FT_Client A;

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