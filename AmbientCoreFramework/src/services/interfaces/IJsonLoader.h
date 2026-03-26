#pragma once
#include <optional>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include "services/json_dtos/BehavioralDtos.h"
#include "services/json_dtos/EntityDtos.h"
#include "services/json_dtos/EnvironmentalConditionDto.h"

namespace AmbientCharacterBehavior {
/**
 * @brief Handles reading and processing JSON configuration files
 */
class IJsonLoader {
public:
    virtual ~IJsonLoader() = default;
    virtual std::optional<nlohmann::json> LoadConfigFileJson(const std::string& config_file_path) = 0;
    virtual std::vector<EnvironmentalConditionDto> ProcessEnvironmentalConditionsConfigFile(
        const std::string& config_file_path) = 0;

    virtual std::vector<ActionDto> ProcessActionsConfigFile(const std::string& config_file_path) = 0;
    virtual std::vector<SequenceDto> ProcessSequencesConfigFile(const std::string& config_file_path) = 0;
    virtual std::optional<EntityDtoResult> ProcessSingleEntityConfigFile(const std::string& config_file_path) = 0;

    virtual std::optional<int64_t> GetTimeoutIntervalFromConfigFile(const std::string &config_file_path) = 0;
};
}
