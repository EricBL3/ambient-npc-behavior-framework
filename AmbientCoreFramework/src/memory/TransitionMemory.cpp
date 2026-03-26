#include "TransitionMemory.h"
using namespace AmbientCharacterBehavior;

TransitionMemory::TransitionMemory(int32_t sequence_id, int32_t node_id, int64_t time) : BaseMemory(time),
    sequence_id(sequence_id), target_node_id(node_id)
{
    // Validate sequence_id invariant
    if(sequence_id < 0)
    {
        throw std::invalid_argument("TransitionMemory: sequence_id cannot be negative, got " +
            std::to_string(sequence_id));
    }

    // Validate node_id invariant
    if(node_id < 0)
    {
        throw std::invalid_argument("TransitionMemory: node_id cannot be negative, got " + std::to_string(node_id));
    }
}

bool TransitionMemory::MatchesMemory(const BaseMemory& other) const
{
    // Type safe cast returning nullptr if other is not a TransitionMemory
    const auto other_transition_memory {dynamic_cast<const TransitionMemory*>(&other)};
    if(!other_transition_memory)
    {
        // If it's a different memory type it cannot match
        return false;
    }

    // Delegate to type specific matching
    return MatchesMemory(other_transition_memory->sequence_id, other_transition_memory->target_node_id);
}

bool TransitionMemory::MatchesMemory(int32_t other_sequence_id, int32_t other_node_id) const
{
    return this->sequence_id == other_sequence_id && this->target_node_id == other_node_id;
}