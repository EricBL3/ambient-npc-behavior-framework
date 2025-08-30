/**
 * @file MemorySystem.h
 * @brief Core memory management for ambient character behavioral variety
 * @author Eric Buitrón López
 * @date 8/13/2025
 */

#pragma once
#include <vector>
#include <string>

#include "ActionMemory.h"
#include "InterruptionMemory.h"
#include "TransitionMemory.h"

namespace AmbientCharacterBehavior {
/**
 * @ingroup memory_group
 * @brief Central coordinator for all character memory types and behavioral variety generation
 *
 * @note While the abstract BaseMemory class could be used to have a single polymorphic collection of memories, each
 * memory type represents something completely different which is why I have opted for having a container for each memory
 * type.
 *
 * @see TransitionMemory, ActionMemory, InterruptionMemory
 */
class MemorySystem {
private:
    // =============================================================================
    // MEMBER VARIABLES
    // =============================================================================

    /**
     * @brief Maximum number of transition memories to maintain
     * @invariant max_transition_memories > 0
     */
    int32_t max_transition_memories;

    /**
     * @brief Maximum number of action memories to maintain
     * @invariant max_action_memories > 0
     */
    int32_t max_action_memories;

    /**
     * @brief Maximum number of interruption memories to maintain
     * @invariant max_interruption_memories > 0
     */
    int32_t max_interruption_memories;

    /**
     * @brief Collection of transition decision records
     *
     * @note Uses a vector as the container to allow for easy modification of its size during runtime (could be used by
     * LOD systems).
     *
     * @note The vector holds a direct reference to the memory objects because it completely owns them.
     */
    std::vector<TransitionMemory> transition_memories;

    /**
     * @brief Collection of action execution records
     *
     * @note Uses a vector as the container to allow for easy modification of its size during runtime (could be used by
     * LOD systems).
     *
     * @note The vector holds a direct reference to the memory objects because it completely owns them.
     */
    std::vector<ActionMemory> action_memories;

    /**
     * @brief Collection of interruption context records
     *
     * @note Uses a vector as the container to allow for easy modification of its size during runtime (could be used by
     * LOD systems).
     *
     * @note The vector holds a direct reference to the memory objects because it completely owns them.
     */
    std::vector<InterruptionMemory> interruption_memories;

public:
    // =============================================================================
    // CONSTRUCTION & CONFIGURATION
    // =============================================================================

    explicit MemorySystem(int32_t max_transitions = 10, int32_t max_actions = 20, int32_t max_interruptions = 5);

    int32_t GetMaxTransitionMemories() const;
    int32_t GetMaxActionMemories() const;
    int32_t GetMaxInterruptionMemories() const;

    void SetMaxTransitionMemories(int32_t max_transitions);
    void SetMaxActionMemories(int32_t max_actions);
    void SetMaxInterruptionMemories(int32_t max_interruptions);

    // =============================================================================
    // MEMORY UPDATE OPERATIONS
    // =============================================================================

    [[nodiscard]]
    bool UpdateTransitionMemory(int32_t target_node_id, int64_t current_time);

    [[nodiscard]]
    bool UpdateActionMemory(int32_t action_id, int32_t target_entity_id, int64_t current_time);

    [[nodiscard]]
    bool UpdateInterruptionMemory(int32_t action_id, int32_t sequence_id, int32_t node_id, int32_t entity_id, int64_t current_time);

    // =============================================================================
    // MEMORY SEARCH OPERATIONS
    // =============================================================================

    [[nodiscard]]
    const TransitionMemory* FindTransitionMemory(int32_t target_node_id) const;

    [[nodiscard]]
    const ActionMemory* FindActionMemory(int32_t action_id, int32_t target_entity_id) const;

    [[nodiscard]]
    const InterruptionMemory* FindInterruptionMemory(int32_t action_id, int32_t sequence_id, int32_t node_id) const;

    // =============================================================================
    // BEHAVIORAL SELECTION
    // =============================================================================

    [[nodiscard]]
    int32_t GetLeastRecentlyVisitedNode(const std::vector<int32_t>& node_ids) const;

    [[nodiscard]]
    int32_t GetLeastRecentlyUsedEntityForAction(int32_t action_id, const std::vector<int32_t>& entity_ids) const;

    // =============================================================================
    // MEMORY CLEANUP OPERATIONS
    // =============================================================================

    void ClearSequenceInterruptionMemories(int32_t sequence_id);

    bool RemoveInterruptionMemory(int32_t action_id, int32_t sequence_id, int32_t node_id);

    void ClearAllMemories();


    // =============================================================================
    // DIAGNOSTIC & MONITORING
    // =============================================================================

    size_t GetTransitionMemoryCount() const;
    size_t GetActionMemoryCount() const;
    size_t GetInterruptionMemoryCount() const;

private:

    // =============================================================================
    // UTILITY HELPERS
    // =============================================================================

    void LogError(const std::string& message) const;

    [[nodiscard]]
    int32_t SelectRandomFromVector(const std::vector<int32_t>& options) const;

    // =============================================================================
    // CAPACITY MANAGEMENT
    // =============================================================================

    void EnforceMaxTransitionMemories();
    void EnforceMaxActionMemories();
    void EnforceMaxInterruptionMemories();

    // =============================================================================
    // MEMORY REMOVAL HELPERS
    // =============================================================================

    void RemoveExistingTransitionMemory(int32_t target_node_id);
    void RemoveExistingActionMemory(int32_t action_id, int32_t target_entity_id);

    // =============================================================================
    // RECENCY SELECTION HELPERS
    // =============================================================================

    [[nodiscard]]
    int32_t FindOldestTransitionNode(const std::vector<int32_t>& node_ids) const;

    [[nodiscard]]
    int32_t FindOldestActionEntity(int32_t action_id, const std::vector<int32_t>& entity_ids) const;

    [[nodiscard]]
    std::vector<int32_t> FindUnusedTransitionNodes(const std::vector<int32_t>& node_ids) const;

    [[nodiscard]]
    std::vector<int32_t> FindUnusedActionEntities(int32_t action_id, const std::vector<int32_t>& entity_ids) const;
};

}