#pragma once
#include <vector>

namespace AmbientCharacterBehavior {

/**
 * @brief Class for evaluating conditions and modifying state in the behavior framework.
 */
class StateOperation {
private:

    int32_t target_id;

    int32_t state_key;

    int32_t operation;

    std::vector<int32_t> parameters;

public:

    explicit StateOperation(int32_t target_id, int32_t state_key, int32_t operation, std::vector<int32_t> parameters) :
        target_id(target_id), state_key(state_key), operation(operation), parameters(std::move(parameters)) {}

    int32_t GetTargetId() const { return target_id; }
    int32_t GetStateKey() const { return state_key; }
    int32_t GetOperation() const { return operation; }
    const std::vector<int32_t>& GetParameters() const { return parameters; }
};

}