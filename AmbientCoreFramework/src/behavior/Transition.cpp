#include "Transition.h"

using namespace AmbientCharacterBehavior;

Transition::Transition(int32_t transition_id, int32_t to_node_index) :
    transition_id(transition_id), destination_node_id(to_node_index)
{
    if (transition_id < 0)
    {
        throw std::invalid_argument("Transition: transition_id cannot be negative");
    }

    if (to_node_index < 0)
    {
        throw std::invalid_argument("Transition: destination_node_id cannot be negative");
    }
}

Transition::Transition(int32_t transition_id, int32_t to_node_index, std::unordered_map<StateOperationTarget,
    std::vector<StateOperation>> preconditions_by_target) :
    Transition(transition_id, to_node_index)
{
    this->preconditions_by_target = std::move(preconditions_by_target);
}

void Transition::AddPrecondition(StateOperationTarget target, const StateOperation& precondition)
{
    preconditions_by_target[target].emplace_back(precondition);
}

const std::vector<StateOperation> * Transition::GetPreconditionsForTarget(StateOperationTarget target) const
{
    auto iterator = preconditions_by_target.find(target);

    return iterator == preconditions_by_target.end() ? nullptr : &iterator->second;
}
