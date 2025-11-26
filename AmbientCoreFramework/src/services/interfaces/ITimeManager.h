#pragma once
#include <cstdint>

namespace AmbientCharacterBehavior {

/**
 * @brief Manages the representation of time.
 */
class ITimeManager {
public:
    virtual ~ITimeManager() = default;
    virtual int64_t GetCurrentTime() const = 0;

    virtual void SetCurrentTime(int64_t time) = 0;
};
}
