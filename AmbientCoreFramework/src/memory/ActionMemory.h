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
    int action_id;

    /**
     * @brief Unique identifier of the entity that was used for the action
     * @invariant target_entity_id >= 0
     */
    int target_entity_id;

public:
    // =============================================================================
    // CONSTRUCTION
    // =============================================================================

    explicit ActionMemory(int action_id, int target_entity_id, int last_used_time);

    // =============================================================================
    // MEMORY MATCHING
    // =============================================================================

    [[nodiscard]]
    bool MatchesMemory(const BaseMemory& other) const override;

    [[nodiscard]]
    bool MatchesMemory(int other_action_id, int other_target_entity_id) const;

    // =============================================================================
    // DATA ACCESS
    // =============================================================================

    int GetActionId() const;
    int GetTargetEntityId() const;

};

}