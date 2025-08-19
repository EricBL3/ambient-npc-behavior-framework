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
 * MemorySystem serves as the primary interface for managing character decision history
 * across all memory types (transitions, actions, interruptions). It implements the core
 * least-recently-used selection algorithms that prevent repetitive character behaviors
 * while maintaining bounded memory usage for performance scalability.
 *
 * **System Architecture:**
 * The MemorySystem manages three specialized memory collections:
 * - **Transition Memories**: Track sequence node visits for path variety
 * - **Action Memories**: Track entity usage for interaction variety
 * - **Interruption Memories**: Preserve context for behavioral continuity
 *
 * **Core Algorithm:**
 * 1. **Priority 1**: Select unused options (never before chosen)
 * 2. **Priority 2**: Select least-recently-used options from memory
 * 3. **Tie Breaking**: Random selection among equally old options
 * 4. **Memory Management**: Bounded capacity with oldest-first removal
 *
 * **Usage Pattern:**
 * ```cpp
 * MemorySystem memory(5, 10, 3);  // 5 transitions, 10 actions, 3 interruptions
 *
 * // Decision-making cycle:
 * std::vector<int> options = {1, 2, 3};
 * int choice = memory.GetLeastRecentlyVisitedNode(options);
 * memory.UpdateTransitionMemory(choice, current_time);
 *
 * // Entity selection:
 * std::vector<int> benches = {bench1, bench2, bench3};
 * int chosen_bench = memory.GetLeastRecentlyUsedEntityForAction(SIT_ACTION, benches);
 * memory.UpdateActionMemory(SIT_ACTION, chosen_bench, current_time);
 * ```
 *
 * **Performance Characteristics:**
 * - Memory updates: O(n) where n = memory capacity
 * - Selection queries: O(n*m) where n = options, m = memory size
 * - Memory footprint: Linear with configured capacity limits
 *
 * **Capacity Guidelines:**
 * - Transition memories: 5-10 (prevents short behavioral loops)
 * - Action memories: 10-20 (handles entity interaction variety)
 * - Interruption memories: 3-5 (fewer interruptions, larger context)
 *
 * @note All memory operations use error-tolerant design - failures are logged
 *       but don't crash the system, allowing characters to continue functioning
 *       even with degraded variety generation.
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
     */
    std::vector<TransitionMemory> transition_memories;

    /**
     * @brief Collection of action execution records
     */
    std::vector<ActionMemory> action_memories;

    /**
     * @brief Collection of interruption context records
     */
    std::vector<InterruptionMemory> interruption_memories;

    // =============================================================================
    // UTILITY HELPERS
    // =============================================================================

    void LogError(const std::string& message) const;
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

    int FindOldestTransitionNode(const std::vector<int>& node_ids) const;
    int FindOldestActionEntity(int action_id, const std::vector<int>& entity_ids) const;
    std::vector<int> FindUnusedTransitionNodes(const std::vector<int>& node_ids) const;
    std::vector<int> FindUnusedActionEntities(int action_id, const std::vector<int>& entity_ids) const;

public:
    // =============================================================================
    // CONSTRUCTION & CONFIGURATION
    // =============================================================================

    explicit MemorySystem(int max_transitions = 10, int  = 20, int max_interruptions = 5);

    int GetMaxTransitionMemories() const;
    int GetMaxActionMemories() const;
    int GetMaxInterruptionMemories() const;

    void SetMaxTransitionMemories(int max_transitions);
    void SetMaxActionMemories(int max_actions);
    void SetMaxInterruptionMemories(int max_interruptions);

    // =============================================================================
    // MEMORY UPDATE OPERATIONS
    // =============================================================================

    bool UpdateTransitionMemory(int target_node_id, int current_time);
    bool UpdateActionMemory(int action_id, int target_entity_id, int current_time);
    bool UpdateInterruptionMemory(int action_id, int sequence_id, int node_id, int entity_id, int current_time);

    // =============================================================================
    // MEMORY SEARCH OPERATIONS
    // =============================================================================

    const TransitionMemory* FindTransitionMemory(int target_node_id) const;
    const ActionMemory* FindActionMemory(int action_id, int target_entity_id) const;
    const InterruptionMemory* FindInterruptionMemory(int action_id, int sequence_id, int node_id) const;

    // =============================================================================
    // BEHAVIORAL SELECTION (Framework's Core Functionality)
    // =============================================================================

    /// Returns the node_id that has been used less recently.
    int GetLeastRecentlyVisitedNode(const std::vector<int>& node_ids) const;

    /// Returns the entity_id that has been used less recently with the action_id.
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
};

}