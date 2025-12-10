#pragma once

#include <gmock/gmock.h>
#include "services/interfaces/IStartCharacterActionProvider.h"

namespace AmbientCharacterBehavior {
class MockStartCharacterActionProvider : public IStartCharacterActionProvider {
public:
    MOCK_METHOD((void), StartCharacterAction, (void* entity_handle, int32_t action_id, int64_t action_token, int64_t action_duration_ms,
        void* target_entity_id), (override));
};
}
