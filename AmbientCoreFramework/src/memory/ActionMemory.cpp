/**
 * @file ActionMemory.cpp
 * @brief Implementation of action execution tracking for ambient character behavioral variety
 * @author Eric Buitrón López
 * @date 8/11/2025
 *
 * Records which entities have been used for specific actions to enable recency-based
 * entity selection that creates natural behavioral patterns.
 */

#include "ActionMemory.h"
using namespace AmbientCharacterBehavior;

// =============================================================================
// CONSTRUCTION
// =============================================================================

/**
 * @brief Constructs action memory for a specific action-entity combination
 * @param action_id Unique identifier of the action that was performed
 * @param target_entity_id Unique identifier of the entity that was used
 * @param last_used_time Simulation timestamp when the action was performed
 * @throws std::invalid_argument if action_id < 0 or target_entity_id < 0 or last_used_time < 0
 *
 * @algorithm Dual validation + member initialization
 * @rationale
 * - Both action_id and target_entity_id must be validated for compound matching
 * - Delegates time validation to base class for consistency
 * - Early validation prevents invalid combinations from being stored
 * @complexity O(1) - simple validation and assignment operations
 */
ActionMemory::ActionMemory(int action_id, int target_entity_id, int last_used_time) :
	BaseMemory(last_used_time), action_id(action_id), target_entity_id(target_entity_id)
{
	if(action_id < 0)
    {
        throw std::invalid_argument("ActionMemory: action_id cannot be negative, got " + std::to_string(action_id));
    }

	if(target_entity_id < 0)
    {
        throw std::invalid_argument("ActionMemory: target_entity_id cannot be negative, got " + std::to_string(target_entity_id));
    }
}

// =============================================================================
// MEMORY MATCHING
// =============================================================================

/**
 * @brief Checks if this memory matches another memory instance polymorphically
 *
 * Implements the BaseMemory interface by safely checking if the other memory
 * is also an ActionMemory with the same action_id AND target_entity_id.
 * Uses dynamic_cast for type safety in polymorphic scenarios.
 *
 * @param other Memory instance to compare against
 * @return true if other is ActionMemory with same action_id and target_entity_id
 *
 * @algorithm Two-stage matching: type check + compound value comparison
 * @rationale
 * - dynamic_cast provides type safety for polymorphic memory collections
 * - Delegates to optimized compound comparison after type verification
 * - Returns false for incompatible types rather than throwing exceptions
 * - ActionMemory requires both identifiers to match (stricter than TransitionMemory)
 *
 * @complexity O(1) - dynamic_cast + two integer comparisons
 * @datastructures Uses dynamic_cast for safe polymorphic type checking
 * @performance_notes
 * - dynamic_cast overhead acceptable for decision-making frequency
 * - Compound matching (2 comparisons) still O(1)
 * - Alternative approaches would require complex type hierarchies
 */
bool ActionMemory::MatchesMemory(const BaseMemory& other) const
{
	// Safe type conversion - returns nullptr if 'other' is not an ActionMemory
	const auto other_transition_memory (dynamic_cast<const ActionMemory*>(&other));
	if(!other_transition_memory)
	{
		return false;
	}

	return MatchesMemory(other_transition_memory->GetActionId(), other_transition_memory->GetTargetEntityId());
}

/**
 * @brief Convenience method to check compound action-entity matching
 * @param other_action_id Action identifier to compare against
 * @param other_target_entity_id Entity identifier to compare against
 * @return true if both action_id and target_entity_id match
 *
 * @algorithm Direct integer comparisons
 * @rationale
 * - Both identifiers must match for ActionMemory identity
 * - Short-circuit evaluation optimizes for action_id mismatches (likely more common)
 * - Separates compound logic from polymorphic type checking
 * @complexity O(1) - two integer comparisons with short-circuit
 */
bool ActionMemory::MatchesMemory(int other_action_id, int other_target_entity_id) const
{
	return this->action_id == other_action_id &&
		this->target_entity_id == other_target_entity_id;
}

// =============================================================================
// DATA ACCESS
// =============================================================================

/**
 * @brief Gets the action identifier that this memory represents
 * @return Unique identifier of the action that was performed
 *
 * @algorithm Direct member access
 * @complexity O(1) - simple member variable access
 */
int ActionMemory::GetActionId() const
{
	return this->action_id;
}

/**
 * @brief Gets the entity identifier that this memory represents
 * @return Unique identifier of the entity that was used for the action
 *
 * @algorithm Direct member access
 * @complexity O(1) - simple member variable access
 */
int ActionMemory::GetTargetEntityId() const
{
	return this->target_entity_id;
}