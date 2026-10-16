#pragma once
#include <string>
#include <nlohmann/json.hpp>
#include "services/layers/0_foundation/FrameworkLogLevel.h"

namespace AmbientCharacterBehavior {

/**
 * @brief Allows components to log any information.
 */
class ILogger {
public:
    virtual ~ILogger() = default;
    virtual bool Initialize(const std::string& log_file_path, FrameworkLogLevel log_level, bool enable_metrics) = 0;
    virtual void LogError(const std::string& message, const std::string& component_name) = 0;
    virtual void LogWarning(const std::string& message, const std::string& component_name) = 0;
    virtual void LogInfo(const std::string& message, const std::string& component_name) = 0;
    virtual void LogDebug(const std::string& message, const std::string& component_name) = 0;
    virtual void LogMetric(const nlohmann::json& event) = 0;
    virtual void SetLogLevel(FrameworkLogLevel level) = 0;
    virtual void SetLoggingEnabled(bool enabled) = 0;
    virtual bool IsLoggingEnabled() const = 0;
    virtual void SetLoggingMetricsEnabled(bool enabled) = 0;
    virtual bool IsLoggingMetricsEnabled() const = 0;
};

}
