#include "ActionMemory.h"
using namespace AmbientCharacterBehavior;

ActionMemory::ActionMemory(int32_t action_id, int32_t target_entity_id, int64_t last_used_time) :
	BaseMemory(last_used_time), action_id(action_id), target_entity_id(target_entity_id)
{
	// Validate action_id invariant
	if(action_id < 0)
    {
        throw std::invalid_argument("ActionMemory: action_id cannot be negative, got " + std::to_string(action_id));
    }

	// Validate target_entity_id invariant
	if(target_entity_id < -1)
	{
		throw std::invalid_argument("ActionMemory: target_entity_id cannot be less than -1 (null/none), got " +
			std::to_string(target_entity_id));
	}
}


bool ActionMemory::MatchesMemory(const BaseMemory& other) const
{
	// Type safe cast returning nullptr if other is not an ActionMemory
	const auto other_transition_memory (dynamic_cast<const ActionMemory*>(&other));
	if(!other_transition_memory)
	{
		// If it's a different memory type it cannot match
		return false;
	}

	// Delegate to type specific matching
	return MatchesMemory(other_transition_memory->GetActionId(), other_transition_memory->GetTargetEntityId());
}

bool ActionMemory::MatchesMemory(int32_t other_action_id, int32_t other_target_entity_id) const
{
	return this->action_id == other_action_id && this->target_entity_id == other_target_entity_id;
}