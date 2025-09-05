#include "./EnvironmentalConditionManager.h"

using namespace AmbientCharacterBehavior;

void EnvironmentalConditionManager::RegisterEnvironmentalConditions(const std::string& config_file_path)
{
    try
    {
        auto condition_dtos = json_loader.ProcessEnvironmentalConditionsConfigFile(config_file_path);
        if (condition_dtos.empty())
        {
            logger.LogWarning("No valid environmental conditions found in config",
                 "EnvironmentalConditionManager");
            return;
        }

        CreateEnvironmentalConditions(condition_dtos);

        logger.LogInfo("Successfully registered " + std::to_string(environmental_conditions_cache.size()) +
             " environmental conditions", "EnvironmentalConditionManager");
    }
    catch (const std::exception& e)
    {
        logger.LogError("Unexpected error loading environmental conditions: " + std::string(e.what()),
             "EnvironmentalConditionManager");
    }
}

void EnvironmentalConditionManager::UpdateEnvironmentalCondition(int32_t condition_key)
{

    auto& condition = environmental_conditions_cache.at(condition_key);

    try
    {
        auto new_value = provider.QueryEnvironmentalCondition(condition_key);

        condition.SetValue(new_value);
        condition.SetLastUpdateMs(time_manager.GetCurrentTime());

        if (!condition.GetIsInitialized())
        {
            condition.SetIsInitialized(true);
        }
    }
    catch (const std::exception& e)
    {
       logger.LogError(e.what(),"EnvironmentalConditionManager" );
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
        logger.LogError("Duplicate condition_key: " + std::to_string(dto.condition_key) +
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

        logger.LogInfo("Registered environmental condition: " + dto.name +
             " (key: " + std::to_string(dto.condition_key) + ")", "EnvironmentalConditionManager");
    }
    catch (const std::exception& e)
    {
        logger.LogError("Failed to create environmental condition '" + dto.name + "': " + e.what(),
             "EnvironmentalConditionManager");
    }
}