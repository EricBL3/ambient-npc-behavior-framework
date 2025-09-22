
#include "TransitionMemory.h"

using namespace AmbientCharacterBehavior;

/**
 * @throws std::invalid_argument if node_id < 0 or time < 0
 */
TransitionMemory::TransitionMemory(int32_t node_id, int64_t time) : BaseMemory(time), target_node_id(node_id)
{
    if(node_id < 0)
    {
        throw std::invalid_argument("TransitionMemory: node_id cannot be negative, got " + std::to_string(node_id));
    }
}

bool TransitionMemory::MatchesMemory(int other_node_id) const
{
    return target_node_id == other_node_id;
}

bool TransitionMemory::MatchesMemory(const BaseMemory& other) const
{
    // Safe type conversion that returns nullptr if other is not a TransitionMemory
    const auto other_transition_memory {dynamic_cast<const TransitionMemory*>(&other)};
    if(!other_transition_memory)
    {
        return false;
    }

    return MatchesMemory(other_transition_memory->target_node_id);
}

int32_t TransitionMemory::GetTargetNodeId() const
{
    return target_node_id;
}