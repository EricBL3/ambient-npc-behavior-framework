#pragma once
#include "BaseMemory.h"

namespace AmbientCharacterBehavior {
/**
 * @brief Records when specific sequence nodes were last visited by a character
 */
class TransitionMemory : public BaseMemory
{
private:
    /**
     * @invariant target_node_id >= 0
     */
    int32_t target_node_id;

public:
    explicit TransitionMemory(int32_t node_id, int64_t time);

    [[nodiscard]]
    bool MatchesMemory(const BaseMemory& other) const override;

    [[nodiscard]]
    bool MatchesMemory(int other_node_id) const;

    int32_t GetTargetNodeId() const;
};
}