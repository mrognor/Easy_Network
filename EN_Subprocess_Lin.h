#pragma once

#include <iostream>
#include <unistd.h> 
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <thread>
#include <queue>
#include <atomic>
#include <condition_variable>

#include "EN_Atomic_Int64.h"

namespace EN
{
    class Subprocess
    {
    private:
        int PipeToChild[2];
        int PipeFromChild[2];
        pid_t Pid;
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