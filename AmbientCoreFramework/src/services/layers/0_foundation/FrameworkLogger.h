#pragma once
#include <iosfwd>
#include <fstream>
#include <memory>
#include <string>
#include "FrameworkLogLevel.h"
#include "services/interfaces/ILogger.h"


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

    explicit FrameworkLogger(const std::string& log_file_path = "");
    ~FrameworkLogger() override = default;

    /**
     * @throw std::runtime_error if the log_file_path was provided and the file couldn't be opened.
     */
    bool Initialize(const std::string& log_file_path, FrameworkLogLevel start_log_level) override;

    void LogError(const std::string& message, const std::string& component_name) override
    { WriteLog(FrameworkLogLevel::ERROR, message, component_name); }

    void LogWarning(const std::string& message, const std::string& component_name) override
    { WriteLog(FrameworkLogLevel::WARNING, message, component_name); }

    void LogInfo(const std::string& message, const std::string& component_name) override
    { WriteLog(FrameworkLogLevel::INFO, message, component_name); }

    void LogDebug(const std::string& message, const std::string& component_name) override
    { WriteLog(FrameworkLogLevel::DEBUG, message, component_name); }

    void LogMetric(const nlohmann::json& event) override
    { WriteLog(FrameworkLogLevel::METRIC, event.dump(), ""); }

    /**
     * @param level Can be INFO, WARNING, or ERROR
     */
    void SetLogLevel(FrameworkLogLevel level) override
    { log_level = level; }

    void SetLoggingEnabled(bool enabled) override
    { is_logging_enabled = enabled; }

    bool IsLoggingEnabled() const override { return is_logging_enabled; }

private:
    void WriteLog(FrameworkLogLevel level, const std::string& message, const std::string& component_name);
};
}
