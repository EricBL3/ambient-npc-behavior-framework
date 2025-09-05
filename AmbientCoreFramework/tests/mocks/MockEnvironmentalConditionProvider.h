#pragma once

#include <gmock/gmock.h>
#include "interfaces/IEnvironmentalConditionProvider.h"

namespace AmbientCharacterBehavior {
class MockEnvironmentalConditionProvider : public IEnvironmentalConditionProvider {
public:
    MOCK_METHOD((int32_t), QueryEnvironmentalCondition, (int32_t condition_key), (override));
};
}
