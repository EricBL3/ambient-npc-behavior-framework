/**
 * @file FrameworkLogger.h
 * @brief 
 * @author Eric Buitrón López
 * @date 9/1/2025
 *
 *
*/

#pragma once

#include <iosfwd>
#include <string>

#include "FrameworkLogLevel.h"


namespace AmbientCharacterBehavior {
/**
 * @brief Allows components within the framework to log any information.
 */
class FrameworkLogger {
private:
    static FrameworkLogLevel log_level;

    static std::ofstream log_file;

    static bool is_logging_enabled;

public:
    /**
     * @brief Initializes the logger to append to the logging file with the default log_level of INFO.
     * @param log_file_path The path to the logging file
     * @throw std::runtime_error if the file couldn't be opened.
     */
    static void Initialize(const std::string& log_file_path);

    static void LogError(const std::string& message, const std::string& component_name)
    {
        WriteLog(FrameworkLogLevel::ERROR, message, component_name);
    }

    static void LogWarning(const std::string& message, const std::string& component_name)
    {
        WriteLog(FrameworkLogLevel::WARNING, message, component_name);
    }

    static void LogInfo(const std::string& message, const std::string& component_name)
    {
        WriteLog(FrameworkLogLevel::INFO, message, component_name);
    }

    /**
     *
     * @param level The log level. Can be INFO, WARNING, or ERROR
     */
    static void SetLogLevel(FrameworkLogLevel level) { log_level = level; }

    static void SetLoggingEnabled(bool enabled) { is_logging_enabled = enabled; }

private:
    static void WriteLog(FrameworkLogLevel level, const std::string& message, const std::string& component_name);
};
} // AmbientCharacterBehavior
