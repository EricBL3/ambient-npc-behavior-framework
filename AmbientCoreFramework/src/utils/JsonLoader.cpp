#include "JsonLoader.h"
#include "FrameworkLogger.h"

#include <fstream>

using json = nlohmann::json;

namespace AmbientCharacterBehavior {

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(EnvironmentalConditionDto, condition_key, name, update_frequency_ms);

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
    std::vector<EnvironmentalConditionDto> condition_dtos;

    try
    {
        auto config_json = LoadConfigFile(config_file_path);
        if (!config_json.has_value())
        {
            return condition_dtos;
        }

        if (!config_json.value().contains("environmental_conditions") ||
            config_json.value()["environmental_conditions"].is_array())
        {
            FrameworkLogger::LogError("Config file missing 'environmental_conditions' array",
                "JsonLoader");
            return condition_dtos;
        }

        for (const auto& condition_json : config_json.value()["environmental_conditions"])
        {
            try
            {
                auto dto = condition_json.get<EnvironmentalConditionDto>();
                condition_dtos.push_back(dto);
            }
            catch (const json::exception& e) {
                FrameworkLogger::LogError("Failed to parse environmental condition from JSON: " +
                    std::string(e.what()),"JsonLoader");
            }
        }

    }
    catch (const std::exception& e)
    {
        FrameworkLogger::LogError("Unexpected error loading environmental conditions: " + std::string(e.what()),
            "JsonLoader");
    }

    return condition_dtos;
}
}