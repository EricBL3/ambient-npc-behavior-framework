#pragma once

namespace AmbientCharacterBehavior {
enum class StateOperationTarget {
    DISTANCE_TO_ENTITY = -3,
    ENVIRONMENT = -2,
    SELF = -1,
    ENTITY = 0,
};

inline std::string ToString(StateOperationTarget target) {
    switch (target) {
        case StateOperationTarget::DISTANCE_TO_ENTITY: return "DISTANCE_TO_ENTITY";
        case StateOperationTarget::ENVIRONMENT: return "ENVIRONMENT";
        case StateOperationTarget::SELF: return "SELF";
        case StateOperationTarget::ENTITY: return "ENTITY";
        default: return "UNKNOWN";
    }
}

}
