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

    AmbientCoreFramework_API void* CreateAmbientBehaviorFramework();
    AmbientCoreFramework_API bool InitializeAmbientBehaviorFramework(void* framework_handle, const char* schema_file_path,
        const char* sequences_file_path, const char* actions_file_path,
        const char* environmental_conditions_file_path, const char*  log_file_path);

    AmbientCoreFramework_API void ShutdownAmbientBehaviorFramework(void* framework_handle);
    AmbientCoreFramework_API void Update(void* framework_handle, int32_t batch_size, int64_t current_time);
    AmbientCoreFramework_API void ProcessInterruption(void* framework_handle, int32_t interruption_id, void** entity_handles,
        int32_t count);

    AmbientCoreFramework_API void RegisterEntity(void* framework_handle, void* entity_handle, const char* config_path);
    AmbientCoreFramework_API void UnregisterEntity(void* framework_handle, void* entity_handle);

    typedef int32_t (*QueryEnvironmentalConditionFn) (int32_t condition_key);
    AmbientCoreFramework_API void RegisterQueryEnvironmentalConditionFunction(QueryEnvironmentalConditionFn fn);

    typedef void (*StartActionCharacterFn) (void* entity_handle, int32_t action_id, int64_t action_token, void* target_entity_handle);
    AmbientCoreFramework_API void RegisterStartActionCharacterFunction(StartActionCharacterFn fn);

    AmbientCoreFramework_API void CompleteCharacterAction(void* framework_handle, void* entity_handle, int32_t action_id,
        int64_t action_token);

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

    /**
     * @brief Starts the requested action for the character.
     * @throw std::runtime_error if the callback to the engine's method is not registered.
     */
    void StartCharacterAction(void* entity_handle, int32_t action_id, int64_t action_token, void* target_entity_handle);
}