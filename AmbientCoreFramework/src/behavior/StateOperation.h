#pragma once
#include <vector>

#include "enums/StateOperationTarget.h"
#include "enums/StateOperationType.h"

namespace AmbientCharacterBehavior {

/**
 * @brief Class for evaluating conditions and modifying state in the behavior framework.
 */
class StateOperation {
private:

    StateOperationTarget target_id;

    int32_t state_key;

    StateOperationType operation_type;

    int32_t value;

public:

    explicit StateOperation(StateOperationTarget target_id, int32_t state_key, StateOperationType operation, int32_t value) :
        target_id(target_id), state_key(state_key), operation_type(operation), value(value) {}

    StateOperationTarget GetTarget() const { return target_id; }
    int32_t GetStateKey() const { return state_key; }
    StateOperationType GetOperationType() const { return operation_type; }
    int32_t GetValue() const { return value; }
};

}
