#include "EN_Logger.h"

std::ofstream LogLevelInfoFile;
std::ofstream LogLevelHintFile;
std::ofstream LogLevelWarningFile;
std::ofstream LogLevelErrorFile;

std::mutex LoggerMtx;

class FileLogger
{
public:
    ~FileLogger()
    {
        LoggerMtx.lock();

        LogLevelInfoFile.close();
        LogLevelHintFile.close();
        LogLevelWarningFile.close();
        LogLevelErrorFile.close();

        LoggerMtx.unlock();
    }
};

std::string LogLevelInfoFileName;
std::string LogLevelHintFileName;
std::string LogLevelWarningFileName;
std::string LogLevelErrorFileName;

FileLogger FL;

namespace EN
{
    int EnabledLogLevels = ENABLE_LOG_LEVEL_INFO | ENABLE_LOG_LEVEL_HINT | ENABLE_LOG_LEVEL_WARNING | ENABLE_LOG_LEVEL_ERROR;
    void (*LogFunc)(LogLevels, std::string) = DefaultLogFunc;

    void DefaultLogFunc(LogLevels logLevel, std::string logMessage)
    {
        switch (logLevel)
        {
        case Info:
            if (EnabledLogLevels & ENABLE_LOG_LEVEL_INFO)
            {
                LoggerMtx.lock();
                std::string msg = GetCurrentDate() + " " + GetCurrentDayTimeWithSecondFraction<std::chrono::milliseconds>() + " [info] " + logMessage;
                std::cerr << msg << std::endl;
                if (!LogLevelInfoFileName.empty())
                    LogLevelInfoFile << msg << std::endl;
            }
            break;

        case Hint:
            if (EnabledLogLevels & ENABLE_LOG_LEVEL_HINT)
            {
                LoggerMtx.lock();
                std::string msg = GetCurrentDate() + " " + GetCurrentDayTimeWithSecondFraction<std::chrono::milliseconds>() + " [hint] " + logMessage;
                std::cerr << "\x1B[94m" << msg << "\033[0m" << std::endl;
                if (!LogLevelHintFileName.empty())
                    LogLevelHintFile << msg << std::endl;
            }
            break;

        case Warning:
            if (EnabledLogLevels & ENABLE_LOG_LEVEL_WARNING)
            {
                LoggerMtx.lock();
                std::string msg = GetCurrentDate() + " " + GetCurrentDayTimeWithSecondFraction<std::chrono::milliseconds>() + " [warning] " + logMessage;
                std::cerr << "\x1B[33m" << msg << "\033[0m" << std::endl;
                if (!LogLevelWarningFileName.empty())
                    LogLevelWarningFile << msg << std::endl;
            }
            break;

        case Error:
            if (EnabledLogLevels & ENABLE_LOG_LEVEL_ERROR)
            {
                LoggerMtx.lock();
                std::string msg = GetCurrentDate() + " " + GetCurrentDayTimeWithSecondFraction<std::chrono::milliseconds>() + " [error] " + logMessage;
                std::cerr << "\x1B[31m" << msg << "\033[0m" << std::endl;
                if (!LogLevelErrorFileName.empty())
                    LogLevelErrorFile << msg << std::endl;
            }
            break;
        }
        LoggerMtx.unlock();
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

    void SetLogLevelsFile(int logLevelsToEnable, std::string fileName, std::ios_base::openmode openMode)
    {
        if (logLevelsToEnable & ENABLE_LOG_LEVEL_INFO)
        {
            LogLevelInfoFileName = fileName;
            LogLevelInfoFile.close();

            if (!fileName.empty())
            {
                LogLevelInfoFile.open(fileName, openMode);
                if (!LogLevelInfoFile.is_open())
                {
                    LOG(EN::LogLevels::Error, "Failed to create log level info log file with name: " + fileName);
                    LogLevelInfoFileName.clear();
                }
            }
            else LOG(EN::LogLevels::Error, "Can not create log file with empty name");
        }

        if (logLevelsToEnable & ENABLE_LOG_LEVEL_HINT)
        {
            LogLevelHintFileName = fileName;
            LogLevelHintFile.close();

            if (!fileName.empty())
            {
                LogLevelHintFile.open(fileName, openMode);
                if (!LogLevelHintFile.is_open())
                {
                    LOG(EN::LogLevels::Error, "Failed to create log level hint log file with name: " + fileName);
                    LogLevelHintFileName.clear();
                }
            }
            else LOG(EN::LogLevels::Error, "Can not create log file with empty name");
        }

        if (logLevelsToEnable & ENABLE_LOG_LEVEL_WARNING)
        {
            LogLevelWarningFileName = fileName;
            LogLevelWarningFile.close();
            
            if (!fileName.empty())
            {
                LogLevelWarningFile.open(fileName, openMode);
                if (!LogLevelWarningFile.is_open())
                {
                    LOG(EN::LogLevels::Error, "Failed to create log level warning log file with name: " + fileName);
                    LogLevelWarningFileName.clear();
                }
            }
            else LOG(EN::LogLevels::Error, "Can not create log file with empty name");
        }

        if (logLevelsToEnable & ENABLE_LOG_LEVEL_ERROR)
        {
            LogLevelErrorFileName = fileName;
            LogLevelErrorFile.close();

            if (!fileName.empty())
            {
                LogLevelErrorFile.open(fileName, openMode);
                if (!LogLevelErrorFile.is_open())
                {
                    LOG(EN::LogLevels::Error, "Failed to create log level error log file with name: " + fileName);
                    LogLevelErrorFileName.clear();
                }
            }
            else LOG(EN::LogLevels::Error, "Can not create log file with empty name");
        }

        if (!(logLevelsToEnable & ENABLE_LOG_LEVEL_INFO) && !(logLevelsToEnable & ENABLE_LOG_LEVEL_HINT) &&
        !(logLevelsToEnable & ENABLE_LOG_LEVEL_WARNING) && !(logLevelsToEnable & ENABLE_LOG_LEVEL_ERROR))
        {
            LOG(EN::LogLevels::Warning, "Wrong log levels to log files");
        }
    }
}