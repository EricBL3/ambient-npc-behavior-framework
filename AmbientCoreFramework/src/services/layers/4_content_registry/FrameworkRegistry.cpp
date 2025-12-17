#include "./FrameworkRegistry.h"

#include <memory>

#include "behavior/enums/StateOperationTarget.h"

using namespace AmbientCharacterBehavior;

FrameworkRegistry::FrameworkRegistry(BehavioralEvaluationServices &services) : services(services), self_bundle(nullptr) {}

void FrameworkRegistry::SetSelfBundle(ContentRegistryServices &bundle)
{
    self_bundle = &bundle;
}

bool FrameworkRegistry::RegisterSequences(const std::string &config_file_path)
{
    auto sequence_dtos = JsonLoader().ProcessSequencesConfigFile(config_file_path);
    if (sequence_dtos.empty())
    {
        Logger().LogWarning("The configuration file did not contain any valid sequences.",
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

    Logger().LogInfo("Registered " + std::to_string(sequence_dtos.size()) + " sequences.",
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
            Logger().LogWarning("Sequence '" + sequence_dto.sequence_name + " ' was not added to the registry.",
                "FrameworkRegistry");

            return false;
        }

        return ConfigureSequenceWithDto(new_sequence_iterator->second, sequence_dto);
    }
    catch (const std::exception &e)
    {
        Logger().LogError("Error while generating the sequence '" + sequence_dto.sequence_name + "', " +
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
        Logger().LogWarning("The entry point node id for sequence '" + sequence_dto.sequence_name +
            "' was not set. Value: " + std::to_string(sequence_dto.entry_point_node_id),
            "FrameworkRegistry");

        return false;
    }

    Logger().LogInfo("Sequence '" + sequence_dto.sequence_name + " ' has been configured.",
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
            Logger().LogError("Unknown node type '" + dto_node.node_type + "' for node " +
                            std::to_string(dto_node.node_id), "FrameworkRegistry");

            return false;
        }
    }
    catch (const std::exception &e)
    {
        Logger().LogError("Error while generating node " + std::to_string(dto_node.node_id) +
            " for the sequence '" + new_sequence->GetSequenceName() + "', " + e.what(),
            "FrameworkRegistry");

        return false;
    }

    return true;
}

bool FrameworkRegistry::GenerateTransitionFromDto(const std::shared_ptr<Sequence> &new_sequence,
    const TransitionDto &dto_transition) const
{
    auto preconditions = GenerateStateOperationHashTableFromDto(dto_transition.preconditions);

    if (!new_sequence->TryAddTransition(dto_transition.transition_id, dto_transition.from_node_id,
        dto_transition.to_node_id, preconditions))
    {
        Logger().LogError("Transition " + std::to_string(dto_transition.transition_id) +
            " was not able to be added to the sequence '" + new_sequence->GetSequenceName(),
            "FrameworkRegistry");

        return false;
    }

    return true;
}

std::unordered_map<StateOperationTarget, std::vector<StateOperation>> FrameworkRegistry::GenerateStateOperationHashTableFromDto(
    const std::vector<StateOperationDto> &dto_state_operations) const
{
    std::unordered_map<StateOperationTarget, std::vector<StateOperation>> state_operations;
    for (const auto & dto_state_operation : dto_state_operations)
    {
        auto target = ParseStateOperationTargetName(dto_state_operation.target_id_name);

        if (target.has_value())
        {
            state_operations[target.value()].emplace_back(GenerateStateOperationFromDto(target.value(), dto_state_operation));
        }
        else
        {
            Logger().LogWarning("State operation target " + dto_state_operation.target_id_name + " does not exist. "
                "The precondition will be skipped.", "GenerateStateOperationHashTableFromDto");
        }
    }

    return state_operations;
}

std::optional<StateOperationTarget> FrameworkRegistry::ParseStateOperationTargetName(const std::string &target_name) const
{
    if (target_name == "ENVIRONMENT")
    {
        return StateOperationTarget::ENVIRONMENT;
    }

    if (target_name == "SELF")
    {
        return StateOperationTarget::SELF;
    }

    if (target_name == "ENTITY")
    {
        return StateOperationTarget::ENTITY;
    }

    if (target_name == "DISTANCE_TO_ENTITY")
    {
        return StateOperationTarget::DISTANCE_TO_ENTITY;
    }


    return std::nullopt;
}

StateOperation FrameworkRegistry::GenerateStateOperationFromDto(StateOperationTarget target, const StateOperationDto &dto_state_operation) const
{
    StateOperationTarget target_id;
    int32_t state_key;
    switch (target)
    {
        case StateOperationTarget::ENVIRONMENT:
            state_key = EnvironmentManager().GetEnvironmentalConditionKey(dto_state_operation.state_key_name);
            break;
        case StateOperationTarget::SELF:
        case StateOperationTarget::ENTITY:
            state_key = SchemaManager().GetStateKey(dto_state_operation.state_key_name);
            break;
        default:
            state_key = 0;
    }

    auto operation_type = SchemaManager().GetStateOperationTypeId(dto_state_operation.operation_name);

    return StateOperation(target, state_key, operation_type, dto_state_operation.value);
}

bool FrameworkRegistry::RegisterActions(const std::string &config_file_path)
{
    auto action_dtos = JsonLoader().ProcessActionsConfigFile(config_file_path);
    if (action_dtos.empty())
    {
        Logger().LogWarning("The configuration file did not contain any valid actions.",
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

    Logger().LogInfo("Registered " + std::to_string(action_dtos.size()) + " actions.", "FrameworkRegistry");

    return true;
}

bool FrameworkRegistry::GenerateActionFromDto(const ActionDto &action_dto)
{
    try
    {
        auto interruption_behavior = ParseInterruptionBehavior(action_dto.interruption_behavior_name);

        auto [new_action_iterator, inserted] = actions.emplace(action_dto.action_id, std::make_shared<Action>(
            Action(action_dto.action_id, action_dto.action_name, action_dto.action_duration_ms,
                interruption_behavior)));

        if (!inserted)
        {
            Logger().LogWarning("Action '" + action_dto.action_name + " ' was not added to the registry.",
                "FrameworkRegistry");

            return false;
        }

        return ConfigureActionWithDto(new_action_iterator->second, action_dto);
    }
    catch (const std::exception &e)
    {
        Logger().LogError("Error while generating the action '" + action_dto.action_name + "', " +
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
        auto target = ParseStateOperationTargetName(precondition_dto.target_id_name);

        if (target.has_value())
        {
            new_action->AddPrecondition(target.value(), GenerateStateOperationFromDto(target.value(),
                precondition_dto));
        }
        else
        {
            Logger().LogWarning("State operation target " + precondition_dto.target_id_name + " does not exist. "
                "The precondition will be skipped.", "ConfigureActionWithDto");
        }
    }

    for (const auto& immediate_effect_dto : action_dto.immediate_effects)
    {
        auto target = ParseStateOperationTargetName(immediate_effect_dto.target_id_name);

        if (target.has_value())
        {
            new_action->AddImmediateEffect(GenerateStateOperationFromDto(target.value(), immediate_effect_dto));
        }
        else
        {
            Logger().LogWarning("State operation target " + immediate_effect_dto.target_id_name + " does not exist. "
                "The immediate effect will be skipped.", "ConfigureActionWithDto");
        }
    }

    for (const auto& completion_effect_dto : action_dto.completion_effects)
    {
        auto target = ParseStateOperationTargetName(completion_effect_dto.target_id_name);

        if (target.has_value())
        {
            new_action->AddCompletionEffect(GenerateStateOperationFromDto(target.value(), completion_effect_dto));
        }
        else
        {
            Logger().LogWarning("State operation target " + completion_effect_dto.target_id_name + " does not exist. "
                "The completion effect will be skipped.", "ConfigureActionWithDto");
        }
    }

    for (const auto& interruption_effect_dto : action_dto.interruption_effects)
    {
        auto target = ParseStateOperationTargetName(interruption_effect_dto.target_id_name);

        if (target.has_value())
        {
            new_action->AddInterruptionEffect(GenerateStateOperationFromDto(target.value(), interruption_effect_dto));
        }
        else
        {
            Logger().LogWarning("State operation target " + interruption_effect_dto.target_id_name + " does not exist. "
                "The interruption effect will be skipped.", "ConfigureActionWithDto");
        }
    }

    Logger().LogInfo("Action '" + action_dto.action_name + " ' has been configured.",
            "FrameworkRegistry");

    return true;
}

void FrameworkRegistry::QueueEntityRegistration(void *handle, const std::string &path, Position3D position)
{
    EntityCommand command {
        .type = EntityCommandType::REGISTER,
        .entity_handle = handle,
        .config_path = path,
        .position = position
    };

    pending_commands.push(command);
}

void FrameworkRegistry::QueueEntityUnregistration(void *handle)
{
    EntityCommand command {
        .type = EntityCommandType::UNREGISTER,
        .entity_handle = handle,
    };

    pending_commands.push(command);
}

size_t FrameworkRegistry::ProcessPendingEntityCommands(int32_t batch_size)
{
    size_t processed = 0;

    auto commands_to_process = DetermineCommandBatchSize(batch_size);
    while (!pending_commands.empty() && processed < commands_to_process)
    {
        auto command = pending_commands.front();
        pending_commands.pop();
        processed++;

        try
        {
            if (command.type == EntityCommandType::REGISTER)
            {
                RegisterEntity(command.entity_handle, command.config_path, command.position);
            }
            else
            {
                UnregisterEntity(command.entity_handle);
            }
        }
        catch (const std::exception &e)
        {
            Logger().LogError(
                    "Failed to process entity command: " + std::string(e.what()),
                    "FrameworkRegistry");
        }
    }

    return processed;
}

int32_t FrameworkRegistry::DetermineCommandBatchSize(int32_t batch_size) const
{
    if (batch_size < 0)
    {
        return std::numeric_limits<int32_t>::max();
    }

    return batch_size;
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

void FrameworkRegistry::RegisterEntity(void *entity_handle, const std::string &config_file_path, Position3D position)
{
    auto entity_dto = JsonLoader().ProcessSingleEntityConfigFile(config_file_path);
    if (!entity_dto.has_value())
    {
        Logger().LogWarning("The configuration file did not contain any valid entity.",
            "FrameworkRegistry");

        return;
    }

    if (entity_dto->entity_type == "FRAMEWORK")
    {
        auto entity = GenerateFrameworkEntityFromDto(entity_handle, entity_dto->framework_entity);
        GenerateFrameworkEntityIdAndHandleMapping(entity);

        PositionManager().RegisterEntityPosition(entity_handle, position, entity_dto->framework_entity->is_static,
            entity_dto->framework_entity->position_update_frequency_ms);

        Logger().LogInfo("Registered Framework Entity: " + entity_dto->framework_entity->entity_name,
            "FrameworkRegistry");
    }
    else if (entity_dto->entity_type == "BEHAVIORAL")
    {
        auto entity = GenerateBehavioralEntityFromDto(entity_handle, entity_dto->behavioral_entity);
        GenerateBehavioralEntityIdAndHandleMapping(entity);

        PositionManager().RegisterEntityPosition(entity_handle, position,
            entity_dto->behavioral_entity->base_properties.is_static,
            entity_dto->behavioral_entity->base_properties.position_update_frequency_ms);

        Logger().LogInfo("Registered Behavioral Entity: " + entity_dto->behavioral_entity->base_properties.entity_name,
            "FrameworkRegistry");
    }
    else
    {
        Logger().LogWarning("The entity type '" + entity_dto->entity_type + " does not exist. The entity was not registered.",
            "FrameworkRegistry");
    }
}

FrameworkEntity * FrameworkRegistry::GenerateFrameworkEntityFromDto(void *entity_handle,
    std::optional<FrameworkEntityDto> entity_dto)
{
    if (!entity_dto.has_value())
    {
        Logger().LogWarning("The entity dto is empty. The entity cannot be generated",
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
            Logger().LogWarning("Entity '" + entity_dto->entity_name + " ' was not added to the registry.",
                "FrameworkRegistry");

            return nullptr;
        }

        ConfigureFrameworkEntityWithDto(new_entity_iterator->second, entity_dto.value());

        return new_entity_iterator->second.get();
    }
    catch (const std::exception &e)
    {
        Logger().LogError("Error while generating the entity '" + entity_dto->entity_name + "', " +
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
        Logger().LogWarning("The framework entity was not generated. The mapping cannot be done.",
            "FrameworkRegistry");

        return;
    }

    handle_to_framework_id[framework_entity->GetEntityHandle()] = framework_entity->GetEntityId();
    framework_id_to_handle[framework_entity->GetEntityId()] = framework_entity->GetEntityHandle();
}

void FrameworkRegistry::ConfigureFrameworkEntityWithDto(const std::unique_ptr<FrameworkEntity> &new_entity,
    const FrameworkEntityDto &entity_dto)
{
    RegisterActionsForEntity(entity_dto.accepted_actions_ids, new_entity);
    AddInitialStateMapToEntity(entity_dto.initial_state, new_entity);
}

BehavioralEntity * FrameworkRegistry::GenerateBehavioralEntityFromDto(void *entity_handle,
                                                                      std::optional<BehavioralEntityDto> entity_dto)
{
    if (!entity_dto.has_value())
    {
        Logger().LogWarning("The entity dto is empty. The entity cannot be generated",
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
            std::make_unique<BehavioralEntity>(BehavioralEntity(Logger(), TimeManager(), ActionProvider(),
                *this, *this, StateEvaluator(),
                entity_handle,
                entity_dto->base_properties.entity_id, entity_dto->memory_limits.max_transition_memories,
                entity_dto->memory_limits.max_action_memories, entity_dto->memory_limits.max_interruption_memories,
                entity_dto->base_properties.entity_name)));

        if (!inserted)
        {
            Logger().LogWarning("Entity '" + entity_dto->base_properties.entity_name + " ' was not added to the registry.",
                "FrameworkRegistry");

            return nullptr;
        }

        ConfigureBehavioralEntityWithDto(new_entity_iterator->second, entity_dto.value());

        return new_entity_iterator->second.get();
    }
    catch (const std::exception &e)
    {
        Logger().LogError("Error while generating the entity '" + entity_dto->base_properties.entity_name + "', " +
            e.what(), "FrameworkRegistry");

        return nullptr;
    }
}

void FrameworkRegistry::GenerateBehavioralEntityIdAndHandleMapping(const BehavioralEntity *behavioral_entity)
{
    if (!behavioral_entity)
    {
        Logger().LogWarning("The framework entity was not generated. The mapping cannot be done.",
            "FrameworkRegistry");

        return;
    }

    handle_to_behavioral_id[behavioral_entity->GetEntityHandle()] = behavioral_entity->GetEntityId();
    behavioral_id_to_handle[behavioral_entity->GetEntityId()] = behavioral_entity->GetEntityHandle();
}

void FrameworkRegistry::ConfigureBehavioralEntityWithDto(const std::unique_ptr<BehavioralEntity> &new_entity,
    const BehavioralEntityDto &entity_dto)
{
    RegisterActionsForEntity(entity_dto.base_properties.accepted_actions_ids, new_entity);
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
            auto interruption_key = SchemaManager().GetInterruptionKey(interruption_handler_pair.first);
            new_entity->AddInterruptionHandler(interruption_key, GetSequenceById(interruption_handler_pair.second));
        }
        catch (const std::exception &e)
        {
            Logger().LogWarning("Interruption id '" + interruption_handler_pair.first + "' does not exist.",
                "FrameworkRegistry");
        }
    }
}

void FrameworkRegistry::UnregisterEntity(void *entity_handle)
{
    if (!entity_handle)
    {
        Logger().LogWarning("Cannot unregister entity with null handle", "FrameworkRegistry");
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

    Logger().LogWarning("Entity with passed handle does not exist", "FrameworkRegistry");
}

bool FrameworkRegistry::UnregisterFrameworkEntity(void* entity_handle)
{
    auto framework_id = GetFrameworkIdFromHandle(entity_handle);
    if (framework_id != -1) {
        PositionManager().UnregisterEntityPosition(entity_handle);
        RemoveEntityFromActionIndex(framework_id);
        handle_to_framework_id.erase(entity_handle);
        framework_id_to_handle.erase(framework_id);
        framework_entities.erase(framework_id);
        Logger().LogInfo("Successfully unregistered framework entity with ID: " + std::to_string(framework_id), "FrameworkRegistry");
        return true;
    }

    return false;
}

bool FrameworkRegistry::UnregisterBehavioralEntity(void* entity_handle)
{
    auto behavioral_id = GetBehavioralIdFromHandle(entity_handle);
    if (behavioral_id != -1) {
        PositionManager().UnregisterEntityPosition(entity_handle);
        RemoveEntityFromActionIndex(behavioral_id);
        handle_to_behavioral_id.erase(entity_handle);
        behavioral_id_to_handle.erase(behavioral_id);
        behavioral_entities.erase(behavioral_id);
        Logger().LogInfo("Successfully unregistered behavioral entity with ID: " + std::to_string(behavioral_id), "FrameworkRegistry");
        return true;
    }

    return false;
}

void FrameworkRegistry::RemoveEntityFromActionIndex(int32_t entity_id)
{
    auto entity_actions_it = entity_to_actions_index.find(entity_id);
    if (entity_actions_it == entity_to_actions_index.end())
    {
        // early exit if entity id didn't support any action.
        return;
    }

    const auto& entity_actions = entity_actions_it->second;
    for (int32_t action_id : entity_actions)
    {
        auto action_entities_it = action_to_entities_index.find(action_id);
        if (action_entities_it != action_to_entities_index.end())
        {
            // erase entity reference from action-entity mapping
            action_entities_it->second.erase(entity_id);

            // if action bucket becomes empty, remove bucket
            if (action_entities_it->second.empty())
            {
                action_to_entities_index.erase(action_entities_it);
            }
        }
    }

    // erase entity reference from entity-action mapping
    entity_to_actions_index.erase(entity_actions_it);

    Logger().LogInfo("Removed entity " + std::to_string(entity_id) + " from action index",
    "RemoveEntityFromActionIndex");
}

bool FrameworkRegistry::HasSequence(int32_t sequence_id) const
{
    return sequences.find(sequence_id) != sequences.end();
}

std::shared_ptr<Sequence> FrameworkRegistry::GetSequenceById(int32_t sequence_id) const
{
    if (!HasSequence(sequence_id))
    {
        Logger().LogWarning("Sequence with id: " + std::to_string(sequence_id) + " is not in the registry",
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
        Logger().LogWarning("Action with id: " + std::to_string(action_id) + " is not in the registry",
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
        Logger().LogWarning("Entity with id: " + std::to_string(entity_id) + " is not registered as a framework entity",
            "GetFrameworkEntityById");

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
        Logger().LogWarning("Entity with id: " + std::to_string(entity_id) + " is not registered as a behavioral entity",
            "GetBehavioralEntityById");

        return nullptr;
    }

    return behavioral_entities.at(entity_id).get();
}

BehavioralEntity * FrameworkRegistry::GetBehavioralEntityByHandle(void *entity_handle) const
{
    auto entity_id = GetBehavioralIdFromHandle(entity_handle);
    if (entity_id == -1)
    {
        Logger().LogWarning("Entity with the provided handle is not in the registry",
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

    auto it = action_to_entities_index.find(action_id);
    if (it == action_to_entities_index.end())
    {
        Logger().LogInfo("Action " + std::to_string(action_id) + " not in index (no entities support it)",
        "GetEntitiesSupportingAction");
        return result;
    }

    const auto& entity_ids = it->second;
    result.reserve(entity_ids.size());

    for (int32_t entity_id : entity_ids)
    {
        FrameworkEntity* entity = GetEntityFromId(entity_id);
        if (entity)
        {
            result.push_back(entity);
        }
        else
        {
            Logger().LogWarning("Entity " + std::to_string(entity_id) + " listed in action " +
                std::to_string(action_id) + " index but not found in registry", "GetEntitiesSupportingAction");
        }
    }

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

bool FrameworkRegistry::EntitySupportsAction(int32_t entity_id, int32_t action_id) const
{
    auto iterator = entity_to_actions_index.find(entity_id);
    if (iterator == entity_to_actions_index.end())
    {
        return false;
    }

    return iterator->second.contains(action_id);
}
