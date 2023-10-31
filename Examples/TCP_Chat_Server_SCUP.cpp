#include "../EN_TCP_Server_SCUP.h"

// Class to save session context.
struct Session
{
    int counter = 0;
};

class TCP_Server_SCUP : public EN::EN_TCP_Server_SCUP<Session>
{
protected:
    virtual void OnClientConnected(EN_SOCKET clientSocket, EN::TCP_SessionContext_SCUP<Session>& sessionContext)
    {
        ++sessionContext->counter;
		LOG(EN::LogLevels::Info, "SCUP client connected");
    }

    virtual void ClientMessageHandler(EN_SOCKET clientSocket, std::string message, EN::TCP_SessionContext_SCUP<Session>& sessionContext)
    {
		LOG(EN::LogLevels::Info, "SCUP client send message: " + message + " Message counter: " + std::to_string(sessionContext->counter));
        ++sessionContext->counter;
	}

    virtual void OnClientDisconnect(EN_SOCKET clientSocket, EN::TCP_SessionContext_SCUP<Session>& sessionContext)
    {
		LOG(EN::LogLevels::Info, "SCUP client disconnected");
    }
public:
	TCP_Server_SCUP()
	{
		// SessionContext.TCP_Recv = ;
	}
};

int main()
{
	TCP_Server_SCUP A;
start:

	std::thread th([&A]() 
	{
		try 
		{
			A.Run(); 
		}
		catch (std::runtime_error& err)
		{
			LOG(EN::LogLevels::Error, "Run throw error with error code: " + std::string(err.what()));
			LOG(EN::LogLevels::Info, "Enter f to end programm");
		}
	});
	
	std::string message;

	while (true)
	{
		getline(std::cin, message);

		if (message == "f")
		{
			A.Shutdown();
			break;
		}
		
		if (message == "r")
		{
			th.join();
			goto start;
		}
	}

	th.join();
}