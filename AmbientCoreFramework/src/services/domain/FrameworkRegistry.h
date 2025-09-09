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

    size_t GetSequencesCount() const
    { return sequences.size(); }

    size_t GetActionsCount() const
    {
        return actions.size();
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
};
}
