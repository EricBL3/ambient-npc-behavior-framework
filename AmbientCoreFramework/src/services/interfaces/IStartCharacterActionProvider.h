#pragma once
#include <cstdint>

namespace AmbientCharacterBehavior {

using StartCharacterActionCallback = void(*)(void* entity_handle, int32_t action_id, int64_t action_token,
    int64_t action_duration_ms, void* target_entity_handle);

class IStartCharacterActionProvider {
public:
    virtual ~IStartCharacterActionProvider() = default;

    virtual void StartCharacterAction(void* entity_handle, int32_t action_id, int64_t action_token,
        int64_t action_duration_ms, void* target_entity_handle) = 0;
};
}
