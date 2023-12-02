#pragma once

#include <iostream>
#include <thread>
#include <queue>
#include <atomic>
#include <condition_variable>
#include <Windows.h>
#include <winioctl.h>
#include <string>

#include "EN_Atomic_Int64.h"

namespace EN
{
    class Subprocess
    {
    private:
        HANDLE pipeToChild[2] = { NULL, NULL };
        HANDLE pipeFromChild[2] = { NULL, NULL };
        PROCESS_INFORMATION ProcInfo;
        std::queue<std::string> MessagesQueue;
        std::thread Th;
        EN::EN_Atomic_Uint64_T DataCounter;
        std::condition_variable Cv;
        std::mutex Mtx;
        std::atomic_bool IsProcessEnded;

    public:
        void Launch(std::string programmName);

        void SendData(std::string data, bool isRequiredNewLineSymbol = true);

        std::string GetData(bool isRemoveNewLineSymbols = true);

        std::string WaitData(bool isRemoveNewLineSymbols = true);

        void StopProcess(std::string commandToSendToProcessToStop = "", bool isRequiredNewLineSymbol = true);

        bool IsData();

        bool GetIsProcessEnded();

        ~Subprocess();
    };
}