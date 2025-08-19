/**
 * @file TransitionMemory.h
 * @brief Memory storage for sequence transition decisions in ambient character behavior.
 * @author Eric Buitrón López
 * @date 8/11/2025
 *
 * Tracks which sequence nodes have been visited to prevent repetitive behavioral loops by implementing recency based
 * selection for transition choices
 */

#pragma once
#include "BaseMemory.h"

namespace AmbientCharacterBehavior {
/**
 * @ingroup memory_group
 * @brief Records when specific sequence nodes were last visited by a character
 *
 * TransitionMemory enables behavioral variety by tracking which sequence nodes
 * (representing different behavioral choices) a character has recently visited.
 * This allows the framework to implement least-recently-used selection, preventing
 * characters from getting stuck in repetitive behavioral loops.
 *
 * **Domain Context:**
 * In behavior sequences, characters frequently encounter choice points where multiple
 * transitions are valid (e.g., "go to park" vs "go to cafe"). Without memory, characters
 * might always choose the first valid option, creating mechanical repetition. TransitionMemory
 * tracks these choices to encourage variety.
 *
 * **Matching Strategy:**
 * Two TransitionMemory instances match if they have the same target_node_id, regardless
 * of when they were created. This means visiting the same sequence node multiple times
 * updates the existing memory rather than creating duplicates.
 *
 * **Usage Pattern:**
 * ```cpp
 * // Character faces choice between nodes 1, 2, 3
 * std::vector<int> options = {1, 2, 3};
 *
 * // Check which was used least recently
 * int chosen_node = memory_system.GetLeastRecentTransition(options);
 *
 * // Record the choice
 * memory_system.UpdateTransitionMemory(chosen_node, current_time);
 * ```
 *
 * **Performance Characteristics:**
 * - Construction: O(1)
 * - Matching: O(1)
 * - Memory footprint: 2 integers (node_id + timestamp)
 *
 *
 * @see IMemory, MemorySystem
 */
class TransitionMemory : public BaseMemory
{
private:
    // =============================================================================
    // MEMBER VARIABLES
    // =============================================================================

    /**
     * @brief Unique identifier of the sequence node that was visited
     * @invariant target_node_id >= 0
     */
    int target_node_id;

public:
    // =============================================================================
    // CONSTRUCTION
    // =============================================================================

    TransitionMemory(int node_id, int time);

    // =============================================================================
    // MEMORY MATCHING
    // =============================================================================

    [[nodiscard]]
    bool MatchesMemory(const BaseMemory& other) const override;

    [[nodiscard]]
    bool MatchesMemory(int other_node_id) const;

    // =============================================================================
    // DATA ACCESS
    // =============================================================================

    int GetTargetNodeId() const;
};

}