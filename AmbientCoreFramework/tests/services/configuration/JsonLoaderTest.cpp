#pragma once
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

#include "../../mocks/MockLogger.h"
#include "services/configuration/JsonLoader.h"

namespace AmbientCharacterBehavior {
class MockLogger;
class JsonLoader;
}

using namespace AmbientCharacterBehavior;

class JsonLoaderTest : public testing::Test {
protected:
    std::unique_ptr<MockLogger> mock_logger;
    const std::string valid_environmental_conditions_file = "test_valid_environmental_conditions.json";
    const std::string missing_fields_environmental_conditions_file = "test_missing_environmental_conditions_fields.json";
    const std::string valid_actions_file = "test_valid_actions.json";
    const std::string missing_fields_actions_file = "test_missing_actions_fields.json";
    const std::string valid_sequences_file = "test_valid_sequences.json";
    const std::string missing_fields_sequences_file = "test_missing_sequences_fields.json";
    const std::string valid_framework_entity_file = "test_valid_framework_entity.json";
    const std::string missing_fields_framework_entity_file = "test_missing_framework_entity_fields.json";
    const std::string valid_behavioral_entity_file = "test_valid_behavioral_entity.json";
    const std::string missing_fields_behavioral_entity_file = "test_missing_behavioral_entity_fields.json";

    const std::string invalid_json_file = "test_invalid.json";
    const std::string nonexistent_file = "nonexistent_file.json";

    void SetUp() override
    {
        mock_logger = std::make_unique<MockLogger>();

        CreateValidFiles();
        CreateInvalidJsonFile();
        CreateMissingFieldsFiles();
    }

    void TearDown() override
    {
        RemoveTestFiles();
    }

private:

    void CreateValidFiles()
    {
        CreateValidEnvironmentalConditionsFile();
        CreateValidActionsFile();
        CreateValidSequencesFile();
        CreateValidFrameworkEntityFile();
        CreateValidBehavioralEntityFile();
    }

    void CreateValidEnvironmentalConditionsFile()
    {
        std::ofstream file(valid_environmental_conditions_file);
        file << R"({
            "environmental_conditions": [
                {
                    "condition_key": 0,
                    "name": "test_condition",
                    "update_frequency_ms": 30000
                }
            ]
        })";
    }

    void CreateValidActionsFile() {
        std::ofstream file(valid_actions_file);
        file << R"({
            "actions": [
                {
                    "action_id": 1,
                    "action_name": "test_action",
                    "preconditions": [],
                    "immediate_effects": [],
                    "completion_effects": [],
                    "max_duration_ms": 5000,
                    "interruption_behavior_name": "NON_RESUMABLE"
                }
            ]
        })";
    }

    void CreateValidSequencesFile()
    {
        std::ofstream file(valid_sequences_file);

        file << R"({
            "sequences": [
                {
                    "sequence_id": 0,
                    "sequence_name": "test_sequence",
                    "entry_point_node_id": 0,
                    "nodes": [],
                    "transitions": []
                }
            ]
        })";
    }

    void CreateValidFrameworkEntityFile()
    {
        std::ofstream file(valid_framework_entity_file);

        file << R"({
            "entities": [
                {
                    "entity_type": "FRAMEWORK",
                    "entity": {
                        "entity_id": 0,
                        "entity_name": "test_entity",
                        "accepted_actions_ids": [],
                        "initial_state": {}
                    }
                }
            ]
        })";
    }

    void CreateValidBehavioralEntityFile()
    {
        std::ofstream file(valid_behavioral_entity_file);

        file << R"({
            "entities": [
                {
                    "entity_type": "BEHAVIORAL",
                    "entity": {
                        "entity_id": 0,
                        "entity_name": "test_entity",
                        "accepted_actions_ids": [],
                        "initial_state": {},
                        "main_sequence_id": 1,
                        "fallback_sequences": [],
                        "interruption_handlers": {},
                        "memory_limits": {
                            "max_transition_memories": 15,
                            "max_action_memories": 10,
                            "max_interruption_memories": 5
                        }
                    }
                }
            ]
        })";
    }

    void CreateMissingFieldsFiles() {
        std::ofstream file(missing_fields_environmental_conditions_file);
        file << R"({
            "environmental_conditions": [
                {
                    "condition_key": 1
                }
            ]
        })";

        file = std::ofstream(missing_fields_actions_file);
        file << R"({
            "actions": [
                {
                    "action_id": 1
                }
            ]
        })";

        file = std::ofstream(missing_fields_sequences_file);
        file << R"({
            "sequences": [
                {
                    "sequence_id": 1
                }
            ]
        })";

        file = std::ofstream(missing_fields_framework_entity_file);
        file << R"({
            "entities": [
                {
                    "entity_type": "FRAMEWORK",
                    "entity": {
                        "entity_id": 0
                    }
                }
            ]
        })";

        file = std::ofstream(missing_fields_behavioral_entity_file);
        file << R"({
            "entities": [
                {
                    "entity_type": "BEHAVIORAL",
                    "entity": {
                        "entity_id": 0,
                        "entity_name": "name",
                        "accepted_actions_ids": [],
                        "initial_state": {
                            "STATE_NAME": 0
                        }
                    }
                }
            ]
        })";
    }

    void CreateInvalidJsonFile() {
        std::ofstream file(invalid_json_file);
        file << "{ invalid json content }";
    }

    void RemoveTestFiles() {
        std::filesystem::remove(valid_actions_file);
        std::filesystem::remove(invalid_json_file);
        std::filesystem::remove(missing_fields_actions_file);
    }
};

// =============================================================================
// CONSTRUCTOR TESTS
// =============================================================================

TEST_F(JsonLoaderTest, Constructor_ValidLogger_CreatesJsonLoader) {
    EXPECT_NO_THROW(JsonLoader loader(*mock_logger));
}

// =============================================================================
// PROCESS ENVIRONMENTAL CONDITIONS CONFIG FILE TESTS
// =============================================================================

TEST_F(JsonLoaderTest, ProcessEnvironmentalConditionsConfigFile_ValidFile_ReturnsEnvironmentalConditions) {
    JsonLoader loader(*mock_logger);

    // Should not log any errors for valid file
    EXPECT_CALL(*mock_logger, LogError(testing::_, testing::_))
        .Times(0);

    auto result = loader.ProcessEnvironmentalConditionsConfigFile(valid_environmental_conditions_file);

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].condition_key, 0);
    EXPECT_EQ(result[0].name, "test_condition");
    EXPECT_EQ(result[0].update_frequency_ms, 30000);
}

TEST_F(JsonLoaderTest, ProcessEnvironmentalConditionsConfigFile_NonexistentFile_LogsErrorAndReturnsEmpty) {
    JsonLoader loader(*mock_logger);

    EXPECT_CALL(*mock_logger, LogError(
        testing::HasSubstr("Failed to open config file"),
        testing::Eq("JsonLoader")))
        .Times(1);

    auto result = loader.ProcessEnvironmentalConditionsConfigFile(nonexistent_file);
    EXPECT_TRUE(result.empty());
}

TEST_F(JsonLoaderTest, ProcessEnvironmentalConditionsConfigFile_InvalidJson_LogsErrorAndReturnsEmpty) {
    JsonLoader loader(*mock_logger);

    EXPECT_CALL(*mock_logger, LogError(
        testing::HasSubstr("JSON parsing error"),
        testing::Eq("JsonLoader")))
        .Times(1);

    auto result = loader.ProcessEnvironmentalConditionsConfigFile(invalid_json_file);
    EXPECT_TRUE(result.empty());
}

TEST_F(JsonLoaderTest, ProcessEnvironmentalConditionsConfigFile_MissingFields_LogsErrorAndReturnsEmpty) {
    JsonLoader loader(*mock_logger);

    EXPECT_CALL(*mock_logger, LogError(
        testing::HasSubstr("Failed to parse element"),
        testing::Eq("JsonLoader")))
        .Times(testing::AtLeast(1));

    auto result = loader.ProcessEnvironmentalConditionsConfigFile(missing_fields_environmental_conditions_file);
    // Should return empty since it failed to parse
    EXPECT_TRUE(result.empty());
}

// =============================================================================
// PROCESS ACTIONS CONFIG FILE TESTS
// =============================================================================

TEST_F(JsonLoaderTest, ProcessActionsConfigFile_ValidFile_ReturnsActions) {
    JsonLoader loader(*mock_logger);

    // Should not log any errors for valid file
    EXPECT_CALL(*mock_logger, LogError(testing::_, testing::_))
        .Times(0);

    auto result = loader.ProcessActionsConfigFile(valid_actions_file);

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].action_id, 1);
    EXPECT_EQ(result[0].action_name, "test_action");
    EXPECT_EQ(result[0].max_duration_ms, 5000);
}

TEST_F(JsonLoaderTest, ProcessActionsConfigFile_NonexistentFile_LogsErrorAndReturnsEmpty) {
    JsonLoader loader(*mock_logger);

    EXPECT_CALL(*mock_logger, LogError(
        testing::HasSubstr("Failed to open config file"),
        testing::Eq("JsonLoader")))
        .Times(1);

    auto result = loader.ProcessActionsConfigFile(nonexistent_file);
    EXPECT_TRUE(result.empty());
}

TEST_F(JsonLoaderTest, ProcessActionsConfigFile_InvalidJson_LogsErrorAndReturnsEmpty) {
    JsonLoader loader(*mock_logger);

    EXPECT_CALL(*mock_logger, LogError(
        testing::HasSubstr("JSON parsing error"),
        testing::Eq("JsonLoader")))
        .Times(1);

    auto result = loader.ProcessActionsConfigFile(invalid_json_file);
    EXPECT_TRUE(result.empty());
}

TEST_F(JsonLoaderTest, ProcessActionsConfigFile_MissingFields_LogsErrorAndReturnsEmpty) {
    JsonLoader loader(*mock_logger);

    EXPECT_CALL(*mock_logger, LogError(
        testing::HasSubstr("Failed to parse element"),
        testing::Eq("JsonLoader")))
        .Times(testing::AtLeast(1));

    auto result = loader.ProcessActionsConfigFile(missing_fields_actions_file);
    // Should return empty since it failed to parse
    EXPECT_TRUE(result.empty());
}

// =============================================================================
// PROCESS SEQUENCES CONFIG FILE TESTS
// =============================================================================

TEST_F(JsonLoaderTest, ProcessSequencesConfigFile_ValidFile_ReturnsSequences) {
    JsonLoader loader(*mock_logger);

    // Should not log any errors for valid file
    EXPECT_CALL(*mock_logger, LogError(testing::_, testing::_))
        .Times(0);

    auto result = loader.ProcessSequencesConfigFile(valid_sequences_file);

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].sequence_id, 0);
    EXPECT_EQ(result[0].sequence_name, "test_sequence");
    EXPECT_EQ(result[0].entry_point_node_id, 0);
}

TEST_F(JsonLoaderTest, ProcessSequencesConfigFile_NonexistentFile_LogsErrorAndReturnsEmpty) {
    JsonLoader loader(*mock_logger);

    EXPECT_CALL(*mock_logger, LogError(
        testing::HasSubstr("Failed to open config file"),
        testing::Eq("JsonLoader")))
        .Times(1);

    auto result = loader.ProcessSequencesConfigFile(nonexistent_file);
    EXPECT_TRUE(result.empty());
}

TEST_F(JsonLoaderTest, ProcessSequencesConfigFile_InvalidJson_LogsErrorAndReturnsEmpty) {
    JsonLoader loader(*mock_logger);

    EXPECT_CALL(*mock_logger, LogError(
        testing::HasSubstr("JSON parsing error"),
        testing::Eq("JsonLoader")))
        .Times(1);

    auto result = loader.ProcessSequencesConfigFile(invalid_json_file);
    EXPECT_TRUE(result.empty());
}

TEST_F(JsonLoaderTest, ProcessSequencesConfigFile_MissingFields_LogsErrorAndReturnsEmpty) {
    JsonLoader loader(*mock_logger);

    EXPECT_CALL(*mock_logger, LogError(
        testing::HasSubstr("Failed to parse element"),
        testing::Eq("JsonLoader")))
        .Times(testing::AtLeast(1));

    auto result = loader.ProcessSequencesConfigFile(missing_fields_sequences_file);
    // Should return empty since it failed to parse
    EXPECT_TRUE(result.empty());
}

// =============================================================================
// PROCESS ENTITY CONFIG FILE TESTS
// =============================================================================

TEST_F(JsonLoaderTest, ProcessSingleEntityConfigFile_ValidFrameworkFile_ReturnsEntity) {
    JsonLoader loader(*mock_logger);

    // Should not log any errors for valid file
    EXPECT_CALL(*mock_logger, LogError(testing::_, testing::_))
        .Times(0);

    auto result = loader.ProcessSingleEntityConfigFile(valid_framework_entity_file);

    EXPECT_TRUE(result);
    EXPECT_EQ(result->entity_type, "FRAMEWORK");
    EXPECT_TRUE(result->framework_entity);
    EXPECT_EQ(std::nullopt, result->behavioral_entity);
    EXPECT_EQ(result->framework_entity->entity_id, 0);
    EXPECT_EQ(result->framework_entity->entity_name, "test_entity");
}

TEST_F(JsonLoaderTest, ProcessSingleEntityConfigFile_ValidBehavioralFile_ReturnsEntity) {
    JsonLoader loader(*mock_logger);

    // Should not log any errors for valid file
    EXPECT_CALL(*mock_logger, LogError(testing::_, testing::_))
        .Times(0);

    auto result = loader.ProcessSingleEntityConfigFile(valid_behavioral_entity_file);

    EXPECT_TRUE(result);
    EXPECT_EQ(result->entity_type, "BEHAVIORAL");
    EXPECT_TRUE(result->behavioral_entity.has_value());
    EXPECT_EQ(std::nullopt, result->framework_entity);
    EXPECT_EQ(result->behavioral_entity->base_properties.entity_id, 0);
    EXPECT_EQ(result->behavioral_entity->base_properties.entity_name, "test_entity");
}

TEST_F(JsonLoaderTest, ProcessSingleEntityConfigFile_MissingFrameworkFields_LogsErrorAndReturnsEmpty) {
    JsonLoader loader(*mock_logger);

    EXPECT_CALL(*mock_logger, LogError(
        testing::HasSubstr("Failed to parse element"),
        testing::Eq("JsonLoader")))
        .Times(testing::AtLeast(1));

    auto result = loader.ProcessSingleEntityConfigFile(missing_fields_framework_entity_file);
    EXPECT_EQ(std::nullopt, result);
}

TEST_F(JsonLoaderTest, ProcessSingleEntityConfigFile_MissingBehavioralFields_LogsErrorAndReturnsEmpty) {
    JsonLoader loader(*mock_logger);

    EXPECT_CALL(*mock_logger, LogError(
        testing::HasSubstr("Failed to parse element"),
        testing::Eq("JsonLoader")))
        .Times(testing::AtLeast(1));

    auto result = loader.ProcessSingleEntityConfigFile(missing_fields_behavioral_entity_file);
    EXPECT_EQ(std::nullopt, result);
}

TEST_F(JsonLoaderTest, ProcessSingleEntityConfigFile_NonexistentFile_LogsErrorAndReturnsEmpty) {
    JsonLoader loader(*mock_logger);

    EXPECT_CALL(*mock_logger, LogError(
        testing::HasSubstr("Failed to open config file"),
        testing::Eq("JsonLoader")))
        .Times(1);

    auto result = loader.ProcessSingleEntityConfigFile(nonexistent_file);
    EXPECT_EQ(std::nullopt, result);
}

TEST_F(JsonLoaderTest, ProcessSingleEntityConfigFile_InvalidJson_LogsErrorAndReturnsEmpty) {
    JsonLoader loader(*mock_logger);

    EXPECT_CALL(*mock_logger, LogError(
        testing::HasSubstr("JSON parsing error"),
        testing::Eq("JsonLoader")))
        .Times(1);

    auto result = loader.ProcessSingleEntityConfigFile(invalid_json_file);
    EXPECT_EQ(std::nullopt, result);
}