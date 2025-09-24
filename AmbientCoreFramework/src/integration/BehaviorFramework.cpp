#include "BehaviorFramework.h"

#include "entity/BehavioralEntity.h"

using namespace AmbientCharacterBehavior;

void BehaviorFramework::InitializeFramework(const std::string &schema_file_path, const std::string &sequences_file_path,
    const std::string &actions_file_path, const std::string &environmental_conditions_file_path,
    const std::string& log_file_path)
{
    if (!is_initialized)
    {
        try
        {
            auto success = InitializeCoreServices(log_file_path) &&
                InitializeDomainServices(schema_file_path, environmental_conditions_file_path) &&
                InitializeRegistry(actions_file_path, sequences_file_path);
            is_initialized = success;

            if (success)
            {
                app_context->Core().logger.LogInfo("The framework has been initialized",
                    "BehaviorFramework");
            }
        }
        catch (const std::exception &e)
        {
            app_context->Core().logger.Initialize("framework_error.log");
            app_context->Core().logger.LogInfo(e.what(), "BehaviorFramework");
        }
    }
}

bool BehaviorFramework::InitializeCoreServices(const std::string& log_file_path) const
{
    try
    {
        if (app_context->Core().logger.Initialize(log_file_path))
        {
            app_context->Core().logger.LogInfo("Initialized Core Services.","BehaviorFramework");
            return true;
        }
    }
    catch (const std::exception &e)
    {
        app_context->Core().logger.Initialize("framework_error.log");
        app_context->Core().logger.LogInfo(e.what(), "BehaviorFramework");
    }

    return false;
}

bool BehaviorFramework::InitializeDomainServices(const std::string& schema_file_path,
    const std::string& environmental_conditions_file_path ) const
{
    app_context->Core().logger.LogInfo("Loading framework schema","BehaviorFramework");

    if (!app_context->Domain().schema_manager.LoadFrameworkSchema(schema_file_path))
    {
        app_context->Core().logger.LogError("Failed to load framework schema","BehaviorFramework");
        return false;
    }

    app_context->Core().logger.LogInfo("Registering environmental conditions","BehaviorFramework");

    if (!app_context->Domain().environmental_condition_manager.RegisterEnvironmentalConditions(environmental_conditions_file_path))
    {
        app_context->Core().logger.LogError("Failed to register environmental conditions",
            "BehaviorFramework");

        return false;
    }

    app_context->Core().logger.LogInfo("Initialized Domain Services","BehaviorFramework");

    return true;
}

bool BehaviorFramework::InitializeRegistry(const std::string& actions_file_path, const std::string& sequences_file_path) const
{
    app_context->Core().logger.LogInfo("Registering actions", "BehaviorFramework");
    if (!app_context->Registry().registry.RegisterActions(actions_file_path))
    {
        app_context->Core().logger.LogError("Failed to register actions",
            "BehaviorFramework");

        return false;
    }

    app_context->Core().logger.LogInfo("Registering sequences", "BehaviorFramework");

    if (!app_context->Registry().registry.RegisterSequences(sequences_file_path))
    {
        app_context->Core().logger.LogError("Failed to register sequences",
            "BehaviorFramework");

        return false;
    }

    app_context->Core().logger.LogInfo("Initialized Registry",
        "BehaviorFramework");

    return true;
}

void BehaviorFramework::Update(int32_t character_batch_size, int64_t current_time_ms)
{
    if (IsFrameworkInitialized())
    {
        app_context->Core().time_manager.SetCurrentTime(current_time_ms);

        if (app_context->Registry().registry.GetPendingCommandCount() > 0)
        {
            ProcessPendingEntityCommands();
        }

        if (CanUpdateBehavioralEntities(character_batch_size))
        {
            UpdateBehavioralEntities(character_batch_size);
        }
    }
}

bool BehaviorFramework::IsFrameworkInitialized() const
{
    if (!is_initialized)
    {
        app_context->Core().logger.LogWarning("The framework must be initialized",
            "BehaviorFramework");

        return false;
    }

    return true;
}

void BehaviorFramework::ProcessPendingEntityCommands() const
{
    try
    {
        auto processed_count = app_context->Registry().registry.ProcessPendingEntityCommands();
        app_context->Core().logger.LogInfo("Processed " + std::to_string(processed_count) + " entity commands",
        "BehaviorFramework");

    }
    catch (const std::exception &e)
    {
        app_context->Core().logger.LogError("Error processing entity commands: " +
                std::string(e.what()),"BehaviorFramework");
    }
}

bool BehaviorFramework::CanUpdateBehavioralEntities(int32_t character_batch_size) const
{
    if (is_processing_entity_batch)
    {
        app_context->Core().logger.LogWarning("The framework is still processing the previous batch",
            "BehaviorFramework");

        return false;
    }

    if (character_batch_size == 0)
    {
        app_context->Core().logger.LogInfo("No characters to process (batch_size = 0)",
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
    is_processing_entity_batch = true;

    auto total_entities = GetTotalEntitiesCount();
    if (total_entities > 0)
    {
        try
        {
            auto entities_range = ComputeBatchRange(character_batch_size, total_entities);

            auto entities_to_process = app_context->Registry().registry.GetBehavioralEntitiesRange(
                entities_range.start_index, entities_range.count);

            for (BehavioralEntity* entity : entities_to_process)
            {
                entity->ExecuteCurrentSequence();
            }

            UpdateCurrentBatchStartIndex(character_batch_size, entities_range, total_entities);

            app_context->Core().logger.LogInfo("Updated " + std::to_string(entities_range.count) +
                " entities starting from index " + std::to_string(entities_range.start_index),
                "BehaviorFramework");
        }
        catch (const std::exception &e)
        {
            app_context->Core().logger.LogError("Error updating behavioral entities: " +
                std::string(e.what()),"BehaviorFramework");
        }
    }

    is_processing_entity_batch = false;
}

int32_t BehaviorFramework::GetTotalEntitiesCount() const
{
    return app_context->Registry().registry.GetBehavioralEntityCount();
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

        app_context->Core().logger.LogInfo("Processed interruption " + std::to_string(interruption_id) +
        " for " + std::to_string(processed_count) + " entities (" + std::to_string(failed_count) + " failed)",
        "BehaviorFramework");
    }
}

bool BehaviorFramework::ProcessInterruptionForEntity(int32_t interruption_id, void* entity_handle) const
{
    try
    {
        if (BehavioralEntity* entity = app_context->Registry().registry.GetBehavioralEntityByHandle(entity_handle))
        {
            entity->ProcessInterruption(interruption_id);
            return true;
        }

        app_context->Core().logger.LogWarning("Invalid entity handle during interruption " +
            std::to_string(interruption_id) + " processing","BehaviorFramework");
    }
    catch (const std::exception &e)
    {
        app_context->Core().logger.LogError("Error processing interruption for entity: " +
            std::string(e.what()), "BehaviorFramework");
    }
    return false;
}

void BehaviorFramework::RegisterEntity(void *entity_handle, const std::string &config_path) const
{
    app_context->Registry().registry.QueueEntityRegistration(entity_handle, config_path);
    app_context->Core().logger.LogInfo("Queued entity registration command" ,"BehaviorFramework");
}

void BehaviorFramework::UnregisterEntity(void *entity_handle) const
{
    app_context->Registry().registry.QueueEntityUnregistration(entity_handle);
    app_context->Core().logger.LogInfo("Queued entity unregistration command" ,"BehaviorFramework");
}
