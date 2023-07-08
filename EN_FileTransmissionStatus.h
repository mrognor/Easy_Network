#pragma once

#include <stdint.h>
#include <atomic>
#include <functional>

namespace EN
{
    class EN_FileTransmissionStatus
    {
    private:
        std::atomic_uint64_t TransferedBytes;
        std::atomic_uint64_t FileSize;
        std::atomic_uint64_t TransmissionSpeed;
        std::atomic_uint64_t TransmissionEta;
        
        bool IsSetProgressFunction = false;
        std::function<void(uint64_t, uint64_t, uint64_t, uint64_t)> ProgressFunction;

    public:
        EN_FileTransmissionStatus();

        void SetTransferedBytes(uint64_t transferedBytes);
        uint64_t GetThisSessionTransferedBytes();
        
        void SetFileSize(uint64_t fileSize);
        uint64_t GetFileSize();

        void SetTransmissionSpeed(uint64_t transmissionSpeed);
        uint64_t GetTransmissionSpeed();

        void SetTransmissionEta(uint64_t transmissionEta);
        uint64_t GetTransmissionEta();

        bool GetIsSetProgressFunction();
        void SetProgressFunction(std::function<void(uint64_t, uint64_t, uint64_t, uint64_t)> progressFunction);
        void InvokeProgressFunction();
    };
}
