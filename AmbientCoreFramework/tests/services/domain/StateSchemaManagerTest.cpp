#include <gtest/gtest.h>
#include "../../mocks/MockLogger.h"
#include "../../mocks/MockJsonLoader.h"
#include "services/domain/StateSchemaManager.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

using namespace AmbientCharacterBehavior;

class StateSchemaManagerTest : public testing::Test {
protected:

    std::unique_ptr<MockLogger> mock_logger;
    std::unique_ptr<MockJsonLoader> mock_json_loader;
    std::unique_ptr<StateSchemaManager> manager;

    void SetUp() override {
        mock_logger = std::make_unique<MockLogger>();
        mock_json_loader = std::make_unique<MockJsonLoader>();

        manager = std::make_unique<StateSchemaManager>(*mock_logger, *mock_json_loader);
    }

    // Helper methods to create test JSON data
    json CreateValidStateSchemaJson() {
        return json{
                        {"entity_states", {
                            {{"name", "HEALTH"}, {"key", 0}},
                            {{"name", "ENERGY"}, {"key", 1}},
                            {{"name", "MOOD"}, {"key", 2}}
                        }}
        };
    }

    json CreateDuplicateNameJson() {
        return json{
                        {"entity_states", {
                            {{"name", "HEALTH"}, {"key", 0}},
                            {{"name", "HEALTH"}, {"key", 1}}  // Duplicate name
                        }}
        };
    }

    json CreateDuplicateKeyJson() {
        return json{
                        {"entity_states", {
                            {{"name", "HEALTH"}, {"key", 0}},
                            {{"name", "ENERGY"}, {"key", 0}}  // Duplicate key
                        }}
        };
    }

    json CreateMissingFieldsJson() {
        return json{
                        {"entity_states", {
                            {{"name", "HEALTH"}},  // Missing key
                            {{"key", 1}}           // Missing name
                        }}
        };
    }

    json CreateEmptyStatesJson() {
        return json{
                        {"entity_states", json::array()}
        };
    }
};


// LOAD STATE SCHEMA TESTS
TEST_F(StateSchemaManagerTest, LoadStateSchema_ValidSchema_LoadsAllStates) {
    auto valid_json = CreateValidStateSchemaJson();

    EXPECT_CALL(*mock_json_loader, LoadConfigFileJson("valid_schema.json"))
        .WillOnce(testing::Return(valid_json));

    // Expect individual state registration logs
    EXPECT_CALL(*mock_logger, LogInfo(testing::HasSubstr("Registered state in schema. Name: HEALTH"),
                                       testing::Eq("StateSchemaManager")));
    EXPECT_CALL(*mock_logger, LogInfo(testing::HasSubstr("Registered state in schema. Name: ENERGY"),
                                       testing::Eq("StateSchemaManager")));
    EXPECT_CALL(*mock_logger, LogInfo(testing::HasSubstr("Registered state in schema. Name: MOOD"),
                                       testing::Eq("StateSchemaManager")));


    EXPECT_CALL(*mock_logger, LogInfo(testing::HasSubstr("Registered 3 state schemas"),
                                       testing::Eq("StateSchemaManager")));

    manager->LoadStateSchema("valid_schema.json");

    // Verify bidirectional mapping works
    EXPECT_EQ(manager->GetStateKey("HEALTH"), 0);
    EXPECT_EQ(manager->GetStateKey("ENERGY"), 1);
    EXPECT_EQ(manager->GetStateKey("MOOD"), 2);

    EXPECT_EQ(manager->GetStateName(0), "HEALTH");
    EXPECT_EQ(manager->GetStateName(1), "ENERGY");
    EXPECT_EQ(manager->GetStateName(2), "MOOD");
}

TEST_F(StateSchemaManagerTest, LoadStateSchema_EmptyValidSchema_LogsAppropriately) {
    auto empty_json = CreateEmptyStatesJson();

    EXPECT_CALL(*mock_json_loader, LoadConfigFileJson("empty_schema.json"))
        .WillOnce(testing::Return(empty_json));

    EXPECT_CALL(*mock_logger, LogInfo(testing::HasSubstr("Registered 0 state schemas"),
                                       testing::Eq("StateSchemaManager")));

    manager->LoadStateSchema("empty_schema.json");
}

TEST_F(StateSchemaManagerTest, LoadStateSchema_DuplicateNames_RejectsDuplicates) {
    auto duplicate_name_json = CreateDuplicateNameJson();

    EXPECT_CALL(*mock_json_loader, LoadConfigFileJson("duplicate_names.json"))
        .WillOnce(testing::Return(duplicate_name_json));

    // First HEALTH should succeed
    EXPECT_CALL(*mock_logger, LogInfo(testing::HasSubstr("Registered state in schema. Name: HEALTH"),
                                       testing::Eq("StateSchemaManager")));

    // Second HEALTH should be rejected
    EXPECT_CALL(*mock_logger, LogWarning(testing::HasSubstr("Duplicate state name: HEALTH"),
                                          testing::Eq("StateSchemaManager")));

    // Final count should be 1, not 2
    EXPECT_CALL(*mock_logger, LogInfo(testing::HasSubstr("Registered 1 state schemas"),
                                       testing::Eq("StateSchemaManager")));

    manager->LoadStateSchema("duplicate_names.json");

    // Verify only first mapping exists
    EXPECT_EQ(manager->GetStateKey("HEALTH"), 0);
    EXPECT_EQ(manager->GetStateName(0), "HEALTH");
    EXPECT_THROW(manager->GetStateName(1), std::out_of_range);
}

TEST_F(StateSchemaManagerTest, LoadStateSchema_DuplicateKeys_RejectsDuplicates) {
    auto duplicate_key_json = CreateDuplicateKeyJson();

    EXPECT_CALL(*mock_json_loader, LoadConfigFileJson("duplicate_keys.json"))
        .WillOnce(testing::Return(duplicate_key_json));

    // First entry should succeed
    EXPECT_CALL(*mock_logger, LogInfo(testing::HasSubstr("Registered state in schema. Name: HEALTH"),
                                       testing::Eq("StateSchemaManager")));

    // Second entry should be rejected
    EXPECT_CALL(*mock_logger, LogWarning(testing::HasSubstr("Duplicate state key: 0 for state: ENERGY"),
                                          testing::Eq("StateSchemaManager")));

    EXPECT_CALL(*mock_logger, LogInfo(testing::HasSubstr("Registered 1 state schemas"),
                                       testing::Eq("StateSchemaManager")));

    manager->LoadStateSchema("duplicate_keys.json");

    // Verify only first mapping exists
    EXPECT_EQ(manager->GetStateKey("HEALTH"), 0);
    EXPECT_EQ(manager->GetStateName(0), "HEALTH");
    EXPECT_THROW(manager->GetStateKey("ENERGY"), std::out_of_range);
}

TEST_F(StateSchemaManagerTest, LoadStateSchema_JsonLoaderFails_ReturnsEarly) {
    EXPECT_CALL(*mock_json_loader, LoadConfigFileJson("bad_file.json"))
        .WillOnce(testing::Return(std::nullopt));

    // Should not log anything since it returns early
    EXPECT_CALL(*mock_logger, LogInfo(testing::_, testing::_))
        .Times(0);
    EXPECT_CALL(*mock_logger, LogError(testing::_, testing::_))
        .Times(0);

    manager->LoadStateSchema("bad_file.json");

    // No states should be loaded
    EXPECT_THROW(manager->GetStateKey("HEALTH"), std::out_of_range);
}

TEST_F(StateSchemaManagerTest, LoadStateSchema_MissingEntityStatesArray_LogsError) {
    json json_without_states = {{"other_field", "value"}};

    EXPECT_CALL(*mock_json_loader, LoadConfigFileJson("no_states.json"))
        .WillOnce(testing::Return(json_without_states));

    EXPECT_CALL(*mock_logger, LogError(testing::HasSubstr("Config file missing 'entity_states' array"),
                                        testing::Eq("StateSchemaManager")));

    manager->LoadStateSchema("no_states.json");
}

TEST_F(StateSchemaManagerTest, LoadStateSchema_MalformedStateEntries_LogsErrorsContinuesProcessing) {
    auto malformed_json = CreateMissingFieldsJson();

    EXPECT_CALL(*mock_json_loader, LoadConfigFileJson("malformed.json"))
        .WillOnce(testing::Return(malformed_json));

    // Should log errors for malformed entries
    EXPECT_CALL(*mock_logger, LogError(testing::HasSubstr("Failed to parse state schema from JSON"),
                                        testing::Eq("StateSchemaManager")))
        .Times(2); // Two malformed entries

    // Should still log final count (0 in this case)
    EXPECT_CALL(*mock_logger, LogInfo(testing::HasSubstr("Registered 0 state schemas"),
                                       testing::Eq("StateSchemaManager")));

    manager->LoadStateSchema("malformed.json");
}

// INVALID LOAD TESTS
TEST_F(StateSchemaManagerTest, LoadStateSchema_EmptyStateName_RejectsEntry) {
    json empty_name_json = {
        {"entity_states", {
            {{"name", ""}, {"key", 0}},
            {{"name", "VALID_STATE"}, {"key", 1}}  // This should still work
        }}
    };

    EXPECT_CALL(*mock_json_loader, LoadConfigFileJson("empty_name.json"))
        .WillOnce(testing::Return(empty_name_json));

    // Should log warning for empty name
    EXPECT_CALL(*mock_logger, LogWarning(testing::HasSubstr("State name cannot be empty for key: 0"),
                                          testing::Eq("StateSchemaManager")));

    // Should log success for valid state
    EXPECT_CALL(*mock_logger, LogInfo(testing::HasSubstr("Registered state in schema. Name: VALID_STATE"),
                                       testing::Eq("StateSchemaManager")));

    // Final count should be 1, not 2
    EXPECT_CALL(*mock_logger, LogInfo(testing::HasSubstr("Registered 1 state schemas"),
                                       testing::Eq("StateSchemaManager")));

    manager->LoadStateSchema("empty_name.json");

    // Empty name should not be accessible
    EXPECT_THROW(manager->GetStateKey(""), std::out_of_range);
    EXPECT_THROW(manager->GetStateName(0), std::out_of_range);

    // Valid state should be accessible
    EXPECT_EQ(manager->GetStateKey("VALID_STATE"), 1);
    EXPECT_EQ(manager->GetStateName(1), "VALID_STATE");
}

TEST_F(StateSchemaManagerTest, LoadStateSchema_WhitespaceOnlyStateName_RejectsEntry) {
    json whitespace_name_json = {
        {"entity_states", {
            {{"name", "   "}, {"key", 0}},  // Only whitespace
            {{"name", "VALID_STATE"}, {"key", 1}}
        }}
    };

    EXPECT_CALL(*mock_json_loader, LoadConfigFileJson("whitespace_name.json"))
        .WillOnce(testing::Return(whitespace_name_json));

    // Should log warning for whitespace-only name (you might want to trim and validate)
    EXPECT_CALL(*mock_logger, LogInfo(testing::HasSubstr("Registered state in schema. Name:    "),
                                       testing::Eq("StateSchemaManager")));
    EXPECT_CALL(*mock_logger, LogInfo(testing::HasSubstr("Registered state in schema. Name: VALID_STATE"),
                                       testing::Eq("StateSchemaManager")));

    EXPECT_CALL(*mock_logger, LogInfo(testing::HasSubstr("Registered 2 state schemas"),
                                       testing::Eq("StateSchemaManager")));

    manager->LoadStateSchema("whitespace_name.json");

    // Whitespace name should be accessible (unless you add trimming logic)
    EXPECT_EQ(manager->GetStateKey("   "), 0);
    EXPECT_EQ(manager->GetStateName(0), "   ");
}

TEST_F(StateSchemaManagerTest, LoadStateSchema_NegativeKeys_RejectsEntry) {
    json negative_key_json = {
        {"entity_states", {
                {{"name", "NEGATIVE_STATE"}, {"key", -1}},
                {{"name", "VALID_STATE"}, {"key", 0}}  // This should still work
        }}
    };

    EXPECT_CALL(*mock_json_loader, LoadConfigFileJson("negative.json"))
        .WillOnce(testing::Return(negative_key_json));

    // Should log warning for negative key
    EXPECT_CALL(*mock_logger, LogWarning(testing::HasSubstr("State key cannot be negative, got: -1 for state: NEGATIVE_STATE"),
                                          testing::Eq("StateSchemaManager")));

    // Should log success for valid state
    EXPECT_CALL(*mock_logger, LogInfo(testing::HasSubstr("Registered state in schema. Name: VALID_STATE"),
                                       testing::Eq("StateSchemaManager")));

    // Final count should be 1, not 2
    EXPECT_CALL(*mock_logger, LogInfo(testing::HasSubstr("Registered 1 state schemas"),
                                       testing::Eq("StateSchemaManager")));

    manager->LoadStateSchema("negative.json");

    // Negative key should not be accessible
    EXPECT_THROW(manager->GetStateKey("NEGATIVE_STATE"), std::out_of_range);
    EXPECT_THROW(manager->GetStateName(-1), std::out_of_range);

    // Valid state should be accessible
    EXPECT_EQ(manager->GetStateKey("VALID_STATE"), 0);
    EXPECT_EQ(manager->GetStateName(0), "VALID_STATE");
}

// LOOKUP OPERATIONS TESTS
TEST_F(StateSchemaManagerTest, GetStateKey_ValidName_ReturnsCorrectKey) {
    // First load a schema
    auto valid_json = CreateValidStateSchemaJson();
    EXPECT_CALL(*mock_json_loader, LoadConfigFileJson(testing::_))
        .WillOnce(testing::Return(valid_json));
    EXPECT_CALL(*mock_logger, LogInfo(testing::_, testing::_))
        .Times(testing::AnyNumber());

    manager->LoadStateSchema("schema.json");

    // Test lookups
    EXPECT_EQ(manager->GetStateKey("HEALTH"), 0);
    EXPECT_EQ(manager->GetStateKey("ENERGY"), 1);
    EXPECT_EQ(manager->GetStateKey("MOOD"), 2);
}

TEST_F(StateSchemaManagerTest, GetStateKey_InvalidName_ThrowsException) {
    EXPECT_THROW(manager->GetStateKey("NONEXISTENT"), std::out_of_range);
}

TEST_F(StateSchemaManagerTest, GetStateName_ValidKey_ReturnsCorrectName) {
    // First load a schema
    auto valid_json = CreateValidStateSchemaJson();
    EXPECT_CALL(*mock_json_loader, LoadConfigFileJson(testing::_))
        .WillOnce(testing::Return(valid_json));
    EXPECT_CALL(*mock_logger, LogInfo(testing::_, testing::_))
        .Times(testing::AnyNumber());

    manager->LoadStateSchema("schema.json");

    // Test lookups
    EXPECT_EQ(manager->GetStateName(0), "HEALTH");
    EXPECT_EQ(manager->GetStateName(1), "ENERGY");
    EXPECT_EQ(manager->GetStateName(2), "MOOD");
}

TEST_F(StateSchemaManagerTest, GetStateName_InvalidKey_ThrowsException) {
    EXPECT_THROW(manager->GetStateName(999), std::out_of_range);
}