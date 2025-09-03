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
    static std::optional<nlohmann::json> LoadConfigFile(const std::string& config_file_path);

    static std::vector<EnvironmentalConditionDto> ProcessEnvironmentalConditionsConfigFile(const std::string& config_file_path);

    static std::vector<ActionDto> ProcessActionsConfigFile(const std::string& config_file_path);

    static std::vector<SequenceDto> ProcessSequencesConfigFile(const std::string& config_file_path);

    static EntityDtoResult ProcessEntityConfigFile(const std::string& config_file_path);

private:
    template <typename T>
    static std::vector<T> ProcessConfigFile(const std::string& config_file_path, const std::string& array_key)
    {
        std::vector<T> result;

        try
        {
            auto config_json = LoadConfigFile(config_file_path);
            if (!config_json.has_value())
            {
                return result;
            }

            if (!config_json.value().contains(array_key) ||
                !config_json.value()[array_key].is_array())
            {
                FrameworkLogger::LogError(
                    "Config file missing '" + array_key + "' array",
                    "JsonLoader"
                );
                return result;
            }

            for (const auto& element_json : config_json.value()[array_key])
            {
                try
                {
                    auto dto = element_json.get<T>();
                    result.push_back(dto);
                }
                catch (const nlohmann::json::exception& e)
                {
                    FrameworkLogger::LogError(
                        "Failed to parse " + array_key + " from JSON: " +
                        std::string(e.what()),
                        "JsonLoader"
                    );
                }
            }
        }
        catch (const std::exception& e)
        {
            FrameworkLogger::LogError(
                "Unexpected error loading " + array_key + "s: " + std::string(e.what()),
                "JsonLoader"
            );
        }

        return result;
    }
};
}
