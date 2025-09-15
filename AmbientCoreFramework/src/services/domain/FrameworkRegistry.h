#pragma once
#include <cstdint>
#include <memory>
#include <unordered_map>

#include "behavior/Action.h"
#include "behavior/Sequence.h"
#include "entity/BehavioralEntity.h"
#include "entity/FrameworkEntity.h"
#include "interfaces/IFrameworkRegistry.h"
#include "interfaces/IJsonLoader.h"
#include "interfaces/ILogger.h"
#include "interfaces/IStateSchemaManager.h"

namespace AmbientCharacterBehavior {
class FrameworkRegistry : public IFrameworkRegistry {
private:
    std::unordered_map<int32_t, std::shared_ptr<Action>> actions;
    std::unordered_map<int32_t, std::shared_ptr<Sequence>> sequences;
    std::unordered_map<int32_t, std::unique_ptr<FrameworkEntity>> framework_entities;
    std::unordered_map<int32_t, std::unique_ptr<BehavioralEntity>> behavioral_entities;

    // Mappings between handle and entity_id for each container.
    std::unordered_map<void*, int32_t> handle_to_framework_id;
    std::unordered_map<void*, int32_t> handle_to_behavioral_id;
    std::unordered_map<int32_t, void*> framework_id_to_handle;
    std::unordered_map<int32_t, void*> behavioral_id_to_handle;

    ILogger& logger;
    IJsonLoader& json_loader;
    IStateSchemaManager& state_schema;

public:
    explicit FrameworkRegistry(ILogger& logger, IJsonLoader& json_loader, IStateSchemaManager& state_schema) :
        logger(logger), json_loader(json_loader), state_schema(state_schema) {}

    void RegisterSequences(const std::string& config_file_path) override;
    void RegisterActions(const std::string& config_file_path) override;
    void RegisterEntity(void* entity_handle, const std::string& config_file_path) override;
    void UnregisterEntity(void* entity_handle) override;

    bool HasSequence(int32_t sequence_id) const override;
    std::shared_ptr<Sequence> GetSequenceById(int32_t sequence_id) const override;
    bool HasAction(int32_t action_id) const override;
    std::shared_ptr<Action> GetActionById(int32_t action_id) const override;

    bool HasFrameworkEntity(int32_t entity_id) const override;
    FrameworkEntity* GetFrameworkEntityById(int32_t entity_id) const override;
    void* GetHandleFromFrameworkId(int32_t entity_id) const;
    int32_t GetFrameworkIdFromHandle(void* entity_handle) const;

    bool HasBehavioralEntity(int32_t entity_id) const override;
    BehavioralEntity* GetBehavioralEntityById(int32_t entity_id) const override;
    void* GetHandleFromBehavioralId(int32_t entity_id) const;
    int32_t GetBehavioralIdFromHandle(void* entity_handle) const;

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

private:
    void GenerateSequenceFromDto(const SequenceDto &sequence_dto);
    void ConfigureSequenceWithDto(const std::shared_ptr<Sequence> &new_sequence, const SequenceDto &sequence_dto) const;
    void GenerateSequenceNodeFromDto(const std::shared_ptr<Sequence> &new_sequence, const SequenceNodeDto & dto_node) const;
    void GenerateTransitionFromDto(const std::shared_ptr<Sequence> &new_sequence, const TransitionDto &dto_transition) const;
    std::vector<StateOperation> GenerateStateOperationVectorFromDto(const std::vector<StateOperationDto> &dto_state_operations) const;
    StateOperation GenerateStateOperationFromDto(const StateOperationDto &dto_state_operation) const;

    void GenerateActionFromDto(const ActionDto &action_dto);
    InterruptionBehaviorType ParseInterruptionBehavior(const std::string& behavior_name) const;
    void ConfigureActionWithDto(const std::shared_ptr<Action> &new_action, const ActionDto &action_dto) const;

    FrameworkEntity* GenerateFrameworkEntityFromDto(void* entity_handle, std::optional<FrameworkEntityDto> entity_dto);
    bool IsEntityDuplicate(void* entity_handle, int32_t entity_id) const;
    void GenerateFrameworkEntityIdAndHandleMapping(const FrameworkEntity* framework_entity);
    void ConfigureFrameworkEntityWithDto(const std::unique_ptr<FrameworkEntity> &new_entity, const FrameworkEntityDto &entity_dto) const;

    BehavioralEntity* GenerateBehavioralEntityFromDto(void* entity_handle, std::optional<BehavioralEntityDto> entity_dto);
    void GenerateBehavioralEntityIdAndHandleMapping(const BehavioralEntity* behavioral_entity);
    void ConfigureBehavioralEntityWithDto(const std::unique_ptr<BehavioralEntity> &new_entity, const BehavioralEntityDto &entity_dto) const;

    void AddFallbackSequencesToEntity(const std::vector<int32_t> &fallback_sequences, const std::unique_ptr<BehavioralEntity> &new_entity) const;
    void AddInterruptionHandlersToEntity(const std::unordered_map<std::string, int32_t> &interruption_handlers, const std::unique_ptr<BehavioralEntity> &new_entity) const;

    template<typename T>
    void AddAcceptedActionsToEntity(const std::vector<int32_t> &accepted_actions_ids, const std::unique_ptr<T> &new_entity) const
    {
        {
            for (const auto& action_id : accepted_actions_ids)
            {
                if (HasAction(action_id))
                {
                    new_entity->AddSupportedAction(action_id);
                }
                else
                {
                    logger.LogWarning("Action with id" + std::to_string(action_id) + " does not exist.",
                        "FrameworkRegistry");
                }
            }
        }
    }

    template<typename T>
    void AddInitialStateMapToEntity(const std::unordered_map<std::string, int32_t> &initial_state, const std::unique_ptr<T> &new_entity) const
    {
        for (const auto& state_pair : initial_state)
        {
            try
            {
                auto state_key = state_schema.GetStateKey(state_pair.first);
                new_entity->SetStateValue(state_key, state_pair.second);
            }
            catch (const std::exception &e)
            {
                logger.LogWarning("State '" + state_pair.first + "' does not exist.",
                    "FrameworkRegistry");
            }
        }
    }
};
}
