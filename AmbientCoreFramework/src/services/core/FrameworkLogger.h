#pragma once

#include <iosfwd>
#include <fstream>
#include <memory>
#include <string>

#include "utils/FrameworkLogLevel.h"
#include "interfaces/ILogger.h"


namespace AmbientCharacterBehavior {
/**
 * @brief Allows components within the framework to log any information.
 */
class FrameworkLogger : public ILogger {
private:
    FrameworkLogLevel log_level;
    std::unique_ptr<std::ofstream> log_file;
    bool is_logging_enabled;

public:
    /**
     * @brief
     * @param log_file_path Optional parameter
     * @throw std::runtime_error if the log_file_path was provided and the file couldn't be opened.
     */
    explicit FrameworkLogger(const std::string& log_file_path = "");
    ~FrameworkLogger() override = default;


    void LogError(const std::string& message, const std::string& component_name) override
    { WriteLog(FrameworkLogLevel::ERROR, message, component_name); }

    void LogWarning(const std::string& message, const std::string& component_name) override
    { WriteLog(FrameworkLogLevel::WARNING, message, component_name); }

    void LogInfo(const std::string& message, const std::string& component_name) override
    { WriteLog(FrameworkLogLevel::INFO, message, component_name); }

    /**
     * @param level Can be INFO, WARNING, or ERROR
     */
    void SetLogLevel(FrameworkLogLevel level) override
    { log_level = level; }

    void SetLoggingEnabled(bool enabled) override
    { is_logging_enabled = enabled; }

private:
    void WriteLog(FrameworkLogLevel level, const std::string& message, const std::string& component_name);
};
}
