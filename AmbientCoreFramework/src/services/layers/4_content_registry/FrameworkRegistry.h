#pragma once
#include <cstdint>
#include <memory>
#include <queue>
#include <unordered_map>
#include <concepts>
#include "behavior/Action.h"
#include "behavior/Sequence.h"
#include "entity/BehavioralEntity.h"
#include "entity/FrameworkEntity.h"
#include "services/composition/ServiceBundles.h"
#include "services/layers/2_simulation/EntityPosition.h"
#include "services/interfaces/IContentProvider.h"
#include "services/interfaces/IEntityPositionManager.h"
#include "services/interfaces/IEntityRegistry.h"
#include "services/interfaces/IEnvironmentalConditionManager.h"
#include "services/interfaces/IJsonLoader.h"
#include "services/interfaces/ILogger.h"
#include "services/interfaces/IFrameworkSchemaManager.h"

namespace AmbientCharacterBehavior {
class FrameworkRegistry : public IContentProvider, public IEntityRegistry, public IEntityQuery {
private:
    BehavioralEvaluationServices& services;
    ContentRegistryServices* self_bundle;

    std::unordered_map<int32_t, std::shared_ptr<Action>> actions;
    std::unordered_map<int32_t, std::shared_ptr<Sequence>> sequences;
    std::unordered_map<int32_t, std::unique_ptr<FrameworkEntity>> framework_entities;
    std::unordered_map<int32_t, std::unique_ptr<BehavioralEntity>> behavioral_entities;

    // Mappings between handle and entity_id for each container.
    std::unordered_map<void*, int32_t> handle_to_framework_id;
    std::unordered_map<void*, int32_t> handle_to_behavioral_id;
    std::unordered_map<int32_t, void*> framework_id_to_handle;
    std::unordered_map<int32_t, void*> behavioral_id_to_handle;

    std::unordered_map<int32_t, std::unordered_set<int32_t>> action_to_entities_index;
    std::unordered_map<int32_t, std::unordered_set<int32_t>> entity_to_actions_index;

    enum class EntityCommandType {
        REGISTER,
        UNREGISTER
    };

    struct EntityCommand {
        EntityCommandType type;
        void* entity_handle;
        std::string config_path;
        Position3D position;
    };

    std::queue<EntityCommand> pending_commands;

    ILogger& Logger() const
    {
        return services.simulation_state.data_access.foundation.logger;
    }

    ITimeManager& TimeManager() const
    {
        return services.simulation_state.data_access.foundation.time_manager;
    }

    ISeedManager& SeedManager() const
    {
        return services.simulation_state.data_access.foundation.seed_manager;
    }

    IStartCharacterActionProvider& ActionProvider() const
    {
        return services.simulation_state.data_access.foundation.start_character_action_provider;
    }

    IJsonLoader& JsonLoader() const
    {
        return services.simulation_state.data_access.json_loader;
    }

    IFrameworkSchemaManager& SchemaManager() const
    {
        return services.simulation_state.schema_manager;
    }

    IEnvironmentalConditionManager& EnvironmentManager() const
    {
        return services.simulation_state.environmental_condition_manager;
    }

    IEntityPositionManager& PositionManager() const
    {
        return services.simulation_state.entity_position_manager;
    }

    IActionTimeoutManager& ActionTimeoutManager() const
    {
        return services.simulation_state.action_timeout_manager;
    }

    IStateOperationEvaluator& StateEvaluator() const
    {
        return services.state_operation_evaluator;
    }


public:
    explicit FrameworkRegistry(BehavioralEvaluationServices& services);

    void SetSelfBundle(ContentRegistryServices& bundle);

    // IContentProvider interface

    bool RegisterSequences(const std::string& config_file_path) override;
    bool RegisterActions(const std::string& config_file_path) override;
    bool HasSequence(int32_t sequence_id) const override;
    std::shared_ptr<Sequence> GetSequenceById(int32_t sequence_id) const override;
    bool HasAction(int32_t action_id) const override;
    std::shared_ptr<Action> GetActionById(int32_t action_id) const override;

    // IEntityRegistry interface
    void QueueEntityRegistration(void* handle, const std::string& path, Position3D position) override;
    void QueueEntityUnregistration(void* handle) override;
    size_t ProcessPendingEntityCommands(int32_t batch_size) override;
    size_t GetPendingCommandCount() const override;
    void ClearPendingCommands() override;
    void RegisterEntity(void* entity_handle, const std::string& config_file_path, Position3D position);
    void UnregisterEntity(void* entity_handle);



    bool HasFrameworkEntity(int32_t entity_id) const override;
    FrameworkEntity* GetFrameworkEntityById(int32_t entity_id) const override;
    void* GetHandleFromFrameworkId(int32_t entity_id) const;
    int32_t GetFrameworkIdFromHandle(void* entity_handle) const;

    bool HasBehavioralEntity(int32_t entity_id) const override;
    BehavioralEntity* GetBehavioralEntityById(int32_t entity_id) const override;
    BehavioralEntity* GetBehavioralEntityByHandle(void* entity_handle) const override;
    void* GetHandleFromBehavioralId(int32_t entity_id) const;
    int32_t GetBehavioralIdFromHandle(void* entity_handle) const;

    std::vector<BehavioralEntity*> GetBehavioralEntitiesRange(int32_t start_index, int32_t count) const override;

    //IEntityQuery interface

    std::vector<FrameworkEntity*> GetEntitiesSupportingAction(int32_t action_id) const override;
    FrameworkEntity* GetEntityFromId(int32_t entity_id) const override;
    bool EntitySupportsAction(int32_t entity_id, int32_t action_id) const override;

    size_t GetSequencesCount() const
    { return sequences.size(); }

    size_t GetActionsCount() const
    {
        return actions.size();
    }

    size_t GetFrameworkEntitiesCount() const
    {
        return framework_entities.size();
    }

    size_t GetBehavioralEntitiesCount() const
    {
        return behavioral_entities.size();
    }

    int32_t GetBehavioralEntityCount() const override
    { return behavioral_entities.size(); }

private:
    bool GenerateSequenceFromDto(const SequenceDto &sequence_dto);
    bool ConfigureSequenceWithDto(const std::shared_ptr<Sequence> &new_sequence, const SequenceDto &sequence_dto) const;
    bool GenerateSequenceNodeFromDto(const std::shared_ptr<Sequence> &new_sequence, const SequenceNodeDto & dto_node) const;
    bool GenerateTransitionFromDto(const std::shared_ptr<Sequence> &new_sequence, const TransitionDto &dto_transition) const;
    std::unordered_map<StateOperationTarget, std::vector<StateOperation>> GenerateStateOperationHashTableFromDto(
        const std::vector<StateOperationDto> &dto_state_operations) const;

    StateOperation GenerateStateOperationFromDto(StateOperationTarget target, const StateOperationDto &dto_state_operation) const;

    bool GenerateActionFromDto(const ActionDto &action_dto);
    InterruptionBehaviorType ParseInterruptionBehavior(const std::string& behavior_name) const;
    bool ConfigureActionWithDto(const std::shared_ptr<Action> &new_action, const ActionDto &action_dto) const;

    std::optional<StateOperationTarget> ParseStateOperationTargetName(const std::string &target_name) const;

    FrameworkEntity* GenerateFrameworkEntityFromDto(void* entity_handle, std::optional<FrameworkEntityDto> entity_dto);
    bool IsEntityDuplicate(void* entity_handle, int32_t entity_id) const;

    void GenerateFrameworkEntityIdAndHandleMapping(const FrameworkEntity* framework_entity);
    void ConfigureFrameworkEntityWithDto(const std::unique_ptr<FrameworkEntity> &new_entity,
        const FrameworkEntityDto &entity_dto);

    BehavioralEntity* GenerateBehavioralEntityFromDto(void* entity_handle, std::optional<BehavioralEntityDto> entity_dto);
    void GenerateBehavioralEntityIdAndHandleMapping(const BehavioralEntity* behavioral_entity);

    void ConfigureBehavioralEntityWithDto(const std::unique_ptr<BehavioralEntity> &new_entity,
        const BehavioralEntityDto &entity_dto);

    void AddFallbackSequencesToEntity(const std::vector<int32_t> &fallback_sequences,
        const std::unique_ptr<BehavioralEntity> &new_entity) const;

    void AddInterruptionHandlersToEntity(const std::unordered_map<std::string, int32_t> &interruption_handlers,
        const std::unique_ptr<BehavioralEntity> &new_entity) const;

    int32_t DetermineCommandBatchSize(int32_t batch_size) const;

    bool UnregisterFrameworkEntity(void* entity_handle);
    bool UnregisterBehavioralEntity(void* entity_handle);

    void RemoveEntityFromActionIndex(int32_t entity_id);

    template<std::derived_from<FrameworkEntity> T>
    void RegisterActionsForEntity(const std::vector<int32_t> &action_ids, const std::unique_ptr<T> & entity)
    {

        auto entity_id = entity->GetEntityId();

        for (const auto& action_id : action_ids)
        {
            if (HasAction(action_id))
            {
                action_to_entities_index[action_id].insert(entity_id);
                entity_to_actions_index[entity_id].insert(action_id);

                Logger().LogInfo("Registered action " + std::to_string(action_id) + " for entity " +
                    std::to_string(entity_id),"RegisterActionForEntity");
            }
            else
            {
                Logger().LogWarning("Action with id: " + std::to_string(action_id) + " does not exist.",
                    "FrameworkRegistry");
            }
        }
    }

    template<typename T>
    void AddInitialStateMapToEntity(const std::unordered_map<std::string, int32_t> &initial_state,
        const std::unique_ptr<T> &new_entity) const
    {
        for (const auto& state_pair : initial_state)
        {
            try
            {
                auto state_key = SchemaManager().GetStateKey(state_pair.first);
                new_entity->SetStateValue(state_key, state_pair.second);
            }
            catch (const std::exception &e)
            {
                Logger().LogWarning("State '" + state_pair.first + "' does not exist.",
                    "FrameworkRegistry");
            }
        }
    }
};
}
