#include "Transition.h"

using namespace AmbientCharacterBehavior;

Transition::Transition(int32_t transition_id, int32_t to_node_index, size_t initial_preconditions_count) :
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

    preconditions.reserve(initial_preconditions_count);
}

Transition::Transition(int32_t transition_id, int32_t to_node_index, std::vector<StateOperation> preconditions) :
    Transition(transition_id, to_node_index, preconditions.size())
{
    this->preconditions = std::move(preconditions);
}

void Transition::AddPrecondition(const StateOperation& precondition)
{
    preconditions.emplace_back(precondition);
}
