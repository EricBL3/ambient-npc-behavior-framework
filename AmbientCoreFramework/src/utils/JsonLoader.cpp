#include "JsonLoader.h"

#include <fstream>
#include "FrameworkLogger.h"

using json = nlohmann::json;

namespace AmbientCharacterBehavior {
std::optional<nlohmann::json> JsonLoader::LoadConfigFile(const std::string &config_file_path)
{
    try
    {
        std::ifstream config_file(config_file_path);
        if (!config_file.is_open())
        {
            FrameworkLogger::LogError("Failed to open config file: " + config_file_path,
                "JsonLoader");
            return std::nullopt;
        }

        json config_json;
        config_file >> config_json;
        return config_json;
    }
    catch (const json::exception& e)
    {
        FrameworkLogger::LogError("JSON parsing error in config file: " + std::string(e.what()),
            "JsonLoader");

        return std::nullopt;
    }
}
} // AmbientCharacterBehavior