#pragma once

namespace AmbientCharacterBehavior {
enum class StateOperationType {
    EQUALS = 0,
    NOT_EQUALS = 1,
    GREATER_THAN = 2,
    LESS_THAN = 3,
    SET = 4,
    INCREMENT = 5,
    DECREMENT = 6,

    EXTERNAL_OPERATIONS = 1000
};
}