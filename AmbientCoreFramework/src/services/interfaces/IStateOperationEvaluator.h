#pragma once
#include "behavior/StateOperation.h"
#include "behavior/StateOperationContext.h"

namespace AmbientCharacterBehavior {
/**
 * @brief Handles the processing of all state operations.
 */
class IStateOperationEvaluator {
public:
    virtual ~IStateOperationEvaluator() = default;
    virtual bool ProcessStateOperation(StateOperation state_operation, StateOperationContext context) = 0;
    virtual bool IsComparisonOperation(StateOperationType operation) = 0;
    virtual bool IsModificationOperation(StateOperationType operation) = 0;
};
}
