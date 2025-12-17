#pragma once
#include <cstdint>

#include "services/layers/2_simulation/EntityPosition.h"

namespace AmbientCharacterBehavior {
using QueryEntityPositionCallback = bool(*)(void* entity_handle, int32_t* out_xyz);

class IEntityPositionProvider {
public:
    virtual ~IEntityPositionProvider() = default;

    virtual PositionQuery QueryEntityPosition(void* entity_handle) = 0;
};
}
