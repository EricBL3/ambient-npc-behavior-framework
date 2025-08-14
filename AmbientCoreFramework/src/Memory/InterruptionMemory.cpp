/*
* InterruptionMemory.cpp
 *
 * Implementation of interruption context storage for ambient character behavioral continuity.
 * Stores action state to enable characters to resume behaviors after handling interruptions
 * (emergencies, player interactions, etc.).
 *
 * Author: Eric Buitrón López
 * Created: 8/12/2025
 */

#include "InterruptionMemory.h"

// =============================================================================
// CONSTRUCTION
// =============================================================================

InterruptionMemory::InterruptionMemory(
    int interrupted_action_id,
    int interrupted_sequence_id,
    int interrupted_sequence_node_id,
    int interrupted_target_entity_id,
    int time
) : IMemory(time)
{
    if(interrupted_action_id < 0)
    {
        throw std::invalid_argument("InterruptionMemory: interrupted_action_id cannot be negative, got " + std::to_string(interrupted_action_id));
    }

    if(interrupted_sequence_id < 0)
    {
        throw std::invalid_argument("InterruptionMemory: interrupted_sequence_id cannot be negative, got " + std::to_string(interrupted_sequence_id));
    }

    if(interrupted_sequence_node_id < 0)
    {
        throw std::invalid_argument("InterruptionMemory: interrupted_sequence_node_id cannot be negative, got " + std::to_string(interrupted_sequence_node_id));
    }

    if(interrupted_target_entity_id < -1)
    {
        throw std::invalid_argument("InterruptionMemory: interrupted_target_entity_id cannot be less than -1 (null/none), got " + std::to_string(interrupted_target_entity_id));
    }

    this->interrupted_action_id = interrupted_action_id;
    this->interrupted_sequence_id = interrupted_sequence_id;
    this->interrupted_sequence_node_id = interrupted_sequence_node_id;
    this->interrupted_target_entity_id = interrupted_target_entity_id;
}

// =============================================================================
// MEMORY MATCHING (Framework Core Functionality)
// =============================================================================

// The interruption memory must match in the action_id, sequence_id and sequence_node_id properties.
bool InterruptionMemory::MatchesMemory(const IMemory &other) const
{
    const auto other_transition_memory = dynamic_cast<const InterruptionMemory*>(&other);
    if(!other_transition_memory)
    {
        return false;
    }

    return MatchesMemory(other_transition_memory->interrupted_action_id,
        other_transition_memory->interrupted_sequence_id,
        other_transition_memory->interrupted_sequence_node_id);
}

bool InterruptionMemory::MatchesMemory(int other_action_id, int other_sequence_id, int other_sequence_node_id) const
{
    return this->interrupted_action_id == other_action_id &&
        this->interrupted_sequence_id == other_sequence_id &&
        this->interrupted_sequence_node_id == other_sequence_node_id;
}

// =============================================================================
// CONTEXT DATA ACCESS
// =============================================================================

int InterruptionMemory::GetInterruptedActionId() const
{
    return interrupted_action_id;
}

int InterruptionMemory::GetInterruptedSequenceId() const
{
    return interrupted_sequence_id;
}

int InterruptionMemory::GetInterruptedSequenceNodeId() const
{
    return interrupted_sequence_node_id;
}

int InterruptionMemory::GetInterruptedTargetEntityId() const
{
    return interrupted_target_entity_id;
}


