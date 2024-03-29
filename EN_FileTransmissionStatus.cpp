#include "EN_FileTransmissionStatus.h"

namespace EN
{
    EN_FileTransmissionStatus::EN_FileTransmissionStatus()
    {
        TransferedBytes.store(0);
        FileSize.store(0);
        TransmissionSpeed.store(0);
        TransmissionEta.store(0);
        IsTransmissionEnded.store(false);
        IsTransmissionSucceed.store(false);
    }

    EN_FileTransmissionStatus::EN_FileTransmissionStatus(EN::EN_FileTransmissionStatus& status)
    {
        TransferedBytes.store(status.TransferedBytes.load());
        FileSize.store(status.FileSize.load());
        TransmissionSpeed.store(status.TransmissionSpeed.load());
        TransmissionEta.store(status.TransmissionEta.load());
        IsTransmissionEnded.store(status.IsTransmissionEnded);
        IsTransmissionSucceed.store(status.IsTransmissionSucceed);

        IsSetProgressFunction = status.IsSetProgressFunction;
        ProgressFunction = status.ProgressFunction;
    }

    void EN_FileTransmissionStatus::SetTransferedBytes(uint64_t transferedBytes)
    {
        TransferedBytes.store(transferedBytes);
    }

    uint64_t EN_FileTransmissionStatus::GetThisSessionTransferedBytes()
    {
        return TransferedBytes.load();
    }
    
    void EN_FileTransmissionStatus::SetFileSize(uint64_t fileSize)
    {
        FileSize.store(fileSize);
    }

    uint64_t EN_FileTransmissionStatus::GetFileSize()
    {
        return FileSize.load();
    }

    void EN_FileTransmissionStatus::SetTransmissionSpeed(uint64_t transmissionSpeed)
    {
        TransmissionSpeed.store(transmissionSpeed);
    }

    uint64_t EN_FileTransmissionStatus::GetTransmissionSpeed()
    {
        return TransmissionSpeed.load();
    }

    void EN_FileTransmissionStatus::SetTransmissionEta(uint64_t transmissionEta)
    {
        TransmissionEta.store(transmissionEta);
    }

    uint64_t EN_FileTransmissionStatus::GetTransmissionEta()
    {
        return TransmissionEta.load();
    }

    void EN_FileTransmissionStatus::SetIsTransmissionEnded(bool isTransmissionEnded)
    {
        IsTransmissionEnded.store(isTransmissionEnded);
    }

    bool EN_FileTransmissionStatus::GetIsTransmissionEnded()
    {
        return IsTransmissionEnded.load();
    }

    void EN_FileTransmissionStatus::SetIsTransmissionSucceed(bool isTransmissionSucceed)
    {
        IsTransmissionSucceed.store(isTransmissionSucceed);
    }

    bool EN_FileTransmissionStatus::GetIsTransmissionSucceed()
    {
        return IsTransmissionSucceed.load();
    }

    bool EN_FileTransmissionStatus::GetIsSetProgressFunction()
    {
        return IsSetProgressFunction;
    }

    void EN_FileTransmissionStatus::SetProgressFunction(std::function<void(uint64_t, uint64_t, uint64_t, uint64_t)> progressFunction)
    {
        IsSetProgressFunction = true;
        ProgressFunction = progressFunction;
    }

    void EN_FileTransmissionStatus::InvokeProgressFunction()
    {
        if (IsSetProgressFunction)
            ProgressFunction(TransferedBytes.load(), FileSize.load(), TransmissionSpeed.load(), TransmissionEta.load());
    }
}