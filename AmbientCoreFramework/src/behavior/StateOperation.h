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

    std::vector<int32_t> parameters;

public:

    explicit StateOperation(StateOperationTarget target_id, int32_t state_key, StateOperationType operation, std::vector<int32_t> parameters) :
        target_id(target_id), state_key(state_key), operation_type(operation), parameters(std::move(parameters)) {}

    StateOperationTarget GetTarget() const { return target_id; }
    int32_t GetStateKey() const { return state_key; }
    StateOperationType GetOperationType() const { return operation_type; }
    const std::vector<int32_t>& GetParameters() const { return parameters; }
};

}
