#pragma once
#include <vector>

#include "utils/StateOperationTarget.h"

namespace AmbientCharacterBehavior {

/**
 * @brief Class for evaluating conditions and modifying state in the behavior framework.
 */
class StateOperation {
private:

    StateOperationTarget target_id;

    int32_t state_key;

    int32_t operation;

    std::vector<int32_t> parameters;

public:

    explicit StateOperation(StateOperationTarget target_id, int32_t state_key, int32_t operation, std::vector<int32_t> parameters) :
        target_id(target_id), state_key(state_key), operation(operation), parameters(std::move(parameters)) {}

    StateOperationTarget GetTargetId() const { return target_id; }
    int32_t GetStateKey() const { return state_key; }
    int32_t GetOperation() const { return operation; }
    const std::vector<int32_t>& GetParameters() const { return parameters; }
};

}
