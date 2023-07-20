#pragma once

#include <cstdint>
#include <mutex>

namespace EN
{
    class EN_Atomic_Int64_T
    {
    private:
        std::int64_t Value;
        std::mutex Mtx;
    public:
        void store(int64_t value);
        const int64_t load();
    };

    class EN_Atomic_Uint64_T
    {
    private:
        std::uint64_t Value;
        std::mutex Mtx;
    public:
        void store(uint64_t value);
        uint64_t load();
    };
}