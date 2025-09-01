/**
 * @file EnvironmentalConditionManager.cpp
 * @brief 
 * @author Eric Buitrón López
 * @date 9/1/2025
 *
 *
*/

#include "EnvironmentalConditionManager.h"

#include <fstream>
#include "FrameworkLogger.h"
#include "TimeManager.h"
#include "../include/EnvironmentalConditionInterface.h"

using json = nlohmann::json;

namespace AmbientCharacterBehavior {

std::unordered_map<int32_t, EnvironmentalCondition> EnvironmentalConditionManager::environmental_conditions_cache;

void EnvironmentalConditionManager::RegisterEnvironmentalConditions(const std::string& config_file_path)
{
    try
    {
        auto config_json = LoadConfigFile(config_file_path);
        if (!config_json.has_value())
        {
            return;
        }

        auto condition_dtos = ParseEnvironmentalConditions(config_json.value());
        if (condition_dtos.empty())
        {
            FrameworkLogger::LogWarning("No valid environmental conditions found in config",
                "EnvironmentalConditionManager");
            return;
        }

        CreateEnvironmentalConditions(condition_dtos);

        FrameworkLogger::LogInfo("Successfully registered " + std::to_string(environmental_conditions_cache.size()) +
            " environmental conditions", "EnvironmentalConditionManager");
    }
    catch (const std::exception& e)
    {
        FrameworkLogger::LogError("Unexpected error loading environmental conditions: " + std::string(e.what()),
            "EnvironmentalConditionManager");
    }
}

void EnvironmentalConditionManager::UpdateEnvironmentalCondition(int32_t condition_key)
{

    auto& condition = environmental_conditions_cache.at(condition_key);

    try
    {
        auto new_value = QueryEnvironmentalCondition(condition_key);

        condition.SetValue(new_value);
        condition.SetLastUpdateMs(TimeManager::GetCurrentTime());
    }
    catch (std::exception& e)
    {
        FrameworkLogger::LogError(e.what(),"EnvironmentalConditionManager" );
    }
}

int32_t const EnvironmentalConditionManager::GetEnvironmentalConditionValue(int32_t condition_key)
{
    auto& condition = environmental_conditions_cache.at(condition_key);

    if (condition.NeedsToBeUpdated())
    {
        UpdateEnvironmentalCondition(condition_key);
    }

    return condition.GetValue();
}

std::optional<nlohmann::json> EnvironmentalConditionManager::LoadConfigFile(const std::string &config_file_path)
{
    try
    {
        std::ifstream config_file(config_file_path);
        if (!config_file.is_open())
        {
            FrameworkLogger::LogError("Failed to open environmental ocnditions config file: " + config_file_path,
                "EnvironmentalConditionManager");
            return std::nullopt;
        }

        json config_json;
        config_file >> config_json;
        return config_json;
    }
    catch (const json::exception& e)
    {
        FrameworkLogger::LogError("JSON parsing error in config file: " + std::string(e.what()),
            "EnvironmentalConditionManager");

        return std::nullopt;
    }
}

std::vector<EnvironmentalConditionDto> EnvironmentalConditionManager::ParseEnvironmentalConditions(
    const nlohmann::json &config_json)
{
    std::vector<EnvironmentalConditionDto> condition_dtos;

    if (!config_json.contains("environmental_conditions") && config_json["environmental_conditions"].is_array())
    {
        FrameworkLogger::LogError("Config file missing 'environmental_conditions' array",
            "EnvironmentalConditionManager");
        return condition_dtos;
    }

    for (const auto& condition_json : config_json["environmental_conditions"])
    {
        auto dto = ParseSingleCondition(condition_json);
        if (dto.has_value())
        {
            condition_dtos.push_back(dto.value());
        }
    }

    return condition_dtos;
}

std::optional<EnvironmentalConditionDto> EnvironmentalConditionManager::ParseSingleCondition(
    const nlohmann::json &condition_json)
{
    try
    {
        EnvironmentalConditionDto dto;
        dto.condition_key = condition_json.at("condition_key").get<int32_t>();
        dto.name = condition_json.at("name").get<std::string>();
        dto.update_frequency_ms = condition_json.at("update_frequency_ms").get<int64_t>();

        return dto;
    }
    catch (const json::exception& e) {
        FrameworkLogger::LogError("Failed to parse environmental condition from JSON: " +
            std::string(e.what()),"EnvironmentalConditionManager");

        return std::nullopt;
    }
}

void EnvironmentalConditionManager::CreateEnvironmentalConditions(
    const std::vector<EnvironmentalConditionDto> &condition_dtos)
{
    for (const auto& dto: condition_dtos)
    {
        if (IsValidForCreation(dto))
        {
            CreateSingleEnvironmentalCondition(dto);
        }
    }
}

bool EnvironmentalConditionManager::IsValidForCreation(const EnvironmentalConditionDto &dto)
{
    // Check for duplicate keys
    if (environmental_conditions_cache.find(dto.condition_key) != environmental_conditions_cache.end()) {
        FrameworkLogger::LogError("Duplicate condition_key: " + std::to_string(dto.condition_key) +
            " for condition: " + dto.name, "EnvironmentalConditionManager");
        return false;
    }

    return true;
}

void EnvironmentalConditionManager::CreateSingleEnvironmentalCondition(const EnvironmentalConditionDto &dto)
{
    try
    {
        environmental_conditions_cache.emplace(dto.condition_key,
            EnvironmentalCondition(dto.condition_key, dto.name, dto.update_frequency_ms));

        FrameworkLogger::LogInfo("Registered environmental condition: " + dto.name +
            " (key: " + std::to_string(dto.condition_key) + ")", "EnvironmentalConditionManager");
    }
    catch (const std::exception& e)
    {
        FrameworkLogger::LogError("Failed to create environmental condition '" + dto.name + "': " + e.what(),
            "EnvironmentalConditionManager");
    }
}
} // AmbientCharacterBehavior