/**
 * @file ActionMemory.h
 * @brief Memory storage for action execution decisions in ambient character behavior
 * @author Eric Buitrón López
 * @date 8/11/2025
 *
 * Tracks which actions have been performed on specific entities to create
 * natural variety in entity selection and prevent repetitive interactions.
 */

#pragma once
#include "BaseMemory.h"

namespace AmbientCharacterBehavior {
/**
 * @ingroup memory_group
 * @brief Records when specific actions were performed on specific entities
 *
 * ActionMemory enables entity selection variety by tracking which entities
 * a character has recently used for different types of actions. This prevents
 * characters from always choosing the same bench to sit on, the same vendor
 * to buy from, or the same NPC to talk to, creating more natural and varied
 * behavioral patterns.
 *
 * **Domain Context:**
 * When characters need to perform actions that require entities (sitting on benches,
 * buying from vendors, talking to NPCs), they often have multiple valid choices.
 * Without memory, characters might always pick the first available option,
 * creating unrealistic repetitive patterns. ActionMemory tracks these choices
 * to encourage variety.
 *
 * **Matching Strategy:**
 * ActionMemory uses compound matching - two memories match only if BOTH the
 * action_id AND target_entity_id are identical. This allows characters to:
 * - Use different entities for the same action (sit on different benches)
 * - Use the same entity for different actions (sit on bench, then read on same bench)
 *
 * **Usage Pattern:**
 * ```cpp
 * // Character wants to sit, multiple benches available
 * std::vector<int> available_benches = {bench1_id, bench2_id, bench3_id};
 *
 * // Find least recently used bench for sitting action
 * int chosen_bench = memory_system.GetLeastRecentEntity(SIT_ACTION, available_benches);
 *
 * // Record the choice
 * memory_system.UpdateActionMemory(SIT_ACTION, chosen_bench, current_time);
 * ```
 *
 * **Performance Characteristics:**
 * - Construction: O(1)
 * - Matching: O(1)
 * - Memory footprint: 3 integers (action_id + entity_id + timestamp)
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

    ActionMemory(int action_id, int target_entity_id, int last_used_time);

    // =============================================================================
    // MEMORY MATCHING (Framework Core Functionality)
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