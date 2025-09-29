#include "../include/PerformanceTrackerInterface.h"
#include "utils/PerformanceTracker.h"

using namespace AmbientCharacterBehavior;

extern "C"
{
    AmbientCoreFramework_API double GetLastUpdateDurationMicroseconds() {
        return PerformanceTracker::GetLastUpdateDurationMicroseconds();
    }

    AmbientCoreFramework_API double GetLastCompleteActionDurationMicroseconds() {
        return PerformanceTracker::GetLastCompleteActionDurationMicroseconds();
    }

    AmbientCoreFramework_API double GetLastProcessInterruptionDurationMicroseconds() {
        return PerformanceTracker::GetLastProcessInterruptionDurationMicroseconds();
    }

    AmbientCoreFramework_API double GetLastRegisterEntityDurationMicroseconds() {
        return PerformanceTracker::GetLastRegisterEntityDurationMicroseconds();
    }

    AmbientCoreFramework_API double GetLastUnregisterEntityDurationMicroseconds() {
        return PerformanceTracker::GetLastUnregisterEntityDurationMicroseconds();
    }

    AmbientCoreFramework_API void ResetAllPerformanceTimings() {
        PerformanceTracker::ResetAll();
    }
}