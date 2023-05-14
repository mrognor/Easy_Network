#pragma once

#include <iostream>

#if defined WIN32 || defined _WIN64
#include <winsock2.h>
#include <Windows.h>
#endif

#include <string>

#ifndef DISABLE_LOGGER
    #define LOG(logLevel, messageString)             \
        if (EN::LogFunc == nullptr)                      \
            DefaultLogFunc(logLevel, messageString); \
        else                                         \
            EN::LogFunc(logLevel, messageString);
#else
    #define LOG(logLevel, messageString)
#endif

#include "EN_Functions.h"

namespace EN
{
    /// Logging levels
    enum logLevels
    {
        Message, ///< Message. A simple message with some debugging information
        Hint, ///< Hint. A hint for correcting errors. All hints are created manually
        Warning, ///< Warning. Warning message about possible errors
        Error ///< Error. Critical error messages
    };

    /**
        \brief Maximum logging level

        Using this variable, you can disable the output of unnecessary messages.
        0 level does not disable anything.
        1 level disables messages and prompts.
        2 level disables the same as the first level plus warnings.
        3 level disables the same as the second level plus errors.
        If you have not added your own levels, then this level is equivalent to completely disabling all logging.
    */
    extern int MaxLogLevel;

    /// Pointer to logging function
    extern void (*LogFunc)(logLevels, std::string);

    /// \brief Default logging function
    /// \param [in] logLevel The type of this message
    /// \param [in] logMessage The message itself
    void DefaultLogFunc(logLevels logLevel, std::string logMessage);

    /**
        \brief Set the logging function and optionally max log level
        
        \param [in] logFunc A pointer to the logging function. 
        The function must accept a parameter of type logLevels and std::string. 
        The message level will be passed to the logLevels type parameter. 
        The message itself is passed to the std::string type parameter
        \param [in] maxLogLevel Optional parametr to set maximal logging level. 
        By default set to Message what means which means that all messages will be processing
    */
    void SetLogFunc(void (*logFunc)(logLevels, std::string), logLevels maxLogLevel = Message);

    /// Set maximal log level
    void SetMaxLogLevel(logLevels maxLogLevel);
}