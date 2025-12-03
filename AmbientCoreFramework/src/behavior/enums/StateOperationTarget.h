#pragma once

namespace AmbientCharacterBehavior {
enum class StateOperationTarget {
    ENVIRONMENT = -2,
    SELF = -1,
    ENTITY = 0,
};

inline std::string ToString(StateOperationTarget target) {
    switch (target) {
        case StateOperationTarget::SELF: return "SELF";
        case StateOperationTarget::ENVIRONMENT: return "ENVIRONMENT";
        case StateOperationTarget::ENTITY: return "ENTITY";
        default: return "UNKNOWN";
    }
}

}
