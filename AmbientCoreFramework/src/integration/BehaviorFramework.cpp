#include "BehaviorFramework.h"
#include <tracy/Tracy.hpp>
#include "entity/BehavioralEntity.h"
#include "services/layers/2_simulation/ActionTimeoutManager.h"

using namespace AmbientCharacterBehavior;

void BehaviorFramework::InitializeFramework(const std::string &schema_file_path, const std::string &sequences_file_path,
    const std::string &actions_file_path, const std::string &environmental_conditions_file_path,
    const std::string& log_file_path, FrameworkLogLevel log_level)
{
    ZoneScoped;

    if (!is_initialized)
    {
        try
        {
            auto success = InitializeFoundationServices(log_file_path, log_level) &&
                InitializeSimulationStateServices(schema_file_path, environmental_conditions_file_path, actions_file_path) &&
                InitializeRegistry(actions_file_path, sequences_file_path);
            is_initialized = success;

            if (success)
            {
                app_context->Foundation().logger.LogInfo("The framework has been initialized",
                    "BehaviorFramework");
            }
        }
        catch (const std::exception &e)
        {
            app_context->Foundation().logger.Initialize("framework_error.log", FrameworkLogLevel::INFO);
            app_context->Foundation().logger.LogInfo(e.what(), "BehaviorFramework");
        }
    }
}

bool BehaviorFramework::InitializeFoundationServices(const std::string& log_file_path, FrameworkLogLevel log_level) const
{
    ZoneScoped;

    try
    {
        if (app_context->Foundation().logger.Initialize(log_file_path, log_level))
        {
            app_context->Foundation().logger.LogInfo("Initialized Foundation Services.",
                "BehaviorFramework");

            return true;
        }
    }
    catch (const std::exception &e)
    {
        app_context->Foundation().logger.Initialize("framework_error.log", FrameworkLogLevel::INFO);
        app_context->Foundation().logger.LogInfo(e.what(), "BehaviorFramework");
    }

    return false;
}

bool BehaviorFramework::InitializeSimulationStateServices(const std::string& schema_file_path,
    const std::string& environmental_conditions_file_path, const std::string& actions_file_path) const
{
    ZoneScoped;

    app_context->Foundation().logger.LogInfo("Loading framework schema",
        "InitializeSimulationStateServices");

    if (!app_context->SimulationState().schema_manager.LoadFrameworkSchema(schema_file_path))
    {
        app_context->Foundation().logger.LogError("Failed to load framework schema",
            "InitializeSimulationStateServices");

        return false;
    }

    app_context->Foundation().logger.LogInfo("Registering environmental conditions",
        "InitializeSimulationStateServices");

    if (!app_context->SimulationState().environmental_condition_manager.RegisterEnvironmentalConditions(
        environmental_conditions_file_path))
    {
        app_context->Foundation().logger.LogError("Failed to register environmental conditions",
            "InitializeSimulationStateServices");

        return false;
    }

    if (!app_context->SimulationState().action_timeout_manager.Initialize(actions_file_path,
        [this](void* entity_handle, int32_t action_id, int64_t action_token)
        {
            this->CompleteCharacterAction(entity_handle, action_id, action_token);
        }
    ))
    {
        app_context->Foundation().logger.LogError("Failed to initialize ActionTimeoutManager",
            "InitializeSimulationStateServices");

        return false;
    }

    app_context->Foundation().logger.LogInfo("Initialized SimulationState Services",
        "InitializeSimulationStateServices");

    return true;
}

bool BehaviorFramework::InitializeRegistry(const std::string& actions_file_path, const std::string& sequences_file_path) const
{
    ZoneScoped;

    app_context->Foundation().logger.LogInfo("Registering actions", "BehaviorFramework");
    if (!app_context->ContentRegistry().content_provider.RegisterActions(actions_file_path))
    {
        app_context->Foundation().logger.LogError("Failed to register actions",
            "BehaviorFramework");

        return false;
    }

    app_context->Foundation().logger.LogInfo("Registering sequences", "BehaviorFramework");

    if (!app_context->ContentRegistry().content_provider.RegisterSequences(sequences_file_path))
    {
        app_context->Foundation().logger.LogError("Failed to register sequences",
            "BehaviorFramework");

        return false;
    }

    app_context->Foundation().logger.LogInfo("Initialized Registry",
        "BehaviorFramework");

    return true;
}

void BehaviorFramework::Update(int32_t character_batch_size, int64_t current_time_ms)
{
    ZoneScopedN("BehaviorFramework::Update");

    if (IsFrameworkInitialized())
    {
        app_context->Foundation().time_manager.SetCurrentTime(current_time_ms);

        if (app_context->ContentRegistry().entity_registry.GetPendingCommandCount() > 0)
        {
            ProcessPendingEntityCommands(character_batch_size);
        }
        else if (CanUpdateBehavioralEntities(character_batch_size))
        {
            app_context->SimulationState().action_timeout_manager.CheckActionTimeouts(current_time_ms);
            UpdateBehavioralEntities(character_batch_size);
        }
    }
}

bool BehaviorFramework::IsFrameworkInitialized() const
{
    if (!is_initialized)
    {
        app_context->Foundation().logger.LogWarning("The framework must be initialized",
            "BehaviorFramework");

        return false;
    }

    return true;
}

void BehaviorFramework::ProcessPendingEntityCommands(int32_t batch_size)
{
    ZoneScoped;

    is_processing_entity_batch = true;

    try
    {
        auto processed_count = app_context->ContentRegistry().entity_registry.ProcessPendingEntityCommands(batch_size);

        auto remaining_count = app_context->ContentRegistry().entity_registry.GetPendingCommandCount();

        app_context->Foundation().logger.LogInfo("Processed " + std::to_string(processed_count) +
            " entity commands, " + std::to_string(remaining_count) + " remaining", "BehaviorFramework");

    }
    catch (const std::exception &e)
    {
        app_context->Foundation().logger.LogError("Error processing entity commands: " +
                std::string(e.what()),"BehaviorFramework");

        is_processing_entity_batch = false;
    }

    is_processing_entity_batch = false;
}

bool BehaviorFramework::CanUpdateBehavioralEntities(int32_t character_batch_size) const
{
    if (is_processing_entity_batch)
    {
        app_context->Foundation().logger.LogWarning("The framework is still processing the previous batch",
            "BehaviorFramework");

        return false;
    }

    if (character_batch_size == 0)
    {
        app_context->Foundation().logger.LogInfo("No characters to process (batch_size = 0)",
            "BehaviorFramework");

        return false;
    }

    return true;
}

/**
 * @brief Follows a round-robin pattern using the character_batch_size.
 */
void BehaviorFramework::UpdateBehavioralEntities(int32_t character_batch_size)
{
    ZoneScoped;
    ZoneText("batch_size", 10);
    ZoneValue(character_batch_size);

    is_processing_entity_batch = true;
    auto total_entities = GetTotalEntitiesCount();
    if (total_entities > 0)
    {
        try
        {
            auto entities_range = ComputeBatchRange(character_batch_size, total_entities);

            auto entities_to_process =
                app_context->ContentRegistry().entity_registry.GetBehavioralEntitiesRange(entities_range.start_index,
                    entities_range.count);

            app_context->Foundation().logger.LogDebug("PERF," + std::to_string(entities_to_process.size()) +
                "," + std::to_string(total_entities), "UpdateBehavioralEntities");

            ZoneText("entities_to_process", 19);
            ZoneValue(entities_to_process.size());

            for (BehavioralEntity* entity : entities_to_process)
            {
                entity->ExecuteCurrentSequence();
            }

            UpdateCurrentBatchStartIndex(character_batch_size, entities_range, total_entities);
        }
        catch (const std::exception &e)
        {
            app_context->Foundation().logger.LogError("Error updating behavioral entities: " +
                std::string(e.what()),"BehaviorFramework");

            is_processing_entity_batch = false;
        }
    }

    is_processing_entity_batch = false;
}

int32_t BehaviorFramework::GetTotalEntitiesCount() const
{
    return app_context->ContentRegistry().entity_registry.GetBehavioralEntityCount();
}

BehaviorFramework::EntityBatchRange BehaviorFramework::ComputeBatchRange(int32_t character_batch_size,
    int32_t total_entities) const
{
    EntityBatchRange result;

    if (character_batch_size == -1)
    {
        result.count = total_entities;
        result.start_index = 0;
    }
    else
    {
        result.count = std::min(character_batch_size, total_entities);
        result.start_index = current_batch_start_index;

        if (result.start_index + result.count > total_entities)
        {
            result.count = total_entities - result.start_index;
        }
    }

    return result;
}

void BehaviorFramework::UpdateCurrentBatchStartIndex(int32_t character_batch_size, const EntityBatchRange& entities_range,
    int32_t total_entities)
{
    if (character_batch_size != -1)
    {
        current_batch_start_index = (entities_range.start_index + entities_range.count) % total_entities;
    }
}

void BehaviorFramework::ProcessInterruption(int32_t interruption_id, const std::vector<void *> &affected_entity_handles) const
{
    ZoneScoped;
    ZoneText("interruption_id", 15);
    ZoneValue(interruption_id);
    ZoneText("affected_entity_handles", 23);
    ZoneValue(affected_entity_handles.size());

    if (IsFrameworkInitialized())
    {
        int32_t processed_count = 0;
        int32_t failed_count = 0;

        for (const auto& entity_handle : affected_entity_handles)
        {
            if (ProcessInterruptionForEntity(interruption_id, entity_handle))
            {
                processed_count++;
            }
            else
            {
                failed_count++;
            }
        }

        app_context->Foundation().logger.LogInfo("Processed interruption " + std::to_string(interruption_id) +
        " for " + std::to_string(processed_count) + " entities (" + std::to_string(failed_count) + " failed)",
        "BehaviorFramework");
    }
}

bool BehaviorFramework::ProcessInterruptionForEntity(int32_t interruption_id, void* entity_handle) const
{
    ZoneScoped;

    try
    {
        if (BehavioralEntity* entity = app_context->ContentRegistry().entity_registry.GetBehavioralEntityByHandle(
            entity_handle))
        {
            entity->ProcessInterruption(interruption_id);
            return true;
        }

        app_context->Foundation().logger.LogWarning("Invalid entity handle during interruption " +
            std::to_string(interruption_id) + " processing","BehaviorFramework");
    }
    catch (const std::exception &e)
    {
        app_context->Foundation().logger.LogError("Error processing interruption for entity: " +
            std::string(e.what()), "BehaviorFramework");
    }
    return false;
}

void BehaviorFramework::RegisterEntity(void *entity_handle, const std::string &config_path, int32_t entity_pos_x,
    int32_t entity_pos_y, int32_t entity_pos_z) const
{
    ZoneScoped;

    Position3D position {entity_pos_x, entity_pos_y, entity_pos_z};
    app_context->ContentRegistry().entity_registry.QueueEntityRegistration(entity_handle, config_path, position);
    app_context->Foundation().logger.LogInfo("Queued entity registration command" ,
        "BehaviorFramework");
}

void BehaviorFramework::UnregisterEntity(void *entity_handle) const
{
    ZoneScoped;

    app_context->ContentRegistry().entity_registry.QueueEntityUnregistration(entity_handle);
    app_context->Foundation().logger.LogInfo("Queued entity unregistration command" ,
        "BehaviorFramework");
}

void BehaviorFramework::CompleteCharacterAction(void *entity_handle, int32_t action_id, int64_t action_token) const
{
    ZoneScoped;
    ZoneText("action_id", 9);
    ZoneValue(action_id);
    ZoneText("action_token", 12);
    ZoneValue(action_token);

    try
    {
        if (BehavioralEntity* entity = app_context->ContentRegistry().entity_registry.GetBehavioralEntityByHandle(entity_handle))
        {
            ZoneText("entity_id", 9);
            ZoneValue(entity->GetEntityId());

            entity->CompleteAction(action_id, action_token);
        }
        else
        {
            app_context->Foundation().logger.LogWarning("Invalid entity handle during action with id " +
                std::to_string(action_id) + " and token " + std::to_string(action_token) + " completion",
                "BehaviorFramework");
        }
    }
    catch (const std::exception &e)
    {
        app_context->Foundation().logger.LogError("Error completing character action: " +
            std::string(e.what()), "BehaviorFramework");
    }
}
