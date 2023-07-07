#include "EN_BackgroundTimer.h"

namespace EN
{
    EN_BackgroundTimer::EN_BackgroundTimer()
    {
        IsDestroy.store(false);
        Code.store(0);
        TimerThread = std::thread([this] ()
        {
            std::mutex condVarMtx;
            std::unique_lock<std::mutex> lk(condVarMtx);

            while (!IsDestroy.load())
            {
                CondVar.wait(lk);
                Code.store(1);
                Mtx.lock();
                Mtx.unlock();
                CondVar.wait_for(lk, SleepTime);
                Code.store(0);
            }

            Code.store(2);
        });
    }

    bool EN_BackgroundTimer::IsSleep()
    {
        return Code.load() == 1;
    }

    EN_BackgroundTimer::~EN_BackgroundTimer()
    {
        IsDestroy.store(true);

        while (Code.load() != 2)
            CondVar.notify_one();
        
        TimerThread.join();
    }
}