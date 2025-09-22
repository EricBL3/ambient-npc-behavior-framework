#include "PerformanceTracker.h"

using namespace AmbientCharacterBehavior;

std::chrono::high_resolution_clock::time_point PerformanceTracker::start_time;
double PerformanceTracker::last_duration_microseconds = 0.0;
bool PerformanceTracker::is_timing = false;

void PerformanceTracker::StartTiming()
{
    start_time = std::chrono::high_resolution_clock::now();
    is_timing = true;
}

void PerformanceTracker::StopTiming()
{
    if (!is_timing)
    {
        last_duration_microseconds = 0.0;
        return;
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();

    last_duration_microseconds = static_cast<double>(duration);
    is_timing = false;
}

double PerformanceTracker::GetLastDurationMicroseconds()
{
    return last_duration_microseconds;
}

bool PerformanceTracker::IsTiming()
{
    return is_timing;
}

void PerformanceTracker::Reset()
{
    last_duration_microseconds = 0.0;
    is_timing = false;
}
