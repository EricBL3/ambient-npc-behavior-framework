#pragma once
#include "services/interfaces/IStateOperationEvaluator.h"
#include <gmock/gmock.h>

namespace AmbientCharacterBehavior {
class MockStateOperationEvaluator : public IStateOperationEvaluator {
public:
    MOCK_METHOD(bool, ProcessStateOperation, (StateOperation state_operation, FrameworkEntity* target_entity), (override));
};
}