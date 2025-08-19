/**
 * @file SequenceState
 * @brief 
 * @author Eric Buitrón López
 * @date 8/18/2025
 *
 *
*/

#pragma once

namespace AmbientCharacterBehavior {
enum class SequenceState {
    NORMAL = 0,
    INTERRUPTED = 1,
    FAILED = 2,
    IN_SUBSEQUENCE = 3,
    EXECUTING_ACTION = 4
};
}