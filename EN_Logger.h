#pragma once

#include <iostream>

namespace EN
{
    #ifndef DISABLE_LOGGER
    #define LOG(logLevel, messageString)             \
        if (EN::LogFunc == nullptr)                      \
            DefaultLogFunc(logLevel, messageString); \
        else                                         \
            EN::LogFunc(logLevel, messageString);
    #else
    #define LOG(logLevel, messageString)
    #endif

    enum logLevels
    {
        Message,
        Warning,
        Error
    };

    extern int MaxLogLevel;
    extern void (*LogFunc)(logLevels, std::string);

    void DefaultLogFunc(logLevels logLevel, std::string logMessage);

    void SetLogFunc(void (*logFunc)(logLevels, std::string), logLevels maxLogLevel = Message);

    void SetMaxLogLevel(logLevels maxLogLevel);
}