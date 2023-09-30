#include "EN_ThreadBarrier.h"

namespace EN
{
    EN_ThreadBarrier::EN_ThreadBarrier() { WaitingAmount.store(0); }

    void EN_ThreadBarrier::Wait(int amount)
    {
        std::mutex sleepMtx;
        std::unique_lock<std::mutex> lk(sleepMtx);

        WaitingAmount.fetch_add(1);

        if (WaitingAmount.compare_exchange_strong(amount, amount - 1))
        {
            while (WaitingAmount.load() != 0) Cv.notify_all();
        }
        else
        {
            Cv.wait(lk);
            WaitingAmount.fetch_sub(1);
        }
    }

    void EN_ThreadBarrier::WaitFor(int amount, std::chrono::seconds time)
    {
        std::mutex sleepMtx;
        std::unique_lock<std::mutex> lk(sleepMtx);

        WaitingAmount.fetch_add(1);

        if (WaitingAmount.compare_exchange_strong(amount, amount - 1))
        {
            while (WaitingAmount.load() != 0) Cv.notify_all();
        }
        else
        {
            Cv.wait_for(lk, time);
            WaitingAmount.fetch_sub(1);
        }
    }
}