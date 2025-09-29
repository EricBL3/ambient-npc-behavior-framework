#include "../include/BehaviorFrameworkInterface.h"
#include "services/composition/ServiceBuilder.h"
#include <stdexcept>


using namespace AmbientCharacterBehavior;

static QueryEnvironmentalConditionFn query_callback = nullptr;
static StartCharacterActionFn start_action_callback = nullptr;


extern "C" {
    AmbientCoreFramework_API void * CreateAmbientBehaviorFramework()
    {
        try
        {
            auto framework = ServiceBuilder::CreateBehaviorFramework();
            return framework.release();
        }
        catch (...)
        {
            return nullptr;
        }
    }

    AmbientCoreFramework_API bool InitializeAmbientBehaviorFramework(void* framework_handle, const char* schema_file_path,
        const char* sequences_file_path, const char* actions_file_path,
        const char* environmental_conditions_file_path, const char*  log_file_path)
    {
        if (const auto framework = static_cast<BehaviorFramework*>(framework_handle))
        {
            framework->InitializeFramework(schema_file_path, sequences_file_path, actions_file_path,
            environmental_conditions_file_path, log_file_path);

            return framework->IsInitialized();
        }

        return false;
    }

    AmbientCoreFramework_API void ShutdownAmbientBehaviorFramework(void* framework_handle)
    {
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

    AmbientCoreFramework_API void RegisterQueryEnvironmentalConditionFunction(QueryEnvironmentalConditionFn fn)
    {
        query_callback = fn;
    }

    AmbientCoreFramework_API void RegisterStartCharacterActionFunction(StartCharacterActionFn fn)
    {
        start_action_callback = fn;
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

}

namespace AmbientCharacterBehavior {
    int32_t QueryEnvironmentalCondition(int32_t condition_key)
    {
        if (!query_callback)
        {
            throw std::runtime_error("QueryEnvironmentalCondition: Callback not registered");
        }

        return query_callback(condition_key);
    }

    void StartCharacterAction(void* entity_handle, int32_t action_id, int64_t action_token, void* target_entity_handle)
    {
        if (!start_action_callback)
        {
            throw std::runtime_error("StartCharacterAction: Callback not registered");
        }

        return start_action_callback(entity_handle, action_id, action_token, target_entity_handle);
    }
}


