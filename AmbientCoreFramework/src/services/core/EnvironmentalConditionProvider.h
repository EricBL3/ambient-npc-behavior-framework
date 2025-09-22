#pragma once
#include <cstdint>

#include "../include/BehaviorFrameworkInterface.h"
#include "interfaces/IEnvironmentalConditionProvider.h"

namespace AmbientCharacterBehavior {
class EnvironmentalConditionProvider : public IEnvironmentalConditionProvider {
public:
    int32_t QueryEnvironmentalCondition(int32_t condition_key) override
    {
        return AmbientCharacterBehavior::QueryEnvironmentalCondition(condition_key);
    }
};
}
