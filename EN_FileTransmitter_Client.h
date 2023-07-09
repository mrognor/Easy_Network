#pragma once

#include "EN_TCP_Client.h"
#include "EN_FileTransmissionStatus.h"

namespace EN
{
    class EN_FileTransmitter_Client : public EN_TCP_Client
    {
    private:
        std::thread FileTransmissionThread;
        std::atomic_bool IsFinishedSendingFile;
        std::atomic_bool IsStopSendingFile;
        std::atomic_int SendingFileSpeed;
        EN::EN_FileTransmissionStatus FileSendingStatus;
    public:
        EN_FileTransmitter_Client();

	    virtual void OnConnect() override;

	    virtual void ServerMessageHandler(std::string message) override;

	    virtual void OnDisconnect() override;

        void SendFileToServer(std::string filePath);

        void SetFileSendingSpeed(int fileSendingSpeed);

        void StopSendingFile();

        EN_FileTransmissionStatus GetFileSendingStatus();
    };
}