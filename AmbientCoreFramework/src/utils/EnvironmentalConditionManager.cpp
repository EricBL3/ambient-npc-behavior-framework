#include "EnvironmentalConditionManager.h"

#include "JsonLoader.h"
#include "../include/EnvironmentalConditionInterface.h"

namespace AmbientCharacterBehavior {

std::unordered_map<int32_t, EnvironmentalCondition> EnvironmentalConditionManager::environmental_conditions_cache;

void EnvironmentalConditionManager::RegisterEnvironmentalConditions(const std::string& config_file_path)
{
    try
    {
        auto condition_dtos = JsonLoader::ProcessEnvironmentalConditionsConfigFile(config_file_path);
        if (condition_dtos.empty())
        {
            // FrameworkLogger::LogWarning("No valid environmental conditions found in config",
            //     "EnvironmentalConditionManager");
            return;
        }

        CreateEnvironmentalConditions(condition_dtos);

        // FrameworkLogger::LogInfo("Successfully registered " + std::to_string(environmental_conditions_cache.size()) +
        //     " environmental conditions", "EnvironmentalConditionManager");
    }
    catch (const std::exception& e)
    {
        // FrameworkLogger::LogError("Unexpected error loading environmental conditions: " + std::string(e.what()),
        //     "EnvironmentalConditionManager");
    }
}

void EnvironmentalConditionManager::UpdateEnvironmentalCondition(int32_t condition_key)
{

    auto& condition = environmental_conditions_cache.at(condition_key);

    try
    {
        auto new_value = QueryEnvironmentalCondition(condition_key);

        condition.SetValue(new_value);
        //condition.SetLastUpdateMs(TimeManager::GetCurrentTime());

        if (!condition.GetIsInitialized())
        {
            condition.SetIsInitialized(true);
        }
    }
    catch (const std::exception& e)
    {
       // FrameworkLogger::LogError(e.what(),"EnvironmentalConditionManager" );
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
        // FrameworkLogger::LogError("Duplicate condition_key: " + std::to_string(dto.condition_key) +
        //     " for condition: " + dto.name, "EnvironmentalConditionManager");
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

        // FrameworkLogger::LogInfo("Registered environmental condition: " + dto.name +
        //     " (key: " + std::to_string(dto.condition_key) + ")", "EnvironmentalConditionManager");
    }
    catch (const std::exception& e)
    {
        // FrameworkLogger::LogError("Failed to create environmental condition '" + dto.name + "': " + e.what(),
        //     "EnvironmentalConditionManager");
    }
}
}