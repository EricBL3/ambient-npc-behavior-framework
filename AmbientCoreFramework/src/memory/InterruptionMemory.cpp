/**
* @file InterruptionMemory.cpp
 * @brief Implementation of interruption context storage for ambient character behavioral continuity
 * @author Eric Buitrón López
 * @date 8/12/2025
 *
 * Stores action state to enable characters to resume behaviors after handling interruptions
 * (emergencies, player interactions, environmental changes).
 */

#include "InterruptionMemory.h"

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
 *
 * @algorithm Comprehensive validation + complete context storage
 * @rationale
 * - All context components must be validated to ensure resumption integrity
 * - Entity ID allows -1 to represent actions that don't require entities
 * - Context preservation enables behavior resumption
 * - Early validation prevents corrupted interruption states
 * @complexity O(1) - validation and assignment operations
 * @datastructures Stores execution context in simple integer fields
 */
InterruptionMemory::InterruptionMemory(
    int interrupted_action_id,
    int interrupted_sequence_id,
    int interrupted_sequence_node_id,
    int interrupted_target_entity_id,
    int time
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
// MEMORY MATCHING (Framework Core Functionality)
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
 *
 * @algorithm Two-stage matching: type check + triple value comparison
 * @rationale
 * - dynamic_cast provides type safety for polymorphic memory collections
 * - Triple matching ensures exact execution context identification
 * - Entity ID not included in matching (preserved for resumption only)
 * - Returns false for incompatible types rather than throwing exceptions
 *
 * @complexity O(1) - dynamic_cast + three integer comparisons
 * @datastructures Uses dynamic_cast for safe polymorphic type checking
 * @performance_notes
 * - Triple comparison still O(1) and cache-friendly
 * - Most complex matching logic but necessary for precise context identification
 * - Entity ID excluded from matching allows flexibility in resumption scenarios
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
 * @brief Convenience method to check triple execution context matching
 * @param other_action_id Action identifier to compare against
 * @param other_sequence_id Sequence identifier to compare against
 * @param other_sequence_node_id Sequence node identifier to compare against
 * @return true if all three context identifiers match
 *
 * @algorithm Triple equality check with direct integer comparisons
 * @rationale
 * - All three identifiers must match for InterruptionMemory identity
 * - Short-circuit evaluation optimizes for early mismatches
 * - Entity ID intentionally excluded from matching logic
 * - Separates triple matching logic from polymorphic type checking
 * @complexity O(1) - three integer comparisons with short-circuit
 */
bool InterruptionMemory::MatchesMemory(int other_action_id, int other_sequence_id, int other_sequence_node_id) const
{
    return this->interrupted_action_id == other_action_id &&
        this->interrupted_sequence_id == other_sequence_id &&
        this->interrupted_sequence_node_id == other_sequence_node_id;
}

// =============================================================================
// CONTEXT DATA ACCESS
// =============================================================================

/**
 * @brief Gets the action identifier from the interruption context
 * @return Unique identifier of the action that was interrupted
 *
 * @algorithm Direct member access
 * @complexity O(1) - simple member variable access
 */
int InterruptionMemory::GetInterruptedActionId() const
{
    return interrupted_action_id;
}

/**
 * @brief Gets the sequence identifier from the interruption context
 * @return Unique identifier of the sequence that was executing
 *
 * @algorithm Direct member access
 * @complexity O(1) - simple member variable access
 */
int InterruptionMemory::GetInterruptedSequenceId() const
{
    return interrupted_sequence_id;
}

/**
 * @brief Gets the sequence node identifier from the interruption context
 * @return Unique identifier of the sequence node that was interrupted
 *
 * @algorithm Direct member access
 * @complexity O(1) - simple member variable access
 */
int InterruptionMemory::GetInterruptedSequenceNodeId() const
{
    return interrupted_sequence_node_id;
}

/**
 * @brief Gets the entity identifier from the interruption context
 * @return Unique identifier of the entity involved in the interrupted action (-1 if none)
 *
 * @algorithm Direct member access
 * @complexity O(1) - simple member variable access
 * @note Returns -1 for actions that don't involve entities
 */
int InterruptionMemory::GetInterruptedTargetEntityId() const
{
    return interrupted_target_entity_id;
}


