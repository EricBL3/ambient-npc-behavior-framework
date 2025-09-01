/**
 * @file EnvironmentalCondition.h
 * @brief 
 * @author Eric Buitrón López
 * @date 9/1/2025
 *
 *
*/
#pragma once
#include <cstdint>
#include <string>

namespace AmbientCharacterBehavior {
/**
 * @brief Represents an environmental condition present in the game or simulation. Generated through a JSON configuration file.
 */
class EnvironmentalCondition {
    int32_t condition_key;

    std::string name;

    int32_t value;

    int64_t update_frequency_ms;

    int64_t last_update_ms;

    bool is_initialized;

public:
    /**
     * @param condition_key The key for the environmental condition
     * @param name The name of the environmental condition
     * @param update_frequency_ms The update frequency of the environmental condition
     * @throw std::invalid_argument if condition_key or update_frequency_ms < 0
     */
    EnvironmentalCondition(int32_t condition_key, std::string name, int64_t update_frequency_ms);

    int32_t GetConditionKey() const { return condition_key; }

    std::string_view GetName() const { return name; }

    int32_t GetValue() const { return value; }

    void SetValue(int32_t val) { value = val; }

    int64_t GetUpdateFrequencyMs() const { return update_frequency_ms; }

    int64_t GetLastUpdateMs() const { return last_update_ms; }

    void SetLastUpdateMs(int64_t val) { last_update_ms = val; }

    bool IsInitialized() const { return is_initialized; }

    void SetIsInitialized(bool val) { is_initialized = val; }

};
} // AmbientCharacterBehavior
