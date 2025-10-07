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

inline std::string ToString(SequenceState state) {
    switch (state) {
        case SequenceState::UNINITIALIZED:
            return "UNINITIALIZED";
        case SequenceState::PROCESSING_NODE:
            return "PROCESSING_NODE";
        case SequenceState::IN_SUBSEQUENCE:
            return "IN_SUBSEQUENCE";
        case SequenceState::WAITING_FOR_ACTION:
            return "WAITING_FOR_ACTION";
        case SequenceState::NODE_EXECUTED:
            return "NODE_EXECUTED";
        case SequenceState::FAILED:
            return "FAILED";
        case SequenceState::INTERRUPTED:
            return "INTERRUPTED";
        default:
            return "UNKNOWN";
    }
}
}