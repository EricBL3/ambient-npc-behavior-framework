
#pragma once
#include <cstdint>
#include "services/interfaces/ITimeManager.h"

namespace AmbientCharacterBehavior {
/**
 * @brief Manages the framework's representation of time.
 */
class TimeManager : public ITimeManager {
private:
    int64_t current_time_ms;

public:
    explicit TimeManager()
    { current_time_ms = 0; }

    ~TimeManager() override = default;

    int64_t GetCurrentTime() const override
    { return current_time_ms; }

    void SetCurrentTime(int64_t time) override
    {
        if (time >= 0)
        {
            current_time_ms = time;
        }
    }
};
}
