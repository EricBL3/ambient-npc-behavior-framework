#pragma once
#include "BaseMemory.h"

namespace AmbientCharacterBehavior {
/**
 * @brief Records when specific sequence nodes in a sequence were last visited by a character
 *
 * This prevents characters from repeatedly choosing the same sequence paths by tracking which nodes have already been
 * visited in a particular sequence recently. Used during transition evaluation to implement exploration-exploitation
 * selection.
 *
 * Matching criteria: Two transition memories match if they refer to the (same sequence_id, node_id) pair.
 *
 * Example: Visiting node 5 (action node) in sequence 1 (park routine) is different from visiting node 5 (end node)
 * in sequence 2 (coffee routine).
 */
class TransitionMemory : public BaseMemory
{
private:
    /**
     * @invariant sequence_id >= 0
     */
    int32_t sequence_id;

    /**
     * @invariant target_node_id >= 0
     */
    int32_t target_node_id;

public:
    /**
     * @brief Create a transition memory
     * @param sequence_id Sequence containing the node (must be >= 0)
     * @param node_id Node that was visited (must be >= 0)
     * @param time Timestamp when visit occurred (must be >= 0)
     * @throws std::invalid_argument if any parameter < 0
     */
    explicit TransitionMemory(int32_t sequence_id, int32_t node_id, int64_t time);

    /**
     * @brief Check if this memory matches another memory (polymorphic)
     * @param other Memory to compare (will attempt dynamic_cast to TransitionMemory)
     * @return true if both are TransitionMemory with same sequence_id and node_id
     */
    [[nodiscard]]
    bool MatchesMemory(const BaseMemory& other) const override;

    /**
     * @brief Check if this memory matches specific IDs (type safe)
     * @param other_sequence_id Sequence ID to match
     * @param other_node_id Node ID to match
     * @return true if both IDs match
     */
    [[nodiscard]]
    bool MatchesMemory(int32_t other_sequence_id, int32_t other_node_id) const;

    [[nodiscard]]
    int32_t GetTargetNodeId() const { return target_node_id; }

    [[nodiscard]]
    int32_t GetSequenceId() const { return sequence_id; }
};
}