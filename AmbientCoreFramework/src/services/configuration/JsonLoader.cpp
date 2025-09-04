#include "JsonLoader.h"

#include <fstream>

using json = nlohmann::json;

namespace AmbientCharacterBehavior {

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(EnvironmentalConditionDto, condition_key, name, update_frequency_ms);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(StateOperationDto, target_id_name, state_key_name, operation_name, values);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TransitionDto, transition_id, from_node_id, to_node_id, preconditions);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SequenceDto, sequence_id, sequence_name, entry_point_node_id, transitions);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ActionDto, action_id, action_name, preconditions, immediate_effects,
    completion_effects, max_duration_ms, interruption_behavior_name);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(FrameworkEntityDto, entity_id, entity_name, accepted_actions_ids, initial_state);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MemoryLimitsDto, max_transition_memories, max_action_memories, max_interruption_memories);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(BehavioralEntityDto, main_sequence_id, fallback_sequences, interruption_handlers, memory_limits);

std::optional<nlohmann::json> JsonLoader::LoadConfigFileJson(const std::string &config_file_path)
{
    try
    {
        std::ifstream config_file(config_file_path);
        if (!config_file.is_open())
        {
            logger.LogError("Failed to open config file: " + config_file_path,
                "JsonLoader");
            return std::nullopt;
        }

        json config_json;
        config_file >> config_json;
        return config_json;
    }
    catch (const json::exception& e)
    {
        logger.LogError("JSON parsing error in config file: " + std::string(e.what()),
             "JsonLoader");

        return std::nullopt;
    }
}

std::vector<EnvironmentalConditionDto> JsonLoader::ProcessEnvironmentalConditionsConfigFile(const std::string& config_file_path)
{
    return ProcessConfigFile<EnvironmentalConditionDto>(config_file_path, "environmental_conditions");
}

std::vector<ActionDto> JsonLoader::ProcessActionsConfigFile(const std::string &config_file_path)
{
    return ProcessConfigFile<ActionDto>(config_file_path, "actions");
}

std::vector<SequenceDto> JsonLoader::ProcessSequencesConfigFile(const std::string &config_file_path)
{

    auto config_array_json = LoadValidConfigJsonArray(config_file_path, "sequences");

    if (!config_array_json.has_value())
    {
        return {};
    }

    std::vector<SequenceDto> result;
    for (const auto& element_json : config_array_json.value())
    {
        try
        {
            auto sequence_dto = element_json.get<SequenceDto>();

            for (const auto& node_json : element_json.at("nodes"))
            {
                SequenceNodeDto node_dto;
                node_dto.node_id = node_json.at("node_id");
                node_dto.node_type = node_json.at("node_type");

                if (node_json.at("node_type") == "ACTION")
                {
                    node_dto.target_action_id = node_json.at("target_action_id");
                }
                else if (node_json.at("node_type") == "SEQUENCE")
                {
                    node_dto.target_sequence_id = node_json.at("target_sequence_id");
                }

                sequence_dto.nodes.push_back(node_dto);
            }

            result.push_back(sequence_dto);
        }
        catch (const nlohmann::json::exception& e)
        {
            logger.LogError( "Failed to parse element from JSON: " +
                 std::string(e.what()),"JsonLoader");
        }
    }

    return result;
}

std::optional<EntityDtoResult> JsonLoader::ProcessSingleEntityConfigFile(const std::string &config_file_path)
{
    auto config_array_json = LoadValidConfigJsonArray(config_file_path, "entities");

    if (!config_array_json.has_value()|| config_array_json->empty())
    {
        return std::nullopt;
    }

    try
    {
        const auto& entity_json = config_array_json->front();

        EntityDtoResult result;
        result.entity_type = entity_json.at("entity_type");

        auto framework_entity_dto = entity_json.at("entity").get<FrameworkEntityDto>();

        if (entity_json.at("entity_type") == "FRAMEWORK")
        {
            result.framework_entity = framework_entity_dto;
            result.behavioral_entity = std::nullopt;
        }
        else if (entity_json.at("entity_type") == "BEHAVIORAL")
        {
            auto behavioral_entity_dto = entity_json.at("entity").get<BehavioralEntityDto>();
            behavioral_entity_dto.base_properties = framework_entity_dto;

            result.framework_entity = std::nullopt;
            result.behavioral_entity = behavioral_entity_dto;
        }
        else
        {
            logger.LogError("Unknown entity type: " + std::string(result.entity_type),
                 "JsonLoader");
        }

        return result;
    }
    catch (const nlohmann::json::exception& e)
    {
        logger.LogError( "Failed to parse element from JSON: " +
             std::string(e.what()),"JsonLoader");

        return std::nullopt;
    }
}

std::optional<nlohmann::json> JsonLoader::LoadValidConfigJsonArray(const std::string &config_file_path, const std::string &array_key)
{
    try
    {
        auto config_json = LoadConfigFileJson(config_file_path);
        if (!config_json.has_value())
        {
            return std::nullopt;
        }

        if (!config_json.value().contains(array_key) || !config_json.value()[array_key].is_array())
        {
            logger.LogError("Config file missing '" + array_key + "' array","JsonLoader");
            return std::nullopt;
        }

        return config_json.value()[array_key];
    }
    catch (const std::exception& e)
    {
        logger.LogError("Unexpected error loading " + array_key + ": " + std::string(e.what()),
             "JsonLoader");

        return std::nullopt;
    }
}
}
