#pragma once
#include <gmock/gmock.h>
#include "services/interfaces/IEntityQuery.h"

namespace AmbientCharacterBehavior {
class MockEntityQuery : public IEntityQuery {
public:
    MOCK_METHOD(std::vector<FrameworkEntity*>, GetEntitiesSupportingAction, (int32_t action_id), (const, override));
    MOCK_METHOD(FrameworkEntity*, GetEntityFromId, (int32_t entity_id), (const, override));
};
}
