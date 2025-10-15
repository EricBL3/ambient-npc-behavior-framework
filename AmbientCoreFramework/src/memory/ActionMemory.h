
#pragma once
#include "BaseMemory.h"

namespace AmbientCharacterBehavior {
/**
 * @brief Records when specific actions were performed on specific entities
 */
class ActionMemory : public BaseMemory {
private:

    /**
     * @invariant action_id >= 0
     */
    int32_t action_id;

    int32_t target_entity_id;

public:

    explicit ActionMemory(int32_t action_id, int32_t target_entity_id, int64_t last_used_time);

    [[nodiscard]]
    bool MatchesMemory(const BaseMemory& other) const override;

    [[nodiscard]]
    bool MatchesMemory(int32_t other_action_id, int32_t other_target_entity_id) const;

    int32_t GetActionId() const;
    int32_t GetTargetEntityId() const;

};

}