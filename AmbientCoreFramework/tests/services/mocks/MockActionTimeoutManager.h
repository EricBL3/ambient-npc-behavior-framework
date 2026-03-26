#pragma once
#include <gmock/gmock.h>
#include "services/interfaces/IActionTimeoutManager.h"

namespace AmbientCharacterBehavior {
class MockActionTimeoutManager : public IActionTimeoutManager {

    MOCK_METHOD((bool), Initialize, (const std::string& config_file_path, ActionCompletionCallback callback), (override));
    MOCK_METHOD((void), RegisterActionTimeout, (void* entity_handle, int32_t action_id, int64_t action_token,
        int64_t start_time, int64_t timeout_duration), (override));

    MOCK_METHOD((void), UnregisterActionTimeout, (void* entity_handle), (override));
    MOCK_METHOD((void), CheckActionTimeouts, (int64_t current_time), (override));
};
}
