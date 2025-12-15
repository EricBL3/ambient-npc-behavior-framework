#include "MemorySystem.h"
#include <algorithm>
#include <iostream>
#include <tracy/Tracy.hpp>

using namespace AmbientCharacterBehavior;


MemorySystem::MemorySystem(int32_t max_transitions, int32_t max_actions, int32_t max_interruptions, ILogger& logger) :
logger(logger), rng(std::random_device{}())
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

        logger.LogInfo("Added transition memory with target_node_id = " + std::to_string(target_node_id) +
            " current_time = " + std::to_string(current_time), "UpdateTransitionMemory");

        return true;
    } catch (const std::exception& e)
    {
        logger.LogError("UpdateTransitionMemory failed: " + std::string(e.what()), "UpdateTransitionMemory");
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
            std::to_string(target_entity_id) + " current_time = " + std::to_string(current_time), "UpdateActionMemory");

        return true;
    } catch (const std::exception& e)
    {
        logger.LogError("UpdateActionMemory failed: " + std::string(e.what()), "UpdateActionMemory");
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

        logger.LogInfo("Added interruption memory with action_id = " + std::to_string(action_id) + " sequence_id = " +
            std::to_string(sequence_id) + " node_id = " + std::to_string(node_id) + " entity_id = " + std::to_string(entity_id) +
            " current_time = " + std::to_string(current_time), "UpdateInterruptionMemory");

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

std::optional<int32_t> MemorySystem::SelectTransitionNodeId(const std::vector<int32_t> &valid_node_ids)
{
    ZoneScoped;

    // Early returns for trivial cases
    if (valid_node_ids.empty())
    {
        return std::nullopt;
    }

    if (valid_node_ids.size() == 1) {
        return valid_node_ids[0];
    }

    // Phase 1: Separate unused from used nodes
    std::vector<int32_t> unused_nodes;
    unused_nodes.reserve(valid_node_ids.size());
    std::vector<const TransitionMemory*> used_transition_memories;
    used_transition_memories.reserve(valid_node_ids.size());
    for (auto node_id : valid_node_ids)
    {
        const auto transition_memory = FindTransitionMemory(node_id);
        if (!transition_memory)
        {
            unused_nodes.emplace_back(node_id);
        }
        else
        {
            used_transition_memories.push_back(transition_memory);
        }
    }

    // Phase 2: Exploration - prefer unused nodes
    if (!unused_nodes.empty())
    {
        if (unused_nodes.size() == 1)
        {
            return unused_nodes[0];
        }

        auto random_index = GetRandomIndex(static_cast<int32_t>(unused_nodes.size()));
        return unused_nodes[random_index];
    }

    // Phase 3: Exploitation - select least recently used
    if (used_transition_memories.empty())
    {
        logger.LogWarning("All nodes unused but exploration failed",
            "MemorySystem");

        return std::nullopt;
    }

    std::vector<int32_t> oldest_nodes;
    oldest_nodes.reserve(used_transition_memories.size());
    int64_t oldest_time = INT64_MAX;
    for (auto transition_memory : used_transition_memories)
    {

        int64_t current_time {transition_memory->GetLastUsedTime()};

        // If there's a new oldest_time, we restart the oldest_nodes vector.
        if (current_time < oldest_time)
        {
            oldest_time = current_time;
            oldest_nodes.clear();
            oldest_nodes.emplace_back(transition_memory->GetTargetNodeId());
        } // If there is a tie, we just add it to the existing oldest_nodes vector.
        else if (current_time == oldest_time)
        {
            oldest_nodes.emplace_back(transition_memory->GetTargetNodeId());
        }
    }

    // This should never happen (oldest_nodes is based on used_transition_memories not being empty) but is kept as a defensive check.
    if (oldest_nodes.empty())
    {
        return std::nullopt;
    }

    if (oldest_nodes.size() == 1)
    {
        return oldest_nodes[0];
    }

    // Phase 4: Random tie-breaking
    auto random_index = GetRandomIndex(static_cast<int32_t>(oldest_nodes.size()));
    return oldest_nodes[random_index];
}

std::optional<int32_t> MemorySystem::SelectActionEntityId(int32_t action_id, const std::vector<int32_t> &valid_entity_ids)
{
    ZoneScoped;

    // Early returns for trivial cases
    if (valid_entity_ids.empty())
    {
        return std::nullopt;
    }

    if (valid_entity_ids.size() == 1)
    {
        return valid_entity_ids[0];
    }

    // Phase 1: Separate unused from used entities
    std::vector<int32_t> unused_entities;
    unused_entities.reserve(valid_entity_ids.size());
    std::vector<const ActionMemory*> used_action_memories;
    used_action_memories.reserve(valid_entity_ids.size());
    for (auto entity_id : valid_entity_ids)
    {
        const auto memory = FindActionMemory(action_id, entity_id);
        if (!memory)
        {
            unused_entities.emplace_back(entity_id);
        }
        else
        {
            used_action_memories.push_back(memory);
        }
    }

    // Phase 2: Exploration - prefer unused entities
    if (!unused_entities.empty())
    {
        if (unused_entities.size() == 1)
        {
            return unused_entities[0];
        }

        auto random_index = GetRandomIndex(static_cast<int32_t>(unused_entities.size()));
        return unused_entities[random_index];
    }

    // Phase 3: Exploitation - select least recently used
    if (used_action_memories.empty())
    {
        return std::nullopt;
    }

    std::vector<int32_t> oldest_entities;
    oldest_entities.reserve(used_action_memories.size());
    int64_t oldest_time = INT64_MAX;
    for (auto memory : used_action_memories)
    {

        int64_t current_time {memory->GetLastUsedTime()};

        // If there's a new oldest_time, we restart the oldest_entities vector.
        if (current_time < oldest_time)
        {
            oldest_time = current_time;
            oldest_entities.clear();
            oldest_entities.emplace_back(memory->GetTargetEntityId());
        } // If it there is a tie, we just add it to the existing oldest_entities vector.
        else if (current_time == oldest_time)
        {
            oldest_entities.emplace_back(memory->GetTargetEntityId());
        }
    }

    // This should never happen (oldest_entities is based on used_action_memories not being empty) but is kept as a defensive check.
    if (oldest_entities.empty())
    {
        return std::nullopt;
    }

    if (oldest_entities.size() == 1)
    {
        return oldest_entities[0];
    }

    // Phase 4: Random tie-breaking
    auto random_index = GetRandomIndex(static_cast<int32_t>(oldest_entities.size()));
    return oldest_entities[random_index];
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

bool MemorySystem::RemoveInterruptionMemory(const InterruptionMemory* memory)
{
    if (!memory) {
        return false;
    }

    return RemoveInterruptionMemory(
        memory->GetInterruptedActionId(),
        memory->GetInterruptedSequenceId(),
        memory->GetInterruptedSequenceNodeId()
    );
}

void MemorySystem::ClearAllMemories()
{
    transition_memories.clear();
    action_memories.clear();
    interruption_memories.clear();
}

void MemorySystem::EnforceMaxTransitionMemories()
{
    while (transition_memories.size() > max_transition_memories)
    {
        transition_memories.pop_front();
    }
}

void MemorySystem::EnforceMaxActionMemories()
{
    while (action_memories.size() > max_action_memories)
    {
        action_memories.pop_front();
    }
}

void MemorySystem::EnforceMaxInterruptionMemories()
{
    while (interruption_memories.size() > max_interruption_memories)
    {
        interruption_memories.pop_front();
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

int32_t MemorySystem::GetRandomIndex(int32_t max_exclusive)
{
    if (max_exclusive <= 1 )
    {
        return 0;
    }

    std::uniform_int_distribution dist(0, max_exclusive - 1);
    return dist(rng);
}