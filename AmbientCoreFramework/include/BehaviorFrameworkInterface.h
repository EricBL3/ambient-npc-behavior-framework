#pragma once
#include <cstdint>

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

    typedef int32_t (*QueryEnvironmentalConditionFn) (int32_t condition_key);
    AmbientCoreFramework_API void RegisterQueryEnvironmentalConditionFunction(QueryEnvironmentalConditionFn fn);

#ifdef __cplusplus
}
#endif

namespace AmbientCharacterBehavior {
    /**
     * @brief Queries the engine to get the most recent value of the environmental condition
     * @return The value of the environmental condition
     * @throw std::runtime_error if the callback to the engine's method is not registered.
     */
    int32_t QueryEnvironmentalCondition(int32_t condition_key);
}