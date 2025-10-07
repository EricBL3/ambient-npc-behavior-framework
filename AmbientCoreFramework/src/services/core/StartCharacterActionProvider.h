#pragma once
#include "../../../include/BehaviorFrameworkInterface.h"
#include "interfaces/IStartCharacterActionProvider.h"

namespace AmbientCharacterBehavior {
class StartCharacterActionProvider : public IStartCharacterActionProvider {
public:
    void StartCharacterAction(void *entity_handle, int32_t action_id, int64_t action_token, int64_t max_duration_ms,
        void *target_entity_id) override
    {
        return AmbientCharacterBehavior::StartCharacterAction(entity_handle, action_id, action_token, max_duration_ms,
            target_entity_id);
    }
};
}
