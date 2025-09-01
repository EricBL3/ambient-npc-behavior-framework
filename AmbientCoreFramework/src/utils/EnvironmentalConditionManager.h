/**
 * @file EnvironmentalConditionManager.h
 * @brief 
 * @author Eric Buitrón López
 * @date 9/1/2025
 *
 *
*/

#pragma once
#include <cstdint>
#include <unordered_map>

#include "EnvironmentalCondition.h"

namespace AmbientCharacterBehavior {
/**
 * @brief Manages a cache of the environmental conditions of the game or simulation.
 */
class EnvironmentalConditionManager {
    static std::unordered_map<int32_t, EnvironmentalCondition> environmental_conditions_cache;

public:
    static void RegisterEnvironmentalConditions(const std::string& config_file_path);

    /**
     * @brief Updates the value of the condition by querying the engine.
     * @param condition_key
     * @throw std::out_of_range if condition_key doesn't exist in the cache.
     */
    static void UpdateEnvironmentalCondition(int32_t condition_key);

    /**
     * @brief Gets the most recent value of the condition key.
     *
     * Will also update the current value if necessary.
     *
     * @param condition_key
     * @return The value of the condition key.
     * @throw std::out_of_range if condition_key doesn't exist in the cache.
     */
    static int32_t const GetEnvironmentalConditionValue(int32_t condition_key);
};
} // AmbientCharacterBehavior
