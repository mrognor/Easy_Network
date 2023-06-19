#include "EN_RAU_Server.h"

class RAU_Server : public EN::EN_RAU_Server
{	
public:
	RAU_Server()
	{
		// IpAddress = "192.168.1.73"; Default set to localhost
		// Port = <put int here> to set port. Default port is 1111
		// MaxUnreliableMessageSize = 512; Default set to 64
		// You have to set synchronizied it with client
	}

	void OnClientConnected(size_t ClientID)
	{
		std::cout << "Client connected! Id: " << ClientID << std::endl;
		SendToClient(ClientID, "Welcome. You are connected to server.");
		SendToClient(ClientID, "Reliable", true);
		SendToClient(ClientID, "Unreliable", false);
	}

	void ClientMessageHandler(std::string message, size_t ClientID)
	{
		// Important. This function is run in a separate thread. 
		// If you want to write data to class variables, you should use mutexes or other algorithms for thread-safe code.
		std::cout << "msg: " << message << std::endl;

		// Disconnect client
		if (message == "TCP d" || message == "UDP d")
			DisconnectClient(ClientID);

		// Shutdown server
		if (message == "TCP F" || message == "UDP F")
			Shutdown();

		for (size_t j = 0; j < GetConnectionsCount(); j++)
		{
			if(message.find("TCP") == 0)
				SendToClient(j, message);
			else SendToClient(j, message, false);
		}
	}

	void OnClientDisconnect(size_t ClientID)
	{
		std::cout << "Client disconnected! ID: " << ClientID << std::endl;
	}
};

int main()
{
	RAU_Server A;
	// Start server
	try 
	{
		A.Run(); 
	}
	catch (std::runtime_error& err)
	{
		LOG(EN::LogLevels::Error, "Run throw error with error code: " + std::string(err.what()));
	}
}