#include "EN_Atomic_Int64.h"

namespace EN
{
    void EN_Atomic_Int64_T::store(int64_t value)
    {
        Mtx.lock();
        Value = value;
        Mtx.unlock();
    }

    const int64_t EN_Atomic_Int64_T::load()
    {
        Mtx.lock();
        int64_t value = Value;
        Mtx.unlock();
        return value;
    }

    void EN_Atomic_Int64_T::fetch_add(int64_t value)
    {
        Mtx.lock();
        Value += value;
        Mtx.unlock();
    }

    void EN_Atomic_Int64_T::fetch_sub(int64_t value)
    {
        Mtx.lock();
        Value -= value;
        Mtx.unlock();
    }

    void EN_Atomic_Uint64_T::store(uint64_t value)
    {
        Mtx.lock();
        Value = value;
        Mtx.unlock();
    }

    uint64_t EN_Atomic_Uint64_T::load()
    {
        Mtx.lock();
        uint64_t value = Value;
        Mtx.unlock();
        return value;
    }

    void EN_Atomic_Uint64_T::fetch_add(uint64_t value)
    {
        Mtx.lock();
        Value += value;
        Mtx.unlock();
    }

    void EN_Atomic_Uint64_T::fetch_sub(uint64_t value)
    {
        Mtx.lock();
        Value -= value;
        Mtx.unlock();
    }
}