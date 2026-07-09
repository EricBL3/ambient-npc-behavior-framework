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

SelectionResult MemorySystem::SelectByRecency(const std::vector<RecencyCandidate>& candidates)
{
    // ===== Phase 1: Separate unused from used candidates =====

    std::vector<int32_t> unused_ids;
    unused_ids.reserve(candidates.size());
    std::vector<const RecencyCandidate*> used_candidates;
    used_candidates.reserve(candidates.size());

    for (const auto& candidate : candidates)
    {
        if (!candidate.last_used.has_value())
        {
            unused_ids.emplace_back(candidate.id);
        }
        else
        {
            used_candidates.push_back(&candidate);
        }
    }

    // ===== Phase 2: prefer unused candidates =====

    if (!unused_ids.empty())
    {
        if (unused_ids.size() == 1)
        {
            return SelectionResult{ unused_ids[0], "unused" };
        }

        auto random_index = GetRandomIndex(static_cast<int32_t>(unused_ids.size()));
        return SelectionResult{ unused_ids[random_index], "unused_random" };
    }

    // ===== Phase 3: select least recently used =====

    if (used_candidates.empty())
    {
        // Should be impossible: we have candidates but neither unused nor used
        Logger().LogWarning("All candidates unused but exploration failed.",
            "MemorySystem");

        return SelectionResult{ std::nullopt, "no_used_or_unused" };
    }

    std::vector<int32_t> oldest_ids;
    oldest_ids.reserve(used_candidates.size());
    int64_t oldest_time = INT64_MAX;

    for (const auto* candidate : used_candidates)
    {
        int64_t current_time = candidate->last_used.value();

        if (current_time < oldest_time)
        {
            oldest_time = current_time;
            oldest_ids.clear();
            oldest_ids.emplace_back(candidate->id);
        }
        else if (current_time == oldest_time)
        {
            oldest_ids.emplace_back(candidate->id);
        }
    }

    // Defensive check: should never happen since used_candidates is non-empty
    if (oldest_ids.empty())
    {
        return SelectionResult{ std::nullopt, "no_oldest_found" };
    }

    // ===== Phase 4: Random tie-breaking =====

    if (oldest_ids.size() == 1)
    {
        return SelectionResult{ oldest_ids[0], "LRU" };
    }

    auto random_index = GetRandomIndex(static_cast<int32_t>(oldest_ids.size()));
    return SelectionResult{ oldest_ids[random_index], "LRU_random" };
}

void MemorySystem::FinalizeSelectionLog(nlohmann::json event, const SelectionResult& result) const
{
    event["branch_fired"] = result.branch_fired;
    event["selected_option"] = result.selected_id.value_or(-1);
    Logger().LogMetric(event);
}

std::optional<int32_t> MemorySystem::SelectTransitionNodeId(const std::vector<int32_t> &valid_node_ids,
    const SelectionAlgorithmInfo& selection_algorithm_info)
{
    // Performance profiling marker
    ZoneScoped;

    // ===== Early returns for trivial cases =====

    // No options available
    if (valid_node_ids.empty())
    {
        nlohmann::json event =
        {
            {"event", "decision"},
            { "ts", TimeManager().GetCurrentTime() },
            { "system_used", "none"},
            { "npc_id", selection_algorithm_info.npc_id},
            {"npc_name", selection_algorithm_info.npc_name},
            {"decision_type", "transition"},
            {"sequence_id", selection_algorithm_info.sequence_id},
            {"current_node_id", selection_algorithm_info.current_node_id},
            {"available_options", valid_node_ids},
            {"branch_fired", "no_options"},
            {"selected_option", -1}
        };

        Logger().LogMetric(event);
        return std::nullopt;
    }

    // Only one option, no selection needed
    if (valid_node_ids.size() == 1) {
        nlohmann::json event =
        {
            {"event", "decision"},
            { "ts", TimeManager().GetCurrentTime() },
            { "system_used", "none"},
            { "npc_id", selection_algorithm_info.npc_id},
            {"npc_name", selection_algorithm_info.npc_name},
            {"decision_type", "transition"},
            {"sequence_id", selection_algorithm_info.sequence_id},
            {"current_node_id", selection_algorithm_info.current_node_id},
            {"available_options", valid_node_ids},
            {"branch_fired", "one_option"},
            {"selected_option", valid_node_ids[0]}
        };

        Logger().LogMetric(event);
        return valid_node_ids[0];
    }

    // Build base event
    nlohmann::json memories = nlohmann::json::array();
    for (const auto& memory : transition_memories)
    {
        memories.push_back({
            {"sequence_id", memory.GetSequenceId()},
            {"target_node_id", memory.GetTargetNodeId()},
            {"creation_time", memory.GetCreationTime()}
        });
    }

    nlohmann::json event =
    {
        {"event", "decision"},
        { "ts", TimeManager().GetCurrentTime() },
        { "system_used", "memory"},
        { "npc_id", selection_algorithm_info.npc_id},
        {"npc_name", selection_algorithm_info.npc_name},
        {"decision_type", "transition"},
        {"sequence_id", selection_algorithm_info.sequence_id},
        {"current_node_id", selection_algorithm_info.current_node_id},
        {"available_options", valid_node_ids},
        {"memory_state", memories}
    };

    // ===== Build candidates (type specific memory lookup) =====

    std::vector<RecencyCandidate> candidates;
    candidates.reserve(valid_node_ids.size());

    // Single-pass separation: check each node for existing memory
    for (auto node_id : valid_node_ids)
    {
        const auto transition_memory = FindTransitionMemory(selection_algorithm_info.sequence_id, node_id);
        candidates.push_back(RecencyCandidate{
            node_id,
            transition_memory ? std::optional<int64_t>(transition_memory->GetCreationTime()) : std::nullopt
        });
    }

    auto result = SelectByRecency(candidates);
    FinalizeSelectionLog(event, result);
    return result.selected_id;
}

std::optional<int32_t> MemorySystem::SelectActionEntityId(const std::vector<int32_t> &valid_entity_ids,
        const SelectionAlgorithmInfo& selection_algorithm_info)
{
    // Performance profiling marker
    ZoneScoped;

    // ===== Early returns for trivial cases =====

    // No options available
    if (valid_entity_ids.empty())
    {
        nlohmann::json event =
        {
            {"event", "decision"},
            { "ts", TimeManager().GetCurrentTime() },
            { "system_used", "none"},
            { "npc_id", selection_algorithm_info.npc_id},
            {"npc_name", selection_algorithm_info.npc_name},
            {"decision_type", "entity"},
            {"action_id", selection_algorithm_info.action_id},
            {"available_options", valid_entity_ids},
            {"branch_fired", "no_options"},
            {"selected_option", -1}
        };

        Logger().LogMetric(event);
        return std::nullopt;
    }

    // Only one option, no selection needed
    if (valid_entity_ids.size() == 1)
    {
        nlohmann::json event =
        {
            {"event", "decision"},
            { "ts", TimeManager().GetCurrentTime() },
            { "system_used", "none"},
            { "npc_id", selection_algorithm_info.npc_id},
            {"npc_name", selection_algorithm_info.npc_name},
            {"decision_type", "entity"},
            {"action_id", selection_algorithm_info.action_id},
            {"available_options", valid_entity_ids},
            {"branch_fired", "one_option"},
            {"selected_option", valid_entity_ids[0]}
        };

        Logger().LogMetric(event);
        return valid_entity_ids[0];
    }

    // Build base event
    nlohmann::json memories = nlohmann::json::array();
    for (const auto& memory : action_memories)
    {
        memories.push_back({
            {"action_id", memory.GetActionId()},
            {"target_entity_id", memory.GetTargetEntityId()},
            {"creation_time", memory.GetCreationTime()}
        });
    }

    nlohmann::json event =
    {
        {"event", "decision"},
        { "ts", TimeManager().GetCurrentTime() },
        { "system_used", "memory"},
        { "npc_id", selection_algorithm_info.npc_id},
        {"npc_name", selection_algorithm_info.npc_name},
        {"decision_type", "entity"},
        {"action_id", selection_algorithm_info.action_id},
        {"available_options", valid_entity_ids},
        {"memory_state", memories}
    };

    // ===== Build candidates (type specific memory lookup) =====
    std::vector<RecencyCandidate> candidates;
    candidates.reserve(valid_entity_ids.size());

    for (auto entity_id : valid_entity_ids)
    {
        const auto action_memory = FindActionMemory(selection_algorithm_info.action_id, entity_id);

        candidates.push_back(RecencyCandidate{
            entity_id,
            action_memory ? std::optional<int64_t>(action_memory->GetCreationTime()) : std::nullopt
        });
    }

    auto result = SelectByRecency(candidates);
    FinalizeSelectionLog(event, result);
    return result.selected_id;
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