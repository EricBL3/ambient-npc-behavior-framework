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
        try
        {
            // Create sequence in container
            auto [new_sequence_iterator, inserted] = sequences.emplace(sequence_dto.sequence_id,
                std::make_shared<Sequence>(Sequence(sequence_dto.sequence_id, sequence_dto.sequence_name)));

            if (!inserted)
            {
                logger.LogWarning("Sequence '" + sequence_dto.sequence_name + " ' was not added to the registry.",
                    "FrameworkRegistry");

                continue;
            }

            // Add nodes
            for (const auto &dto_node : sequence_dto.nodes)
            {
                try
                {
                    if (dto_node.node_type == "ACTION")
                    {
                        new_sequence_iterator->second->AddActionSequenceNode(dto_node.node_id, dto_node.target_action_id.value());
                    }
                    else if (dto_node.node_type == "SEQUENCE")
                    {
                        new_sequence_iterator->second->AddNestedSequenceNode(dto_node.node_id, dto_node.target_sequence_id.value());
                    }
                    else
                    {
                        new_sequence_iterator->second->AddEndSequenceNode(dto_node.node_id);
                    }
                }
                catch (const std::exception &e)
                {
                    logger.LogError("Error while generating node " + std::to_string(dto_node.node_id) +
                        " for the sequence '" + sequence_dto.sequence_name + "', " + e.what(),
                        "FrameworkRegistry");
                }
            }

            // Add transitions
            for (const auto &dto_transition : sequence_dto.transitions)
            {
                // Generate preconditions
                std::vector<StateOperation> preconditions;
                preconditions.reserve(dto_transition.preconditions.size());
                for (const auto & dto_precondition : dto_transition.preconditions)
                {
                    int32_t target_id;
                    int32_t state_key;
                    if (dto_precondition.target_id_name == "ENVIRONMENT")
                    {
                        target_id = -2;
                        //TODO: get state key from environmental manager or from schema
                    }
                    else if (dto_precondition.target_id_name == "SELF")
                    {
                        target_id = -1;
                        state_key = state_schema.GetStateKey(dto_precondition.state_key_name);
                    }
                    else
                    {
                        // Assume that all other names will reference ENTITY
                        target_id = 0;
                        state_key = state_schema.GetStateKey(dto_precondition.state_key_name);
                    }

                    //todo: This is hardcoded and incomplete. Look for a different way to handle this.
                    // Map operation name to value
                    int32_t operation;
                    if (dto_precondition.operation_name == "EQUALS")
                    {
                        operation = 0;
                    }
                    else if (dto_precondition.operation_name == "GREATER_THAN")
                    {
                        operation = 1;
                    }
                    else if (dto_precondition.operation_name == "LESS_THAN")
                    {
                        operation = 2;
                    }
                    else
                    {
                        operation = 3;
                    }

                    preconditions.emplace_back(target_id, state_key, operation, dto_precondition.parameters);
                }

                // Generate transition and add to sequence
                if (!new_sequence_iterator->second->TryAddTransition(dto_transition.transition_id, dto_transition.from_node_id,
                    dto_transition.to_node_id, preconditions))
                {
                    logger.LogError("Transition " + std::to_string(dto_transition.transition_id) +
                        " was not able to be added to the sequence '" + sequence_dto.sequence_name, "FrameworkRegistry");
                }
            }

            // Set entry point of sequence
            if (!new_sequence_iterator->second->TrySetEntryPoint(sequence_dto.entry_point_node_id))
            {
                logger.LogWarning("The entry point node id for sequence '" + sequence_dto.sequence_name +
                    "' was not set. Value: " + std::to_string(sequence_dto.entry_point_node_id),
                    "FrameworkRegistry");
            }

            logger.LogInfo("Sequence '" + sequence_dto.sequence_name + " ' has been configured.",
                    "FrameworkRegistry");
        }
        catch (const std::exception &e)
        {
            logger.LogError("Error while generating the sequence '" + sequence_dto.sequence_name + "', " +
                e.what(), "FrameworkRegistry");
        }
    }
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
