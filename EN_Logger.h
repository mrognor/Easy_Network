#pragma once

#include <iostream>
#include <string>
#include <fstream>

#if defined WIN32 || defined _WIN64
#include <winsock2.h>
#include <Windows.h>
#endif

#include "EN_Functions.h"

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
        Use EnableLogLevels functions to turn on only required log levels.
    */
    extern int EnabledLogLevels;

    /// Pointer to logging function
    extern void (*LogFunc)(LogLevels, std::string);

    /// \brief Default logging function. Write all to cerr
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

        \param [in] logLevelsToEnable Parametr for fine-tuning the required logging levels.
        Possible values: ENABLE_LOG_LEVEL_INFO, ENABLE_LOG_LEVEL_HINT, ENABLE_LOG_LEVEL_WARNING, ENABLE_LOG_LEVEL_ERROR.
        Using the logical operator |, you can combine the necessary logging levels.
    */
    void EnableLogLevels(int logLevelsToEnable);

    /**
        \brief Enable file log levels.

        \param [in] logLevelsToEnable Parametr for fine-tuning the required logging levels.
        Possible values: ENABLE_LOG_LEVEL_INFO, ENABLE_LOG_LEVEL_HINT, ENABLE_LOG_LEVEL_WARNING, ENABLE_LOG_LEVEL_ERROR.
        Using the logical operator |, you can combine the necessary logging levels.
        \param [in] fileName The name of the file to record this logging level
        \param [in] openMode Optional parametr to open log files. Works like standart fstream open. 
    */
    void SetLogLevelsFile(int logLevelsToEnable, std::string fileName, std::ios_base::openmode openMode = (std::ios_base::openmode)16);
}