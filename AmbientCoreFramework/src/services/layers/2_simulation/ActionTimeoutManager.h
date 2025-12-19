#pragma once
#include "services/interfaces/IActionTimeoutManager.h"
#include "services/interfaces/IJsonLoader.h"
#include "services/interfaces/ILogger.h"

namespace AmbientCharacterBehavior {

struct ActionTimeout {
    int32_t action_id;
    int64_t action_token;
    int64_t start_time_ms;
    int64_t framework_timeout_ms;
};

struct ExpiredAction {
    void* entity_handle;
    int32_t action_id;
    int64_t action_token;
    int64_t elapsed_time_ms;
};

class ActionTimeoutManager : public IActionTimeoutManager {
private:
    ILogger& logger;
    IJsonLoader& json_loader;

    int32_t timeout_check_interval_ms;
    int64_t last_check_time_ms;
    std::unordered_map<void*, ActionTimeout> active_action_timeouts;
    ActionCompletionCallback action_completion_callback;

public:
    ActionTimeoutManager(ILogger &logger, IJsonLoader &json_loader) : logger(logger), json_loader(json_loader),
        timeout_check_interval_ms(0), last_check_time_ms(0) {}

    bool Initialize(const std::string &config_file_path, ActionCompletionCallback callback) override;

    void RegisterActionTimeout(void *entity_handle, int32_t action_id, int64_t action_token, int64_t start_time,
        int64_t timeout_duration) override;

    void UnregisterActionTimeout(void *entity_handle) override;

    void CheckActionTimeouts(int64_t current_time) override;
};
}
