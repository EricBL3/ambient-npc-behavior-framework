/*
* ActionMemory.cpp
 *
 * Implementation of action execution tracking for ambient character behavioral variety.
 * Records which entities have been used for specific actions to enable recency-based
 * entity selection that creates natural behavioral patterns.
 *
 * Author: Eric Buitrón López
 * Created: 8/11/2025
 */

#include "ActionMemory.h"

// =============================================================================
// CONSTRUCTION
// =============================================================================

ActionMemory::ActionMemory(int action_id, int target_entity_id, int last_used_time) : IMemory(last_used_time)
{
	if(action_id < 0)
    {
        throw std::invalid_argument("ActionMemory: action_id cannot be negative, got " + std::to_string(action_id));
    }

	if(target_entity_id < 0)
    {
        throw std::invalid_argument("ActionMemory: target_entity_id cannot be negative, got " + std::to_string(target_entity_id));
    }

	this->action_id = action_id;
	this->target_entity_id = target_entity_id;
}

// =============================================================================
// MEMORY MATCHING (Framework Core Functionality)
// =============================================================================

// The action memory must match in action_id and target_entity_id.
bool ActionMemory::MatchesMemory(const IMemory& other) const
{
	const auto other_transition_memory = dynamic_cast<const ActionMemory*>(&other);
	if(!other_transition_memory)
	{
		return false;
	}

	return MatchesMemory(other_transition_memory->action_id, other_transition_memory->target_entity_id);
}

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