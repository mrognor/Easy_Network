#include "EN_FileTransmitter_Client.h"

namespace EN
{
    EN_FileTransmitter_Client::EN_FileTransmitter_Client()
	{
		IsRunMessageHadlerThread = false;
        IsStopSendingFile.store(false);
        IsFinishedSendingFile.store(true);
        SendingFileSpeed.store(0);
	}

    void EN_FileTransmitter_Client::OnConnect() {}

    void EN_FileTransmitter_Client::ServerMessageHandler(std::string message) {}

    void EN_FileTransmitter_Client::OnDisconnect() {}

    void EN_FileTransmitter_Client::SendFileToServer(std::string filePath)
    {
        FileTransmissionThread = std::thread([&]()
        {
            IsFinishedSendingFile.store(false);
            SendFile(GetSocket(), filePath, IsStopSendingFile, SendingFileSpeed, 0, FileSendingStatus);
            IsFinishedSendingFile.store(true);
        });
    }

    void EN_FileTransmitter_Client::SetFileSendingSpeed(int fileSendingSpeed)
    {
        SendingFileSpeed.store(fileSendingSpeed);
    }

    void EN_FileTransmitter_Client::StopSendingFile()
    {
        if (!IsStopSendingFile.load())
        {
            IsStopSendingFile.store(true);
            while (!IsFinishedSendingFile.load());
            IsStopSendingFile.store(false);
        }
    }

    EN_FileTransmissionStatus EN_FileTransmitter_Client::GetFileSendingStatus()
    {
        return FileSendingStatus;
    }
}