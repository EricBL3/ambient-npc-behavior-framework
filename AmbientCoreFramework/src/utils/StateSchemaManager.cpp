#include "StateSchemaManager.h"
#include "FrameworkLogger.h"
#include "JsonLoader.h"

namespace AmbientCharacterBehavior {

std::unordered_map<std::string, int32_t> StateSchemaManager::state_name_to_key;
std::unordered_map<int32_t, std::string> StateSchemaManager::state_key_to_name;

void StateSchemaManager::LoadStateSchema(const std::string &config_file_path)
{
    auto config_json = JsonLoader::LoadConfigFile(config_file_path);

    if (!config_json.has_value())
    {
        return;
    }

    if (config_json.contains("entity_states") && config_json["entity_states"].is_array())
    {
        for (const auto& state_json : config_json["entity_states"])
        {
            try
            {
                auto name = state_json.at("name").get<std::string>();
                auto key = state_json.at("key").get<int32_t>();

                if (IsValidForCreation(name, key))
                {
                    state_name_to_key[name] = key;
                    state_key_to_name[key] = name;

                    FrameworkLogger::LogInfo("Registered state in schema. Name: " + name + " Key: " + key,
                        "StateSchemaManager");

                }

            }
            catch (const json::exception& e) {
                FrameworkLogger::LogError("Failed to parse state schema from JSON: " +
                    std::string(e.what()),"StateSchemaManager");
            }
        }

        FrameworkLogger::LogInfo("Registered " + std::to_string(state_name_to_key.size()) + " state schemas",
            "StateSchemaManager");

    }
    else
    {
        FrameworkLogger::LogError("Config file missing 'entity_states' array",
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

bool StateSchemaManager::IsValidForCreation(const std::string &state_name, int32_t state_key)
{
    if (state_name_to_key.find(state_name) != state_name_to_key.end()) {
        FrameworkLogger::LogWarning("Duplicate state name: " + state_name, "StateSchemaManager");
        return false;
    }
    if (state_key_to_name.find(state_key) != state_key_to_name.end()) {
        FrameworkLogger::LogWarning("Duplicate state key: " + std::to_string(state_key) + " for state: " + state_name,
            "StateSchemaManager");

        return false;
    }

    return true;
}
}
