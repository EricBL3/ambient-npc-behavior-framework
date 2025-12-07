#pragma once
#include <cstdint>

namespace AmbientCharacterBehavior {

struct Position3D {
    int32_t x;
    int32_t y;
    int32_t z;

    Position3D() : x(0), y(0), z(0) {}

    Position3D(int32_t x, int32_t y, int32_t z) : x(x), y(y), z(z) {}

};

struct PositionQuery {
    Position3D position;
    bool success = false;
};

struct EntityPosition {
    Position3D value;
    int64_t last_update_ms;
    int64_t update_frequency_ms;
    bool is_static;

    bool NeedsUpdate(int64_t current_time_ms) const
    {
        return !is_static && current_time_ms - last_update_ms >= update_frequency_ms;
    }
};
}
