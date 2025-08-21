/**
* @file InterruptionMemory.h
 * @brief Memory storage for interrupted action context in ambient character behavior
 * @author Eric Buitrón López
 * @date 8/12/2025
 *
 */

#pragma once
#include "BaseMemory.h"

namespace AmbientCharacterBehavior {
/**
 * @ingroup memory_group
 * @brief Preserves execution context when character actions are interrupted
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

    explicit InterruptionMemory(
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

}