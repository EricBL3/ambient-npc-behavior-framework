#pragma once
#include <cstdint>
#include <vector>

#include "SequenceNode.h"
#include "StateOperation.h"

namespace AmbientCharacterBehavior {

/**
 * @brief This class represents the conditional connection that exists between sequence nodes in a sequence.
 *
 * @see Sequence
 */
class Transition {
private:

    int32_t transition_id;
    int32_t destination_node_index;
    std::vector<StateOperation> preconditions;

public:

    /**
     * @throw std::invalid_argument if transition_id or to_node_index < 0
     */
    explicit Transition(int32_t transition_id, int32_t to_node_index, size_t initial_preconditions_count = 0);

    /**
     * @throw std::invalid_argument if transition_id or to_node_index < 0
     */
    explicit Transition(int32_t transition_id, int32_t to_node_index, std::vector<StateOperation> preconditions);

    void AddPrecondition(const StateOperation& precondition);

    int32_t GetDestinationNodeIndex() const { return destination_node_index; }

    const std::vector<StateOperation>& GetPreconditions() const { return preconditions; }

    int32_t GetTransitionId() const { return transition_id; }

};

}