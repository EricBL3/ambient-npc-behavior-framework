#pragma once
#include <gmock/gmock.h>
#include "services/interfaces/IEntityPositionProvider.h"

namespace AmbientCharacterBehavior {
class MockEntityPositionProvider : public IEntityPositionProvider {
public:
    MOCK_METHOD((PositionQuery), QueryEntityPosition, (void* entity_handle), (override));
};
}
