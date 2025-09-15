#pragma once
#include <cstdint>
#include <string>

namespace AmbientCharacterBehavior {
class IEnvironmentalConditionManager {
public:
    virtual ~IEnvironmentalConditionManager() = default;
    virtual void RegisterEnvironmentalConditions(const std::string& config_file_path) = 0;
    virtual int32_t GetEnvironmentalConditionKey(const std::string& state_name) = 0;
    virtual std::string GetEnvironmentalConditionName(int32_t state_key) = 0;
    virtual void UpdateEnvironmentalCondition(int32_t condition_key) = 0;
    virtual int32_t const GetEnvironmentalConditionValue(int32_t condition_key) = 0;
};
}
