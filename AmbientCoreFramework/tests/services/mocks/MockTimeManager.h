#pragma once
#include "services/interfaces/ITimeManager.h"
#include <gmock/gmock.h>

namespace AmbientCharacterBehavior {
class MockTimeManager : public ITimeManager {
public:
    MOCK_METHOD(int64_t, GetCurrentTime, (), (const, override));
    MOCK_METHOD(void, SetCurrentTime, (int64_t time), (override));
};
}
