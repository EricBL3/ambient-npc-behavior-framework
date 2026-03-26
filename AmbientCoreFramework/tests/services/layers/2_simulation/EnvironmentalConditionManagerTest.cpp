#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include "../../mocks/MockEnvironmentalConditionProvider.h"
#include "../../mocks/MockJsonLoader.h"
#include "../../mocks/MockLogger.h"
#include "../../mocks/MockTimeManager.h"
#include "services/layers/2_simulation/EnvironmentalConditionManager.h"

using namespace AmbientCharacterBehavior;

class EnvironmentalConditionManagerTest : public testing::Test {
protected:
    std::unique_ptr<MockLogger> mock_logger;
    std::unique_ptr<MockTimeManager> mock_time_manager;
    std::unique_ptr<MockJsonLoader> mock_json_loader;
    std::unique_ptr<MockEnvironmentalConditionProvider> mock_provider;
    std::unique_ptr<EnvironmentalConditionManager> manager;

    void SetUp() override
    {
        mock_logger = std::make_unique<MockLogger>();
        mock_time_manager = std::make_unique<MockTimeManager>();
        mock_json_loader = std::make_unique<MockJsonLoader>();
        mock_provider = std::make_unique<MockEnvironmentalConditionProvider>();

        manager = std::make_unique<EnvironmentalConditionManager>(*mock_logger, *mock_time_manager,
        *mock_json_loader, *mock_provider);
    }

    // Helper methods to create test data
    std::vector<EnvironmentalConditionDto> CreateValidConditions() {
        return {
                {1, "Weather", 5000},  // Updates every 5 seconds
                {2, "TimeOfDay", 1000}, // Updates every 1 second
                {3, "Temperature", 10000} // Updates every 10 seconds
        };
    }

    std::vector<EnvironmentalConditionDto> CreateDuplicateKeyConditions() {
        return {
                {1, "Weather", 5000},
                {1, "DuplicateWeather", 3000}  // Same key as first
        };
    }
};

TEST_F(EnvironmentalConditionManagerTest, Constructor_ValidServices_CreatesEnvironmentalConditionManager) {
    EXPECT_NO_THROW(EnvironmentalConditionManager env_condition_manager(*mock_logger, *mock_time_manager,
        *mock_json_loader, *mock_provider));
}

// CONFIGURATION LOADING TESTS

TEST_F(EnvironmentalConditionManagerTest, RegisterEnvironmentalConditions_ValidConfig_LoadsSuccessfully) {
    auto conditions = CreateValidConditions();

    EXPECT_CALL(*mock_json_loader, ProcessEnvironmentalConditionsConfigFile("valid_config.json"))
        .WillOnce(testing::Return(conditions));

    EXPECT_CALL(*mock_logger, LogInfo(testing::HasSubstr("Registered environmental condition"),
                                       testing::Eq("EnvironmentalConditionManager"))).Times(3);

    EXPECT_CALL(*mock_logger, LogInfo(testing::HasSubstr("Successfully registered 3"),
                                       testing::Eq("EnvironmentalConditionManager")));

    EXPECT_NO_THROW(manager->RegisterEnvironmentalConditions("valid_config.json"));
}

TEST_F(EnvironmentalConditionManagerTest, RegisterEnvironmentalConditions_EmptyConfig_LogsWarning) {
    std::vector<EnvironmentalConditionDto> empty_conditions;

    EXPECT_CALL(*mock_json_loader, ProcessEnvironmentalConditionsConfigFile("empty_config.json"))
        .WillOnce(testing::Return(empty_conditions));

    EXPECT_CALL(*mock_logger, LogWarning(testing::HasSubstr("No valid environmental conditions"),
                                          testing::_));

    manager->RegisterEnvironmentalConditions("empty_config.json");
}

TEST_F(EnvironmentalConditionManagerTest, RegisterEnvironmentalConditions_JsonLoaderThrows_LogsError) {
    EXPECT_CALL(*mock_json_loader, ProcessEnvironmentalConditionsConfigFile("bad_config.json"))
        .WillOnce(testing::Throw(std::runtime_error("JSON parsing failed")));

    EXPECT_CALL(*mock_logger, LogError(testing::HasSubstr("Unexpected error loading"),
                                        testing::_));

    manager->RegisterEnvironmentalConditions("bad_config.json");
}

// DUPLICATE KEY VALIDATION TESTS
TEST_F(EnvironmentalConditionManagerTest, RegisterEnvironmentalConditions_DuplicateKeys_RejectsDuplicate) {
    auto duplicate_conditions = CreateDuplicateKeyConditions();

    EXPECT_CALL(*mock_json_loader, ProcessEnvironmentalConditionsConfigFile("duplicate_config.json"))
        .WillOnce(testing::Return(duplicate_conditions));

    EXPECT_CALL(*mock_logger, LogError(testing::HasSubstr("Duplicate condition_key: 1"),
                                        testing::_));

    EXPECT_CALL(*mock_logger, LogInfo(testing::HasSubstr("Registered environmental condition: Weather"),
                                       testing::_));

    // Final count should be 1, not 2
    EXPECT_CALL(*mock_logger, LogInfo(testing::HasSubstr("Successfully registered 1"),
                                       testing::_));

    manager->RegisterEnvironmentalConditions("duplicate_config.json");
}

// UPDATE ENVIRONMENTAL CONDITION TESTS
TEST_F(EnvironmentalConditionManagerTest, UpdateEnvironmentalCondition_ValidKey_UpdatesSuccessfully) {
    auto conditions = CreateValidConditions();
    EXPECT_CALL(*mock_json_loader, ProcessEnvironmentalConditionsConfigFile(testing::_))
        .WillOnce(testing::Return(conditions));
    manager->RegisterEnvironmentalConditions("config.json");

    EXPECT_CALL(*mock_time_manager, GetCurrentTime())
        .WillOnce(testing::Return(12345));
    EXPECT_CALL(*mock_provider, QueryEnvironmentalCondition(1))
        .WillOnce(testing::Return(100)); // Sunny weather

    EXPECT_NO_THROW(manager->UpdateEnvironmentalCondition(1));
}

TEST_F(EnvironmentalConditionManagerTest, UpdateEnvironmentalCondition_InvalidKey_ThrowsException) {
    EXPECT_THROW(manager->UpdateEnvironmentalCondition(999), std::out_of_range);
}

TEST_F(EnvironmentalConditionManagerTest, UpdateEnvironmentalCondition_ProviderThrows_LogsError) {
    auto conditions = CreateValidConditions();
    EXPECT_CALL(*mock_json_loader, ProcessEnvironmentalConditionsConfigFile(testing::_))
        .WillOnce(testing::Return(conditions));
    manager->RegisterEnvironmentalConditions("config.json");

    EXPECT_CALL(*mock_provider, QueryEnvironmentalCondition(1))
        .WillOnce(testing::Throw(std::runtime_error("Provider connection failed")));

    EXPECT_CALL(*mock_logger, LogError(testing::HasSubstr("Provider connection failed"),
                                        testing::_));

    EXPECT_NO_THROW(manager->UpdateEnvironmentalCondition(1));
}

// GET ENVIRONMENTAL CONDITION TESTS
TEST_F(EnvironmentalConditionManagerTest, GetEnvironmentalConditionValue_ConditionNeedsUpdate_TriggersUpdate) {
    // Register condition
    auto conditions = CreateValidConditions();
    EXPECT_CALL(*mock_json_loader, ProcessEnvironmentalConditionsConfigFile(testing::_))
        .WillOnce(testing::Return(conditions));
    manager->RegisterEnvironmentalConditions("config.json");

    EXPECT_CALL(*mock_provider, QueryEnvironmentalCondition(1))
        .WillOnce(testing::Return(75));
    EXPECT_CALL(*mock_time_manager, GetCurrentTime())
        .WillRepeatedly(testing::Return(10000));

    int32_t value = manager->GetEnvironmentalConditionValue(1);
    EXPECT_EQ(value, 75);
}

TEST_F(EnvironmentalConditionManagerTest, GetEnvironmentalConditionValue_InvalidKey_ThrowsException) {
    EXPECT_THROW(manager->GetEnvironmentalConditionValue(999), std::out_of_range);
}

// INTEGRATION TESTS
TEST_F(EnvironmentalConditionManagerTest, CompleteWorkflow_RegisterAndRetrieve_WorksCorrectly) {
    auto conditions = CreateValidConditions();

    // Setup expectations for registration
    EXPECT_CALL(*mock_json_loader, ProcessEnvironmentalConditionsConfigFile("config.json"))
        .WillOnce(testing::Return(conditions));
    EXPECT_CALL(*mock_logger, LogInfo(testing::_, testing::_))
        .Times(testing::AtLeast(1));

    // Register conditions
    manager->RegisterEnvironmentalConditions("config.json");

    // Setup expectations for value retrieval
    EXPECT_CALL(*mock_provider, QueryEnvironmentalCondition(2))
        .WillOnce(testing::Return(50));
    EXPECT_CALL(*mock_time_manager, GetCurrentTime())
        .WillRepeatedly(testing::Return(5000));

    // Get value (should trigger update since it's uninitialized)
    int32_t value = manager->GetEnvironmentalConditionValue(2);
    EXPECT_EQ(value, 50);
}