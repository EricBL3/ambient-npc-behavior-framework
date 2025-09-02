#pragma once
#include <nlohmann/json.hpp>
#include <optional>
#include <string>

namespace AmbientCharacterBehavior {
/**
 * @brief Handles reading and processing JSON configuration files
 */
class JsonLoader {

public:
    static std::optional<nlohmann::json> LoadConfigFile(const std::string& config_file_path);
};
}
