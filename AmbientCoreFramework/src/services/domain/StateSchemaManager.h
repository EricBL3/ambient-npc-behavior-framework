#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>

#include "interfaces/IJsonLoader.h"
#include "interfaces/ILogger.h"
#include "interfaces/IStateSchemaManager.h"

namespace AmbientCharacterBehavior {
/**
 * @brief Handles the mappings of state names and keys.
 */
class StateSchemaManager : public IStateSchemaManager {
    std::unordered_map<std::string, int32_t> state_name_to_key;
    std::unordered_map<int32_t, std::string> state_key_to_name;

    ILogger& logger;
    IJsonLoader& json_loader;

public:
    explicit StateSchemaManager(ILogger& logger, IJsonLoader& json_loader) : logger(logger), json_loader(json_loader) {}
    void LoadStateSchema(const std::string& config_file_path) override;
    int32_t GetStateKey(const std::string& state_name) override;
    std::string GetStateName(int32_t state_key) override;

private:

    bool IsValidForCreation(const std::string& state_name, int32_t state_key);
};
}
