#include "ActionTimeoutManager.h"
#include <tracy/Tracy.hpp>

namespace AmbientCharacterBehavior {
bool ActionTimeoutManager::Initialize(const std::string &config_file_path, ActionCompletionCallback callback)
{
    if (!callback)
    {
        logger.LogError("ActionCompletionCallback cannot be null",
            "ActionTimeoutManager::Initialize");

        return false;
    }
    auto timeout_interval = json_loader.GetTimeoutIntervalFromConfigFile(config_file_path);
    if (!timeout_interval.has_value())
    {
        logger.LogError("The timeout interval could not be loaded properly",
            "ActionTimeoutManager::Initialize");

        return false;
    }
    if (timeout_interval.value() <= 0)
    {
        logger.LogError("Invalid timeout_check_interval_ms: " + std::to_string(timeout_interval.value()) +
            " (must be positive)","ActionTimeoutManager::Initialize");

        return false;
    }

    action_completion_callback = callback;
    timeout_check_interval_ms = timeout_interval.value();

    return true;
}

void ActionTimeoutManager::RegisterActionTimeout(void *entity_handle, int32_t action_id, int64_t action_token,
    int64_t start_time, int64_t timeout_duration)
{
    if (active_action_timeouts.contains(entity_handle))
    {
        logger.LogInfo("An action timeout for this entity already existed. Overwriting with new data.",
            "RegisterActionTimeout");
    }

    ActionTimeout action_timeout{
        .action_id = action_id,
        .action_token = action_token,
        .start_time_ms = start_time,
        .framework_timeout_ms = timeout_duration,
    };

    active_action_timeouts[entity_handle] = action_timeout;
}

void ActionTimeoutManager::UnregisterActionTimeout(void *entity_handle)
{
    auto iterator = active_action_timeouts.find(entity_handle);
    if (iterator == active_action_timeouts.end())
    {
        return;
    }

    active_action_timeouts.erase(iterator);

    logger.LogInfo("Unregistered action timeout for entity","UnregisterActionTimeout");
}

void ActionTimeoutManager::CheckActionTimeouts(int64_t current_time)
{
    if (current_time - last_check_time_ms < timeout_check_interval_ms)
    {
        return;
    }

    ZoneScoped;

    logger.LogInfo("Checking all active action timeouts", "CheckActionTimeouts");
    last_check_time_ms = current_time;

    std::vector<ExpiredAction> expired_actions;

    for (const auto& [entity_handle, action_timeout] : active_action_timeouts)
    {
        int64_t elapsed = current_time - action_timeout.start_time_ms;

        if (elapsed >= action_timeout.framework_timeout_ms)
        {
            expired_actions.emplace_back(entity_handle, action_timeout.action_id, action_timeout.action_token, elapsed);
        }
    }

    for (const auto& expired : expired_actions)
    {
        logger.LogInfo("Action timed out - action_id: " + std::to_string(expired.action_id) + ", elapsed: " +
            std::to_string(expired.elapsed_time_ms) + "ms","CheckActionTimeouts");

        action_completion_callback(expired.entity_handle, expired.action_id, expired.action_token);

        active_action_timeouts.erase(expired.entity_handle);
    }
}
}
