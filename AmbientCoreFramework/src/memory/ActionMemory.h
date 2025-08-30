/**
 * @file ActionMemory.h
 * @brief Memory storage for action execution decisions in ambient character behavior
 * @author Eric Buitrón López
 * @date 8/11/2025
 *
 */

#pragma once
#include "BaseMemory.h"

namespace AmbientCharacterBehavior {
/**
 * @ingroup memory_group
 * @brief Records when specific actions were performed on specific entities
 *
 * @see IMemory, MemorySystem
 */
class ActionMemory : public BaseMemory {
private:
    // =============================================================================
    // MEMBER VARIABLES
    // =============================================================================

    /**
     * @brief Unique identifier of the action that was performed
     * @invariant action_id >= 0
     */
    int32_t action_id;

    /**
     * @brief Unique identifier of the entity that was used for the action
     * @invariant target_entity_id >= 0
     */
    int32_t target_entity_id;

public:
    // =============================================================================
    // CONSTRUCTION
    // =============================================================================

    explicit ActionMemory(int32_t action_id, int32_t target_entity_id, int64_t last_used_time);

    // =============================================================================
    // MEMORY MATCHING
    // =============================================================================

    [[nodiscard]]
    bool MatchesMemory(const BaseMemory& other) const override;

    [[nodiscard]]
    bool MatchesMemory(int32_t other_action_id, int32_t other_target_entity_id) const;

    // =============================================================================
    // DATA ACCESS
    // =============================================================================

    int32_t GetActionId() const;
    int32_t GetTargetEntityId() const;

};

}