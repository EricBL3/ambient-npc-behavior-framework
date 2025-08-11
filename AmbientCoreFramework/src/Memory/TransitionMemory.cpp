#include "TransitionMemory.h"
#include <stdexcept>
#include <string> 

TransitionMemory::TransitionMemory(int node_id, int time)
{
    if(node_id < 0)
    {
        throw std::invalid_argument("TransitionMemory: node_id cannot be negative, got " + std::to_string(node_id));
    }

    if(time < 0)
    {
        throw std::invalid_argument("TransitionMemory: time cannot be negative, got " + std::to_string(time));
    }
    
    target_node_id = node_id;
    last_used_time = time;
}

bool TransitionMemory::MatchesMemory(const IMemory& other) const
{
    const TransitionMemory* other_transition_memory = dynamic_cast<const TransitionMemory*>(&other);
    if(!other_transition_memory)
    {
        return false;
    }

    return this->target_node_id == other_transition_memory->target_node_id;
}

int TransitionMemory::GetLastUsedTime() const
{
    return last_used_time;
}

bool TransitionMemory::IsOlderThan(const IMemory& other) const
{
    return GetLastUsedTime() < other.GetLastUsedTime();
}

int TransitionMemory::GetTargetNodeId() const
{
    return target_node_id;
}


