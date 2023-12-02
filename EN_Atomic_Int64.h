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

        void fetch_add(int64_t value);
        void fetch_sub(int64_t value); 
    };

    class EN_Atomic_Uint64_T
    {
    private:
        std::uint64_t Value;
        std::mutex Mtx;
    public:
        void store(uint64_t value);
        uint64_t load();

        void fetch_add(uint64_t value);
        void fetch_sub(uint64_t value); 
    };
}