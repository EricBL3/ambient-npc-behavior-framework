
#include "InterruptionMemory.h"

using namespace AmbientCharacterBehavior;

/**
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

bool InterruptionMemory::MatchesMemory(const BaseMemory &other) const
{
    const auto other_transition_memory (dynamic_cast<const InterruptionMemory*>(&other));
    if(!other_transition_memory)
    {
        return false;
    }

    return MatchesMemory(other_transition_memory->GetInterruptedActionId(),
        other_transition_memory->GetInterruptedSequenceId(),
        other_transition_memory->GetInterruptedSequenceNodeId());
}

bool InterruptionMemory::MatchesMemory(int32_t other_action_id, int32_t other_sequence_id, int32_t other_sequence_node_id) const
{
    return this->interrupted_action_id == other_action_id &&
        this->interrupted_sequence_id == other_sequence_id &&
        this->interrupted_sequence_node_id == other_sequence_node_id;
}

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