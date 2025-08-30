/**
 * @file MemorySystem.cpp
 * @brief Implementation of the core memory management for ambient character behavioral variety
 * @author Eric Buitrón López
 * @date 8/13/2025
 */

#include "MemorySystem.h"
#include <algorithm>
#include <iostream>
#include "utils/PerformanceTracker.h"

using namespace AmbientCharacterBehavior;

// =============================================================================
// CONSTRUCTION & CONFIGURATION
// =============================================================================

/**
 * @brief Constructs MemorySystem with specified capacity limits for each memory type
 * @param max_transitions Maximum transition memories (default is 10)
 * @param max_actions Maximum action memories (default is 20)
 * @param max_interruptions Maximum interruption memories (default is 5)
 *
 */
MemorySystem::MemorySystem(int32_t max_transitions, int32_t max_actions, int32_t max_interruptions)
{
    SetMaxTransitionMemories(max_transitions);
    SetMaxActionMemories(max_actions);
    SetMaxInterruptionMemories(max_interruptions);

    ClearAllMemories();
}

int32_t MemorySystem::GetMaxTransitionMemories() const
{
    return max_transition_memories;
}

int32_t MemorySystem::GetMaxActionMemories() const
{
    return max_action_memories;
}

int32_t MemorySystem::GetMaxInterruptionMemories() const
{
    return max_interruption_memories;
}

/**
 * @brief Sets the maximum number of transition memories (increasing its container if necessary) and enforces the
 * new memory limits.
 *
 * @param max_transitions New maximum capacity (must be > 0)
 *
 */
void MemorySystem::SetMaxTransitionMemories(int32_t max_transitions)
{
    if (max_transitions <= 0 || max_transitions == max_transition_memories)
    {
        LogError("MemorySystem: max_transitions must be greater than 0 and different from the current maximum, "
                 "keeping current value");
        return;
    }

    max_transition_memories = max_transitions;
    transition_memories.reserve(max_transitions);
    EnforceMaxTransitionMemories();
}

/**
 * @brief Sets the maximum number of action memories (increasing its container if necessary) and enforces the
 * new memory limits.
 *
 * @param max_actions New maximum capacity (must be > 0)
 *
 */
void MemorySystem::SetMaxActionMemories(int32_t max_actions)
{
    if (max_actions <= 0 || max_actions == max_action_memories)
    {
        LogError("MemorySystem: max_actions must be greater than 0, and different from the current maximum, "
                 "keeping current value");
        return;
    }

    max_action_memories = max_actions;
    action_memories.reserve(max_actions);
    EnforceMaxActionMemories();

}

/**
 * @brief Sets the maximum number of interruption memories (increasing its container if necessary) and enforces the
 * new memory limits.
 *
 * @param max_interruptions New maximum capacity (must be > 0)
 *
 */
void MemorySystem::SetMaxInterruptionMemories(int32_t max_interruptions)
{
    if (max_interruptions <= 0 || max_interruptions == max_interruption_memories)
    {
        LogError("MemorySystem: max_interruptions must be greater than 0, and different from the current maximum, "
                 "keeping current value");
        return;
    }

    max_interruption_memories = max_interruptions;
    interruption_memories.reserve(max_interruptions);
    EnforceMaxInterruptionMemories();

}

// =============================================================================
// MEMORY UPDATE OPERATIONS
// =============================================================================

/**
 * @brief Records or updates a transition decision in character memory
 * @param target_node_id Unique identifier of the sequence node that was visited
 * @param current_time Simulation timestamp when the transition was taken
 * @return true if update succeeded, false if validation or creation failed
 *
 */
bool MemorySystem::UpdateTransitionMemory(int32_t target_node_id, int64_t current_time)
{
    try
    {
        // Create the new memory first. If the arguments are invalid this will throw an exception.
        TransitionMemory new_memory(target_node_id, current_time);

        // Remove existing memory to prevent duplicates
        RemoveExistingTransitionMemory(target_node_id);

        // Add new memory at end (newest memories at back, oldest at front)
        transition_memories.push_back(new_memory);

        EnforceMaxTransitionMemories();

        return true;
    } catch (const std::exception& e)
    {
        LogError("UpdateTransitionMemory failed: " + std::string(e.what()));
        return false;
    }
}

/**
 * @brief Records or updates an action execution decision in character memory
 * @param action_id Unique identifier of the action that was performed
 * @param target_entity_id Unique identifier of the entity that was used
 * @param current_time Simulation timestamp when the action was executed
 * @return true if update succeeded, false if validation or creation failed
 *
 */
bool MemorySystem::UpdateActionMemory(int32_t action_id, int32_t target_entity_id, int64_t current_time)
{
    try
    {
        // Create the new memory first. If the arguments are invalid this will throw an exception.
        ActionMemory new_memory(action_id, target_entity_id, current_time);

        // remove the action memory if it exists already to prevent duplicates.
        RemoveExistingActionMemory(action_id, target_entity_id);

        // Add new memory at end (newest memories at back, oldest at front)
        action_memories.push_back(new_memory);

        EnforceMaxActionMemories();

        return true;
    } catch (const std::exception& e)
    {
        LogError("UpdateActionMemory failed: " + std::string(e.what()));
        return false;
    }
}

/**
 * @brief Records or updates an interruption context in character memory
 * @param action_id Unique identifier of the action that was interrupted
 * @param sequence_id Unique identifier of the sequence that was executing
 * @param node_id Unique identifier of the sequence node that was interrupted
 * @param entity_id Unique identifier of the entity involved (-1 if none)
 * @param current_time Simulation timestamp when the interruption occurred
 * @return true if update succeeded, false if validation or creation failed
 *
 */
bool MemorySystem::UpdateInterruptionMemory(
    int32_t action_id,
    int32_t sequence_id,
    int32_t node_id,
    int32_t entity_id,
    int64_t current_time
)
{
    try
    {
        // Create the new memory first. If the arguments are invalid this will throw an exception.
        InterruptionMemory new_memory(action_id, sequence_id, node_id, entity_id, current_time);

        // remove interruption memory if it exists already to prevent duplicates.
        RemoveInterruptionMemory(action_id, sequence_id, node_id);

        // Add new memory at end (newest memories at back, oldest at front)
        interruption_memories.push_back(new_memory);

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

/**
 * @brief Searches for a transition memory matching the specified node ID
 * @param target_node_id Node identifier to search for
 * @return Pointer to matching memory or nullptr if not found
 *
 */
const TransitionMemory * MemorySystem::FindTransitionMemory(int32_t target_node_id) const
{
    // find_if performs a linear search over the memories
    auto iterator = std::find_if(transition_memories.begin(), transition_memories.end(),
        [target_node_id](const TransitionMemory& memory) // lambda function: for each memory in the vector
        {
           return  memory.MatchesMemory(target_node_id); // return true if it matches
        });

    // if found, return pointer to the memory; otherwise return nullptr
    return (iterator !=  transition_memories.end()) ? &(*iterator) : nullptr;
}

/**
 * @brief Searches for an action memory matching the specified action and entity
 * @param action_id Action identifier to search for
 * @param target_entity_id Entity identifier to search for
 * @return Pointer to matching memory or nullptr if not found
 *
 */
const ActionMemory * MemorySystem::FindActionMemory(int32_t action_id, int32_t target_entity_id) const
{
    // find_if performs a linear search over the memories
    auto iterator = std::find_if(action_memories.begin(), action_memories.end(),
        [action_id, target_entity_id](const ActionMemory& memory) // lambda function: for each memory in the vector
        {
           return  memory.MatchesMemory(action_id, target_entity_id); // return true if it matches
        });

    // if found, return pointer to the memory; otherwise return nullptr
    return (iterator !=  action_memories.end()) ? &(*iterator) : nullptr;
}

/**
 * @brief Searches for an interruption memory matching the specified execution context
 * @param action_id Action identifier to search for
 * @param sequence_id Sequence identifier to search for
 * @param node_id Sequence node identifier to search for
 * @return Pointer to matching memory or nullptr if not found
 *
 */
const InterruptionMemory * MemorySystem::FindInterruptionMemory(int32_t action_id, int32_t sequence_id, int32_t node_id) const
{
    // find_if performs a linear search over the memories
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

/**
 * @brief Selects the least recently visited node from available options
 * @param node_ids Vector of node identifiers that are currently valid choices
 * @return Node ID that should be selected for behavioral variety, or -1 on error
 *
 */
int32_t MemorySystem::GetLeastRecentlyVisitedNode(const std::vector<int32_t> &node_ids) const
{
    if (node_ids.empty())
    {
        LogError("GetLeastRecentlyVisitedNode: There are no node_ids to search through");
        return -1;
    }

    // Try to get an unused node first
    std::vector<int32_t> unused {FindUnusedTransitionNodes(node_ids)};
    if (!unused.empty())
    {
        // Random selection among unused nodes prevents deterministic patterns
        return SelectRandomFromVector(unused);
    }

    // If all are used find the oldest one
    return FindOldestTransitionNode(node_ids);
}

/**
 * @brief Selects the least recently used entity for a specific action
 * @param action_id Unique identifier of the action being performed
 * @param entity_ids Vector of entity identifiers that support the action
 * @return Entity ID that should be selected for variety, or -1 on error
 *
 */
int32_t MemorySystem::GetLeastRecentlyUsedEntityForAction(int32_t action_id, const std::vector<int32_t> &entity_ids) const
{
    if (entity_ids.empty())
    {
        LogError("GetLeastRecentlyUsedEntityForAction: There are no entity_ids to search through");
        return -1;
    }

    // Try to get an unused entity first
    std::vector<int32_t> unused {FindUnusedActionEntities(action_id, entity_ids)};
    if (!unused.empty())
    {
        // Random selection among unused entities prevents deterministic patterns
        return SelectRandomFromVector(unused);
    }

    // If all are used find the oldest one
    return FindOldestActionEntity(action_id, entity_ids);
}

// =============================================================================
// MEMORY CLEANUP OPERATIONS
// =============================================================================

/**
 * @brief Removes all interruption memories associated with a specific sequence
 * @param sequence_id Unique identifier of the sequence whose memories should be cleared
 *
 */
void MemorySystem::ClearSequenceInterruptionMemories(int32_t sequence_id)
{
    // Iterate backwards through the vector to remove the elements safely.
    for (auto i = static_cast<int>(interruption_memories.size()) - 1; i >= 0; i--)
    {
        if (interruption_memories[i].GetInterruptedSequenceId() == sequence_id)
        {
            interruption_memories.erase(interruption_memories.begin() + i);
        }
    }
}

/**
 * @brief Removes a specific interruption memory matching the execution context
 * @param action_id Action identifier of the memory to remove
 * @param sequence_id Sequence identifier of the memory to remove
 * @param node_id Sequence node identifier of the memory to remove
 * @return true if memory was found and removed, false if not found
 *
 */
bool MemorySystem::RemoveInterruptionMemory(int32_t action_id, int32_t sequence_id, int32_t node_id)
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

/**
 * @brief Clears all memories from all memory collections
 *
 */
void MemorySystem::ClearAllMemories()
{
    transition_memories.clear();
    action_memories.clear();
    interruption_memories.clear();
}

// =============================================================================
// UTILITY HELPERS
// =============================================================================

/**
 * @brief Logs error messages to standard error output
 * @param message Error message to log
 *
 */
void MemorySystem::LogError(const std::string &message) const
{
    std::cerr << "[MemorySystem Error]: " << message << std::endl;
}

/**
 * @brief Selects a random element from a vector of integers
 * @param options Vector of integer options to choose from
 * @return Randomly selected integer from the vector, or -1 if empty
 *
 * @algorithm Modulo-based random selection using rand()
 */
int32_t MemorySystem::SelectRandomFromVector(const std::vector<int32_t> &options) const
{
    if (options.empty())
    {
        LogError("SelectRandomFromVector: There are no options to search through");
        return -1;
    }

    //TODO: Modify to use c++11 random
    return options[rand() % options.size()];
}

// =============================================================================
// CAPACITY MANAGEMENT
// =============================================================================

/**
 * @brief Enforces maximum transition memory capacity by removing the oldest entries
 *
 * @algorithm Conditional oldest-first removal using front-of-vector deletion
 */
void MemorySystem::EnforceMaxTransitionMemories()
{
    auto excess_memories_count = transition_memories.size() > max_transition_memories ?
        static_cast<int32_t>(transition_memories.size() - max_transition_memories) :
        0;

    if ( excess_memories_count > 0)
    {
        transition_memories.erase(transition_memories.begin(), transition_memories.begin() + excess_memories_count);
    }
}

/**
 * @brief Enforces maximum action memory capacity by removing the oldest entries
 *
 */
void MemorySystem::EnforceMaxActionMemories()
{
    auto excess_memories_count = action_memories.size() > max_action_memories ?
        static_cast<int32_t>(action_memories.size() - max_action_memories) :
        0;

    if ( excess_memories_count > 0)
    {
        action_memories.erase(action_memories.begin(), action_memories.begin() + excess_memories_count);
    }
}

/**
 * @brief Enforces maximum interruption memory capacity by removing the oldest entries
 *
 */
void MemorySystem::EnforceMaxInterruptionMemories()
{
    auto excess_memories_count = interruption_memories.size() > max_interruption_memories ?
        static_cast<int32_t>(interruption_memories.size() - max_interruption_memories) :
        0;

    if ( excess_memories_count > 0)
    {
        interruption_memories.erase(interruption_memories.begin(), interruption_memories.begin() + excess_memories_count);
    }
}

// =============================================================================
// MEMORY REMOVAL HELPERS
// =============================================================================

/**
 * @brief Removes existing transition memory matching the specified node ID
 * @param target_node_id Node identifier of the memory to remove
 *
 */
void MemorySystem::RemoveExistingTransitionMemory(int32_t target_node_id)
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

/**
 * @brief Removes existing action memory matching the specified action and entity
 * @param action_id Action identifier of the memory to remove
 * @param target_entity_id Entity identifier of the memory to remove
 *
 */
void MemorySystem::RemoveExistingActionMemory(int32_t action_id, int32_t target_entity_id)
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

/**
 * @brief Finds the oldest transition node among the given options
 * @param node_ids Vector of node identifiers to search through
 * @return Node ID with the oldest timestamp, or -1 on error
 *
 */
int32_t MemorySystem::FindOldestTransitionNode(const std::vector<int32_t> &node_ids) const
{
    if (node_ids.empty())
    {
        LogError("FindOldestTransitionNode: There are no node_ids to search through");
        return -1;
    }

    std::vector<int32_t> oldest_nodes;
    oldest_nodes.reserve(node_ids.size());
    int64_t oldest_time = INT_MAX;

    // Search for memories and track oldest timestamp(s)
    for (auto node_id : node_ids)
    {
        const TransitionMemory* memory = FindTransitionMemory(node_id);
        if (memory == nullptr)
        {
            LogError("FindOldestTransitionNode: Expected memory not found for node " + std::to_string(node_id));
            continue;
        }

        int64_t current_time {memory->GetLastUsedTime()};

        // If there's a new oldest_time, we restart the oldest_nodes vector.
        if (current_time < oldest_time)
        {
            oldest_time = current_time;
            oldest_nodes.clear();
            oldest_nodes.emplace_back(node_id);
        } // If it there is a tie, we just add it to the existing oldest_nodes vector.
        else if (current_time == oldest_time)
        {
            oldest_nodes.emplace_back(node_id);
        }
    }

    if (oldest_nodes.empty())
    {
        LogError("FindOldestTransitionNode: No valid memories found");
        return -1;
    }

    // Random selection among tied options prevents deterministic patterns
    return SelectRandomFromVector(oldest_nodes);
}

/**
 * @brief Finds the oldest action entity for a specific action among given options
 * @param action_id Action identifier to search for
 * @param entity_ids Vector of entity identifiers to compare
 * @return Entity ID with the oldest timestamp for this action, or -1 on error
 *
 */
int32_t MemorySystem::FindOldestActionEntity(int32_t action_id, const std::vector<int32_t> &entity_ids) const
{
    if (entity_ids.empty())
    {
        LogError("FindOldestActionEntity: There are no entity_ids to search through");
        return -1;
    }

    std::vector<int32_t> oldest_nodes;
    oldest_nodes.reserve(entity_ids.size());
    int64_t oldest_time = INT_MAX;

    // Search for action-specific memories and track oldest timestamp(s)
    for (int entity_id : entity_ids)
    {
        const ActionMemory* memory = FindActionMemory(action_id, entity_id);
        if (memory == nullptr)
        {
            LogError("FindOldestActionEntity: Expected memory not found for action " + std::to_string(action_id) +
                " and entity " + std::to_string(entity_id));

            continue;
        }

        int64_t current_time = memory->GetLastUsedTime();

        // If there's a new oldest_time, we restart the oldest_nodes vector.
        if (current_time < oldest_time)
        {
            oldest_time = current_time;
            oldest_nodes.clear();
            oldest_nodes.emplace_back(entity_id);
        }
        else if (current_time == oldest_time)
        {
            oldest_nodes.emplace_back(entity_id);
        }
    }

    if (oldest_nodes.empty())
    {
        LogError("FindOldestActionEntity: No valid memories found");
        return -1;
    }

    // Random selection among tied options prevents deterministic patterns
    return SelectRandomFromVector(oldest_nodes);
}

/**
 * @brief Finds transition nodes that have never been visited (no memory exists)
 * @param node_ids Vector of node identifiers to check
 * @return Vector of node IDs that have no corresponding transition memory
 *
 */
std::vector<int32_t> MemorySystem::FindUnusedTransitionNodes(const std::vector<int32_t> &node_ids) const
{
    std::vector<int32_t> unused_nodes;
    for (int32_t node_id : node_ids)
    {
        const TransitionMemory* memory = FindTransitionMemory(node_id);
        if (memory == nullptr)
        {
            unused_nodes.emplace_back(node_id);
        }
    }

    return unused_nodes;
}

/**
 * @brief Finds entities that have never been used for a specific action
 * @param action_id Action identifier to check for
 * @param entity_ids Vector of entity identifiers to check
 * @return Vector of entity IDs that have no corresponding action memory for this action
 *
 */
std::vector<int32_t> MemorySystem::FindUnusedActionEntities(int32_t action_id, const std::vector<int32_t> &entity_ids) const
{
    std::vector<int32_t> unused_entities;
    for (int32_t entity_id : entity_ids)
    {
        const ActionMemory* memory = FindActionMemory(action_id, entity_id);
        if (memory == nullptr)
        {
            unused_entities.emplace_back(entity_id);
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
