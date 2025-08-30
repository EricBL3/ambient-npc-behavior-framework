/**
* @file InterruptionMemory.cpp
 * @brief Implementation of interruption context storage for ambient character behavioral continuity
 * @author Eric Buitrón López
 * @date 8/12/2025
 *
 */

#include "InterruptionMemory.h"

using namespace AmbientCharacterBehavior;

// =============================================================================
// CONSTRUCTION
// =============================================================================

/**
 * @brief Constructs interruption memory with complete execution context
 * @param interrupted_action_id Unique identifier of the action that was interrupted
 * @param interrupted_sequence_id Unique identifier of the sequence that was executing
 * @param interrupted_sequence_node_id Unique identifier of the sequence node that was interrupted
 * @param interrupted_target_entity_id Unique identifier of the entity involved (-1 if none)
 * @param time Simulation timestamp when the interruption occurred
 * @throws std::invalid_argument if any ID < 0 (except entity_id which allows -1)
 */
InterruptionMemory::InterruptionMemory(
    int32_t interrupted_action_id,
    int32_t interrupted_sequence_id,
    int32_t interrupted_sequence_node_id,
    int32_t interrupted_target_entity_id,
    int64_t time
) : BaseMemory(time), interrupted_action_id(interrupted_action_id), interrupted_sequence_id(interrupted_sequence_id),
    interrupted_sequence_node_id(interrupted_sequence_node_id), interrupted_target_entity_id(interrupted_target_entity_id)
{
    if(interrupted_action_id < 0)
    {
        throw std::invalid_argument("InterruptionMemory: interrupted_action_id cannot be negative, got " +
            std::to_string(interrupted_action_id));
    }

    if(interrupted_sequence_id < 0)
    {
        throw std::invalid_argument("InterruptionMemory: interrupted_sequence_id cannot be negative, got " +
            std::to_string(interrupted_sequence_id));
    }

    if(interrupted_sequence_node_id < 0)
    {
        throw std::invalid_argument("InterruptionMemory: interrupted_sequence_node_id cannot be negative, got " +
            std::to_string(interrupted_sequence_node_id));
    }

    if(interrupted_target_entity_id < -1)
    {
        throw std::invalid_argument("InterruptionMemory: interrupted_target_entity_id cannot be less than -1 (null/none), got " +
            std::to_string(interrupted_target_entity_id));
    }
}

// =============================================================================
// MEMORY MATCHING
// =============================================================================

/**
 * @brief Checks if this memory matches another memory instance polymorphically
 *
 * Implements the BaseMemory interface by safely checking if the other memory
 * is also an InterruptionMemory with the same execution context (action_id,
 * sequence_id, and sequence_node_id). Uses dynamic_cast for type safety.
 *
 * @param other Memory instance to compare against
 * @return true if other is InterruptionMemory with same execution context
 */
bool InterruptionMemory::MatchesMemory(const BaseMemory &other) const
{
    // Safe type conversion - returns nullptr if 'other' is not an InterruptionMemory
    const auto other_transition_memory (dynamic_cast<const InterruptionMemory*>(&other));
    if(!other_transition_memory)
    {
        return false;
    }

    return MatchesMemory(other_transition_memory->GetInterruptedActionId(),
        other_transition_memory->GetInterruptedSequenceId(),
        other_transition_memory->GetInterruptedSequenceNodeId());
}

/**
 * @brief Checks if this memory matches the provided parameters.
 *
 * Alternative way of checking if a memory matches if full memory object is not available.
 *
 * @param other_action_id Action identifier to compare against
 * @param other_sequence_id Sequence identifier to compare against
 * @param other_sequence_node_id Sequence node identifier to compare against
 * @return true if all three context identifiers match
 *
 */
bool InterruptionMemory::MatchesMemory(int32_t other_action_id, int32_t other_sequence_id, int32_t other_sequence_node_id) const
{
    return this->interrupted_action_id == other_action_id &&
        this->interrupted_sequence_id == other_sequence_id &&
        this->interrupted_sequence_node_id == other_sequence_node_id;
}

// =============================================================================
// CONTEXT DATA ACCESS
// =============================================================================

int32_t InterruptionMemory::GetInterruptedActionId() const
{
    return interrupted_action_id;
}

int32_t InterruptionMemory::GetInterruptedSequenceId() const
{
    return interrupted_sequence_id;
}

int32_t InterruptionMemory::GetInterruptedSequenceNodeId() const
{
    return interrupted_sequence_node_id;
}

int32_t InterruptionMemory::GetInterruptedTargetEntityId() const
{
    return interrupted_target_entity_id;
}


