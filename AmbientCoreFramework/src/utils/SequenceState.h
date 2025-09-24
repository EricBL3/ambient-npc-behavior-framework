#pragma once

namespace AmbientCharacterBehavior {
enum class SequenceState {
    UNINITIALIZED = 0,
    PROCESSING_NODE = 1,
    IN_SUBSEQUENCE = 2,
    WAITING_FOR_ACTION = 3,
    NODE_EXECUTED = 4,
    FAILED = 5,
    INTERRUPTED = 6
};
}