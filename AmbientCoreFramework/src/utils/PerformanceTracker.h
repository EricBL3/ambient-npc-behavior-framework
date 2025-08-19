
#pragma once
#include <chrono>

namespace AmbientCharacterBehavior {
/**
 * @brief Static method timing tracker for DLL performance measurement
 *
 * Provides simple start/stop timing for benchmarking individual method calls.
 * All methods are static so no instance creation is needed.
 */
class PerformanceTracker {
private:
    static std::chrono::high_resolution_clock::time_point start_time;
    static double last_duration_microseconds;
    static bool is_timing;

public:
    static void StartTiming();
    static void StopTiming();

    static double GetLastDurationMicroseconds();

    static bool IsTiming();

    static void Reset();
};
}