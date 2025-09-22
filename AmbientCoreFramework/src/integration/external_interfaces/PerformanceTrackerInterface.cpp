#include "../include/PerformanceTrackerInterface.h"
#include "utils/PerformanceTracker.h"

using namespace AmbientCharacterBehavior;

extern "C"
{
    AmbientCoreFramework_API double GetLastDurationMicroseconds()
    {
        return PerformanceTracker::GetLastDurationMicroseconds();
    }
}