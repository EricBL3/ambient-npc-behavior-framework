#pragma once
#include <cstdint>

namespace AmbientCharacterBehavior {
class IStartCharacterActionProvider {
public:
    virtual ~IStartCharacterActionProvider() = default;

    virtual void StartCharacterAction(void* entity_handle, int32_t action_id, int64_t action_token, void* target_entity_handle) = 0;
};
}
