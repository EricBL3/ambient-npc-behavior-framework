#include "./FrameworkRegistry.h"

#include <memory>

#include "utils/StateOperationTarget.h"

using namespace AmbientCharacterBehavior;

bool FrameworkRegistry::RegisterSequences(const std::string &config_file_path)
{
    auto sequence_dtos = json_loader.ProcessSequencesConfigFile(config_file_path);
    if (sequence_dtos.empty())
    {
        logger.LogWarning("The configuration file did not contain any valid sequences.",
            "FrameworkRegistry");

        return false;
    }

    for (const auto &sequence_dto : sequence_dtos)
    {
        if (!GenerateSequenceFromDto(sequence_dto))
        {
            return false;
        }
    }

    logger.LogInfo("Registered " + std::to_string(sequence_dtos.size()) + " sequences.",
        "FrameworkRegistry");

    return true;
}

bool FrameworkRegistry::GenerateSequenceFromDto(const SequenceDto &sequence_dto)
{
    try
    {
        auto [new_sequence_iterator, inserted] = sequences.emplace(sequence_dto.sequence_id,
            std::make_shared<Sequence>(Sequence(sequence_dto.sequence_id, sequence_dto.sequence_name)));

        if (!inserted)
        {
            logger.LogWarning("Sequence '" + sequence_dto.sequence_name + " ' was not added to the registry.",
                "FrameworkRegistry");

            return false;
        }

        return ConfigureSequenceWithDto(new_sequence_iterator->second, sequence_dto);
    }
    catch (const std::exception &e)
    {
        logger.LogError("Error while generating the sequence '" + sequence_dto.sequence_name + "', " +
            e.what(), "FrameworkRegistry");

        return true;
    }
}

bool FrameworkRegistry::ConfigureSequenceWithDto(const std::shared_ptr<Sequence> &new_sequence,
    const SequenceDto &sequence_dto) const
{
    for (const auto &dto_node : sequence_dto.nodes)
    {
        if (!GenerateSequenceNodeFromDto(new_sequence, dto_node))
        {
            return false;
        }
    }

    for (const auto &dto_transition : sequence_dto.transitions)
    {
        if (!GenerateTransitionFromDto(new_sequence, dto_transition))
        {
            return false;
        }
    }

    if (!new_sequence->TrySetEntryPoint(sequence_dto.entry_point_node_id))
    {
        logger.LogWarning("The entry point node id for sequence '" + sequence_dto.sequence_name +
            "' was not set. Value: " + std::to_string(sequence_dto.entry_point_node_id),
            "FrameworkRegistry");

        return false;
    }

    logger.LogInfo("Sequence '" + sequence_dto.sequence_name + " ' has been configured.",
            "FrameworkRegistry");

    return true;
}

bool FrameworkRegistry::GenerateSequenceNodeFromDto(const std::shared_ptr<Sequence> &new_sequence,
    const SequenceNodeDto &dto_node) const
{
    try
    {
        if (dto_node.node_type == "ACTION")
        {
            new_sequence->AddActionSequenceNode(dto_node.node_id, dto_node.target_action_id.value());
        }
        else if (dto_node.node_type == "SEQUENCE")
        {
            new_sequence->AddNestedSequenceNode(dto_node.node_id, dto_node.target_sequence_id.value());
        }
        else if (dto_node.node_type == "END") {
            new_sequence->AddEndSequenceNode(dto_node.node_id);
        }
        else
        {
            logger.LogError("Unknown node type '" + dto_node.node_type + "' for node " +
                            std::to_string(dto_node.node_id), "FrameworkRegistry");

            return false;
        }
    }
    catch (const std::exception &e)
    {
        logger.LogError("Error while generating node " + std::to_string(dto_node.node_id) +
            " for the sequence '" + new_sequence->GetSequenceName() + "', " + e.what(),
            "FrameworkRegistry");

        return false;
    }

    return true;
}

bool FrameworkRegistry::GenerateTransitionFromDto(const std::shared_ptr<Sequence> &new_sequence,
    const TransitionDto &dto_transition) const
{
    auto preconditions = GenerateStateOperationVectorFromDto(dto_transition.preconditions);

    if (!new_sequence->TryAddTransition(dto_transition.transition_id, dto_transition.from_node_id,
        dto_transition.to_node_id, preconditions))
    {
        logger.LogError("Transition " + std::to_string(dto_transition.transition_id) +
            " was not able to be added to the sequence '" + new_sequence->GetSequenceName(),
            "FrameworkRegistry");

        return false;
    }

    return true;
}

std::vector<StateOperation> FrameworkRegistry::GenerateStateOperationVectorFromDto(
    const std::vector<StateOperationDto> &dto_state_operations) const
{
    std::vector<StateOperation> state_operations;
    state_operations.reserve(dto_state_operations.size());
    for (const auto & dto_state_operation : dto_state_operations)
    {
        state_operations.emplace_back(GenerateStateOperationFromDto(dto_state_operation));
    }

    return state_operations;
}

StateOperation FrameworkRegistry::GenerateStateOperationFromDto(const StateOperationDto &dto_state_operation) const
{
    StateOperationTarget target_id;
    int32_t state_key;
    if (dto_state_operation.target_id_name == "ENVIRONMENT")
    {
        target_id = StateOperationTarget::ENVIRONMENT;
        state_key = environment_manager.GetEnvironmentalConditionKey(dto_state_operation.state_key_name);
    }
    else if (dto_state_operation.target_id_name == "SELF")
    {
        target_id = StateOperationTarget::SELF;
        state_key = schema_manager.GetStateKey(dto_state_operation.state_key_name);
    }
    else
    {
        // Assume that all other names will reference ENTITY
        target_id = StateOperationTarget::ENTITY;
        state_key = schema_manager.GetStateKey(dto_state_operation.state_key_name);
    }

    auto operation_type = schema_manager.GetStateOperationTypeId(dto_state_operation.operation_name);

    return StateOperation(target_id, state_key, operation_type, dto_state_operation.parameters);
}

bool FrameworkRegistry::RegisterActions(const std::string &config_file_path)
{
    auto action_dtos = json_loader.ProcessActionsConfigFile(config_file_path);
    if (action_dtos.empty())
    {
        logger.LogWarning("The configuration file did not contain any valid actions.",
            "FrameworkRegistry");

        return false;
    }

    for (const auto &action_dto : action_dtos)
    {
        if (!GenerateActionFromDto(action_dto))
        {
            return false;
        }
    }

    logger.LogInfo("Registered " + std::to_string(action_dtos.size()) + " actions.", "FrameworkRegistry");

    return true;
}

bool FrameworkRegistry::GenerateActionFromDto(const ActionDto &action_dto)
{
    try
    {
        auto interruption_behavior = ParseInterruptionBehavior(action_dto.interruption_behavior_name);

        auto [new_action_iterator, inserted] = actions.emplace(action_dto.action_id, std::make_shared<Action>(
            Action(action_dto.action_id, action_dto.action_name, action_dto.requires_target_entity, action_dto.max_duration_ms,
                interruption_behavior)));

        if (!inserted)
        {
            logger.LogWarning("Action '" + action_dto.action_name + " ' was not added to the registry.",
                "FrameworkRegistry");

            return false;
        }

        return ConfigureActionWithDto(new_action_iterator->second, action_dto);
    }
    catch (const std::exception &e)
    {
        logger.LogError("Error while generating the action '" + action_dto.action_name + "', " +
            e.what(), "FrameworkRegistry");

        return false;
    }
}

InterruptionBehaviorType FrameworkRegistry::ParseInterruptionBehavior(const std::string& behavior_name) const {
    if (behavior_name == "RESUMABLE") {
        return InterruptionBehaviorType::RESUMABLE;
    }
    if (behavior_name == "NON_RESUMABLE") {
        return InterruptionBehaviorType::NON_RESUMABLE;
    }
    throw std::invalid_argument("Unknown interruption behavior: " + behavior_name);
}

bool FrameworkRegistry::ConfigureActionWithDto(const std::shared_ptr<Action> &new_action,
    const ActionDto &action_dto) const
{
    for (const auto& precondition_dto : action_dto.preconditions)
    {
        new_action->AddPrecondition(GenerateStateOperationFromDto(precondition_dto));
    }

    for (const auto& immediate_effect_dto : action_dto.immediate_effects)
    {
        new_action->AddImmediateEffect(GenerateStateOperationFromDto(immediate_effect_dto));
    }

    for (const auto& completion_effect_dto : action_dto.completion_effects)
    {
        new_action->AddCompletionEffect(GenerateStateOperationFromDto(completion_effect_dto));
    }

    logger.LogInfo("Action '" + action_dto.action_name + " ' has been configured.",
            "FrameworkRegistry");

    return true;
}

void FrameworkRegistry::QueueEntityRegistration(void *handle, const std::string &path)
{
    EntityCommand command {
    EntityCommandType::REGISTER,
    handle,
    path,
    };

    pending_commands.push(command);
}

void FrameworkRegistry::QueueEntityUnregistration(void *handle)
{
    EntityCommand command {
        EntityCommandType::UNREGISTER,
        handle,
    };

    pending_commands.push(command);
}

size_t FrameworkRegistry::ProcessPendingEntityCommands()
{
    size_t processed = 0;

    while (!pending_commands.empty())
    {
        auto command = pending_commands.front();
        pending_commands.pop();

        try
        {
            if (command.type == EntityCommandType::REGISTER)
            {
                RegisterEntity(command.entity_handle, command.config_path);
            }
            else
            {
                UnregisterEntity(command.entity_handle);
            }
            processed++;
        }
        catch (const std::exception &e)
        {
            logger.LogError(
                    "Failed to process entity command: " + std::string(e.what()),
                    "FrameworkRegistry");
        }
    }

    return processed;
}

size_t FrameworkRegistry::GetPendingCommandCount() const
{
    return pending_commands.size();
}

void FrameworkRegistry::ClearPendingCommands()
{
    while (!pending_commands.empty())
    {
        pending_commands.pop();
    }
}

void FrameworkRegistry::RegisterEntity(void *entity_handle, const std::string &config_file_path)
{
    auto entity_dto = json_loader.ProcessSingleEntityConfigFile(config_file_path);
    if (!entity_dto.has_value())
    {
        logger.LogWarning("The configuration file did not contain any valid entity.",
            "FrameworkRegistry");

        return;
    }

    if (entity_dto->entity_type == "FRAMEWORK")
    {
        auto entity = GenerateFrameworkEntityFromDto(entity_handle, entity_dto->framework_entity);
        GenerateFrameworkEntityIdAndHandleMapping(entity);

        logger.LogInfo("Registered Framework Entity: " + entity_dto->framework_entity->entity_name,
            "FrameworkRegistry");
    }
    else if (entity_dto->entity_type == "BEHAVIORAL")
    {
        auto entity = GenerateBehavioralEntityFromDto(entity_handle, entity_dto->behavioral_entity);
        GenerateBehavioralEntityIdAndHandleMapping(entity);

        logger.LogInfo("Registered Behavioral Entity: " + entity_dto->behavioral_entity->base_properties.entity_name,
            "FrameworkRegistry");
    }
    else
    {
        logger.LogWarning("The entity type '" + entity_dto->entity_type + " does not exist. The entity was not registered.",
            "FrameworkRegistry");
    }
}

FrameworkEntity * FrameworkRegistry::GenerateFrameworkEntityFromDto(void *entity_handle,
    std::optional<FrameworkEntityDto> entity_dto)
{
    if (!entity_dto.has_value())
    {
        logger.LogWarning("The entity dto is empty. The entity cannot be generated",
            "FrameworkRegistry");

        return nullptr;
    }

    try
    {
        if (IsEntityDuplicate(entity_handle, entity_dto->entity_id))
        {
            return nullptr;
        }

        auto [new_entity_iterator, inserted] = framework_entities.emplace(entity_dto->entity_id,
            std::make_unique<FrameworkEntity>(FrameworkEntity(entity_handle, entity_dto->entity_id, entity_dto->entity_name)));

        if (!inserted)
        {
            logger.LogWarning("Entity '" + entity_dto->entity_name + " ' was not added to the registry.",
                "FrameworkRegistry");

            return nullptr;
        }

        ConfigureFrameworkEntityWithDto(new_entity_iterator->second, entity_dto.value());

        return new_entity_iterator->second.get();
    }
    catch (const std::exception &e)
    {
        logger.LogError("Error while generating the entity '" + entity_dto->entity_name + "', " +
            e.what(), "FrameworkRegistry");

        return nullptr;
    }
}

bool FrameworkRegistry::IsEntityDuplicate(void* entity_handle, int32_t entity_id) const
{
    return (
        GetFrameworkIdFromHandle(entity_handle) >= 0 ||
        GetBehavioralIdFromHandle(entity_handle) >= 0 ||
        GetHandleFromFrameworkId(entity_id) ||
        GetHandleFromBehavioralId(entity_id)
    );
}

void FrameworkRegistry::GenerateFrameworkEntityIdAndHandleMapping(const FrameworkEntity *framework_entity)
{
    if (!framework_entity)
    {
        logger.LogWarning("The framework entity was not generated. The mapping cannot be done.",
            "FrameworkRegistry");

        return;
    }

    handle_to_framework_id[framework_entity->GetEntityHandle()] = framework_entity->GetEntityId();
    framework_id_to_handle[framework_entity->GetEntityId()] = framework_entity->GetEntityHandle();
}

void FrameworkRegistry::ConfigureFrameworkEntityWithDto(const std::unique_ptr<FrameworkEntity> &new_entity,
    const FrameworkEntityDto &entity_dto) const
{
    AddAcceptedActionsToEntity(entity_dto.accepted_actions_ids, new_entity);
    AddInitialStateMapToEntity(entity_dto.initial_state, new_entity);
}

BehavioralEntity * FrameworkRegistry::GenerateBehavioralEntityFromDto(void *entity_handle,
                                                                      std::optional<BehavioralEntityDto> entity_dto)
{
    if (!entity_dto.has_value())
    {
        logger.LogWarning("The entity dto is empty. The entity cannot be generated",
            "FrameworkRegistry");

        return nullptr;
    }

    try
    {
        if (IsEntityDuplicate(entity_handle, entity_dto->base_properties.entity_id))
        {
            return nullptr;
        }

        auto [new_entity_iterator, inserted] = behavioral_entities.emplace(entity_dto->base_properties.entity_id,
            std::make_unique<BehavioralEntity>(BehavioralEntity(logger, time_manager, start_action_provider,
                *this, *this, state_operation_evaluator,
                entity_handle,
                entity_dto->base_properties.entity_id, entity_dto->memory_limits.max_transition_memories,
                entity_dto->memory_limits.max_action_memories, entity_dto->memory_limits.max_interruption_memories,
                entity_dto->base_properties.entity_name)));

        if (!inserted)
        {
            logger.LogWarning("Entity '" + entity_dto->base_properties.entity_name + " ' was not added to the registry.",
                "FrameworkRegistry");

            return nullptr;
        }

        ConfigureBehavioralEntityWithDto(new_entity_iterator->second, entity_dto.value());

        return new_entity_iterator->second.get();
    }
    catch (const std::exception &e)
    {
        logger.LogError("Error while generating the entity '" + entity_dto->base_properties.entity_name + "', " +
            e.what(), "FrameworkRegistry");

        return nullptr;
    }
}

void FrameworkRegistry::GenerateBehavioralEntityIdAndHandleMapping(const BehavioralEntity *framework_entity)
{
    if (!framework_entity)
    {
        logger.LogWarning("The framework entity was not generated. The mapping cannot be done.",
            "FrameworkRegistry");

        return;
    }

    handle_to_behavioral_id[framework_entity->GetEntityHandle()] = framework_entity->GetEntityId();
    behavioral_id_to_handle[framework_entity->GetEntityId()] = framework_entity->GetEntityHandle();
}

void FrameworkRegistry::ConfigureBehavioralEntityWithDto(const std::unique_ptr<BehavioralEntity> &new_entity,
    const BehavioralEntityDto &entity_dto) const
{
    AddAcceptedActionsToEntity(entity_dto.base_properties.accepted_actions_ids, new_entity);
    AddInitialStateMapToEntity(entity_dto.base_properties.initial_state, new_entity);

    new_entity->SetMainSequence(GetSequenceById(entity_dto.main_sequence_id));
    AddFallbackSequencesToEntity(entity_dto.fallback_sequences, new_entity);
    AddInterruptionHandlersToEntity(entity_dto.interruption_handlers, new_entity);
}

void FrameworkRegistry::AddFallbackSequencesToEntity(const std::vector<int32_t> &fallback_sequences,
    const std::unique_ptr<BehavioralEntity> &new_entity) const
{
    for (const auto& fallback_sequence_id : fallback_sequences)
    {
        new_entity->AddFallbackSequence(GetSequenceById(fallback_sequence_id));
    }
}

void FrameworkRegistry::AddInterruptionHandlersToEntity(const std::unordered_map<std::string, int32_t> &interruption_handlers,
    const std::unique_ptr<BehavioralEntity> &new_entity) const
{
    for (const auto& interruption_handler_pair : interruption_handlers)
    {
        try
        {
            auto interruption_key = schema_manager.GetInterruptionKey(interruption_handler_pair.first);
            new_entity->AddInterruptionHandler(interruption_key, GetSequenceById(interruption_handler_pair.second));
        }
        catch (const std::exception &e)
        {
            logger.LogWarning("Interruption id '" + interruption_handler_pair.first + "' does not exist.",
                "FrameworkRegistry");
        }
    }
}

void FrameworkRegistry::UnregisterEntity(void *entity_handle)
{
    if (!entity_handle)
    {
        logger.LogWarning("Cannot unregister entity with null handle", "FrameworkRegistry");
        return;
    }

    if (UnregisterFrameworkEntity(entity_handle))
    {
        return;
    }

    if (UnregisterBehavioralEntity(entity_handle))
    {
        return;
    }

    logger.LogWarning("Entity with passed handle does not exist", "FrameworkRegistry");
}

bool FrameworkRegistry::UnregisterFrameworkEntity(void* entity_handle)
{
    auto framework_id = GetFrameworkIdFromHandle(entity_handle);
    if (framework_id != -1) {
        handle_to_framework_id.erase(entity_handle);
        framework_id_to_handle.erase(framework_id);
        framework_entities.erase(framework_id);
        logger.LogInfo("Successfully unregistered framework entity with ID: " + std::to_string(framework_id), "FrameworkRegistry");
        return true;
    }

    return false;
}

bool FrameworkRegistry::UnregisterBehavioralEntity(void* entity_handle)
{
    auto behavioral_id = GetBehavioralIdFromHandle(entity_handle);
    if (behavioral_id != -1) {
        handle_to_behavioral_id.erase(entity_handle);
        behavioral_id_to_handle.erase(behavioral_id);
        behavioral_entities.erase(behavioral_id);
        logger.LogInfo("Successfully unregistered behavioral entity with ID: " + std::to_string(behavioral_id), "FrameworkRegistry");
        return true;
    }

    return false;
}

bool FrameworkRegistry::HasSequence(int32_t sequence_id) const
{
    return sequences.find(sequence_id) != sequences.end();
}

std::shared_ptr<Sequence> FrameworkRegistry::GetSequenceById(int32_t sequence_id) const
{
    if (!HasSequence(sequence_id))
    {
        logger.LogWarning("Sequence with id: " + std::to_string(sequence_id) + " is not in the registry",
            "FrameworkRegistry");
        return nullptr;
    }

    return sequences.at(sequence_id);
}

bool FrameworkRegistry::HasAction(int32_t action_id) const
{
    return actions.find(action_id) != actions.end();
}

std::shared_ptr<Action> FrameworkRegistry::GetActionById(int32_t action_id) const
{
    if (!HasAction(action_id))
    {
        logger.LogWarning("Action with id: " + std::to_string(action_id) + " is not in the registry",
            "FrameworkRegistry");

        return nullptr;
    }

    return actions.at(action_id);
}

bool FrameworkRegistry::HasFrameworkEntity(int32_t entity_id) const
{
    return framework_entities.find(entity_id) != framework_entities.end();
}

FrameworkEntity* FrameworkRegistry::GetFrameworkEntityById(int32_t entity_id) const
{
    if (!HasFrameworkEntity(entity_id))
    {
        logger.LogWarning("Entity with id: " + std::to_string(entity_id) + " is not in the registry",
            "FrameworkRegistry");

        return nullptr;
    }

    return framework_entities.at(entity_id).get();
}

void* FrameworkRegistry::GetHandleFromFrameworkId(int32_t entity_id) const
{
    auto iterator = framework_id_to_handle.find(entity_id);
    return iterator != framework_id_to_handle.end() ? iterator->second : nullptr;
}

int32_t FrameworkRegistry::GetFrameworkIdFromHandle(void *entity_handle) const
{
    auto iterator = handle_to_framework_id.find(entity_handle);
    // -1 is an invalid id
    return iterator != handle_to_framework_id.end() ? iterator->second : -1;
}

bool FrameworkRegistry::HasBehavioralEntity(int32_t entity_id) const
{
    return behavioral_entities.find(entity_id) != behavioral_entities.end();
}

BehavioralEntity * FrameworkRegistry::GetBehavioralEntityById(int32_t entity_id) const
{
    if (!HasBehavioralEntity(entity_id))
    {
        logger.LogWarning("Entity with id: " + std::to_string(entity_id) + " is not in the registry",
            "FrameworkRegistry");

        return nullptr;
    }

    return behavioral_entities.at(entity_id).get();
}

BehavioralEntity * FrameworkRegistry::GetBehavioralEntityByHandle(void *entity_handle) const
{
    auto entity_id = GetBehavioralIdFromHandle(entity_handle);
    if (entity_id == -1)
    {
        logger.LogWarning("Entity with the provided handle is not in the registry",
            "FrameworkRegistry");

        return nullptr;
    }
    return GetBehavioralEntityById(entity_id);
}

void * FrameworkRegistry::GetHandleFromBehavioralId(int32_t entity_id) const
{
    auto iterator = behavioral_id_to_handle.find(entity_id);
    return iterator != behavioral_id_to_handle.end() ? iterator->second : nullptr;
}


int32_t FrameworkRegistry::GetBehavioralIdFromHandle(void *entity_handle) const
{
    auto iterator = handle_to_behavioral_id.find(entity_handle);
    // -1 is an invalid id
    return iterator != handle_to_behavioral_id.end() ? iterator->second : -1;
}

std::vector<BehavioralEntity *> FrameworkRegistry::GetBehavioralEntitiesRange(int32_t start_index, int32_t count) const
{
    std::vector<BehavioralEntity *> result;
    result.reserve(count);

    auto iterator = behavioral_entities.begin();
    std::advance(iterator, start_index);

    for (int32_t i = 0; i < count && iterator != behavioral_entities.end(); ++i, ++iterator)
    {
        BehavioralEntity* entity = iterator->second.get();
        if (entity && entity->CanUpdate())
        {
            result.emplace_back(entity);
        }
    }

    return result;
}

std::vector<FrameworkEntity *> FrameworkRegistry::GetEntitiesSupportingAction(int32_t action_id) const
{
    std::vector<FrameworkEntity *> result;

    for (const auto& [id, entity]: framework_entities)
    {
        if (entity->SupportsAction(action_id))
        {
            result.push_back(entity.get());
        }
    }

    for (const auto& [id, entity]: behavioral_entities)
    {
        if (entity->SupportsAction(action_id))
        {
            result.push_back(entity.get());
        }
    }

    logger.LogInfo("Found " + std::to_string(result.size()) + " entities that support action " + std::to_string(action_id),
        "FrameworkRegistry");

    return result;
}

FrameworkEntity * FrameworkRegistry::GetEntityFromId(int32_t entity_id) const
{
    FrameworkEntity* result = nullptr;

    // Try to get framework entity first
    result = GetFrameworkEntityById(entity_id);
    if (!result)
    {
        // Try behavioral entity if not found
        result = GetBehavioralEntityById(entity_id);
    }

    return result;
}
