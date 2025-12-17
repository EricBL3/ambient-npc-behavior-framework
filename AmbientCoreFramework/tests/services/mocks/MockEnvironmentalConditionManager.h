#pragma once
#include "services/interfaces/IEnvironmentalConditionManager.h"
#include <gmock/gmock.h>

namespace AmbientCharacterBehavior {
class MockEnvironmentalConditionManager : public IEnvironmentalConditionManager {
public:
    MOCK_METHOD(bool, RegisterEnvironmentalConditions, (const std::string& config_file_path), (override));
    MOCK_METHOD(int32_t, GetEnvironmentalConditionKey, (const std::string& name), (override));
    MOCK_METHOD(std::string, GetEnvironmentalConditionName, (int32_t key), (override));
    MOCK_METHOD(void, UpdateEnvironmentalCondition, (int32_t condition_key), (override));
    MOCK_METHOD((int32_t const), GetEnvironmentalConditionValue, (int32_t condition_key), (override));
};
}