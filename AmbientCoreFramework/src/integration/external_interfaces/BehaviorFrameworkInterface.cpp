#include "../include/BehaviorFrameworkInterface.h"
#include "services/composition/ServiceBuilder.h"
#include <stdexcept>
#include <tracy/Tracy.hpp>

#include "services/layers/0_foundation/SelectionAlgorithmOption.h"

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
        const char* environmental_conditions_file_path, const char*  log_file_path, int32_t log_level,
        int32_t selection_algorithm_option,
        int64_t seed)
    {

        if (const auto framework = static_cast<BehaviorFramework*>(framework_handle))
        {
            // Set log level
            auto log_level_candidate = static_cast<FrameworkLogLevel>(log_level);
            const bool is_log_level_valid =
                log_level_candidate == FrameworkLogLevel::DEBUG ||
                log_level_candidate == FrameworkLogLevel::INFO ||
                log_level_candidate == FrameworkLogLevel::WARNING ||
                log_level_candidate == FrameworkLogLevel::ERROR ||
                log_level_candidate == FrameworkLogLevel::METRIC;

            FrameworkLogLevel framework_log_level = is_log_level_valid ? log_level_candidate : FrameworkLogLevel::ERROR;

            // Set selection algorithm option
            auto selection_algorithm_option_candidate = static_cast<SelectionAlgorithmOption>(selection_algorithm_option);
            const bool is_selection_algorithm_option_valid =
                selection_algorithm_option_candidate == SelectionAlgorithmOption::MEMORY_BASED ||
                selection_algorithm_option_candidate == SelectionAlgorithmOption::UNIFORM_RANDOM;

            SelectionAlgorithmOption selection_algorithm = is_selection_algorithm_option_valid ?
            selection_algorithm_option_candidate : SelectionAlgorithmOption::MEMORY_BASED;

            // Set seed value if valid (not negative)
            std::optional<uint32_t> framework_seed;
            if (seed >= 0)
            {
                framework_seed = static_cast<uint32_t>(seed);
            }
            else
            {
                framework_seed = std::nullopt;
            }


            framework->InitializeFramework(schema_file_path, sequences_file_path, actions_file_path,
            environmental_conditions_file_path, log_file_path, framework_log_level, selection_algorithm,
            framework_seed);

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
            framework->GetServices().Foundation().logger.LogInfo("Shutting down ambient character behavior "
                "framework","BehaviorFrameworkInterface");

            // pass -1 as batch size to process all remaining entity commands
            framework->ProcessPendingEntityCommands(-1);

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

    AmbientCoreFramework_API void ProcessInterruption(void* framework_handle, int32_t interruption_id,
        void** entity_handles, int32_t count)
    {
        if (framework_handle && entity_handles && count > 0)
        {
            auto framework = static_cast<BehaviorFramework*>(framework_handle);
            std::vector<void*> affected_entities(entity_handles, entity_handles + count);
            framework->ProcessInterruption(interruption_id, affected_entities);
        }
    }

    AmbientCoreFramework_API void RegisterEntity(void* framework_handle, void* entity_handle, const char* config_path,
        int32_t entity_pos_x, int32_t entity_pos_y, int32_t entity_pos_z)
    {
        if (framework_handle)
        {
            auto framework = static_cast<BehaviorFramework*>(framework_handle);

            framework->RegisterEntity(entity_handle, config_path, entity_pos_x, entity_pos_y, entity_pos_z);
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


