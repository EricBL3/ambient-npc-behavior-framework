#pragma once
#include <cstdint>
#include <functional>
#include <string>

namespace AmbientCharacterBehavior {
using ActionCompletionCallback = std::function<void(void* entity_handle, int32_t action_id, int64_t action_token)>;

class IActionTimeoutManager {
public:
    virtual ~IActionTimeoutManager() = default;
    virtual bool Initialize(const std::string& config_file_path, ActionCompletionCallback callback) = 0;
    virtual void RegisterActionTimeout(void* entity_handle, int32_t action_id, int64_t action_token, int64_t start_time,
        int64_t timeout_duration) = 0;

    virtual void UnregisterActionTimeout(void* entity_handle) = 0;
    virtual void CheckActionTimeouts(int64_t current_time) = 0;
};
}
