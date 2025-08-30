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
    int32_t interrupted_action_id;

    /**
     * @brief Unique identifier of the sequence that was executing
     * @invariant interrupted_sequence_id >= 0
     */
    int32_t interrupted_sequence_id;

    /**
     * @brief Unique identifier of the sequence node that was interrupted
     * @invariant interrupted_sequence_node_id >= 0
     */
    int32_t interrupted_sequence_node_id;

    /**
     * @brief Unique identifier of the entity involved in the interrupted action
     * @invariant interrupted_target_entity_id >= -1 (allows -1 for no entity)
     */
    int32_t interrupted_target_entity_id;
public:
    // =============================================================================
    // CONSTRUCTION
    // =============================================================================

    explicit InterruptionMemory(
        int32_t interrupted_action_id,
        int32_t interrupted_sequence_id,
        int32_t interrupted_sequence_node_id,
        int32_t interrupted_target_entity_id,
        int64_t time
    );

    // =============================================================================
    // MEMORY MATCHING
    // =============================================================================

    [[nodiscard]]
    bool MatchesMemory(const BaseMemory& other) const override;

    [[nodiscard]]
    bool MatchesMemory(int32_t other_action_id, int32_t other_sequence_id, int32_t other_sequence_node_id) const;

    // =============================================================================
    // CONTEXT DATA ACCESS
    // =============================================================================

    int32_t GetInterruptedActionId() const;
    int32_t GetInterruptedSequenceId() const;
    int32_t GetInterruptedSequenceNodeId() const;
    int32_t GetInterruptedTargetEntityId() const;
};

}