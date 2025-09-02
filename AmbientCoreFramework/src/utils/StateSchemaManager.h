#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>

namespace AmbientCharacterBehavior {
/**
 * @brief Handles the mappings of state names and keys.
 */
class StateSchemaManager {
    static std::unordered_map<std::string, int32_t> state_name_to_key;
    static std::unordered_map<int32_t, std::string> state_key_to_name;

public:
    static void LoadStateSchema(const std::string& config_file_path);

    static int32_t GetStateKey(const std::string& state_name);

    static std::string GetStateName(int32_t state_key);

private:

    static bool IsValidForCreation(const std::string& state_name, int32_t state_key);
};
}
