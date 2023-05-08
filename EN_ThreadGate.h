#include <thread>
#include <condition_variable>
#include <chrono>
#include <iostream>

namespace EN
{
    // This class implements a gate for a thread. It works as condition variable, 
    // but if the Open method was called in another thread before the Close method, 
    // then the Close method will not block the thread. Doesn't make sense when working in more than two threads
    class EN_Gate
    {
    private:
        std::mutex mtx, condVarMutex, lockMutex;
        std::condition_variable cv;
        bool IsActivated = true;
    public:
        // Blocks the execution of the thread until the Open method is called. 
        // If the Open method was called before this method, then the thread is not blocked.
        // The Close and Open methods are synchronized with each other using a mutex
        void Close()
        {
            std::unique_lock<std::mutex> lk(lockMutex);
            mtx.lock();
            if (IsActivated)
            {
                condVarMutex.lock();
                mtx.unlock();
                cv.wait(lk);
                condVarMutex.unlock();
                mtx.lock();
            }
            else
            {
                IsActivated = true;
            }
            mtx.unlock();
        }

        // Causes the thread to continue executing after the Close method. 
        // If called before the Close method, then the Close method will not block the thread.
        // The Close and Open methods are synchronized with each other using a mutex
        void Open()
        {
            mtx.lock();
            if (condVarMutex.try_lock())
            {
                IsActivated = false;
                condVarMutex.unlock();
            }
            else
            {
                while (condVarMutex.try_lock() != true)
                    cv.notify_all();

                condVarMutex.unlock();
            }
            mtx.unlock();
        }
    };

    // This class implements a gate for a thread. It works as condition variable, 
    // but if the Open method was called in another thread before the Close method, 
    // then the Close method will not block the thread. Doesn't make sense when working in more than two threads. 
    // Supports opening and closing counter. You can call the Open method N times and then the 
    // Close method will block the execution of the thread only for N call
    class EN_RecursiveGate
    {
    private:
        std::mutex mtx, condVarMutex, lockMutex;
        std::condition_variable cv;
        int ClosingAmount = 0;
    public:
        // Blocks the execution of the thread until the Open method is called. 
        // If the Open method was called before this method, then the thread is not blocked.
        // The Close and Open methods are synchronized with each other using a mutex
        void Close()
        {
            std::unique_lock<std::mutex> lk(lockMutex);
            mtx.lock();
            if (ClosingAmount <= 0)
            {
                condVarMutex.lock();
                mtx.unlock();
                cv.wait(lk);
                condVarMutex.unlock();
                mtx.lock();
            }
            --ClosingAmount;
            mtx.unlock();
        }

        // Causes the thread to continue executing after the Close method. 
        // If called before the Close method, then the Close method will not block the thread.
        // The Close and Open methods are synchronized with each other using a mutex
        void Open()
        {
            mtx.lock();
            if (!condVarMutex.try_lock())
            {
                while (condVarMutex.try_lock() != true)
                    cv.notify_all();

                condVarMutex.unlock();
            }
            else
                condVarMutex.unlock();

            ++ClosingAmount;
            mtx.unlock();
        }
    };

    // This class implements a gate for a thread. It works as condition variable, 
    // but if the Open method was called in another thread before the Close method, 
    // then the Close method will not block the thread. Doesn't make sense when working in more than two threads
    class EN_TimeGate
    {
    private:
        std::mutex mtx, condVarMutex, lockMutex;
        std::condition_variable cv;
        bool IsActivated = true;
    public:
        // Blocks the execution of the thread until the Open method is called. 
        // If the Open method was called before this method, then the thread is not blocked.
        // The Close and Open methods are synchronized with each other using a mutex
        void Close()
        {
            std::unique_lock<std::mutex> lk(lockMutex);
            mtx.lock();
            if (IsActivated)
            {
                condVarMutex.lock();
                mtx.unlock();
                cv.wait(lk);
                condVarMutex.unlock();
                mtx.lock();
            }
            else
            {
                IsActivated = true;
            }
            mtx.unlock();
        }

        // Blocks the execution of the thread until the Open method is called or time's not up.
        // If the Open method was called before this method, then the thread is not blocked.
        // The Close and Open methods are synchronized with each other using a mutex.
        // After the time expires, the gate remains active, which means that the following Close method will block the thread.
        template<class T>
        void CloseFor(std::chrono::duration<T> duration)
        {
            std::unique_lock<std::mutex> lk(lockMutex);
            mtx.lock();
            if (IsActivated)
            {
                condVarMutex.lock();
                mtx.unlock();
                cv.wait_for(lk, duration);
                condVarMutex.unlock();
                mtx.lock();
            }
            else
            {
                IsActivated = true;
            }
            mtx.unlock();
        }

        // Blocks the execution of the thread until the Open method is called or the time has come for.
        // If the Open method was called before this method, then the thread is not blocked.
        // The Close and Open methods are synchronized with each other using a mutex.
        // After the time expires, the gate remains active, which means that the following Close method will block the thread.
        template<class T>
        void CloseUntil(std::chrono::time_point<T> timePoint)
        {
            std::unique_lock<std::mutex> lk(lockMutex);
            mtx.lock();
            if (IsActivated)
            {
                condVarMutex.lock();
                mtx.unlock();
                cv.wait_until(lk, timePoint);
                condVarMutex.unlock();
                mtx.lock();
            }
            else
            {
                IsActivated = true;
            }
            mtx.unlock();
        }

        // Causes the thread to continue executing after the Close method. 
        // If called before the Close method or after CloseFor time expires, then the Close method will not block the thread.
        // The Close and Open methods are synchronized with each other using a mutex
        void Open()
        {
            mtx.lock();
            if (condVarMutex.try_lock())
            {   
                IsActivated = false;
                condVarMutex.unlock();
            }
            else
            {
                while (condVarMutex.try_lock() != true)
                    cv.notify_all();

                condVarMutex.unlock();
            }
            mtx.unlock();
        }

        // Causes the thread to continue executing after the CloseFor method. If the time is over before this method is called, then it will not do anything
        // If called before the Close method, then the Close method will block the thread.
        // The Close and Open methods are synchronized with each other using a mutex
        void OpenIfClosed()
        {
            mtx.lock();

            while (condVarMutex.try_lock() != true)
                cv.notify_all();

            condVarMutex.unlock();
            
            mtx.unlock();
        }
    };
}