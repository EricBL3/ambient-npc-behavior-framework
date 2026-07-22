#pragma once
#include "services/interfaces/ISeedManager.h"

namespace AmbientCharacterBehavior {
class SeedManager : public ISeedManager {
private:
    std::optional<uint32_t> framework_seed;

public:
    explicit SeedManager()
    { framework_seed = std::nullopt; }

    std::optional<uint32_t> GetSeed() const override
    { return framework_seed; }

    void SetSeed(std::optional<uint32_t> new_value) override
    { framework_seed = new_value; }
};
}
