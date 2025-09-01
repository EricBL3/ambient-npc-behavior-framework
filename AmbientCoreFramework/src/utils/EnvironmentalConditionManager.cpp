/**
 * @file EnvironmentalConditionManager.cpp
 * @brief 
 * @author Eric Buitrón López
 * @date 9/1/2025
 *
 *
*/

#include "EnvironmentalConditionManager.h"

#include <stdexcept>

#include "FrameworkLogger.h"
#include "TimeManager.h"
#include "../include/EnvironmentalConditionInterface.h"

namespace AmbientCharacterBehavior {

std::unordered_map<int32_t, EnvironmentalCondition> EnvironmentalConditionManager::environmental_conditions_cache;

void EnvironmentalConditionManager::RegisterEnvironmentalConditions(const std::string& config_file_path)
{

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
} // AmbientCharacterBehavior