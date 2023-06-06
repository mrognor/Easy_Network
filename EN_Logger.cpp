#include "EN_Logger.h"

namespace EN
{
    int EnabledLogLevels = ENABLE_LOG_LEVEL_INFO | ENABLE_LOG_LEVEL_HINT | ENABLE_LOG_LEVEL_WARNING | ENABLE_LOG_LEVEL_ERROR;
    void (*LogFunc)(LogLevels, std::string) = DefaultLogFunc;

    void DefaultLogFunc(LogLevels logLevel, std::string logMessage)
    {
        static std::mutex mtx;

        switch (logLevel)
        {
        case Info:
            if (EnabledLogLevels & ENABLE_LOG_LEVEL_INFO)
            {
                mtx.lock();
                std::cerr << GetCurrentDate() << " " << GetCurrentDayTimeWithSecondFraction<std::chrono::milliseconds>() << " [message] " << logMessage << std::endl;
            }
            break;

        case Hint:
            if (EnabledLogLevels & ENABLE_LOG_LEVEL_HINT)
            {
                mtx.lock();
                std::cerr << "\x1B[94m" << GetCurrentDate() << " " << GetCurrentDayTimeWithSecondFraction<std::chrono::milliseconds>() << " [hint] " << logMessage << "\033[0m" << std::endl;
            }
            break;

        case Warning:
            if (EnabledLogLevels & ENABLE_LOG_LEVEL_WARNING)
            {
                mtx.lock();
                std::cerr << "\x1B[33m" << GetCurrentDate() << " " << GetCurrentDayTimeWithSecondFraction<std::chrono::milliseconds>() << " [warning] " << logMessage << "\033[0m" << std::endl;
            }
            break;

        case Error:
            if (EnabledLogLevels & ENABLE_LOG_LEVEL_ERROR)
            {
                mtx.lock();
                std::cerr << "\x1B[31m" << GetCurrentDate() << " " << GetCurrentDayTimeWithSecondFraction<std::chrono::milliseconds>() << " [error] " << logMessage << "\033[0m" << std::endl;
            }
            break;
        }
        mtx.unlock();
    }

    void SetLogFunc(void (*logFunc)(LogLevels, std::string), int logLevelsToEnable)
    {
        LogFunc = logFunc;
        EnabledLogLevels = logLevelsToEnable;
    }

    void EnableLogLevels(int logLevelsToEnable)
    {
        EnabledLogLevels = logLevelsToEnable;
    }
}