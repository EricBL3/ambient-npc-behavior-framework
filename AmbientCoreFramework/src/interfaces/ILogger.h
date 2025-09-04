#pragma once
#include <string>

#include "utils/FrameworkLogLevel.h"

namespace AmbientCharacterBehavior {

/**
 * @brief Allows components to log any information.
 */
class ILogger {
public:
    virtual ~ILogger() = default;
    virtual void LogError(const std::string& message, const std::string& component_name) = 0;
    virtual void LogWarning(const std::string& message, const std::string& component_name) = 0;
    virtual void LogInfo(const std::string& message, const std::string& component_name) = 0;
    virtual void SetLogLevel(FrameworkLogLevel level) = 0;
    virtual void SetLoggingEnabled(bool enabled) = 0;
};

}
