#pragma once
#include "behavior/StateOperation.h"

namespace AmbientCharacterBehavior {
/**
 * @brief Handles the processing of all state operations.
 */
class IStateOperationEvaluator {
public:
    virtual ~IStateOperationEvaluator() = default;
    virtual bool ProcessStateOperation(StateOperation state_operation) = 0;
};
}
