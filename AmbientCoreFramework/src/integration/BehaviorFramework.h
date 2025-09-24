#pragma once
#include "services/composition/ApplicationContext.h"

namespace AmbientCharacterBehavior {
class BehaviorFramework {
private:
    struct EntityBatchRange {
        int32_t start_index;
        int32_t count;
    };

    std::unique_ptr<ApplicationContext> app_context;
    bool is_initialized;
    bool is_processing_entity_batch;
    int32_t current_batch_start_index;


public:
    explicit BehaviorFramework(std::unique_ptr<ApplicationContext> context) :
        app_context(std::move(context)), is_initialized(false), is_processing_entity_batch(false),
        current_batch_start_index(0) {}

    ApplicationContext& GetServices() const { return *app_context; }

    void InitializeFramework(const std::string &schema_file_path, const std::string &sequences_file_path,
        const std::string &actions_file_path, const std::string &environmental_conditions_file_path,
        const std::string& log_file_path);

    bool IsInitialized() const { return is_initialized; }

    void Update(int32_t character_batch_size, int64_t current_time_ms);
    void ProcessInterruption(int32_t interruption_id, const std::vector<void*> &affected_entity_handles) const;

    void RegisterEntity(void* entity_handle, const std::string& config_path) const;
    void UnregisterEntity(void* entity_handle) const;

    void ProcessPendingEntityCommands() const;

private:

    bool InitializeCoreServices(const std::string& log_file_path) const;
    bool InitializeDomainServices(const std::string& schema_file_path,
        const std::string& environmental_conditions_file_path ) const;

    bool InitializeRegistry(const std::string& actions_file_path, const std::string& sequences_file_path) const;

    void UpdateBehavioralEntities(int32_t character_batch_size);
    bool IsFrameworkInitialized() const;

    bool CanUpdateBehavioralEntities(int32_t character_batch_size) const;
    int32_t GetTotalEntitiesCount() const;
    EntityBatchRange ComputeBatchRange(int32_t character_batch_size, int32_t total_entities) const;
    void UpdateCurrentBatchStartIndex(int32_t character_batch_size, const EntityBatchRange& entities_range,
        int32_t total_entities);

    bool ProcessInterruptionForEntity(int32_t interruption_id, void* entity_handle) const;
};
}
