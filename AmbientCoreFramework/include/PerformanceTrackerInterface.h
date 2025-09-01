/**
 * @file PerformanceTrackerInterface.h
 * @brief Public C interface for DLL performance measurement
 * @author Eric Buitrón López
 * @date 8/15/2025
 *
 * This interface allows an engine to retrieve timing data from DLL method calls.
 * DLL methods automatically time themselves during execution, and the engine can query the results to
 * measure communication overhead and algorithm performance.
 */

#pragma once

#ifdef _WIN32
    #ifdef AmbientCoreFramework_EXPORTS
        #define AmbientCoreFramework_API __declspec(dllexport)
    #else
        #define AmbientCoreFramework_API __declspec(dllimport)
    #endif

#else
    #define AmbientCoreFramework_API __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

    /**
     * @brief Retrieves timing from the last completed DLL method call
     * @return Duration in microseconds of the last timed operation, or 0.0 if no timing is available
     *
     * Call this immediately after invoking any DLL method to get its execution time.
     * DLL methods time themselves, so this retrieves their internal measurements.
     *
     * @note Timing data is overwritten on each new method call
     * @note Returns 0.0 if no method has been timed or if timing failed
     */
    AmbientCoreFramework_API double GetLastDurationMicroseconds();

#ifdef __cplusplus
}
#endif