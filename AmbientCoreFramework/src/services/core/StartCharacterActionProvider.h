#pragma once
#include "../../../include/BehaviorFrameworkInterface.h"
#include "interfaces/IStartCharacterActionProvider.h"

namespace AmbientCharacterBehavior {
class StartCharacterActionProvider : public IStartCharacterActionProvider {
public:
    void StartCharacterAction(void *entity_handle, int32_t action_id, int64_t action_token, void *target_entity_id) override
    {
        return AmbientCharacterBehavior::StartCharacterAction(entity_handle, action_id, action_token, target_entity_id);
    }
};
}
