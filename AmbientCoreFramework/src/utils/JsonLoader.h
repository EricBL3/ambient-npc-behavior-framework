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
    static std::optional<nlohmann::json> LoadConfigFileJson(const std::string& config_file_path);

    static std::vector<EnvironmentalConditionDto> ProcessEnvironmentalConditionsConfigFile(const std::string& config_file_path);

    static std::vector<ActionDto> ProcessActionsConfigFile(const std::string& config_file_path);

    static std::vector<SequenceDto> ProcessSequencesConfigFile(const std::string& config_file_path);

    static std::optional<EntityDtoResult> ProcessSingleEntityConfigFile(const std::string& config_file_path);

private:

    static std::optional<nlohmann::json> LoadValidConfigJsonArray(const std::string& config_file_path, const std::string& array_key);

    /**
     * @tparam T The type of the items in the vector
     * @param config_file_path
     * @param array_key The key of the array in the json configuration file that will be parsed
     * @return A vector of items of type T.
     */
    template <typename T>
    static std::vector<T> ProcessConfigFile(const std::string& config_file_path, const std::string& array_key)
    {
        auto config_array_json = LoadValidConfigJsonArray(config_file_path, array_key);
        if (!config_array_json.has_value())
        {
            return {};
        }

        return ProcessJsonArray<T>(config_array_json.value());
    }

    /**
     * @tparam T The type of the items in the vector
     * @param config_array_json The json array that will be processed
     * @return A vector of items of type T.
     */
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