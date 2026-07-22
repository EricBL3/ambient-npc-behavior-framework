#pragma once
#include <gmock/gmock.h>

#include "services/interfaces/ISeedManager.h"

namespace AmbientCharacterBehavior {
class MockSeedManager : public ISeedManager {
    MOCK_METHOD(std::optional<uint32_t>, GetSeed, (), (const, override));
    MOCK_METHOD(void, SetSeed, (std::optional<uint32_t> new_value), (override));
};
}
