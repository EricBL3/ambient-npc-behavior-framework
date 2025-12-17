#pragma once
#include <stdexcept>
#include "services/interfaces/IEntityPositionProvider.h"

namespace AmbientCharacterBehavior {
class EntityPositionProvider : public IEntityPositionProvider {
    QueryEntityPositionCallback query_callback;

public:

    explicit EntityPositionProvider(QueryEntityPositionCallback query_callback) : query_callback(query_callback)
    {
        if (!query_callback)
        {
            throw std::invalid_argument("EntityPositionProvider: Query function cannot be null");
        }
    }

    PositionQuery QueryEntityPosition(void *entity_handle) override
    {
        int32_t xyz[3] = {0, 0, 0};

        bool success = query_callback(entity_handle, xyz);

        return PositionQuery{Position3D(xyz[0], xyz[1], xyz[2]), success};
    }
};
}
