#pragma once
#include <cstdint>
#include <vector>

namespace AmbientCharacterBehavior {
class FrameworkEntity;

class IEntityQuery {
public:
    virtual ~IEntityQuery() = default;

    virtual std::vector<FrameworkEntity*> GetEntitiesSupportingAction(int32_t action_id) const = 0;

    virtual FrameworkEntity* GetEntityFromId(int32_t entity_id) const = 0;

    virtual bool EntitySupportsAction(int32_t entity_id, int32_t action_id) const = 0;
};
}
