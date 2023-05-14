#include "EN_Logger.h"

namespace EN
{
    int MaxLogLevel = -1;
    void (*LogFunc)(logLevels, std::string) = nullptr;

    void DefaultLogFunc(logLevels logLevel, std::string logMessage)
    {
        if (MaxLogLevel == -1)
            MaxLogLevel = Message;

        switch (logLevel)
        {
        case Message:
            if (MaxLogLevel > 0)
                break;
            std::cerr << GetCurrentDayTime() << " [message] " << logMessage << std::endl;
            break;

        case Hint:
            if (MaxLogLevel > 0)
                break;
            std::cerr << "\x1B[94m" << GetCurrentDayTime() << " [hint] " << logMessage << "\033[0m" << std::endl;
            break;

        case Warning:
            if (MaxLogLevel > 1)
                break;
            std::cerr << "\x1B[33m" << GetCurrentDayTime() << " [warning] " << logMessage << "\033[0m" << std::endl;
            break;

        case Error:
            if (MaxLogLevel > 2)
                break;
            std::cerr << "\x1B[31m" << GetCurrentDayTime() << " [error] " << logMessage << "\033[0m" << std::endl;
            break;
        }
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