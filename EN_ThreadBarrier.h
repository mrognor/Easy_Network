#include <thread>
#include <mutex>
#include <iostream>
#include <condition_variable>
#include <atomic>

namespace EN
{
    class EN_ThreadBarrier
    {
    private:
        std::mutex Mtx;
        std::condition_variable Cv;
        std::atomic_int WaitingAmount;
    public:
        EN_ThreadBarrier();

        void Wait(int amount);
    };
}