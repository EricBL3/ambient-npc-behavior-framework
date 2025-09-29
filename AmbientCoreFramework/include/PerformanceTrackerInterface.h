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
    * @brief Get execution time of the last Update() call
    * @return Duration in microseconds, or 0.0 if not available
    */
    AmbientCoreFramework_API double GetLastUpdateDurationMicroseconds();

    /**
     * @brief Get execution time of the last CompleteCharacterAction() call
     * @return Duration in microseconds, or 0.0 if not available
     */
    AmbientCoreFramework_API double GetLastCompleteActionDurationMicroseconds();

    /**
     * @brief Get execution time of the last ProcessInterruption() call
     * @return Duration in microseconds, or 0.0 if not available
     */
    AmbientCoreFramework_API double GetLastProcessInterruptionDurationMicroseconds();

    /**
     * @brief Get execution time of the last RegisterEntity() call
     * @return Duration in microseconds, or 0.0 if not available
     */
    AmbientCoreFramework_API double GetLastRegisterEntityDurationMicroseconds();

    /**
     * @brief Get execution time of the last UnregisterEntity() call
     * @return Duration in microseconds, or 0.0 if not available
     */
    AmbientCoreFramework_API double GetLastUnregisterEntityDurationMicroseconds();

    /**
     * @brief Reset all performance timing data
     */
    AmbientCoreFramework_API void ResetAllPerformanceTimings();

#ifdef __cplusplus
}
#endif