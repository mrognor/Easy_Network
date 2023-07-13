#include <iostream>
#include "EN_UDP_Server.h"

class MyServer : public EN::EN_UDP_Server
{
public:
	MyServer()
	{
		// IpAddress = "192.168.1.64"; Default set to localhost
		// Port = <put int here> to set port. Default port is 1111
		// You have to set synchronizied it with client
		// ServerBuferType = EN::Queue; Can be queue or stack
		// ThreadAmount = 1; Shows how many threads will process incoming messages. Default set to 2
		// MaxStackBuffSize = 4; Shows the maximum size of the incoming message stack. 
		// Work only if ServerBuferType == EN::Stack
	}

	// Third parametr in milliseconds
	virtual void ClientMessageHandler(std::string message, std::string ClientIpAddress, long long TimeSincePackageArrived) override
	{
		// Checking how much time has passed since the arrival of the message
		if (TimeSincePackageArrived > 700)
			return;

		// Shutdown server
		if (message == "f")
			Shutdown();
		
		LOG(EN::LogLevels::Info, "From: " + ClientIpAddress + " Message: " + message);

		SendToClient(ClientIpAddress, message);
	}

	// Function work between putting message in buffer. Return true if you want to put message in buffer
	virtual bool InstantClientMessageHandler(std::string message, std::string ClientIpAddress, long long TimeWhenPackageArrived) override
	{
		LOG(EN::LogLevels::Info, "Instant client message hadler. From: " + ClientIpAddress + " Message: " + message);
		if (message == "false")
			return false;
		return true;
	}
};


int main()
{
	MyServer A;
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
