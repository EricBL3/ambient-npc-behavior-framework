#pragma once
#include <stdexcept>
#include "services/interfaces/IStartCharacterActionProvider.h"

namespace AmbientCharacterBehavior {
class StartCharacterActionProvider : public IStartCharacterActionProvider {
private:
    StartCharacterActionCallback start_action_callback;

public:

    explicit StartCharacterActionProvider(StartCharacterActionCallback start_action_callback) :
    start_action_callback(start_action_callback)
    {
        if (!start_action_callback)
        {
            throw std::invalid_argument("StartCharacterActionProvider: Start action function cannot be null");
        }
    }

    void StartCharacterAction(void *entity_handle, int32_t action_id, int64_t action_token, int64_t action_duration_ms,
        void *target_entity_id) override
    {
        start_action_callback(entity_handle, action_id, action_token, action_duration_ms, target_entity_id);
    }
};
}
