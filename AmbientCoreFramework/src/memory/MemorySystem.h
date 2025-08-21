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
    int max_transition_memories;

    /**
     * @brief Maximum number of action memories to maintain
     * @invariant max_action_memories > 0
     */
    int max_action_memories;

    /**
     * @brief Maximum number of interruption memories to maintain
     * @invariant max_interruption_memories > 0
     */
    int max_interruption_memories;

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

    explicit MemorySystem(int max_transitions = 10, int max_actions = 20, int max_interruptions = 5);

    int GetMaxTransitionMemories() const;
    int GetMaxActionMemories() const;
    int GetMaxInterruptionMemories() const;

    void SetMaxTransitionMemories(int max_transitions);
    void SetMaxActionMemories(int max_actions);
    void SetMaxInterruptionMemories(int max_interruptions);

    // =============================================================================
    // MEMORY UPDATE OPERATIONS
    // =============================================================================

    [[nodiscard]]
    bool UpdateTransitionMemory(int target_node_id, int current_time);

    [[nodiscard]]
    bool UpdateActionMemory(int action_id, int target_entity_id, int current_time);

    [[nodiscard]]
    bool UpdateInterruptionMemory(int action_id, int sequence_id, int node_id, int entity_id, int current_time);

    // =============================================================================
    // MEMORY SEARCH OPERATIONS
    // =============================================================================

    [[nodiscard]]
    const TransitionMemory* FindTransitionMemory(int target_node_id) const;

    [[nodiscard]]
    const ActionMemory* FindActionMemory(int action_id, int target_entity_id) const;

    [[nodiscard]]
    const InterruptionMemory* FindInterruptionMemory(int action_id, int sequence_id, int node_id) const;

    // =============================================================================
    // BEHAVIORAL SELECTION
    // =============================================================================

    [[nodiscard]]
    int GetLeastRecentlyVisitedNode(const std::vector<int>& node_ids) const;

    [[nodiscard]]
    int GetLeastRecentlyUsedEntityForAction(int action_id, const std::vector<int>& entity_ids) const;

    // =============================================================================
    // MEMORY CLEANUP OPERATIONS
    // =============================================================================

    void ClearSequenceInterruptionMemories(int sequence_id);

    bool RemoveInterruptionMemory(int action_id, int sequence_id, int node_id);

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
    int SelectRandomFromVector(const std::vector<int>& options) const;

    // =============================================================================
    // CAPACITY MANAGEMENT
    // =============================================================================

    void EnforceMaxTransitionMemories();
    void EnforceMaxActionMemories();
    void EnforceMaxInterruptionMemories();

    // =============================================================================
    // MEMORY REMOVAL HELPERS
    // =============================================================================

    void RemoveExistingTransitionMemory(int target_node_id);
    void RemoveExistingActionMemory(int action_id, int target_entity_id);

    // =============================================================================
    // RECENCY SELECTION HELPERS
    // =============================================================================

    [[nodiscard]]
    int FindOldestTransitionNode(const std::vector<int>& node_ids) const;

    [[nodiscard]]
    int FindOldestActionEntity(int action_id, const std::vector<int>& entity_ids) const;

    [[nodiscard]]
    std::vector<int> FindUnusedTransitionNodes(const std::vector<int>& node_ids) const;

    [[nodiscard]]
    std::vector<int> FindUnusedActionEntities(int action_id, const std::vector<int>& entity_ids) const;
};

}