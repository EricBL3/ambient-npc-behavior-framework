
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
    struct TimingSlot {
        std::chrono::high_resolution_clock::time_point start_time;
        double last_duration_microseconds = 0.0;
        bool is_timing = false;
    };

    static TimingSlot update_timing;
    static TimingSlot complete_action_timing;
    static TimingSlot process_interruption_timing;
    static TimingSlot register_entity_timing;
    static TimingSlot unregister_entity_timing;

    static void StartTiming(TimingSlot& slot);
    static void StopTiming(TimingSlot& slot);
    static double GetDuration(const TimingSlot& slot);
public:
    static void StartUpdateTiming();
    static void StopUpdateTiming();
    static double GetLastUpdateDurationMicroseconds();

    static void StartCompleteActionTiming();
    static void StopCompleteActionTiming();
    static double GetLastCompleteActionDurationMicroseconds();

    static void StartProcessInterruptionTiming();
    static void StopProcessInterruptionTiming();
    static double GetLastProcessInterruptionDurationMicroseconds();

    static void StartRegisterEntityTiming();
    static void StopRegisterEntityTiming();
    static double GetLastRegisterEntityDurationMicroseconds();

    static void StartUnregisterEntityTiming();
    static void StopUnregisterEntityTiming();
    static double GetLastUnregisterEntityDurationMicroseconds();

    static void ResetAll();
};
}