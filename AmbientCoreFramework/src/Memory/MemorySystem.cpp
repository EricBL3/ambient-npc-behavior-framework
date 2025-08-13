//
// Created by Eric on 8/12/2025.
//

#include "MemorySystem.h"

#include <algorithm>
#include <iostream>
#include <optional>

void MemorySystem::LogError(const std::string &message) const
{
    std::cerr << "[MemorySystem Error]: " << message << std::endl;
}

void MemorySystem::EnforceMaxTransitionMemories()
{
    if (GetTransitionMemoryCount() > GetMaxTransitionMemories())
    {
        // Removes first entry because it is the oldest one.
        transition_memories.erase(transition_memories.begin());
    }
}

void MemorySystem::EnforceMaxActionMemories()
{
    if (GetActionMemoryCount() > GetMaxActionMemories())
    {
        // Removes first entry because it is the oldest one.
        action_memories.erase(action_memories.begin());
    }
}

void MemorySystem::EnforceMaxInterruptionMemories()
{
    if (GetInterruptionMemoryCount() > GetMaxInterruptionMemories())
    {
        // Removes first entry because it is the oldest one.
        interruption_memories.erase(interruption_memories.begin());
    }
}

void MemorySystem::RemoveExistingTransitionMemory(int target_node_id)
{
    // Search for existing memory with same target_node_id
    auto iterator = std::find_if(transition_memories.begin(), transition_memories.end(),
        [target_node_id](const TransitionMemory& memory) // lambda function: for each memory in the vector
        {
           return  memory.MatchesMemory(target_node_id); // return true if it matches
        });

    // Remove existing memory if found
    if (iterator != transition_memories.end()) {
        transition_memories.erase(iterator);
    }
}

MemorySystem::MemorySystem(int max_transitions, int max_actions, int max_interruptions)
{
    SetMaxTransitionMemories(max_transitions);
    SetMaxActionMemories(max_actions);
    SetMaxInterruptionMemories(max_interruptions);

    ClearAllMemories();
}

int MemorySystem::GetMaxTransitionMemories() const
{
    return max_transition_memories;
}

int MemorySystem::GetMaxActionMemories() const
{
    return max_action_memories;
}

int MemorySystem::GetMaxInterruptionMemories() const
{
    return max_interruption_memories;
}

bool MemorySystem::UpdateTransitionMemory(int target_node_id, int current_time)
{
    try
    {
        TransitionMemory new_memory(target_node_id, current_time);

        // remove the transition memory if it exists already to prevent having duplicates.
        RemoveExistingTransitionMemory(target_node_id);

        // use std::move to move the created memory instead of copying it in another object
        transition_memories.push_back(std::move(new_memory));
        EnforceMaxTransitionMemories();

        return true;
    } catch (const std::exception& e)
    {
        LogError("UpdateTransitionMemory failed: " + std::string(e.what()));
        return false;
    }
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

// Search for the first transition memory that matches the target_node_id
const TransitionMemory * MemorySystem::FindTransitionMemory(int target_node_id) const
{
    auto iterator = std::find_if(transition_memories.begin(), transition_memories.end(),
        [target_node_id](const TransitionMemory& memory) // lambda function: for each memory in the vector
        {
           return  memory.MatchesMemory(target_node_id); // return true if it matches
        });

    // if found, return pointer to the memory; otherwise return nullptr
    return (iterator !=  transition_memories.end()) ? &(*iterator) : nullptr;
}

// Search for the first action memory that matches the action_id AND target_entity_id
const ActionMemory * MemorySystem::FindActionMemory(int action_id, int target_entity_id) const
{
    auto iterator = std::find_if(action_memories.begin(), action_memories.end(),
        [action_id, target_entity_id](const ActionMemory& memory) // lambda function: for each memory in the vector
        {
           return  memory.MatchesMemory(action_id, target_entity_id); // return true if it matches
        });

    // if found, return pointer to the memory; otherwise return nullptr
    return (iterator !=  action_memories.end()) ? &(*iterator) : nullptr;
}

// Search for the first interruption memory that matches the action_id AND sequence_id AND node_id
const InterruptionMemory * MemorySystem::FindInterruptionMemory(int action_id, int sequence_id, int node_id) const
{
    auto iterator = std::find_if(interruption_memories.begin(), interruption_memories.end(),
        [action_id, sequence_id, node_id](const InterruptionMemory& memory) // lambda function: for each memory in the vector
        {
           return  memory.MatchesMemory(action_id, sequence_id, node_id); // return true if it matches
        });

    // if found, return pointer to the memory; otherwise return nullptr
    return (iterator !=  interruption_memories.end()) ? &(*iterator) : nullptr;
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
    if (max_transitions <= 0)
    {
        LogError("MemorySystem: max_transitions must be greater than 0, keeping current value");
        return;
    }

    this->max_transition_memories = max_transitions;
    EnforceMaxTransitionMemories();
}

void MemorySystem::SetMaxActionMemories(int max_actions)
{
    if (max_actions <= 0)
    {
        LogError("MemorySystem: max_actions must be greater than 0, keeping current value");
        return;
    }

    this->max_action_memories = max_actions;
    EnforceMaxActionMemories();
}

void MemorySystem::SetMaxInterruptionMemories(int max_interruptions)
{
    if (max_interruptions <= 0)
    {
        LogError("MemorySystem: max_interruptions must be greater than 0, keeping current value");
        return;
    }

    this->max_interruption_memories = max_interruptions;
    EnforceMaxInterruptionMemories();
}

void MemorySystem::ClearAllMemories()
{
    transition_memories.clear();
    action_memories.clear();
    interruption_memories.clear();
}

size_t MemorySystem::GetTransitionMemoryCount() const
{
    return transition_memories.size();
}

size_t MemorySystem::GetActionMemoryCount() const
{
    return action_memories.size();
}

size_t MemorySystem::GetInterruptionMemoryCount() const
{
    return interruption_memories.size();
}
