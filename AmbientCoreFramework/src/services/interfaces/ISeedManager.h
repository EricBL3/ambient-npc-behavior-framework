#pragma once
#include <cstdint>
#include <optional>

namespace AmbientCharacterBehavior {
class ISeedManager {
public:
    virtual ~ISeedManager() = default;
    virtual std::optional<uint32_t> GetSeed() const = 0;
    virtual void SetSeed(std::optional<uint32_t> new_value) = 0;
};
}
