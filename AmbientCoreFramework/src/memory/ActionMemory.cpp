/**
 * @file ActionMemory.cpp
 * @brief Implementation of action execution tracking for ambient character behavioral variety
 * @author Eric Buitrón López
 * @date 8/11/2025
 *
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
* @brief Checks if this memory matches the provided parameters.
 *
 * Alternative way of checking if a memory matches if full memory object is not available.
 *
 * @param other_action_id Action identifier to compare against
 * @param other_target_entity_id Entity identifier to compare against
 * @return true if both action_id and target_entity_id match
 *
 */
bool ActionMemory::MatchesMemory(int other_action_id, int other_target_entity_id) const
{
	return this->action_id == other_action_id &&
		this->target_entity_id == other_target_entity_id;
}

// =============================================================================
// DATA ACCESS
// =============================================================================

int ActionMemory::GetActionId() const
{
	return this->action_id;
}

int ActionMemory::GetTargetEntityId() const
{
	return this->target_entity_id;
}