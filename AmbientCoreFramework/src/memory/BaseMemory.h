#pragma once
#include <stdexcept>
#include <string>
#include <cstdint>

namespace AmbientCharacterBehavior {
/**
 * @brief Abstract base class for all memory types in the behavior framework.
 */
class BaseMemory
{
protected:
    int64_t last_used_time;
public:
    /**
     * @throws std::invalid_argument if time < 0
     */
    explicit BaseMemory(const int64_t creation_time) : last_used_time(creation_time) {
        if(creation_time < 0)
        {
            throw std::invalid_argument("BaseMemory: time cannot be negative, got " + std::to_string(creation_time));
        }
    }

    virtual ~BaseMemory() = default;

    [[nodiscard]]
    virtual bool MatchesMemory(const BaseMemory& other) const = 0;

    int64_t GetLastUsedTime() const { return last_used_time; }

    virtual bool IsOlderThan(BaseMemory& other) const {
        return last_used_time < other.GetLastUsedTime();
    }
};

}