#pragma once

#include <cstdint>
#include <atomic>
#include <functional>
#include "EN_Atomic_Int64.h"

#ifdef NATIVE_ATOMIC_INT64_NOT_SUPPORTED
    typedef EN::EN_Atomic_Uint64_T AtomicUint64;
#else
    typedef std::atomic_uint64_t AtomicUint64;
#endif

namespace EN
{
    class EN_FileTransmissionStatus
    {
    private:
        AtomicUint64 TransferedBytes;
        AtomicUint64 FileSize;
        AtomicUint64 TransmissionSpeed;
        AtomicUint64 TransmissionEta;
        
        bool IsSetProgressFunction = false;
        std::function<void(uint64_t, uint64_t, uint64_t, uint64_t)> ProgressFunction;

    public:
        EN_FileTransmissionStatus();

        EN_FileTransmissionStatus(EN::EN_FileTransmissionStatus& status);

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
