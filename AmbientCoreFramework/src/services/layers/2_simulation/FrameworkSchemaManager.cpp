#include <nlohmann/json.hpp>
#include "FrameworkSchemaManager.h"

using json = nlohmann::json;
using namespace AmbientCharacterBehavior;

bool FrameworkSchemaManager::LoadFrameworkSchema(const std::string &config_file_path)
{
    auto config_json = json_loader.LoadConfigFileJson(config_file_path);

    if (!config_json.has_value())
    {
        return false;
    }

    return LoadSchemaConfiguration(config_json, "entity_states", state_name_to_key, state_key_to_name) &&
        LoadSchemaConfiguration(config_json, "interruption_handlers", interruption_name_to_key,
            interruption_key_to_name);

}

bool FrameworkSchemaManager::LoadSchemaConfiguration(const std::optional<json> &config_json, const std::string& schema_name,
    std::unordered_map<std::string, int32_t>& name_to_key_map, std::unordered_map<int32_t, std::string>& key_to_name_map) const
{
    if (config_json.value().contains(schema_name) && config_json.value()[schema_name].is_array())
    {
        for (const auto& state_json : config_json.value()[schema_name])
        {
            try
            {
                auto name = state_json.at("name").get<std::string>();
                auto key = state_json.at("key").get<int32_t>();

                if (IsValidForCreation(name, key, name_to_key_map, key_to_name_map))
                {
                    name_to_key_map[name] = key;
                    key_to_name_map[key] = name;

                    logger.LogInfo("Registered " + schema_name + " in schema. Name: " + std::string(name) +
                         " Key: " + std::to_string(key), "FrameworkSchemaManager");

                }

            }
            catch (const json::exception& e) {
                logger.LogError("Failed to parse "+ schema_name + " schema from JSON: " +
                     std::string(e.what()),"FrameworkSchemaManager");

                return false;
            }
        }

        logger.LogInfo("Registered " + std::to_string(name_to_key_map.size()) + " " + schema_name + " schemas",
             "FrameworkSchemaManager");

        return true;
    }

    logger.LogError("Config file missing '" + schema_name + "' array",
                    "FrameworkSchemaManager");

    return false;
}

bool FrameworkSchemaManager::IsValidForCreation(const std::string &name, int32_t key,
    std::unordered_map<std::string, int32_t>& name_to_key_map, std::unordered_map<int32_t, std::string>& key_to_name_map) const
{
    if (name.empty()) {
        logger.LogWarning("name cannot be empty for key: " + std::to_string(key),
                         "FrameworkSchemaManager");
        return false;
    }

    if (key < 0) {
        logger.LogWarning("key cannot be negative, got: " + std::to_string(key) +
                         " for state: " + name, "FrameworkSchemaManager");
        return false;
    }

    if (name_to_key_map.find(name) != name_to_key_map.end()) {
        logger.LogWarning("Duplicate name: " + name, "FrameworkSchemaManager");
        return false;
    }

    if (key_to_name_map.find(key) != key_to_name_map.end()) {
        logger.LogWarning("Duplicate key: " + std::to_string(key) + " for name: " + name,
             "FrameworkSchemaManager");

        return false;
    }

    return true;
}

/**
 * @throw std::out_of_range if state_name is not in the schema.
 */
int32_t FrameworkSchemaManager::GetStateKey(const std::string &state_name)
{
    return state_name_to_key.at(state_name);
}

/**
 * @throw std::out_of_range if state_key is not in the schema.
 */
std::string FrameworkSchemaManager::GetStateName(int32_t state_key)
{
    return state_key_to_name.at(state_key);
}

StateOperationType FrameworkSchemaManager::GetStateOperationTypeId(const std::string &name)
{
    logger.LogInfo("Getting state operation type: " + name, "FrameworkSchemaManager");

    if (name == "EQUALS")
    {
        return StateOperationType::EQUALS;
    }
    if (name == "NOT_EQUALS")
    {
        return StateOperationType::NOT_EQUALS;
    }
    if (name == "GREATER_THAN")
    {
        return StateOperationType::GREATER_THAN;
    }
    if (name == "LESS_THAN")
    {
        return StateOperationType::LESS_THAN;
    }
    if (name == "SET")
    {
        return StateOperationType::SET;
    }
    if (name == "INCREMENT")
    {
        return StateOperationType::INCREMENT;
    }
    if (name == "DECREMENT")
    {
        return StateOperationType::DECREMENT;
    }

    logger.LogWarning("Unknown operation type '" + name + "', treating as external operation",
        "FrameworkSchemaManager");

    return StateOperationType::EXTERNAL_OPERATIONS;
}

std::string FrameworkSchemaManager::GetStateOperationTypeName(StateOperationType operation_type)
{
    switch (operation_type)
    {
        case StateOperationType::EQUALS:
            return "EQUALS";
        case StateOperationType::NOT_EQUALS:
            return "NOT_EQUALS";
        case StateOperationType::GREATER_THAN:
            return "GREATER_THAN";
        case StateOperationType::LESS_THAN:
            return "LESS_THAN";
        case StateOperationType::SET:
            return "SET";
        case StateOperationType::INCREMENT:
            return "INCREMENT";
        case StateOperationType::DECREMENT:
            return "DECREMENT";
        default:
            logger.LogWarning("Unknown operation type '" + std::to_string(static_cast<int>(operation_type)) +
                "', treating as external operation", "FrameworkSchemaManager");

            return "EXTERNAL_OPERATIONS";
    }
}

/**
 * @throw std::out_of_range if interruption_name is not in the schema.
 */
int32_t FrameworkSchemaManager::GetInterruptionKey(const std::string &interruption_name)
{
    return interruption_name_to_key.at(interruption_name);
}

/**
 * @throw std::out_of_range if interruption_key is not in the schema.
 */
std::string FrameworkSchemaManager::GetInterruptionName(int32_t interruption_key)
{
    return interruption_key_to_name.at(interruption_key);
}

