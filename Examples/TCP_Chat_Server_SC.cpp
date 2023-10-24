#pragma once

#include "../EN_TCP_Server.h"

namespace EN
{
    template <class T>
    struct TCP_SessionContext
    {
        T data;

        // A pointer to a function for sending messages. Allows you to use custom network protocols. Send message to socket
		bool (*TCP_Send)(EN_SOCKET sock, const std::string& message) = EN::Default_TCP_Send;
		
		// A pointer to a function for recv messages. Allows you to use custom network protocols. Recv message from socket
		bool (*TCP_Recv)(EN_SOCKET sock, std::string& message) = EN::Default_TCP_Recv;

        T* operator-> ()
        {
            return &data;
        }
    };

    template <class T>
    class EN_TCP_Server_SC : public EN::EN_TCP_Server
    {
    private:
		virtual void OnClientConnected(EN_SOCKET clientSocket) {};

		virtual void ClientMessageHandler(EN_SOCKET clientSocket, std::string message) {};

		virtual void OnClientDisconnect(EN_SOCKET clientSocket) {};
    
    protected:
        TCP_SessionContext<T> SessionContext;

    	virtual void OnClientConnected(EN_SOCKET clientSocket, TCP_SessionContext<T>& sessionContext) = 0;

		virtual void ClientMessageHandler(EN_SOCKET clientSocket, std::string message, TCP_SessionContext<T>& sessionContext) = 0;

		virtual void OnClientDisconnect(EN_SOCKET clientSocket, TCP_SessionContext<T>& sessionContext) = 0;

    public:
        void ClientHandler(EN_SOCKET clientSocket)
        {
            OnClientConnected(clientSocket, SessionContext);

            std::string message;
            bool ConnectionStatus;

            while (true)
            {
                ConnectionStatus = TCP_Recv(clientSocket, message);

                if (ConnectionStatus == false)
                {
                    OnClientDisconnect(clientSocket, SessionContext);
                    break;
                }

                ClientMessageHandler(clientSocket, message, SessionContext);
            }

            CloseSocket(clientSocket);

            CrossWalk.PedestrianStartCrossRoad();

            ClientSockets.erase(clientSocket);

            CrossWalk.PedestrianStopCrossRoad();
        }
    };
}

struct Session
{
    int counter = 0;
};

class TCP_Server_SC : public EN::EN_TCP_Server_SC<Session>
{
protected:
    virtual void OnClientConnected(EN_SOCKET clientSocket, EN::TCP_SessionContext<Session>& sessionContext)
    {
        ++sessionContext->counter;
    }

    virtual void ClientMessageHandler(EN_SOCKET clientSocket, std::string message, EN::TCP_SessionContext<Session>& sessionContext)
    {
        std::cout << ++sessionContext->counter << std::endl;
    }

    virtual void OnClientDisconnect(EN_SOCKET clientSocket, EN::TCP_SessionContext<Session>& sessionContext)
    {

    }
};

int main()
{
    EN::IsCanBeDigit("");
    
	TCP_Server_SC A;
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

		A.MulticastSend(message);
	}

	th.join();
}