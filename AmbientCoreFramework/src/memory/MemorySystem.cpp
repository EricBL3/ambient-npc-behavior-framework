#include "MemorySystem.h"
#include <algorithm>
#include <iostream>
#include "utils/PerformanceTracker.h"

using namespace AmbientCharacterBehavior;


MemorySystem::MemorySystem(int32_t max_transitions, int32_t max_actions, int32_t max_interruptions, ILogger& logger) :
logger(logger)
{
    SetAndEnforceMaxTransitionMemories(max_transitions);
    SetAndEnforceMaxActionMemories(max_actions);
    SetAndEnforceMaxInterruptionMemories(max_interruptions);

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

void MemorySystem::SetAndEnforceMaxTransitionMemories(int32_t max_transitions)
{
    if (max_transitions <= 0 || max_transitions == max_transition_memories)
    {
        logger.LogError("max_transitions must be greater than 0 and different from the current "
                        "maximum. Keeping current value", "MemorySystem");

        return;
    }

    max_transition_memories = max_transitions;
    transition_memories.reserve(max_transitions);
    EnforceMaxTransitionMemories();
}

void MemorySystem::SetAndEnforceMaxActionMemories(int32_t max_actions)
{
    if (max_actions <= 0 || max_actions == max_action_memories)
    {
        logger.LogError("max_actions must be greater than 0, and different from the current "
                        "maximum. Keeping current value", "MemorySystem");

        return;
    }

    max_action_memories = max_actions;
    action_memories.reserve(max_actions);
    EnforceMaxActionMemories();

}

void MemorySystem::SetAndEnforceMaxInterruptionMemories(int32_t max_interruptions)
{
    if (max_interruptions <= 0 || max_interruptions == max_interruption_memories)
    {
        logger.LogError("max_interruptions must be greater than 0, and different from the current maximum. "
                 "Keeping current value", "MemorySystem");
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
 * @return true if update succeeded, false if validation or creation failed
 */
bool MemorySystem::UpdateTransitionMemory(int32_t target_node_id, int64_t current_time)
{
    try
    {
        TransitionMemory new_memory(target_node_id, current_time);
        RemoveExistingTransitionMemory(target_node_id);
        transition_memories.push_back(new_memory);
        EnforceMaxTransitionMemories();

        return true;
    } catch (const std::exception& e)
    {
        logger.LogError("UpdateTransitionMemory failed: " + std::string(e.what()), "MemorySystem");
        return false;
    }
}

/**
 * @return true if update succeeded, false if validation or creation failed
 *
 */
bool MemorySystem::UpdateActionMemory(int32_t action_id, int32_t target_entity_id, int64_t current_time)
{
    try
    {
        ActionMemory new_memory(action_id, target_entity_id, current_time);
        RemoveExistingActionMemory(action_id, target_entity_id);
        action_memories.push_back(new_memory);
        EnforceMaxActionMemories();

        logger.LogInfo("Added action memory with action_id = " + std::to_string(action_id) + " target_entity_id = " +
            std::to_string(target_entity_id) + " current_time = " + std::to_string(current_time), "MemorySystem");

        return true;
    } catch (const std::exception& e)
    {
        logger.LogError("UpdateActionMemory failed: " + std::string(e.what()), "MemorySystem");
        return false;
    }
}

/**
 * @return true if update succeeded, false if validation or creation failed
 */
bool MemorySystem::UpdateInterruptionMemory(int32_t action_id, int32_t sequence_id, int32_t node_id, int32_t entity_id,
    int64_t current_time)
{
    try
    {
        InterruptionMemory new_memory(action_id, sequence_id, node_id, entity_id, current_time);
        RemoveInterruptionMemory(action_id, sequence_id, node_id);
        interruption_memories.push_back(new_memory);
        EnforceMaxInterruptionMemories();

        return true;

    } catch (const std::exception& e)
    {
        logger.LogError("UpdateInterruptionMemory failed: " + std::string(e.what()), "MemorySystem");
        return false;
    }
}

const TransitionMemory * MemorySystem::FindTransitionMemory(int32_t target_node_id) const
{
    // find_if performs a linear search over the memories
    auto iterator = std::find_if(transition_memories.begin(), transition_memories.end(),
        [target_node_id](const TransitionMemory& memory) // lambda function: for each memory in the vector
        {
           return  memory.MatchesMemory(target_node_id); // return true if it matches
        });

    return (iterator !=  transition_memories.end()) ? &(*iterator) : nullptr;
}

const ActionMemory * MemorySystem::FindActionMemory(int32_t action_id, int32_t target_entity_id) const
{
    // find_if performs a linear search over the memories
    auto iterator = std::find_if(action_memories.begin(), action_memories.end(),
        [action_id, target_entity_id](const ActionMemory& memory) // lambda function: for each memory in the vector
        {
           return  memory.MatchesMemory(action_id, target_entity_id); // return true if it matches
        });

    return (iterator !=  action_memories.end()) ? &(*iterator) : nullptr;
}

const InterruptionMemory * MemorySystem::FindInterruptionMemory(int32_t action_id, int32_t sequence_id, int32_t node_id) const
{
    // find_if performs a linear search over the memories
    auto iterator = std::find_if(interruption_memories.begin(), interruption_memories.end(),
        [action_id, sequence_id, node_id](const InterruptionMemory& memory) // lambda function: for each memory in the vector
        {
           return  memory.MatchesMemory(action_id, sequence_id, node_id); // return true if it matches
        });

    return (iterator !=  interruption_memories.end()) ? &(*iterator) : nullptr;

}

int32_t MemorySystem::GetLeastRecentlyVisitedNodeId(const std::vector<int32_t> &node_ids) const
{
    if (node_ids.empty())
    {
        logger.LogError("GetLeastRecentlyVisitedNode: There are no node_ids to search through",
            "MemorySystem");

        return -1;
    }

    if (node_ids.size() == 1) {
        return node_ids[0];
    }

    std::vector<int32_t> unused {FindUnusedTransitionNodeIds(node_ids)};
    if (!unused.empty())
    {
        return SelectRandomFromVector(unused);
    }

    return FindOldestTransitionNodeId(node_ids);
}

int32_t MemorySystem::GetLeastRecentlyUsedEntityIdForAction(int32_t action_id, const std::vector<int32_t> &entity_ids) const
{
    if (entity_ids.empty())
    {
        logger.LogError("GetLeastRecentlyUsedEntityForAction: There are no entity_ids to search through",
            "MemorySystem");

        return -1;
    }

    std::vector<int32_t> unused {FindUnusedActionEntityIds(action_id, entity_ids)};
    if (!unused.empty())
    {
        return SelectRandomFromVector(unused);
    }

    return FindOldestActionEntityId(action_id, entity_ids);
}

void MemorySystem::ClearSequenceInterruptionMemories(int32_t sequence_id)
{
    for (auto i = static_cast<int>(interruption_memories.size()) - 1; i >= 0; i--)
    {
        if (interruption_memories[i].GetInterruptedSequenceId() == sequence_id)
        {
            interruption_memories.erase(interruption_memories.begin() + i);
        }
    }
}

/**
 * @return true if memory was found and removed, false if not found
 */
bool MemorySystem::RemoveInterruptionMemory(int32_t action_id, int32_t sequence_id, int32_t node_id)
{
    // Search for existing memory with same action_id AND sequence_id AND node_id
    auto iterator = std::find_if(interruption_memories.begin(), interruption_memories.end(),
        [action_id, sequence_id, node_id](const InterruptionMemory& memory)
        {
           return  memory.MatchesMemory(action_id, sequence_id, node_id);
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


int32_t MemorySystem::SelectRandomFromVector(const std::vector<int32_t> &options) const
{
    if (options.empty())
    {
        logger.LogError("SelectRandomFromVector: There are no options to search through",
            "MemorySystem");

        return -1;
    }

    //TODO: Modify to use c++11 random
    return options[rand() % options.size()];
}

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

void MemorySystem::RemoveExistingTransitionMemory(int32_t target_node_id)
{
    // Search for existing memory with same target_node_id
    auto iterator = std::find_if(transition_memories.begin(), transition_memories.end(),
        [target_node_id](const TransitionMemory& memory)
        {
           return  memory.MatchesMemory(target_node_id);
        });

    // Remove existing memory if found
    if (iterator != transition_memories.end()) {
        transition_memories.erase(iterator);
    }
}

void MemorySystem::RemoveExistingActionMemory(int32_t action_id, int32_t target_entity_id)
{
    // Search for existing memory with same target_node_id
    auto iterator = std::find_if(action_memories.begin(), action_memories.end(),
        [action_id, target_entity_id](const ActionMemory& memory)
        {
           return  memory.MatchesMemory(action_id, target_entity_id);
        });

    // Remove existing memory if found
    if (iterator != action_memories.end()) {
        action_memories.erase(iterator);
    }
}

int32_t MemorySystem::FindOldestTransitionNodeId(const std::vector<int32_t> &node_ids) const
{
    if (node_ids.empty())
    {
        logger.LogError("FindOldestTransitionNode: There are no node_ids to search through",
            "MemorySystem");

        return -1;
    }

    std::vector<int32_t> oldest_nodes;
    oldest_nodes.reserve(node_ids.size());
    int64_t oldest_time = INT64_MAX;

    // Search for memories and track oldest timestamp(s)
    for (auto node_id : node_ids)
    {
        const TransitionMemory* memory = FindTransitionMemory(node_id);
        if (memory == nullptr)
        {
            logger.LogError("FindOldestTransitionNode: Expected memory not found for node " + std::to_string(node_id),
                "MemorySystem");
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
        logger.LogError("FindOldestTransitionNode: No valid memories found", "MemorySystem");
        return -1;
    }

    // Random selection among tied options prevents deterministic patterns
    return SelectRandomFromVector(oldest_nodes);
}

int32_t MemorySystem::FindOldestActionEntityId(int32_t action_id, const std::vector<int32_t> &entity_ids) const
{
    if (entity_ids.empty())
    {
        logger.LogError("FindOldestActionEntity: There are no entity_ids to search through",
            "MemorySystem");
        return -1;
    }

    logger.LogInfo("FindOldestActionEntityId: Searching for action " + std::to_string(action_id) +
                   " among " + std::to_string(entity_ids.size()) + " entities", "MemorySystem");

    logger.LogInfo("Total action memories: " + std::to_string(action_memories.size()), "MemorySystem");

    // Log all action memories
    for (const auto& mem : action_memories)
    {
        logger.LogInfo("  Memory: action=" + std::to_string(mem.GetActionId()) +
                       ", entity=" + std::to_string(mem.GetTargetEntityId()) +
                       ", time=" + std::to_string(mem.GetLastUsedTime()), "MemorySystem");
    }

    std::vector<int32_t> oldest_nodes;
    oldest_nodes.reserve(entity_ids.size());
    int64_t oldest_time = INT64_MAX;

    // Search for action-specific memories and track oldest timestamp(s)
    for (int entity_id : entity_ids)
    {
        const ActionMemory* memory = FindActionMemory(action_id, entity_id);
        if (memory == nullptr)
        {
            logger.LogError("FindOldestActionEntity: Expected memory not found for action " + std::to_string(action_id) +
                " and entity " + std::to_string(entity_id), "MemorySystem");

            continue;
        }

        int64_t current_time = memory->GetLastUsedTime();

        // If there's a new oldest_time, we restart the oldest_nodes vector.
        if (current_time < oldest_time)
        {
            logger.LogInfo("Found a new oldest time", "MemorySystem");
            oldest_time = current_time;
            oldest_nodes.clear();
            oldest_nodes.emplace_back(entity_id);
        }
        else if (current_time == oldest_time)
        {
            logger.LogInfo("Oldest time is equal to current time", "MemorySystem");
            oldest_nodes.emplace_back(entity_id);
        }
    }

    if (oldest_nodes.empty())
    {
        logger.LogError("FindOldestActionEntity: No valid memories found", "MemorySystem");
        return -1;
    }

    // Random selection among tied options prevents deterministic patterns
    return SelectRandomFromVector(oldest_nodes);
}

std::vector<int32_t> MemorySystem::FindUnusedTransitionNodeIds(const std::vector<int32_t> &node_ids) const
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

std::vector<int32_t> MemorySystem::FindUnusedActionEntityIds(int32_t action_id, const std::vector<int32_t> &entity_ids) const
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
