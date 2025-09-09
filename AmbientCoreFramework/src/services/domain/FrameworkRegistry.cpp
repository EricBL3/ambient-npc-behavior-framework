#include "FrameworkRegistry.h"

#include <memory>

using namespace AmbientCharacterBehavior;

void FrameworkRegistry::RegisterSequences(const std::string &config_file_path)
{
    auto sequence_dtos = json_loader.ProcessSequencesConfigFile(config_file_path);
    if (sequence_dtos.empty())
    {
        logger.LogWarning("The configuration file did not contain any valid sequences.",
            "FrameworkRegistry");

        return;
    }

    for (const auto &sequence_dto : sequence_dtos)
    {
        GenerateSequenceFromDto(sequence_dto);
    }
}

void FrameworkRegistry::GenerateSequenceFromDto(const SequenceDto &sequence_dto)
{
    try
        {
            auto [new_sequence_iterator, inserted] = sequences.emplace(sequence_dto.sequence_id,
                std::make_shared<Sequence>(Sequence(sequence_dto.sequence_id, sequence_dto.sequence_name)));

            if (!inserted)
            {
                logger.LogWarning("Sequence '" + sequence_dto.sequence_name + " ' was not added to the registry.",
                    "FrameworkRegistry");

                return;
            }

            ConfigureSequenceWithDto(new_sequence_iterator->second, sequence_dto);
        }
        catch (const std::exception &e)
        {
            logger.LogError("Error while generating the sequence '" + sequence_dto.sequence_name + "', " +
                e.what(), "FrameworkRegistry");
        }
}

void FrameworkRegistry::ConfigureSequenceWithDto(const std::shared_ptr<Sequence> &new_sequence,
    const SequenceDto &sequence_dto) const
{
    for (const auto &dto_node : sequence_dto.nodes)
    {
        GenerateSequenceNodeFromDto(new_sequence, dto_node);
    }

    for (const auto &dto_transition : sequence_dto.transitions)
    {
        GenerateTransitionFromDto(new_sequence, dto_transition);
    }

    if (!new_sequence->TrySetEntryPoint(sequence_dto.entry_point_node_id))
    {
        logger.LogWarning("The entry point node id for sequence '" + sequence_dto.sequence_name +
            "' was not set. Value: " + std::to_string(sequence_dto.entry_point_node_id),
            "FrameworkRegistry");
    }

    logger.LogInfo("Sequence '" + sequence_dto.sequence_name + " ' has been configured.",
            "FrameworkRegistry");
}

void FrameworkRegistry::GenerateSequenceNodeFromDto(const std::shared_ptr<Sequence> &new_sequence,
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
        }
    }
    catch (const std::exception &e)
    {
        logger.LogError("Error while generating node " + std::to_string(dto_node.node_id) +
            " for the sequence '" + new_sequence->GetSequenceName() + "', " + e.what(),
            "FrameworkRegistry");
    }
}

void FrameworkRegistry::GenerateTransitionFromDto(const std::shared_ptr<Sequence> &new_sequence,
    const TransitionDto &dto_transition) const
{
    auto preconditions = GenerateStateOperationVectorFromDto(dto_transition.preconditions);

    if (!new_sequence->TryAddTransition(dto_transition.transition_id, dto_transition.from_node_id,
        dto_transition.to_node_id, preconditions))
    {
        logger.LogError("Transition " + std::to_string(dto_transition.transition_id) +
            " was not able to be added to the sequence '" + new_sequence->GetSequenceName(), "FrameworkRegistry");
    }
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
    int32_t target_id;
    int32_t state_key;
    if (dto_state_operation.target_id_name == "ENVIRONMENT")
    {
        target_id = -2;
        //TODO: get state key from environmental manager or from schema
        state_key = -1; //environment_manager.GetStateKey(dto_state_operation.state_key_name);
    }
    else if (dto_state_operation.target_id_name == "SELF")
    {
        target_id = -1;
        state_key = state_schema.GetStateKey(dto_state_operation.state_key_name);
    }
    else
    {
        // Assume that all other names will reference ENTITY
        target_id = 0;
        state_key = state_schema.GetStateKey(dto_state_operation.state_key_name);
    }

    //todo: This is hardcoded and incomplete. Look for a different way to handle this.
    // Map operation name to value
    int32_t operation;
    if (dto_state_operation.operation_name == "EQUALS")
    {
        operation = 0;
    }
    else if (dto_state_operation.operation_name == "GREATER_THAN")
    {
        operation = 1;
    }
    else if (dto_state_operation.operation_name == "LESS_THAN")
    {
        operation = 2;
    }
    else
    {
        operation = 3;
    }

    return StateOperation(target_id, state_key, operation, dto_state_operation.parameters);
}

void FrameworkRegistry::RegisterActions(const std::string &config_file_path)
{
}

void FrameworkRegistry::RegisterEntity(void *entity_handle, const std::string &config_file_path)
{
}

void FrameworkRegistry::UnregisterEntity(void *entity_handle)
{
}

bool FrameworkRegistry::HasSequence(int32_t sequence_id) const
{
    return sequences.find(sequence_id) != sequences.end();
}

std::shared_ptr<Sequence> FrameworkRegistry::GetSequenceById(int32_t sequence_id) const
{
    if (!HasSequence(sequence_id))
    {
        logger.LogWarning("Sequence with id: " + std::to_string(sequence_id) + "is not in the registry",
            "FrameworkRegistry");
        return nullptr;
    }

    return sequences.at(sequence_id);
}
