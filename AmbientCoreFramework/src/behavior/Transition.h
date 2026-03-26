#pragma once
#include <cstdint>
#include <unordered_map>
#include <vector>
#include "sequence_nodes/SequenceNode.h"
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
    int32_t destination_node_id;
    std::unordered_map<StateOperationTarget, std::vector<StateOperation>> preconditions_by_target;

public:

    /**
     * @throw std::invalid_argument if transition_id or to_node_index < 0
     */
    explicit Transition(int32_t transition_id, int32_t to_node_index);

    /**
     * @throw std::invalid_argument if transition_id or to_node_index < 0
     */
    explicit Transition(int32_t transition_id, int32_t to_node_index, std::unordered_map<StateOperationTarget,
        std::vector<StateOperation>> preconditions_by_target);

    void AddPrecondition(StateOperationTarget target, const StateOperation& precondition);

    int32_t GetDestinationNodeId() const { return destination_node_id; }

    const std::vector<StateOperation>* GetPreconditionsForTarget(StateOperationTarget target) const;

    std::unordered_map<StateOperationTarget, std::vector<StateOperation>> GetAllPreconditions() const
    { return preconditions_by_target; }

    int32_t GetTransitionId() const { return transition_id; }

};

}