#include <nlohmann/json.hpp>
#include "StateSchemaManager.h"

using json = nlohmann::json;
using namespace AmbientCharacterBehavior;

void StateSchemaManager::LoadStateSchema(const std::string &config_file_path)
{
    auto config_json = json_loader.LoadConfigFileJson(config_file_path);

    if (!config_json.has_value())
    {
        return;
    }

    if (config_json.value().contains("entity_states") && config_json.value()["entity_states"].is_array())
    {
        for (const auto& state_json : config_json.value()["entity_states"])
        {
            try
            {
                auto name = state_json.at("name").get<std::string>();
                auto key = state_json.at("key").get<int32_t>();

                if (IsValidForCreation(name, key))
                {
                    state_name_to_key[name] = key;
                    state_key_to_name[key] = name;

                    logger.LogInfo("Registered state in schema. Name: " + name +
                         " Key: " + std::to_string(key), "StateSchemaManager");

                }

            }
            catch (const json::exception& e) {
                logger.LogError("Failed to parse state schema from JSON: " +
                     std::string(e.what()),"StateSchemaManager");
            }
        }

        logger.LogInfo("Registered " + std::to_string(state_name_to_key.size()) + " state schemas",
             "StateSchemaManager");

    }
    else
    {
        logger.LogError("Config file missing 'entity_states' array",
            "StateSchemaManager");
    }
}

/**
 * @throw std::out_of_range if state_name is not in the schema.
 */
int32_t StateSchemaManager::GetStateKey(const std::string &state_name)
{
    return state_name_to_key.at(state_name);
}

/**
 * @throw std::out_of_range if state_key is not in the schema.
 */
std::string StateSchemaManager::GetStateName(int32_t state_key)
{
    return state_key_to_name.at(state_key);
}

StateOperationType StateSchemaManager::GetStateOperationTypeId(const std::string &name)
{
    logger.LogInfo("Getting state operation type: " + name, "StateSchemaManager");

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
    if (name == "EQUALS_STATE")
    {
        return StateOperationType::EQUALS_STATE;
    }
    if (name == "NOT_EQUALS_STATE")
    {
        return StateOperationType::NOT_EQUALS_STATE;
    }
    if (name == "GREATER_THAN_STATE")
    {
        return StateOperationType::GREATER_THAN_STATE;
    }
    if (name == "LESS_THAN_STATE")
    {
        return StateOperationType::LESS_THAN_STATE;
    }

    logger.LogWarning("Unknown operation type '" + name + "', treating as external operation", "FrameworkSchemaManager");
    return StateOperationType::EXTERNAL_OPERATIONS;
}

std::string StateSchemaManager::GetStateOperationTypeName(StateOperationType operation_type)
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
        case StateOperationType::EQUALS_STATE:
            return "EQUALS_STATE";
        case StateOperationType::NOT_EQUALS_STATE:
            return "NOT_EQUALS_STATE";
        case StateOperationType::GREATER_THAN_STATE:
            return "GREATER_THAN_STATE";
        case StateOperationType::LESS_THAN_STATE:
            return "LESS_THAN_STATE";
        default:
            logger.LogWarning("Unknown operation type '" + std::to_string(static_cast<int>(operation_type)) +
                "', treating as external operation", "FrameworkSchemaManager");

            return "EXTERNAL_OPERATIONS";
    }
}

bool StateSchemaManager::IsValidForCreation(const std::string &state_name, int32_t state_key)
{
    if (state_name.empty()) {
        logger.LogWarning("State name cannot be empty for key: " + std::to_string(state_key),
                         "StateSchemaManager");
        return false;
    }

    if (state_key < 0) {
        logger.LogWarning("State key cannot be negative, got: " + std::to_string(state_key) +
                         " for state: " + state_name, "StateSchemaManager");
        return false;
    }

    if (state_name_to_key.find(state_name) != state_name_to_key.end()) {
        logger.LogWarning("Duplicate state name: " + state_name, "StateSchemaManager");
        return false;
    }

    if (state_key_to_name.find(state_key) != state_key_to_name.end()) {
        logger.LogWarning("Duplicate state key: " + std::to_string(state_key) + " for state: " + state_name,
             "StateSchemaManager");

        return false;
    }

    return true;
}
