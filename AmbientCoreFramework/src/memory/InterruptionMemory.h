
#pragma once
#include "BaseMemory.h"

namespace AmbientCharacterBehavior {
/**
 * @brief Preserves execution context when character actions are interrupted
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

    explicit InterruptionMemory(
        int32_t interrupted_action_id,
        int32_t interrupted_sequence_id,
        int32_t interrupted_sequence_node_id,
        int32_t interrupted_target_entity_id,
        int64_t time
    );

    [[nodiscard]]
    bool MatchesMemory(const BaseMemory& other) const override;

    [[nodiscard]]
    bool MatchesMemory(int32_t other_action_id, int32_t other_sequence_id, int32_t other_sequence_node_id) const;

    int32_t GetInterruptedActionId() const;
    int32_t GetInterruptedSequenceId() const;
    int32_t GetInterruptedSequenceNodeId() const;
    int32_t GetInterruptedTargetEntityId() const;
};

}