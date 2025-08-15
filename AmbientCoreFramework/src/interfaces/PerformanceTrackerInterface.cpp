/**
* @file PerformanceTrackerInterface.cpp
 * @brief Implementation of performance tracker public interface
 * @author Eric Buitrón López
 * @date 8/15/2025
 */

#include "../include/PerformanceTrackerInterface.h"
#include "utils/PerformanceTracker.h"

extern "C" double GetLastDurationMicroseconds()
{
    return PerformanceTracker::GetLastDurationMicroseconds();
}
