/*
* MemorySystem.cpp
 *
 * Implementation of the core memory management for ambient character behavioral variety
 *
 * Author: Eric Buitrón López
 * Created: 8/13/2025
*/

#include "MemorySystem.h"

#include <algorithm>
#include <iostream>

// =============================================================================
// CONSTRUCTION & CONFIGURATION
// =============================================================================

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

// =============================================================================
// MEMORY UPDATE OPERATIONS
// =============================================================================

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
    try
    {
        ActionMemory new_memory(action_id, target_entity_id, current_time);

        // remove the action memory if it exists already to prevent duplicates.
        RemoveExistingActionMemory(action_id, target_entity_id);

        // use std::move to move the created memory instead of copying it in another object
        action_memories.push_back(std::move(new_memory));
        EnforceMaxActionMemories();

        return true;
    } catch (const std::exception& e)
    {
        LogError("UpdateActionMemory failed: " + std::string(e.what()));
        return false;
    }
}

bool MemorySystem::UpdateInterruptionMemory(int action_id, int sequence_id, int node_id, int entity_id,
    int current_time)
{
    try
    {
        InterruptionMemory new_memory(action_id, sequence_id, node_id, entity_id, current_time);

        // remove interruption memory if it exists already to prevent duplicates.
        RemoveInterruptionMemory(action_id, sequence_id, node_id);

        // use std::move to move the created memory instead of copying it in another object
        interruption_memories.push_back(std::move(new_memory));
        EnforceMaxInterruptionMemories();

        return true;

    } catch (const std::exception& e)
    {
        LogError("UpdateInterruptionMemory failed: " + std::string(e.what()));
        return false;
    }
}

// =============================================================================
// MEMORY SEARCH OPERATIONS
// =============================================================================

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

// =============================================================================
// BEHAVIORAL SELECTION
// =============================================================================

int MemorySystem::GetLeastRecentlyVisitedNode(const std::vector<int> &node_ids) const
{
    if (node_ids.empty())
    {
        LogError("GetLeastRecentlyVisitedNode: There are no node_ids to search through");
        return -1;
    }

    // Try to get an unused node first
    std::vector<int> unused = FindUnusedTransitionNodes(node_ids);
    if (!unused.empty())
    {
        return SelectRandomFromVector(unused);
    }

    // If all are used find the oldest one
    return FindOldestTransitionNode(node_ids);
}

int MemorySystem::GetLeastRecentlyUsedEntityForAction(int action_id, const std::vector<int> &entity_ids) const
{
    if (entity_ids.empty())
    {
        LogError("GetLeastRecentlyUsedEntityForAction: There are no entity_ids to search through");
        return -1;
    }

    // Try to get an unused entity first
    std::vector<int> unused = FindUnusedActionEntities(action_id, entity_ids);
    if (!unused.empty())
    {
        return SelectRandomFromVector(unused);
    }

    // If all are used find the oldest one
    return FindOldestActionEntity(action_id, entity_ids);
}

// =============================================================================
// MEMORY CLEANUP OPERATIONS
// =============================================================================

// Removes all the interruption memories of the specified sequence_id.
void MemorySystem::ClearSequenceInterruptionMemories(int sequence_id)
{
    // Iterate backwards through the vector to remove the elements safely.
    for (int i = static_cast<int>(interruption_memories.size()) - 1; i >= 0; i--)
    {
        if (interruption_memories[i].GetInterruptedSequenceId() == sequence_id)
        {
            interruption_memories.erase(interruption_memories.begin() + i);
        }
    }
}

bool MemorySystem::RemoveInterruptionMemory(int action_id, int sequence_id, int node_id)
{
    // Search for existing memory with same action_id AND sequence_id AND node_id
    auto iterator = std::find_if(interruption_memories.begin(), interruption_memories.end(),
        [action_id, sequence_id, node_id](const InterruptionMemory& memory) // lambda function: for each memory in the vector
        {
           return  memory.MatchesMemory(action_id, sequence_id, node_id); // return true if it matches
        });

    // Remove existing memory if found
    if (iterator != interruption_memories.end()) {
        interruption_memories.erase(iterator);
        return true;
    }

    return false;
}

void MemorySystem::ClearAllMemories()
{
    transition_memories.clear();
    action_memories.clear();
    interruption_memories.clear();
}

// =============================================================================
// UTILITY HELPERS
// =============================================================================

void MemorySystem::LogError(const std::string &message) const
{
    std::cerr << "[MemorySystem Error]: " << message << std::endl;
}

int MemorySystem::SelectRandomFromVector(const std::vector<int> &options) const
{
    if (options.empty())
    {
        LogError("SelectRandomFromVector: There are no options to search through");
        return -1;
    }

    return options[rand() % options.size()];
}

// =============================================================================
// CAPACITY MANAGEMENT
// =============================================================================

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

// =============================================================================
// MEMORY REMOVAL HELPERS
// =============================================================================

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

void MemorySystem::RemoveExistingActionMemory(int action_id, int target_entity_id)
{
    // Search for existing memory with same target_node_id
    auto iterator = std::find_if(action_memories.begin(), action_memories.end(),
        [action_id, target_entity_id](const ActionMemory& memory) // lambda function: for each memory in the vector
        {
           return  memory.MatchesMemory(action_id, target_entity_id); // return true if it matches
        });

    // Remove existing memory if found
    if (iterator != action_memories.end()) {
        action_memories.erase(iterator);
    }
}

// =============================================================================
// RECENCY SELECTION HELPERS
// =============================================================================

int MemorySystem::FindOldestTransitionNode(const std::vector<int> &node_ids) const
{
    if (node_ids.empty())
    {
        LogError("FindOldestTransitionNode: There are no node_ids to search through");
        return -1;
    }

    std::vector<int> oldest_nodes;
    int oldest_time = INT_MAX;

    for (int node_id : node_ids)
    {
        const TransitionMemory* memory = FindTransitionMemory(node_id);
        if (memory == nullptr)
        {
            LogError("FindOldestTransitionNode: Expected memory not found for node " + std::to_string(node_id));
            continue;
        }

        int current_time = memory->GetLastUsedTime();

        // If there's a new oldest_time, we restart the oldest_nodes vector.
        if (current_time < oldest_time)
        {
            oldest_time = current_time;
            oldest_nodes.clear();
            oldest_nodes.push_back(node_id);
        }
        else if (current_time == oldest_time)
        {
            oldest_nodes.push_back(node_id);
        }
    }

    if (oldest_nodes.empty())
    {
        LogError("FindOldestTransitionNode: No valid memories found");
        return -1;
    }

    return SelectRandomFromVector(oldest_nodes);
}

int MemorySystem::FindOldestActionEntity(int action_id, const std::vector<int> &entity_ids) const
{
    if (entity_ids.empty())
    {
        LogError("FindOldestActionEntity: There are no entity_ids to search through");
        return -1;
    }

    std::vector<int> oldest_nodes;
    int oldest_time = INT_MAX;

    for (int entity_id : entity_ids)
    {
        const ActionMemory* memory = FindActionMemory(action_id, entity_id);
        if (memory == nullptr)
        {
            LogError("FindOldestActionEntity: Expected memory not found for action " + std::to_string(action_id) +
                " and entity " + std::to_string(entity_id));

            continue;
        }

        int current_time = memory->GetLastUsedTime();

        // If there's a new oldest_time, we restart the oldest_nodes vector.
        if (current_time < oldest_time)
        {
            oldest_time = current_time;
            oldest_nodes.clear();
            oldest_nodes.push_back(entity_id);
        }
        else if (current_time == oldest_time)
        {
            oldest_nodes.push_back(entity_id);
        }
    }

    if (oldest_nodes.empty())
    {
        LogError("FindOldestActionEntity: No valid memories found");
        return -1;
    }

    return SelectRandomFromVector(oldest_nodes);
}

std::vector<int> MemorySystem::FindUnusedTransitionNodes(const std::vector<int> &node_ids) const
{
    std::vector<int> unused_nodes;
    for (int node_id : node_ids)
    {
        const TransitionMemory* memory = FindTransitionMemory(node_id);
        if (memory == nullptr)
        {
            unused_nodes.push_back(node_id);
        }
    }

    return unused_nodes;
}

std::vector<int> MemorySystem::FindUnusedActionEntities(int action_id, const std::vector<int> &entity_ids) const
{
    std::vector<int> unused_entities;
    for (int entity_id : entity_ids)
    {
        const ActionMemory* memory = FindActionMemory(action_id, entity_id);
        if (memory == nullptr)
        {
            unused_entities.push_back(entity_id);
        }
    }

    return unused_entities;
}

// =============================================================================
// DIAGNOSTIC & MONITORING
// =============================================================================

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
