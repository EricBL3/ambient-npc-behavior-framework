/**
 * @file StateOperation.h
 * @author Eric Buitrón López
 * @date 8/18/2025
 *
 *
*/

#pragma once
#include <vector>

/**
 * @brief Class for evaluating conditions and modifying state in the behavior framework.
 */
class StateOperation {
private:
    /**
     * @brief Defines where to access the state
     */
    int target_id;

    /**
     * @brief Defines which state property to access
     */
    int state_key;

    /**
     * @brief Defines the operation to perform
     */
    int operation;

    /**
     * @brief Defines the values that will be used to perform the operation
     */
    std::vector<int> values;

public:
    /**
     *
     * @param target_id The identifier of where to access the state (such as location_id or entity_id)
     * @param state_key The key of the state that will be accessed when performing the operation
     * @param operation The operation that will be performed
     * @param values The values that will be used to perform the operation
     */
    explicit StateOperation(int target_id, int state_key, int operation, std::vector<int> values) :
        target_id(target_id), state_key(state_key), operation(operation), values(std::move(values)) {}

    int GetTargetId() const { return target_id; }
    int GetStateKey() const { return state_key; }
    int GetOperation() const { return operation; }
    const std::vector<int>& GetValues() const { return values; }
};