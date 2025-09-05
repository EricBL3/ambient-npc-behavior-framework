#pragma once
#include "interfaces/IJsonLoader.h"
#include <gmock/gmock.h>
#include <nlohmann/json.hpp>

namespace AmbientCharacterBehavior {
class MockJsonLoader : public IJsonLoader {
public:
    MOCK_METHOD(std::optional<nlohmann::json>, LoadConfigFileJson,
        (const std::string& config_file_path), (override));

    MOCK_METHOD(std::vector<EnvironmentalConditionDto>, ProcessEnvironmentalConditionsConfigFile,
        (const std::string& config_file_path), (override));

    MOCK_METHOD(std::vector<ActionDto>, ProcessActionsConfigFile,
        (const std::string& config_file_path), (override));

    MOCK_METHOD(std::vector<SequenceDto>, ProcessSequencesConfigFile,
        (const std::string& config_file_path), (override));

    MOCK_METHOD(std::optional<EntityDtoResult>, ProcessSingleEntityConfigFile,
        (const std::string& config_file_path), (override));
};
}
