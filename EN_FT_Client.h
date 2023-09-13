#pragma once

#include "EN_TCP_Client.h"

namespace EN
{
    class EN_FT_EternalClient : public EN::EN_TCP_Client
    {
    protected:
	    void OnConnect();

	    void ServerMessageHandler(std::string message);

	    void OnDisconnect();
    };

    class EN_FT_Client : public EN::EN_TCP_Client
    {
    private:
        EN_FT_EternalClient EternalFTCient;
        std::atomic_bool IsStop;
        std::atomic_int DelayBetweenSendingFileChunks;
        EN::EN_FileTransmissionStatus FileTransmissionStatus;
        std::thread FileSendingThread;
    public:
        EN_FT_Client()
        {
            IsStop.store(false);
            DelayBetweenSendingFileChunks.store(0);
        }

		bool Connect(std::string ipAddr, int tcpPort, int ftPort);

        void SendFileToServer(std::string filePath, uint64_t previoslySendedBytes)
        {
            FileSendingThread = std::thread([&]()
            {
                SendFile(GetSocket(), filePath, IsStop, DelayBetweenSendingFileChunks, previoslySendedBytes, FileTransmissionStatus);
            });
        }

        void StopSendingFile()
        {
            IsStop.store(true);
            if (FileSendingThread.joinable())
			    FileSendingThread.join();
        }

        void SetFileSendingSpeed(int delayBetweenSendingFileChunks)
        {
            DelayBetweenSendingFileChunks = delayBetweenSendingFileChunks;
        }

        void Disconnect()
        {
            EN::EN_TCP_Client::Disconnect();
            EternalFTCient.Disconnect();
        }
    };
}