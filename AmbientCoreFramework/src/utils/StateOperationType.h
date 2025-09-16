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
    EQUALS_STATE = 7,
    NOT_EQUALS_STATE = 8,
    GREATER_THAN_STATE = 9,
    LESS_THAN_STATE = 10,

    EXTERNAL_OPERATIONS = 1000
};
}