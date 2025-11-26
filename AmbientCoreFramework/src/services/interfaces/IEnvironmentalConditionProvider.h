#pragma once
#include <cstdint>

namespace AmbientCharacterBehavior {

// The callback type that external systems must provide
using QueryEnvironmentalConditionCallback = int32_t(*)(int32_t);

class IEnvironmentalConditionProvider {
public:
    virtual ~IEnvironmentalConditionProvider() = default;

    virtual int32_t QueryEnvironmentalCondition(int32_t key) = 0;
};
}