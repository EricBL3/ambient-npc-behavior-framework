#pragma once
#include "EnvironmentalCondition.h"
#include "json_dtos/EnvironmentalConditionDto.h"

#include <cstdint>
#include <unordered_map>
#include <vector>

#include "interfaces/IEnvironmentalConditionManager.h"
#include "interfaces/IEnvironmentalConditionProvider.h"
#include "interfaces/IJsonLoader.h"
#include "interfaces/ILogger.h"
#include "interfaces/ITimeManager.h"

namespace AmbientCharacterBehavior {
/**
 * @brief Manages a cache of the environmental conditions of the game or simulation.
 */
class EnvironmentalConditionManager : public IEnvironmentalConditionManager {
    std::unordered_map<int32_t, EnvironmentalCondition> environmental_conditions_cache;
    std::unordered_map<std::string, int32_t> condition_name_to_key;
    std::unordered_map<int32_t, std::string> condition_key_to_name;

public:
    explicit EnvironmentalConditionManager(ILogger& logger, ITimeManager& time_manager, IJsonLoader& json_loader,
        IEnvironmentalConditionProvider& provider) :
        logger(logger), time_manager(time_manager), json_loader(json_loader), provider(provider) {}

    bool RegisterEnvironmentalConditions(const std::string& config_file_path) override;
    int32_t GetEnvironmentalConditionKey(const std::string& state_name) override;
    std::string GetEnvironmentalConditionName(int32_t state_key) override;

    /**
     * @brief Updates the value of the condition by querying the engine.
     * @throw std::out_of_range if condition_key doesn't exist in the cache.
     */
    void UpdateEnvironmentalCondition(int32_t condition_key) override;

    /**
     * @brief Updates the current value if necessary and returns it
     * @throw std::out_of_range if condition_key doesn't exist in the cache.
     */
    int32_t const GetEnvironmentalConditionValue(int32_t condition_key) override;

private:

    ILogger& logger;
    ITimeManager& time_manager;
    IJsonLoader& json_loader;
    IEnvironmentalConditionProvider& provider;

    void LoadConditionSchema(const std::vector<EnvironmentalConditionDto> & condition_dtos);
    bool IsValidForCreation(const std::string &name, int32_t key);

    void CreateEnvironmentalConditions(const std::vector<EnvironmentalConditionDto>& condition_dtos);
    bool IsValidForCreation(const EnvironmentalConditionDto& dto);
    void CreateSingleEnvironmentalCondition(const EnvironmentalConditionDto& dto);
};
}
