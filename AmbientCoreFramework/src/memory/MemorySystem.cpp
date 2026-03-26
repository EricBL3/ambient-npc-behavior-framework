#include "MemorySystem.h"
#include <algorithm>
#include <iostream>
#include <tracy/Tracy.hpp>
using namespace AmbientCharacterBehavior;

// =============================================================================
// CONSTRUCTION & CONFIGURATION
// =============================================================================

MemorySystem::MemorySystem(int32_t max_transitions, int32_t max_actions, int32_t max_interruptions, FoundationServices& services) :
    max_transition_memories(0), max_action_memories(0), max_interruption_memories(0), services(services),
    rng(std::random_device{}()) // Seed RNG with random device
{
    ConfigureMaxTransitionMemories(max_transitions);
    ConfigureMaxActionMemories(max_actions);
    ConfigureMaxInterruptionMemories(max_interruptions);

    ClearAllMemories();
}

void MemorySystem::ConfigureMaxTransitionMemories(int32_t max_transitions)
{
    // Validate new capacity
    if (max_transitions <= 0 || max_transitions == max_transition_memories)
    {
        Logger().LogWarning("max_transitions must be greater than 0 and different from the current "
                        "maximum. Keeping current value", "MemorySystem");

        return;
    }

    max_transition_memories = max_transitions;
    // Remove excess if new limit is lower
    EnforceMaxTransitionMemories();
}

void MemorySystem::ConfigureMaxActionMemories(int32_t max_actions)
{
    // Validate new capacity
    if (max_actions <= 0 || max_actions == max_action_memories)
    {
        Logger().LogWarning("max_actions must be greater than 0, and different from the current "
                        "maximum. Keeping current value", "MemorySystem");

        return;
    }

    max_action_memories = max_actions;
    // Remove excess if new limit is lower
    EnforceMaxActionMemories();

}

void MemorySystem::ConfigureMaxInterruptionMemories(int32_t max_interruptions)
{
    // Validate new capacity
    if (max_interruptions <= 0 || max_interruptions == max_interruption_memories)
    {
        Logger().LogWarning("max_interruptions must be greater than 0, and different from the current maximum. "
                 "Keeping current value", "MemorySystem");
        return;
    }

    max_interruption_memories = max_interruptions;
    // Remove excess if new limit is lower
    EnforceMaxInterruptionMemories();

}

// =============================================================================
// MEMORY CREATION
// =============================================================================

bool MemorySystem::CreateTransitionMemory(int32_t sequence_id, int32_t target_node_id, int64_t current_time)
{
    try
    {
        // Create new memory (validates parameters in constructor)
        const TransitionMemory new_memory(sequence_id, target_node_id, current_time);
        RemoveExistingTransitionMemory(sequence_id, target_node_id);
        transition_memories.push_back(new_memory);

        // Remove the oldest memories if over capacity
        EnforceMaxTransitionMemories();

        Logger().LogInfo("Created transition memory: sequence=" + std::to_string(sequence_id) + " node=" +
            std::to_string(target_node_id), "MemorySystem");

        return true;
    } catch (const std::exception& e)
    {
        Logger().LogError("CreateTransitionMemory failed: " + std::string(e.what()),
            "CreateTransitionMemory");

        return false;
    }
}

bool MemorySystem::CreateActionMemory(int32_t action_id, int32_t target_entity_id, int64_t current_time)
{
    try
    {
        // Create new memory (validates parameters in constructor)
        const ActionMemory new_memory(action_id, target_entity_id, current_time);

        RemoveExistingActionMemory(action_id, target_entity_id);
        action_memories.push_back(new_memory);

        // Remove the oldest memories if over capacity
        EnforceMaxActionMemories();

        Logger().LogInfo("Created action memory: action_id = " + std::to_string(action_id) + " target_entity_id = " +
            std::to_string(target_entity_id), "CreateActionMemory");

        return true;
    } catch (const std::exception& e)
    {
        Logger().LogError("CreateActionMemory failed: " + std::string(e.what()), "CreateActionMemory");
        return false;
    }
}

bool MemorySystem::CreateInterruptionMemory(int32_t action_id, int32_t sequence_id, int32_t node_id, int32_t entity_id,
    int64_t current_time)
{
    try
    {
        // Create new memory (validates parameters in constructor)
        const InterruptionMemory new_memory(action_id, sequence_id, node_id, entity_id, current_time);
        RemoveInterruptionMemory(action_id, sequence_id, node_id);
        interruption_memories.push_back(new_memory);

        // Remove the oldest memories if over capacity
        EnforceMaxInterruptionMemories();

        Logger().LogInfo("Created interruption memory: action_id = " + std::to_string(action_id) + " sequence_id = " +
            std::to_string(sequence_id) + " node_id = " + std::to_string(node_id) + " entity_id = " + std::to_string(entity_id),
            "CreateInterruptionMemory");

        return true;

    } catch (const std::exception& e)
    {
        Logger().LogError("CreateInterruptionMemory failed: " + std::string(e.what()),
            "CreateInterruptionMemory");
        return false;
    }
}

// =============================================================================
// MEMORY QUERY
// =============================================================================

const TransitionMemory * MemorySystem::FindTransitionMemory(int32_t sequence_id, int32_t target_node_id) const
{
    // linear search is appropriate for small containers
    const auto iterator = std::ranges::find_if(
        transition_memories,
         [sequence_id, target_node_id](const TransitionMemory& memory) // lambda function: for each memory in the container
         {
             return memory.MatchesMemory(sequence_id, target_node_id); // return if it matches
         }
    );

    return iterator !=  transition_memories.end() ? &(*iterator) : nullptr;
}

const ActionMemory * MemorySystem::FindActionMemory(int32_t action_id, int32_t target_entity_id) const
{
    // linear search is appropriate for small containers
    const auto iterator = std::ranges::find_if(
        action_memories,
         [action_id, target_entity_id](const ActionMemory& memory) // lambda function: for each memory in the container
         {
             return memory.MatchesMemory(action_id, target_entity_id); // return if it matches
         }
    );

    return iterator !=  action_memories.end() ? &(*iterator) : nullptr;
}

const InterruptionMemory * MemorySystem::FindInterruptionMemory(int32_t action_id, int32_t sequence_id, int32_t node_id) const
{
    // linear search is appropriate for small containers
    const auto iterator = std::ranges::find_if(
        interruption_memories,
         [action_id, sequence_id, node_id](const InterruptionMemory& memory) // lambda function: for each memory in the container
         {
             return  memory.MatchesMemory(action_id, sequence_id, node_id); // return if it matches
         }
    );

    return iterator !=  interruption_memories.end() ? &(*iterator) : nullptr;

}

// =============================================================================
// MEMORY-DRIVEN SELECTION
// =============================================================================

std::optional<int32_t> MemorySystem::SelectTransitionNodeId(int32_t sequence_id, const std::vector<int32_t> &valid_node_ids)
{
    // Performance profiling marker
    ZoneScoped;

    // ===== Early returns for trivial cases =====

    if (valid_node_ids.empty())
    {
        // No options available
        return std::nullopt;
    }

    if (valid_node_ids.size() == 1) {
        // Only one option, no selection needed
        return valid_node_ids[0];
    }

    // ===== Phase 1: Separate unused from used nodes =====

    std::vector<int32_t> unused_nodes;
    unused_nodes.reserve(valid_node_ids.size());
    std::vector<const TransitionMemory*> used_transition_memories;
    used_transition_memories.reserve(valid_node_ids.size());

    // Single-pass separation: check each node for existing memory
    for (auto node_id : valid_node_ids)
    {
        const auto transition_memory = FindTransitionMemory(sequence_id, node_id);
        if (!transition_memory)
        {
            // No memory = unused
            unused_nodes.emplace_back(node_id);
        }
        else
        {
            // Store memory pointer for least recently used selection
            used_transition_memories.push_back(transition_memory);
        }
    }

    // ===== Phase 2: prefer unused nodes =====

    if (!unused_nodes.empty())
    {
        if (unused_nodes.size() == 1)
        {
            // Only one unused, no randomization needed
            return unused_nodes[0];
        }

        auto random_index = GetRandomIndex(static_cast<int32_t>(unused_nodes.size()));
        return unused_nodes[random_index];
    }

    // ===== Phase 3: select least recently used =====

    if (used_transition_memories.empty())
    {
        // Should be impossible: we have valid nodes but neither unused nor used
        Logger().LogWarning("All nodes unused but exploration failed",
            "MemorySystem");

        return std::nullopt;
    }

    // Find node(s) with oldest timestamp
    std::vector<int32_t> oldest_nodes;
    oldest_nodes.reserve(used_transition_memories.size());
    int64_t oldest_time = INT64_MAX;

    for (const auto transition_memory : used_transition_memories)
    {
        int64_t current_time {transition_memory->GetCreationTime()};

        if (current_time < oldest_time)
        {
            // Found new oldest - restart collection
            oldest_time = current_time;
            oldest_nodes.clear();
            oldest_nodes.emplace_back(transition_memory->GetTargetNodeId());
        } // If there is a tie, we just add it to the existing oldest_nodes vector.
        else if (current_time == oldest_time)
        {
            // Tie with current oldest - add to collection
            oldest_nodes.emplace_back(transition_memory->GetTargetNodeId());
        }
    }

    // Defensive check: should never happen since used_transition_memories is non-empty
    if (oldest_nodes.empty())
    {
        return std::nullopt;
    }

    // ===== Phase 4: Random tie-breaking =====

    if (oldest_nodes.size() == 1)
    {
        // Only one oldest, no randomization needed
        return oldest_nodes[0];
    }

    auto random_index = GetRandomIndex(static_cast<int32_t>(oldest_nodes.size()));
    return oldest_nodes[random_index];
}

std::optional<int32_t> MemorySystem::SelectActionEntityId(int32_t action_id, const std::vector<int32_t> &valid_entity_ids)
{
    // Performance profiling marker
    ZoneScoped;

    // ===== Early returns for trivial cases =====

    if (valid_entity_ids.empty())
    {
        // No options available
        return std::nullopt;
    }

    if (valid_entity_ids.size() == 1)
    {
        // Only one option, no selection needed
        return valid_entity_ids[0];
    }

    // ===== Phase 1: Separate unused from used entities =====

    std::vector<int32_t> unused_entities;
    unused_entities.reserve(valid_entity_ids.size());
    std::vector<const ActionMemory*> used_action_memories;
    used_action_memories.reserve(valid_entity_ids.size());

    // Single-pass separation: check each entity for existing memory
    for (auto entity_id : valid_entity_ids)
    {
        const auto memory = FindActionMemory(action_id, entity_id);
        if (!memory)
        {
            // No memory = unused
            unused_entities.emplace_back(entity_id);
        }
        else
        {
            // Store pointer for least recently used selection
            used_action_memories.push_back(memory);
        }
    }

    // ===== Phase 2:prefer unused entities =====

    if (!unused_entities.empty())
    {
        if (unused_entities.size() == 1)
        {
            // Only one unused, no randomization needed
            return unused_entities[0];
        }

        auto random_index = GetRandomIndex(static_cast<int32_t>(unused_entities.size()));
        return unused_entities[random_index];
    }

    // ===== Phase 3: select least recently used =====

    if (used_action_memories.empty())
    {
        // Should be impossible: we have valid entities but neither unused nor used
        return std::nullopt;
    }

    // Find entity(s) with oldest timestamp
    std::vector<int32_t> oldest_entities;
    oldest_entities.reserve(used_action_memories.size());
    int64_t oldest_time = INT64_MAX;

    for (const auto memory : used_action_memories)
    {
        int64_t current_time {memory->GetCreationTime()};

        if (current_time < oldest_time)
        {
            // Found new oldest - restart collection
            oldest_time = current_time;
            oldest_entities.clear();
            oldest_entities.emplace_back(memory->GetTargetEntityId());
        }
        else if (current_time == oldest_time)
        {
            // Tie with current oldest - add to collection
            oldest_entities.emplace_back(memory->GetTargetEntityId());
        }
    }

    // Defensive check: should never happen since used_action_memories is non-empty
    if (oldest_entities.empty())
    {
        return std::nullopt;
    }

    // ===== Phase 4: Random tie-breaking =====

    if (oldest_entities.size() == 1)
    {
        // Only one oldest, no randomization needed
        return oldest_entities[0];
    }

    auto random_index = GetRandomIndex(static_cast<int32_t>(oldest_entities.size()));
    return oldest_entities[random_index];
}

// =============================================================================
// MEMORY MANAGEMENT
// =============================================================================

void MemorySystem::EnforceMaxTransitionMemories()
{
    // Remove the oldest memories (from front) until within capacity using deque::pop_front() for O(1) removal
    while (transition_memories.size() > max_transition_memories)
    {
        transition_memories.pop_front();
    }
}

void MemorySystem::RemoveExistingTransitionMemory(int32_t sequence_id, int32_t target_node_id)
{
    // Search for duplicate memory
    const auto iterator = std::ranges::find_if(
        transition_memories,
         [sequence_id, target_node_id](const TransitionMemory& memory)
         {
             return  memory.MatchesMemory(sequence_id, target_node_id);
         }
    );

    // Remove if found (ensures each decision recorded only once)
    if (iterator != transition_memories.end()) {
        transition_memories.erase(iterator);
    }
}

void MemorySystem::EnforceMaxActionMemories()
{
    // Remove the oldest memories (from front) until within capacity using deque::pop_front() for O(1) removal
    while (action_memories.size() > max_action_memories)
    {
        action_memories.pop_front();
    }
}

void MemorySystem::RemoveExistingActionMemory(int32_t action_id, int32_t target_entity_id)
{
    // Search for duplicate memory
    const auto iterator = std::ranges::find_if(
        action_memories,
         [action_id, target_entity_id](const ActionMemory& memory)
         {
             return  memory.MatchesMemory(action_id, target_entity_id);
         }
    );

    // Remove if found (ensures each decision recorded only once)
    if (iterator != action_memories.end()) {
        action_memories.erase(iterator);
    }
}

void MemorySystem::EnforceMaxInterruptionMemories()
{
    // Remove the oldest memories (from front) until within capacity using deque::pop_front() for O(1) removal
    while (interruption_memories.size() > max_interruption_memories)
    {
        interruption_memories.pop_front();
    }
}

/**
 * @return true if memory was found and removed, false if not found
 */
bool MemorySystem::RemoveInterruptionMemory(int32_t action_id, int32_t sequence_id, int32_t node_id)
{
    // Search for duplicate memory
    const auto iterator = std::ranges::find_if(
        interruption_memories,
         [action_id, sequence_id, node_id](const InterruptionMemory& memory)
         {
             return  memory.MatchesMemory(action_id, sequence_id, node_id);
         }
    );

    // Remove if found (ensures each decision recorded only once)
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

void MemorySystem::ClearSequenceInterruptionMemories(int32_t sequence_id)
{
    // Iterate through interruption memories and erase those that match sequence_id
    std::erase_if(
        interruption_memories,
        [sequence_id](const InterruptionMemory& memory)
        {
            return memory.GetInterruptedSequenceId() == sequence_id;
        }
    );
}

void MemorySystem::ClearAllMemories()
{
    transition_memories.clear();
    action_memories.clear();
    interruption_memories.clear();
}

int32_t MemorySystem::GetRandomIndex(int32_t max_exclusive)
{
    if (max_exclusive <= 1 )
    {
        // No randomization needed
        return 0;
    }

    // Uniform distribution over [0, max_exclusive)
    std::uniform_int_distribution dist(0, max_exclusive - 1);
    return dist(rng);
}