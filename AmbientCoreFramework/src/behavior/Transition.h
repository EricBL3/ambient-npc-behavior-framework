/**
 * @file Transition.h
 * @author Eric Buitrón López
 * @date 8/18/2025
 *
 *
*/

#pragma once
#include <cstdint>
#include <vector>

#include "SequenceNode.h"
#include "integration/StateOperation.h"

namespace AmbientCharacterBehavior {

/**
 * @brief This class represents the conditional connection that exists between sequence nodes in a sequence.
 *
 * @see Sequence
 */
class Transition {
private:
    /**
     * @brief The unique identifier of the transition
     */
    int32_t transition_id;

    /**
     * @brief The identifier of the destination node of this transition
     */
    int32_t to_node_index;

    /**
     * @brief The preconditions that must be satisfied for the transition to be valid
     */
    std::vector<StateOperation> preconditions;

public:

    /**
     *
     * @param transition_id The identifier of the transition
     * @param to_node_index The identifier of the destination node of this transition
     * @param initial_preconditions_count The initial number of preconditions for this transition (default is 0).
     *
     * @throw std::invalid_argument if transition_id or to_node_index < 0
     */
    explicit Transition(int32_t transition_id, int32_t to_node_index, size_t initial_preconditions_count = 0);

    /**
     * @brief Adds a new precondition to the transition.
     * @param precondition The StateOperation that will be used as a precondition for the transition
     *
     */
    void AddPrecondition(const StateOperation& precondition);

    int32_t GetToNodeIndex() const { return to_node_index; }

    const std::vector<StateOperation>& GetPreconditions() const { return preconditions; }

    int32_t GetTransitionId() const { return transition_id; }

};

}