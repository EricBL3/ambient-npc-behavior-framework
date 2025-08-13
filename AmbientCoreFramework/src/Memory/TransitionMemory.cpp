//
// Created by Eric on 8/11/2025.
//

#include "TransitionMemory.h"

TransitionMemory::TransitionMemory(int node_id, int time) : IMemory(time)
{
    if(node_id < 0)
    {
        throw std::invalid_argument("TransitionMemory: node_id cannot be negative, got " + std::to_string(node_id));
    }
    
    target_node_id = node_id;
    last_used_time = time;
}

// The transition memory must match on target_node_id.
bool TransitionMemory::MatchesMemory(int other_node_id) const
{
    return this->target_node_id == other_node_id;
}


bool TransitionMemory::MatchesMemory(const IMemory& other) const
{
    const auto other_transition_memory = dynamic_cast<const TransitionMemory*>(&other);
    if(!other_transition_memory)
    {
        return false;
    }

    return MatchesMemory(other_transition_memory->target_node_id);
}

int TransitionMemory::GetTargetNodeId() const
{
    return this->target_node_id;
}


