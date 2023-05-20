#include "EN_Logger.h"

namespace EN
{
    int MaxLogLevel = -1;
    void (*LogFunc)(logLevels, std::string) = nullptr;

    void DefaultLogFunc(logLevels logLevel, std::string logMessage)
    {
        static std::mutex mtx;

        if (MaxLogLevel == -1)
            MaxLogLevel = Info;

        switch (logLevel)
        {
        case Info:
            if (MaxLogLevel > 0)
                break;
            mtx.lock();
            std::cerr << GetCurrentDate() << " " << GetCurrentDayTimeWithSecondFraction<std::chrono::milliseconds>() << " [message] " << logMessage << std::endl;
            break;

        case Hint:
            if (MaxLogLevel > 0)
                break;
            mtx.lock();
            std::cerr << "\x1B[94m" << GetCurrentDate() << " " << GetCurrentDayTimeWithSecondFraction<std::chrono::milliseconds>() << " [hint] " << logMessage << "\033[0m" << std::endl;
            break;

        case Warning:
            if (MaxLogLevel > 1)
                break;
            mtx.lock();
            std::cerr << "\x1B[33m" << GetCurrentDate() << " " << GetCurrentDayTimeWithSecondFraction<std::chrono::milliseconds>() << " [warning] " << logMessage << "\033[0m" << std::endl;
            break;

        case Error:
            if (MaxLogLevel > 2)
                break;
            mtx.lock();
            std::cerr << "\x1B[31m" << GetCurrentDate() << " " << GetCurrentDayTimeWithSecondFraction<std::chrono::milliseconds>() << " [error] " << logMessage << "\033[0m" << std::endl;
            break;
        }
        mtx.unlock();
    }

    void SetLogFunc(void (*logFunc)(logLevels, std::string), logLevels maxLogLevel)
    {
        LogFunc = logFunc;
        MaxLogLevel = maxLogLevel;
    }

    void SetMaxLogLevel(logLevels maxLogLevel)
    {
        MaxLogLevel = maxLogLevel;
    }
}