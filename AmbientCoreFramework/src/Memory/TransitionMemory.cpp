/*
* TransitionMemory.cpp
 *
 * Implementation of transition decision tracking for ambient character behavioral variety.
 * Stores which sequence nodes characters have visited to enable recency-based selection.
 *
 * Author: Eric Buitrón López
 * Created: 8/11/2025
 */

#include "TransitionMemory.h"

// =============================================================================
// CONSTRUCTION
// =============================================================================

TransitionMemory::TransitionMemory(int node_id, int time) : IMemory(time)
{
    if(node_id < 0)
    {
        throw std::invalid_argument("TransitionMemory: node_id cannot be negative, got " + std::to_string(node_id));
    }
    
    target_node_id = node_id;
    last_used_time = time;
}

// =============================================================================
// MEMORY MATCHING
// =============================================================================

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

// =============================================================================
// DATA ACCESS
// =============================================================================

int TransitionMemory::GetTargetNodeId() const
{
    return this->target_node_id;
}


