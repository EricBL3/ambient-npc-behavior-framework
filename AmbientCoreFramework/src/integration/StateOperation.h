/**
 * @file StateOperation.h
 * @author Eric Buitrón López
 * @date 8/18/2025
 *
 *
*/

#pragma once
#include <vector>

namespace AmbientCharacterBehavior {

/**
 * @brief Class for evaluating conditions and modifying state in the behavior framework.
 */
class StateOperation {
private:
    /**
     * @brief Defines where to access the state
     */
    int32_t target_id;

    /**
     * @brief Defines which state property to access
     */
    int32_t state_key;

    /**
     * @brief Defines the operation to perform
     */
    int32_t operation;

    /**
     * @brief Defines the values that will be used to perform the operation
     */
    std::vector<int32_t> values;

public:
    /**
     *
     * @param target_id The identifier of where to access the state (such as location_id or entity_id)
     * @param state_key The key of the state that will be accessed when performing the operation
     * @param operation The operation that will be performed
     * @param values The values that will be used to perform the operation
     */
    explicit StateOperation(int32_t target_id, int32_t state_key, int32_t operation, std::vector<int32_t> values) :
        target_id(target_id), state_key(state_key), operation(operation), values(std::move(values)) {}

    int32_t GetTargetId() const { return target_id; }
    int32_t GetStateKey() const { return state_key; }
    int32_t GetOperation() const { return operation; }
    const std::vector<int32_t>& GetValues() const { return values; }
};

}