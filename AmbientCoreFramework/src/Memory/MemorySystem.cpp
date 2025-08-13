//
// Created by Eric on 8/12/2025.
//

#include "MemorySystem.h"

void MemorySystem::LogError(const std::string &message) const
{
    throw std::logic_error("Not implemented");
}

MemorySystem::MemorySystem(int max_transitions, int max_actions, int max_interruptions)
{
    throw std::logic_error("Not implemented");
}

int MemorySystem::GetMaxTransitionMemories() const
{
    throw std::logic_error("Not implemented");
}

int MemorySystem::GetMaxActionMemories() const
{
    throw std::logic_error("Not implemented");
}

int MemorySystem::GetMaxInterruptionMemories() const
{
    throw std::logic_error("Not implemented");
}

bool MemorySystem::UpdateTransitionMemory(int target_node_id, int current_time)
{
    throw std::logic_error("Not implemented");
}

bool MemorySystem::UpdateActionMemory(int action_id, int target_entity_id, int current_time)
{
    throw std::logic_error("Not implemented");
}

bool MemorySystem::UpdateInterruptionMemory(int action_id, int sequence_id, int node_id, int entity_id,
    int current_time)
{
    throw std::logic_error("Not implemented");
}

TransitionMemory * MemorySystem::FindTransitionMemory(int target_node_id) const
{
    throw std::logic_error("Not implemented");
}

ActionMemory * MemorySystem::FindActionMemory(int action_id, int target_entity_id) const
{
    throw std::logic_error("Not implemented");
}

InterruptionMemory * MemorySystem::FindInterruptionMemory(int action_id, int sequence_id, int node_id) const
{
    throw std::logic_error("Not implemented");
}

int MemorySystem::GetLeastRecentlyVisitedNode(const std::vector<int> &node_ids) const
{
    throw std::logic_error("Not implemented");
}

int MemorySystem::GetLeastRecentlyUsedEntityForAction(int action_id, const std::vector<int> &entity_ids) const
{
    throw std::logic_error("Not implemented");
}

void MemorySystem::ClearInterruptionMemories(int sequence_id)
{
    throw std::logic_error("Not implemented");
}

bool MemorySystem::RemoveInterruptionMemory(int action_id, int sequence_id, int node_id)
{
    throw std::logic_error("Not implemented");
}

void MemorySystem::SetMaxTransitionMemories(int max_transitions)
{
    throw std::logic_error("Not implemented");
}

void MemorySystem::SetMaxActionMemories(int max_actions)
{
    throw std::logic_error("Not implemented");
}

void MemorySystem::SetMaxInterruptionMemories(int max_interruptions)
{
    throw std::logic_error("Not implemented");
}

void MemorySystem::ClearAllMemories()
{
    throw std::logic_error("Not implemented");
}

size_t MemorySystem::GetTransitionMemoryCount() const
{
    throw std::logic_error("Not implemented");
}

size_t MemorySystem::GetActionMemoryCount() const
{
    throw std::logic_error("Not implemented");
}

size_t MemorySystem::GetInterruptionMemoryCount() const
{
    throw std::logic_error("Not implemented");
}
