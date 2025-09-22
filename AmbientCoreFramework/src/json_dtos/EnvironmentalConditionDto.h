
#pragma once
#include <cstdint>
#include <string>

namespace AmbientCharacterBehavior {
struct EnvironmentalConditionDto {
    int32_t condition_key;
    std::string name;
    int64_t update_frequency_ms;
};
}
