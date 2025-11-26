#pragma once
#include "services/interfaces/ILogger.h"
#include <gmock/gmock.h>


namespace AmbientCharacterBehavior {
class MockLogger : public ILogger {
public:
    MOCK_METHOD(bool, Initialize, (const std::string& log_file_path, FrameworkLogLevel log_level), (override));
    MOCK_METHOD(void, LogError, (const std::string& message, const std::string& component_name), (override));
    MOCK_METHOD(void, LogWarning, (const std::string& message, const std::string& component_name), (override));
    MOCK_METHOD(void, LogInfo, (const std::string& message, const std::string& component_name), (override));
    MOCK_METHOD(void, SetLogLevel, (FrameworkLogLevel level), (override));
    MOCK_METHOD(void, SetLoggingEnabled, (bool enabled), (override));
    MOCK_METHOD(bool, IsLoggingEnabled, (), (const, override));
};
}
