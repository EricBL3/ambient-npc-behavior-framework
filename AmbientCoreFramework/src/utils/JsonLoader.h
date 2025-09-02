#pragma once
#include "json_dtos/EnvironmentalConditionDto.h"
#include "json_dtos/BehavioralDtos.h"
#include "json_dtos/EntityDtos.h"

#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <vector>



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
};
}
