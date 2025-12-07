#pragma once
#include <cstdint>

#include "services/domain/EntityPosition.h"

namespace AmbientCharacterBehavior {
class IEntityPositionManager {
public:
    virtual ~IEntityPositionManager() = default;
    virtual void RegisterEntityPosition(void* entity_handle, Position3D position, bool is_static,
        int64_t update_frequency_ms) = 0;

    virtual void UnregisterEntityPosition(void* entity_entity_handle) = 0;
    virtual int32_t CalculateDistance(void* entity_a_handle, void* entity_b_handle) = 0;
    virtual void UpdateEntityPosition(void* entity_handle) = 0;
};
}
