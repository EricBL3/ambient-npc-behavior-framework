#pragma once
#include "behavior/StateOperation.h"
#include "entity/FrameworkEntity.h"

namespace AmbientCharacterBehavior {
/**
 * @brief Handles the processing of all state operations.
 */
class IStateOperationEvaluator {
public:
    virtual ~IStateOperationEvaluator() = default;
    virtual bool ProcessStateOperation(StateOperation state_operation, FrameworkEntity* target_entity = nullptr) = 0;
};
}
