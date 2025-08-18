/**
* @file InterruptionMemory.h
 * @brief Memory storage for interrupted action context in ambient character behavior
 * @author Eric Buitrón López
 * @date 8/12/2025
 *
 * Enables resumption of actions after interruptions by preserving the execution state.
 */

#pragma once
#include "BaseMemory.h"

/**
 * @ingroup memory_group
 * @brief Preserves execution context when character actions are interrupted
 *
 * InterruptionMemory enables behavioral continuity by storing context
 * information when a character's action is interrupted by external events
 * (emergencies, player interactions, environmental changes). This allows
 * characters to try to resume their previous activity after handling
 * the interruption, maintaining believable behavioral flow.
 *
 * **Domain Context:**
 * Ambient characters often need to respond to immediate events that interrupt
 * their current actions - a fire alarm while eating lunch, rain while reading
 * in the park, or a player interaction while walking. Without interruption
 * memory, characters would lose their context and start completely new behaviors,
 * creating unrealistic discontinuity in their apparent "lives."
 *
 * **Matching Strategy:**
 * InterruptionMemory uses triple matching - memories match only if action_id,
 * sequence_id, AND sequence_node_id are identical. The target_entity_id is
 * preserved for resumption but not used for matching, since the same action
 * context can be interrupted in multiple ways.
 *
 * **Usage Pattern:**
 * ```cpp
 * // Character is sitting on bench when fire alarm triggers
 * if (action.IsResumable()) {
 *     memory_system.UpdateInterruptionMemory(
 *         SIT_ACTION, LEISURE_SEQUENCE, SITTING_NODE, bench_id, current_time
 *     );
 * }
 *
 * // After handling emergency, attempt to resume
 * auto context = memory_system.FindInterruptionContext(
 *     SIT_ACTION, LEISURE_SEQUENCE, SITTING_NODE
 * );
 * if (context && bench_still_available) {
 *     // Resume sitting on the same bench
 *     character.ResumeAction(context);
 * }
 * ```
 *
 * **Performance Characteristics:**
 * - Construction: O(1)
 * - Matching: O(1) (triple comparison)
 * - Memory footprint: 5 integers (3 for matching + entity_id + timestamp)
 *
 * @see IMemory, MemorySystem
 */
class InterruptionMemory : public BaseMemory {
private:
    // =============================================================================
    // MEMBER VARIABLES
    // =============================================================================

    /**
     * @brief Unique identifier of the action that was interrupted
     * @invariant interrupted_action_id >= 0
     */
    int interrupted_action_id;

    /**
     * @brief Unique identifier of the sequence that was executing
     * @invariant interrupted_sequence_id >= 0
     */
    int interrupted_sequence_id;

    /**
     * @brief Unique identifier of the sequence node that was interrupted
     * @invariant interrupted_sequence_node_id >= 0
     */
    int interrupted_sequence_node_id;

    /**
     * @brief Unique identifier of the entity involved in the interrupted action
     * @invariant interrupted_target_entity_id >= -1 (allows -1 for no entity)
     */
    int interrupted_target_entity_id;
public:
    // =============================================================================
    // CONSTRUCTION
    // =============================================================================

    InterruptionMemory(
        int interrupted_action_id,
        int interrupted_sequence_id,
        int interrupted_sequence_node_id,
        int interrupted_target_entity_id,
        int time
    );

    // =============================================================================
    // MEMORY MATCHING
    // =============================================================================

    [[nodiscard]]
    bool MatchesMemory(const BaseMemory& other) const override;

    [[nodiscard]]
    bool MatchesMemory(int other_action_id, int other_sequence_id, int other_sequence_node_id) const;

    // =============================================================================
    // CONTEXT DATA ACCESS
    // =============================================================================

    int GetInterruptedActionId() const;
    int GetInterruptedSequenceId() const;
    int GetInterruptedSequenceNodeId() const;
    int GetInterruptedTargetEntityId() const;
};
