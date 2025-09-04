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
    const std::string valid_actions_file = "test_valid_actions.json";
    const std::string missing_fields_actions_file = "test_missing_actions_fields.json";
    const std::string invalid_json_file = "test_invalid.json";
    const std::string nonexistent_file = "nonexistent_file.json";

    void SetUp() override
    {
        mock_logger = std::make_unique<MockLogger>();

        CreateValidActionsFile();
        CreateInvalidJsonFile();
        CreateMissingFieldsFiles();
    }

    void TearDown() override
    {
        RemoveTestFiles();
    }

private:
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

    void CreateInvalidJsonFile() {
        std::ofstream file(invalid_json_file);
        file << "{ invalid json content }";
    }

    void CreateMissingFieldsFiles() {
        std::ofstream file(missing_fields_actions_file);
        file << R"({
            "actions": [
                {
                    "action_id": 1
                }
            ]
        })";
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

TEST_F(JsonLoaderTest, ProcessActionsConfigFile_MissingFields_LogsErrorAndReturnsPartial) {
    JsonLoader loader(*mock_logger);

    EXPECT_CALL(*mock_logger, LogError(
        testing::HasSubstr("Failed to parse element"),
        testing::Eq("JsonLoader")))
        .Times(testing::AtLeast(1));

    auto result = loader.ProcessActionsConfigFile(missing_fields_actions_file);
    // Should return empty since the one element failed to parse
    EXPECT_TRUE(result.empty());
}

// =============================================================================
// PROCESS SEQUENCES CONFIG FILE TESTS
// =============================================================================



// =============================================================================
// PROCESS ENVIRONMENTAL CONDITIONS CONFIG FILE TESTS
// =============================================================================



// =============================================================================
// PROCESS ENTITY CONFIG FILE TESTS
// =============================================================================