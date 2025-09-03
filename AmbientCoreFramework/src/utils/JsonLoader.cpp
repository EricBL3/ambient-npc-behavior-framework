#include "JsonLoader.h"

#include <fstream>

using json = nlohmann::json;

namespace AmbientCharacterBehavior {

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(EnvironmentalConditionDto, condition_key, name, update_frequency_ms);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(StateOperationDto, target_id_name, state_key_name, operation_name, values);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ActionDto, action_id, action_name, preconditions, immediate_effects,
    completion_effects, max_duration_ms, interruption_behavior_name);


std::optional<nlohmann::json> JsonLoader::LoadConfigFile(const std::string &config_file_path)
{
    try
    {
        std::ifstream config_file(config_file_path);
        if (!config_file.is_open())
        {
            FrameworkLogger::LogError("Failed to open config file: " + config_file_path,
                "JsonLoader");
            return std::nullopt;
        }

        json config_json;
        config_file >> config_json;
        return config_json;
    }
    catch (const json::exception& e)
    {
        FrameworkLogger::LogError("JSON parsing error in config file: " + std::string(e.what()),
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

}
