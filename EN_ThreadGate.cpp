#include "EN_ThreadGate.h"

namespace EN
{
    void EN_ThreadGate::Close()
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

    void EN_ThreadGate::Open()
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

    void EN_RecursiveThreadGate::Close()
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

    void EN_RecursiveThreadGate::Open()
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

    void EN_TimeThreadGate::Close()
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

    void EN_TimeThreadGate::Open()
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

    void EN_TimeThreadGate::OpenIfClosed()
    {
        mtx.lock();

        while (condVarMutex.try_lock() != true)
            cv.notify_all();

        condVarMutex.unlock();
        
        mtx.unlock();
    }
}