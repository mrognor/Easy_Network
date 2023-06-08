#pragma once

#include <iostream>

#if defined WIN32 || defined _WIN64
#include <winsock2.h>
#include <Windows.h>
#endif

#include <string>

#ifndef DISABLE_LOGGER
    #define LOG(logLevel, messageString) EN::LogFunc(logLevel, messageString);
#else
    #define LOG(logLevel, messageString)
#endif

// Dont use this. Diable logger using -DDISABLE_LOGGER key when compile project
#define ENABLE_LOG_LEVEL_NONE    0b00000000
#define ENABLE_LOG_LEVEL_INFO    0b00000001
#define ENABLE_LOG_LEVEL_HINT    0b00000010
#define ENABLE_LOG_LEVEL_WARNING 0b00000100
#define ENABLE_LOG_LEVEL_ERROR   0b00001000

#include "EN_Functions.h"

namespace EN
{
    /// Logging levels
    enum LogLevels
    {
        Info, ///< Info. A simple message with some debugging information
        Hint, ///< Hint. A message with hint for correcting errors. All hints are created manually
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
    extern int EnabledLogLevels;

    /// Pointer to logging function
    extern void (*LogFunc)(LogLevels, std::string);

    /// \brief Default logging function
    /// \param [in] logLevel The type of this message
    /// \param [in] logMessage The message itself
    void DefaultLogFunc(LogLevels logLevel, std::string logMessage);

    /**
        \brief Set the logging function and optionally max log level
        
        \param [in] logFunc A pointer to the logging function. 
        The function must accept a parameter of type LogLevels and std::string. 
        The message level will be passed to the LogLevels type parameter. 
        The message itself is passed to the std::string type parameter
        \param [in] logLevelsToEnable Optional parametr for fine-tuning the required logging levels.
        Possible values: ENABLE_LOG_LEVEL_INFO, ENABLE_LOG_LEVEL_HINT, ENABLE_LOG_LEVEL_WARNING, ENABLE_LOG_LEVEL_ERROR.
        Using the logical operator |, you can combine the necessary logging levels.
    */
    void SetLogFunc(void (*logFunc)(LogLevels, std::string), int logLevelsToEnable = (ENABLE_LOG_LEVEL_INFO | ENABLE_LOG_LEVEL_HINT | ENABLE_LOG_LEVEL_WARNING | ENABLE_LOG_LEVEL_ERROR));

    /**
        \brief Enable log levels.

        \param [in] logLevelsToEnable Optional parametr for fine-tuning the required logging levels.
        Possible values: ENABLE_LOG_LEVEL_INFO, ENABLE_LOG_LEVEL_HINT, ENABLE_LOG_LEVEL_WARNING, ENABLE_LOG_LEVEL_ERROR.
        Using the logical operator |, you can combine the necessary logging levels.
    */
    void EnableLogLevels(int logLevelsToEnable);
}