#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>

#include "interfaces/IJsonLoader.h"
#include "interfaces/ILogger.h"
#include "interfaces/IFrameworkSchemaManager.h"
#include "../../behavior/enums/StateOperationType.h"

namespace AmbientCharacterBehavior {
/**
 * @brief Handles the mappings of state names and keys.
 */
class FrameworkSchemaManager : public IFrameworkSchemaManager {
    std::unordered_map<std::string, int32_t> state_name_to_key;
    std::unordered_map<int32_t, std::string> state_key_to_name;

    std::unordered_map<std::string, int32_t> interruption_name_to_key;
    std::unordered_map<int32_t, std::string> interruption_key_to_name;

    ILogger& logger;
    IJsonLoader& json_loader;

public:
    explicit FrameworkSchemaManager(ILogger& logger, IJsonLoader& json_loader) : logger(logger), json_loader(json_loader) {}

    bool LoadFrameworkSchema(const std::string& config_file_path) override;
    int32_t GetStateKey(const std::string& state_name) override;
    std::string GetStateName(int32_t state_key) override;

    StateOperationType GetStateOperationTypeId(const std::string &name) override;
    std::string GetStateOperationTypeName(StateOperationType id) override;

    int32_t GetInterruptionKey(const std::string& interruption_name) override;
    std::string GetInterruptionName(int32_t interruption_key) override;

private:
    bool LoadSchemaConfiguration(const std::optional<nlohmann::json> &config_json, const std::string& schema_name,
        std::unordered_map<std::string, int32_t>& name_to_key_map, std::unordered_map<int32_t, std::string>& key_to_name_map) const;

    bool IsValidForCreation(const std::string& name, int32_t key,
        std::unordered_map<std::string, int32_t>& name_to_key_map, std::unordered_map<int32_t, std::string>& key_to_name_map) const;
};
}
