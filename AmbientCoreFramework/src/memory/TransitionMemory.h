/**
 * @file TransitionMemory.h
 * @brief Memory storage for sequence transition decisions in ambient character behavior.
 * @author Eric Buitrón López
 * @date 8/11/2025
 *
 */

#pragma once
#include "BaseMemory.h"

namespace AmbientCharacterBehavior {
/**
 * @ingroup memory_group
 * @brief Records when specific sequence nodes were last visited by a character
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
    int32_t target_node_id;

public:
    // =============================================================================
    // CONSTRUCTION
    // =============================================================================

    explicit TransitionMemory(int32_t node_id, int64_t time);

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

    int32_t GetTargetNodeId() const;
};

}