#pragma once
#include "BaseMemory.h"

namespace AmbientCharacterBehavior {
/**
 * @brief Records which specific entities were used for performing an action.
 *
 * Prevents characters from repeatedly using the same entities for the same action by tracking which action and entity
 * pairs have been used recently. Used during entity selection to distribute interaction across valid entities.
 *
 * Matching criteria: Two action memories match if they refer to the same (action_id, target_entity_id) pair.
 *
 * Example: using entity 10 (bench A) for action 3 (sit) is different from using entity 11 (bench B) for action 3 (sit).
 */
class ActionMemory : public BaseMemory {
private:

    /**
     * @invariant action_id >= 0
     */
    int32_t action_id;

    /**
     * @invariant target_entity_id >= -1 (allows -1 for actions that don't require an entity)
     */
    int32_t target_entity_id;

public:

    /**
     * @brief Create an action memory
     * @param action_id Action that was performed (must be >= 0)
     * @param target_entity_id Entity used for the action (must be >= -1)
     * @param last_used_time Timestamp when action occurred (must be >= 0)
     * @throws std::invalid_argument if action_id < 0 or time < 0 or target_entity_id < -1
     */
    explicit ActionMemory(int32_t action_id, int32_t target_entity_id, int64_t last_used_time);

    /**
     * @brief Check if this memory matches another memory (polymorphic)
     * @param other Memory to compare (will attempt dynamic_cast to ActionMemory)
     * @return true if both are ActionMemory with same action_id and target_entity_id
     */
    [[nodiscard]]
    bool MatchesMemory(const BaseMemory& other) const override;

    /**
     * @brief Check if this memory matches specific IDs (type safe)
     * @param other_action_id action ID to match
     * @param other_target_entity_id target entity ID to match
     * @return true if both IDs match
     */
    [[nodiscard]]
    bool MatchesMemory(int32_t other_action_id, int32_t other_target_entity_id) const;

    [[nodiscard]]
    int32_t GetActionId() const { return action_id; }

    [[nodiscard]]
    int32_t GetTargetEntityId() const { return target_entity_id; }

};

}