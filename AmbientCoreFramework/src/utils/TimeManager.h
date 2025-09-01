/**
 * @file TimeManager.h
 * @brief 
 * @author Eric Buitrón López
 * @date 9/1/2025
 *
 *
*/

#pragma once
#include <cstdint>

namespace AmbientCharacterBehavior {
/**
 * @brief Manages the framework's representation of time.
 */
class TimeManager {
private:
    static int64_t current_time_ms;

public:
    static int64_t GetCurrentTime() { return current_time_ms; }

    static void SetCurrentTime(int64_t time) { current_time_ms = time; }
};
} // AmbientCharacterBehavior
