#include "./EnvironmentalConditionManager.h"

using namespace AmbientCharacterBehavior;
constexpr int32_t CONDITION_QUERY_FAILED = INT32_MIN;

bool EnvironmentalConditionManager::RegisterEnvironmentalConditions(const std::string& config_file_path)
{
    try
    {
        auto condition_dtos =
            json_loader.ProcessEnvironmentalConditionsConfigFile(config_file_path);

        if (condition_dtos.empty())
        {
            logger.LogWarning("No valid environmental conditions found in config",
                 "EnvironmentalConditionManager");
            return false;
        }

        CreateEnvironmentalConditions(condition_dtos);
        LoadConditionSchema(condition_dtos);

        logger.LogInfo("Successfully registered " + std::to_string(environmental_conditions_cache.size()) +
             " environmental conditions", "EnvironmentalConditionManager");

        return true;
    }
    catch (const std::exception& e)
    {
        logger.LogError("Unexpected error loading environmental conditions: " + std::string(e.what()),
             "EnvironmentalConditionManager");

        return false;
    }
}

void EnvironmentalConditionManager::LoadConditionSchema(const std::vector<EnvironmentalConditionDto> & condition_dtos)
{
    if (condition_dtos.empty())
    {
        return;
    }

    for (const auto& dto : condition_dtos)
    {
        auto name = dto.name;
        auto key = dto.condition_key;
        if (IsValidForCreation(name, key))
        {
            condition_name_to_key[name] = key;
            condition_key_to_name[key] = name;
        }
    }
}

bool EnvironmentalConditionManager::IsValidForCreation(const std::string &name, int32_t key)
{
    if (name.empty()) {
        logger.LogWarning("name cannot be empty for key: " + std::to_string(key),
                         "EnvironmentalConditionManager");
        return false;
    }

    if (key < 0) {
        logger.LogWarning("key cannot be negative, got: " + std::to_string(key) +
                         " for state: " + name, "EnvironmentalConditionManager");
        return false;
    }

    if (condition_name_to_key.find(name) != condition_name_to_key.end()) {
        logger.LogWarning("Duplicate name: " + name, "EnvironmentalConditionManager");
        return false;
    }

    if (condition_key_to_name.find(key) != condition_key_to_name.end()) {
        logger.LogWarning("Duplicate key: " + std::to_string(key) + " for state: " + name,
             "EnvironmentalConditionManager");

        return false;
    }

    return true;
}

/**
 * @throw std::out_of_range if name is not in the schema.
 */
int32_t EnvironmentalConditionManager::GetEnvironmentalConditionKey(const std::string &name)
{
    return condition_name_to_key.at(name);
}


/**
 * @throw std::out_of_range if key is not in the schema.
 */
std::string EnvironmentalConditionManager::GetEnvironmentalConditionName(int32_t key)
{
    return condition_key_to_name.at(key);
}

void EnvironmentalConditionManager::UpdateEnvironmentalCondition(int32_t condition_key)
{

    auto& condition = environmental_conditions_cache.at(condition_key);

    try
    {
        auto new_value = provider.QueryEnvironmentalCondition(condition_key);
        logger.LogInfo("Updating environment condition key: " + std::to_string(condition_key) + " to value: " +
            std::to_string(new_value), "UpdateEnvironmentalCondition");

        if (new_value == CONDITION_QUERY_FAILED)
        {
            logger.LogInfo("Updating environment condition " + std::to_string(condition_key) + " failed.",
                "UpdateEnvironmentalCondition");

            condition.SetLastUpdateMs(time_manager.GetCurrentTime());
            return;
        }

        condition.SetValue(new_value);
        condition.SetLastUpdateMs(time_manager.GetCurrentTime());

        if (!condition.GetIsInitialized())
        {
            condition.SetIsInitialized(true);
        }
    }
    catch (const std::exception& e)
    {
       logger.LogError(e.what(),"UpdateEnvironmentalCondition" );
    }
}

int32_t const EnvironmentalConditionManager::GetEnvironmentalConditionValue(int32_t condition_key)
{
    auto& condition = environmental_conditions_cache.at(condition_key);

    if (NeedsToBeUpdated(condition))
    {
        UpdateEnvironmentalCondition(condition_key);
    }

    return condition.GetValue();
}

bool EnvironmentalConditionManager::NeedsToBeUpdated(const EnvironmentalCondition& condition) const
{

    return !condition.GetIsInitialized() ||
        time_manager.GetCurrentTime() - condition.GetLastUpdateMs() >= condition.GetUpdateFrequencyMs();
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