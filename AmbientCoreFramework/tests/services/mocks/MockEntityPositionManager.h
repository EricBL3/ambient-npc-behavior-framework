#pragma once
#include <gmock/gmock.h>
#include "services/interfaces/IEntityPositionManager.h"

namespace AmbientCharacterBehavior {
class MockEntityPositionManager : public IEntityPositionManager {
public:
    MOCK_METHOD((void), RegisterEntityPosition, (void* entity_handle, Position3D position, bool is_static,
        int64_t update_frequency_ms), (override));

    MOCK_METHOD((void), UnregisterEntityPosition, (void* entity_handle), (override));
    MOCK_METHOD((int32_t), CalculateDistance, (void* entity_a_handle, void* entity_b_handle), (override));
    MOCK_METHOD((void), UpdateEntityPosition, (void* entity_handle), (override));
};
}
