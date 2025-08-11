//
// Created by Eric on 8/11/2025.
//

#include <stdexcept>
#include <string>

class IMemory
{
protected:
    int last_used_time;
public:
    IMemory(int time) {
        if(time < 0)
        {
            throw std::invalid_argument("TransitionMemory: time cannot be negative, got " + std::to_string(time));
        }
        last_used_time = time;
    }

    virtual ~IMemory() = default;

    virtual bool MatchesMemory(const IMemory& other) const = 0;
    int GetLastUsedTime() const { return last_used_time; }
    virtual bool IsOlderThan(const IMemory& other) const {
        return last_used_time < other.last_used_time;
    }
};