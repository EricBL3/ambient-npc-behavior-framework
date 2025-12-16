#pragma once
#include "BaseMemory.h"

namespace AmbientCharacterBehavior {
/**
 * @brief Preserves execution context when character actions are interrupted
 *
 * Enables resumable actions by storing the complete context needed to resume after an interruption completes. Unlike
 * transition/action memories which influence selection, interruption memories enable continuity by preserving execution
 * state.
 *
 * Matching criteria: Two interruption memories match if they refer to the same (action_id, sequence_id, node_id) triple.
 * This identifies a unique interruption context within a character's execution.
 *
 * Lifecycle:
 * - Created: When a resumable action is interrupted
 * - Used: When attempting to resume after the interruption completes
 * - Removed: After trying to resume OR when a sequence fails
 *
 * Example: Character is interrupted while sitting (action = SIT, sequence = PARK_VISITOR, node = 5, entity=bench_10).
 * After handling interruption, this memory enables resuming the sit action on bench_10.
 */
class InterruptionMemory : public BaseMemory {
private:

    /**
     * @invariant interrupted_action_id >= 0
     */
    int32_t interrupted_action_id;

    /**
     * @invariant interrupted_sequence_id >= 0
     */
    int32_t interrupted_sequence_id;

    /**
     * @invariant interrupted_sequence_node_id >= 0
     */
    int32_t interrupted_sequence_node_id;

    /**
     * @invariant interrupted_target_entity_id >= -1 (allows -1 for no entity)
     */
    int32_t interrupted_target_entity_id;
public:

    /**
     * @brief Create an interruption memory
     * @param interrupted_action_id Action that was interrupted (must be >= 0)
     * @param interrupted_sequence_id Sequence that was interrupted (must be >= 0)
     * @param interrupted_sequence_node_id Node that was interrupted (must be >= 0)
     * @param interrupted_target_entity_id Entity being used (>= -1, where -1 = no entity)
     * @param time Timestamp when interruption occurred (must be >= 0)
     * @throws std::invalid_argument if any parameter violates constraints
     */
    explicit InterruptionMemory(
        int32_t interrupted_action_id,
        int32_t interrupted_sequence_id,
        int32_t interrupted_sequence_node_id,
        int32_t interrupted_target_entity_id,
        int64_t time
    );

    /**
    * @brief Check if this memory matches another memory (polymorphic)
    * @param other Memory to compare (will attempt dynamic_cast to InterruptionMemory)
    * @return true if both are InterruptionMemory with matching action, sequence, and node
    */
    [[nodiscard]]
    bool MatchesMemory(const BaseMemory& other) const override;

    /**
     * @brief Check if this memory matches specific IDs (type-safe)
     *
     * Note: Entity ID is NOT part of matching criteria. This is intentional because
     * interruption context is identified by (action, sequence, node), and entity
     * is just part of the stored context for resumption.
     *
     * @param other_action_id Action ID to match
     * @param other_sequence_id Sequence ID to match
     * @param other_sequence_node_id Node ID to match
     * @return true if all three IDs match
     */
    [[nodiscard]]
    bool MatchesMemory(int32_t other_action_id, int32_t other_sequence_id, int32_t other_sequence_node_id) const;

    [[nodiscard]]
    int32_t GetInterruptedActionId() const { return interrupted_action_id; }

    [[nodiscard]]
    int32_t GetInterruptedSequenceId() const { return interrupted_sequence_id; }

    [[nodiscard]]
    int32_t GetInterruptedSequenceNodeId() const { return interrupted_sequence_node_id; }

    [[nodiscard]]
    int32_t GetInterruptedTargetEntityId() const { return interrupted_target_entity_id; }
};

}