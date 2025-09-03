#pragma once
#include "json_dtos/EnvironmentalConditionDto.h"
#include "json_dtos/BehavioralDtos.h"
#include "json_dtos/EntityDtos.h"

#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <vector>

#include "FrameworkLogger.h"


namespace AmbientCharacterBehavior {
/**
 * @brief Handles reading and processing JSON configuration files
 */
class JsonLoader {

public:


    static std::vector<EnvironmentalConditionDto> ProcessEnvironmentalConditionsConfigFile(const std::string& config_file_path);

    static std::vector<ActionDto> ProcessActionsConfigFile(const std::string& config_file_path);

    static std::vector<SequenceDto> ProcessSequencesConfigFile(const std::string& config_file_path);

    static EntityDtoResult ProcessEntityConfigFile(const std::string& config_file_path);

private:
    static std::optional<nlohmann::json> LoadConfigFile(const std::string& config_file_path);

    static std::optional<nlohmann::json> LoadValidJsonConfigArray(const std::string& config_file_path, const std::string& array_key);

    template <typename T>
    static std::vector<T> ProcessConfigFile(const std::string& config_file_path, const std::string& array_key)
    {
        auto config_array_json = LoadValidJsonConfigArray(config_file_path, array_key);
        if (!config_array_json.has_value())
        {
            return {};
        }

        return ProcessJsonArray<T>(config_array_json.value());
    }

    template <typename T>
    static std::vector<T> ProcessJsonArray(const nlohmann::json& config_array_json)
    {
        std::vector<T> result;

        for (const auto& element_json : config_array_json)
        {
            try
            {
                result.push_back(element_json.get<T>());
            }
            catch (const nlohmann::json::exception& e)
            {
                FrameworkLogger::LogError( "Failed to parse element from JSON: " +
                    std::string(e.what()),"JsonLoader");
            }
        }

        return result;
    }
};
}