#include <thread>
#include <mutex>
#include <iostream>
#include <condition_variable>
#include <atomic>
#include <chrono>

namespace EN
{
    class EN_BackgroundTimer
    {
    private:
        std::thread TimerThread;
        std::atomic_bool IsDestroy;
        std::atomic_int Code;
        std::condition_variable CondVar;
        std::chrono::microseconds SleepTime;
        std::mutex Mtx;
    public:
        EN_BackgroundTimer();

        template<class T = std::chrono::seconds>
        void StartTimer(uint64_t timeToSleep)
        {
            Mtx.lock();
            while (Code.load() == 0)
                CondVar.notify_one();
            SleepTime = T(timeToSleep);
            Mtx.unlock();
        }

        bool IsSleep();

        ~EN_BackgroundTimer();
    };
}