#pragma once

#include <gmock/gmock.h>
#include "interfaces/IStartCharacterActionProvider.h"

namespace AmbientCharacterBehavior {
class MockStartCharacterActionProvider : public IStartCharacterActionProvider {
    MOCK_METHOD((void), StartCharacterAction, (void* entity_handle, int32_t action_id, int64_t action_token,
        void* target_entity_id), (override));
};
}
