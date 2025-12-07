#include "../include/BehaviorFrameworkInterface.h"
#include "services/composition/ServiceBuilder.h"
#include <stdexcept>

#include <tracy/Tracy.hpp>


using namespace AmbientCharacterBehavior;

extern "C" {
    AmbientCoreFramework_API void * CreateAmbientBehaviorFramework(QueryEnvironmentalConditionFn env_callback,
        StartCharacterActionFn start_action_callback, QueryEntityPositionFn query_position_callback)
    {
        ZoneScoped;

        try
        {
            auto framework = ServiceBuilder::CreateBehaviorFramework(
                env_callback, start_action_callback, query_position_callback);

            return framework.release();
        }
        catch (...)
        {
            return nullptr;
        }
    }

    AmbientCoreFramework_API bool InitializeAmbientBehaviorFramework(void* framework_handle, const char* schema_file_path,
        const char* sequences_file_path, const char* actions_file_path,
        const char* environmental_conditions_file_path, const char*  log_file_path, int32_t log_level)
    {

        if (const auto framework = static_cast<BehaviorFramework*>(framework_handle))
        {

            FrameworkLogLevel framework_log_level;
            switch (log_level)
            {
                case 0:
                    framework_log_level = FrameworkLogLevel::INFO;
                    break;
                case 1:
                    framework_log_level = FrameworkLogLevel::WARNING;
                    break;
                case 2:
                default:
                    framework_log_level = FrameworkLogLevel::ERROR;
                    break;
            }

            framework->InitializeFramework(schema_file_path, sequences_file_path, actions_file_path,
            environmental_conditions_file_path, log_file_path, framework_log_level);

            return framework->IsInitialized();
        }

        return false;
    }

    AmbientCoreFramework_API void ShutdownAmbientBehaviorFramework(void* framework_handle)
    {
        ZoneScoped;

        if (framework_handle)
        {
            auto framework = static_cast<BehaviorFramework*>(framework_handle);
            framework->GetServices().Core().logger.LogInfo("Shutting down ambient character behavior framework",
                "BehaviorFrameworkInterface");

            framework->ProcessPendingEntityCommands();

            delete framework;
        }
    }

    AmbientCoreFramework_API void Update(void* framework_handle, int32_t batch_size, int64_t current_time)
    {
        if (framework_handle)
        {
            auto framework = static_cast<BehaviorFramework*>(framework_handle);
            framework->Update(batch_size, current_time);
        }
    }

    AmbientCoreFramework_API void ProcessInterruption(void* framework_handle, int32_t interruption_id, void** entity_handles,
        int32_t count)
    {
        if (framework_handle && entity_handles && count > 0)
        {
            auto framework = static_cast<BehaviorFramework*>(framework_handle);
            std::vector<void*> affected_entities(entity_handles, entity_handles + count);
            framework->ProcessInterruption(interruption_id, affected_entities);
        }
    }

    AmbientCoreFramework_API void RegisterEntity(void* framework_handle, void* entity_handle, const char* config_path)
    {
        if (framework_handle)
        {
            auto framework = static_cast<BehaviorFramework*>(framework_handle);
            framework->RegisterEntity(entity_handle, config_path);
        }
    }

    AmbientCoreFramework_API void UnregisterEntity(void* framework_handle, void* entity_handle)
    {
        if (framework_handle)
        {
            auto framework = static_cast<BehaviorFramework*>(framework_handle);
            framework->UnregisterEntity(entity_handle);
        }
    }

    AmbientCoreFramework_API void CompleteCharacterAction(void* framework_handle, void *entity_handle, int32_t action_id,
        int64_t action_token)
    {
        if (framework_handle)
        {
            auto framework = static_cast<BehaviorFramework*>(framework_handle);
            framework->CompleteCharacterAction(entity_handle, action_id, action_token);
        }
    }

    AmbientCoreFramework_API void TracyFrameMarkWithTime(double engineTimeMs, int engineFrame)
    {
        FrameMark;

        auto msg = "Engine Frame " + std::to_string(engineFrame) +
                  " at " + std::to_string(engineTimeMs) + " ms";

        TracyMessage(msg.c_str(), static_cast<uint32_t>(msg.size()));
    }
}


