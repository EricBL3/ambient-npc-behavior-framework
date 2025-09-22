
#pragma once
#include <vector>
#include <string>

#include "ActionMemory.h"
#include "InterruptionMemory.h"
#include "TransitionMemory.h"
#include "interfaces/ILogger.h"

namespace AmbientCharacterBehavior {
/**
 * @brief Central coordinator for all character memory types and behavioral variety generation
 */
class MemorySystem {
private:

    /**
     * @invariant max_transition_memories > 0
     */
    int32_t max_transition_memories;

    /**
     * @invariant max_action_memories > 0
     */
    int32_t max_action_memories;

    /**
     * @invariant max_interruption_memories > 0
     */
    int32_t max_interruption_memories;

    std::vector<TransitionMemory> transition_memories;
    std::vector<ActionMemory> action_memories;
    std::vector<InterruptionMemory> interruption_memories;

    ILogger& logger;

public:

    explicit MemorySystem(int32_t max_transitions, int32_t max_actions, int32_t max_interruptions, ILogger& logger);

    int32_t GetMaxTransitionMemories() const;
    int32_t GetMaxActionMemories() const;
    int32_t GetMaxInterruptionMemories() const;

    void SetAndEnforceMaxTransitionMemories(int32_t max_transitions);
    void SetAndEnforceMaxActionMemories(int32_t max_actions);
    void SetAndEnforceMaxInterruptionMemories(int32_t max_interruptions);

    [[nodiscard]]
    bool UpdateTransitionMemory(int32_t target_node_id, int64_t current_time);

    [[nodiscard]]
    bool UpdateActionMemory(int32_t action_id, int32_t target_entity_id, int64_t current_time);

    [[nodiscard]]
    bool UpdateInterruptionMemory(int32_t action_id, int32_t sequence_id, int32_t node_id, int32_t entity_id, int64_t current_time);

    [[nodiscard]]
    const TransitionMemory* FindTransitionMemory(int32_t target_node_id) const;

    [[nodiscard]]
    const ActionMemory* FindActionMemory(int32_t action_id, int32_t target_entity_id) const;

    [[nodiscard]]
    const InterruptionMemory* FindInterruptionMemory(int32_t action_id, int32_t sequence_id, int32_t node_id) const;

    [[nodiscard]]
    int32_t GetLeastRecentlyVisitedNodeId(const std::vector<int32_t>& node_ids) const;

    [[nodiscard]]
    int32_t GetLeastRecentlyUsedEntityIdForAction(int32_t action_id, const std::vector<int32_t>& entity_ids) const;

    void ClearSequenceInterruptionMemories(int32_t sequence_id);
    bool RemoveInterruptionMemory(int32_t action_id, int32_t sequence_id, int32_t node_id);
    void ClearAllMemories();

    size_t GetTransitionMemoryCount() const;
    size_t GetActionMemoryCount() const;
    size_t GetInterruptionMemoryCount() const;

private:

    [[nodiscard]]
    int32_t SelectRandomFromVector(const std::vector<int32_t>& options) const;

    void EnforceMaxTransitionMemories();
    void EnforceMaxActionMemories();
    void EnforceMaxInterruptionMemories();

    void RemoveExistingTransitionMemory(int32_t target_node_id);
    void RemoveExistingActionMemory(int32_t action_id, int32_t target_entity_id);

    [[nodiscard]]
    int32_t FindOldestTransitionNodeId(const std::vector<int32_t>& node_ids) const;

    [[nodiscard]]
    int32_t FindOldestActionEntityId(int32_t action_id, const std::vector<int32_t>& entity_ids) const;

    [[nodiscard]]
    std::vector<int32_t> FindUnusedTransitionNodeIds(const std::vector<int32_t>& node_ids) const;

    [[nodiscard]]
    std::vector<int32_t> FindUnusedActionEntityIds(int32_t action_id, const std::vector<int32_t>& entity_ids) const;
};

}
