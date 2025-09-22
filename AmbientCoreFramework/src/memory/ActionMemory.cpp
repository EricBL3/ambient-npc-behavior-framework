#include "ActionMemory.h"

using namespace AmbientCharacterBehavior;


/**
 * @throws std::invalid_argument if action_id < 0 or target_entity_id < 0 or last_used_time < 0
 *
 */
ActionMemory::ActionMemory(int32_t action_id, int32_t target_entity_id, int64_t last_used_time) :
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


bool ActionMemory::MatchesMemory(const BaseMemory& other) const
{
	const auto other_transition_memory (dynamic_cast<const ActionMemory*>(&other));
	if(!other_transition_memory)
	{
		return false;
	}

	return MatchesMemory(other_transition_memory->GetActionId(), other_transition_memory->GetTargetEntityId());
}

bool ActionMemory::MatchesMemory(int32_t other_action_id, int32_t other_target_entity_id) const
{
	return this->action_id == other_action_id &&
		this->target_entity_id == other_target_entity_id;
}

int32_t ActionMemory::GetActionId() const
{
	return this->action_id;
}

int32_t ActionMemory::GetTargetEntityId() const
{
	return this->target_entity_id;
}