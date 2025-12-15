
#include "TransitionMemory.h"

using namespace AmbientCharacterBehavior;

/**
 * @throws std::invalid_argument if sequence_id < 0 or node_id < 0 or time < 0
 */
TransitionMemory::TransitionMemory(int32_t sequence_id, int32_t node_id, int64_t time) : BaseMemory(time),
    sequence_id(sequence_id), target_node_id(node_id)
{
    if(sequence_id < 0)
    {
        throw std::invalid_argument("TransitionMemory: sequence_id cannot be negative, got " + std::to_string(sequence_id));
    }

    if(node_id < 0)
    {
        throw std::invalid_argument("TransitionMemory: node_id cannot be negative, got " + std::to_string(node_id));
    }
}

bool TransitionMemory::MatchesMemory(int32_t other_sequence_id, int32_t other_node_id) const
{
    return this->sequence_id == other_sequence_id && this->target_node_id == other_node_id;
}

bool TransitionMemory::MatchesMemory(const BaseMemory& other) const
{
    const auto other_transition_memory {dynamic_cast<const TransitionMemory*>(&other)};
    if(!other_transition_memory)
    {
        return false;
    }

    return MatchesMemory(other_transition_memory->sequence_id, other_transition_memory->target_node_id);
}