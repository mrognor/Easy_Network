#include "EN_FileTransmissionStatus.h"

namespace EN
{
    EN_FileTransmissionStatus::EN_FileTransmissionStatus()
    {
        TransferedBytes.store(0);
        FileSize.store(0);
        TransmissionSpeed.store(0);
        TransmissionEta.store(0);
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