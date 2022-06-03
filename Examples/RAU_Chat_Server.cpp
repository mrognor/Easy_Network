#include "../EN_RAU_Server.h"

class RAU_Server : public EN::EN_RAU_Server
{	
public:
	RAU_Server()
	{
		// IpAddress = "192.168.1.64"; //Default set to localhost
		// Port = <put int here> to set port. Default port is 1111
	}

	void OnClientConnected(int ClientID)
	{
		std::cout << "Client Connected! id: " << ClientID << std::endl;
		SendToClient(ClientID, "Welcome. You are connected to server.");
	}

	void ClientMessageHandler(std::string message, int ClientID)
	{
		// Important. This function is run in a separate thread. 
		// If you want to write data to class variables, you should use mutexes or other algorithms for thread-safe code.
		std::cout << message << std::endl;

		if (message == "d")
			DisconnectClient(ClientID);

		if (message == "F")
			Shutdown();

		for (int j = 0; j < GetConnectionsCount(); j++)
		{
			if (j != ClientID)
				SendToClient(j, message);
		}
	}

	void OnClientDisconnect(int ClientID)
	{
		std::cout << "Client disconnected! ID: " << ClientID << std::endl;
	}
};

int main()
{
	RAU_Server A;
	A.Run();
}