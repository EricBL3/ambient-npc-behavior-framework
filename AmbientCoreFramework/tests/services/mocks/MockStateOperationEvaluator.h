#pragma once
#include "services/interfaces/IStateOperationEvaluator.h"
#include <gmock/gmock.h>

namespace AmbientCharacterBehavior {
class MockStateOperationEvaluator : public IStateOperationEvaluator {
public:
    MOCK_METHOD(bool, ProcessStateOperation, (StateOperation state_operation, StateOperationContext context), (override));
    MOCK_METHOD(bool, IsComparisonOperation, (StateOperationType operation), (override));
    MOCK_METHOD(bool, IsModificationOperation, (StateOperationType operation), (override));
};
}