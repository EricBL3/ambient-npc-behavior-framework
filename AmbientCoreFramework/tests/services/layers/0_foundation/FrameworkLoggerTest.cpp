
#include <filesystem>
#include <gtest/gtest.h>
#include <fstream>

#include "services/layers/0_foundation/FrameworkLogLevel.h"
#include "services/layers/0_foundation/FrameworkLogger.h"

using namespace AmbientCharacterBehavior;

class FrameworkLoggerTest : public testing::Test {
protected:

    std::string test_file_path;

    void SetUp() override
    {
        test_file_path = "test_framework.log";

        if (std::filesystem::exists(test_file_path))
        {
            std::remove(test_file_path.c_str());
        }
    }

    void TearDown() override
    {
        if (std::filesystem::exists(test_file_path))
        {
            std::remove(test_file_path.c_str());
        }
    }

    std::string ReadLogFile()
    {
        std::ifstream file(test_file_path);
        if (!file.good())
            return "";

        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }
};

// =============================================================================
// INITIALIZATION TESTS
// =============================================================================

TEST_F(FrameworkLoggerTest, Constructor_NoFile_CreatesValidLogger) {
    FrameworkLogger logger;
    EXPECT_NO_THROW(logger.LogInfo("Test", "Component"));
}

TEST_F(FrameworkLoggerTest, Constructor_WithFile_CreatesFileLogger) {
    FrameworkLogger logger(test_file_path);
    logger.LogInfo("Test message", "TestComponent");

    EXPECT_TRUE(std::filesystem::exists(test_file_path));
    auto content = ReadLogFile();
    EXPECT_FALSE(content.empty());
}

// =============================================================================
// WRITING MESSAGE TESTS
// =============================================================================

TEST_F(FrameworkLoggerTest, LogError_WritesToFile) {
    FrameworkLogger logger(test_file_path);
    logger.LogError("Error message", "TestComponent");

    auto content = ReadLogFile();
    EXPECT_TRUE(content.find("ERROR") != std::string::npos);
    EXPECT_TRUE(content.find("Error message") != std::string::npos);
    EXPECT_TRUE(content.find("TestComponent") != std::string::npos);
}

TEST_F(FrameworkLoggerTest, LogWarning_WritesToFile) {
    FrameworkLogger logger(test_file_path);
    logger.LogWarning("Warning message", "TestComponent");

    auto content = ReadLogFile();
    EXPECT_TRUE(content.find("WARNING") != std::string::npos);
    EXPECT_TRUE(content.find("Warning message") != std::string::npos);
    EXPECT_TRUE(content.find("TestComponent") != std::string::npos);
}

TEST_F(FrameworkLoggerTest, LogInfo_WritesToFile) {
    FrameworkLogger logger(test_file_path);
    logger.LogInfo("Info message", "TestComponent");

    auto content = ReadLogFile();
    EXPECT_TRUE(content.find("INFO") != std::string::npos);
    EXPECT_TRUE(content.find("Info message") != std::string::npos);
    EXPECT_TRUE(content.find("TestComponent") != std::string::npos);
}

TEST_F(FrameworkLoggerTest, MultipleMessages_MaintainOrder) {
    FrameworkLogger logger(test_file_path);
    logger.LogError("First", "Test");
    logger.LogInfo("Second", "Test");
    logger.LogWarning("Third", "Test");

    auto content = ReadLogFile();
    auto first_pos = content.find("First");
    auto second_pos = content.find("Second");
    auto third_pos = content.find("Third");

    EXPECT_LT(first_pos, second_pos);
    EXPECT_LT(second_pos, third_pos);
}

// =============================================================================
// LOG LEVEL FILTERING TESTS
// =============================================================================

TEST_F(FrameworkLoggerTest, SetLogLevel_FiltersMessages) {
    FrameworkLogger logger(test_file_path);
    logger.SetLogLevel(FrameworkLogLevel::WARNING);

    logger.LogInfo("Info message", "Test");        // Should be filtered
    logger.LogWarning("Warning message", "Test");  // Should appear
    logger.LogError("Error message", "Test");      // Should appear

    auto content = ReadLogFile();

    EXPECT_TRUE(content.find("Error message") != std::string::npos);
    EXPECT_TRUE(content.find("Warning message") != std::string::npos);
    EXPECT_FALSE(content.find("Info message") != std::string::npos);
}

// =============================================================================
// ENABLE/DISABLE LOG  TESTS
// =============================================================================
TEST_F(FrameworkLoggerTest, SetLoggingEnabled_DisablesLogging) {
    FrameworkLogger logger(test_file_path);

    // First, verify logging works when enabled
    logger.LogInfo("Should appear", "Test");
    auto content_enabled = ReadLogFile();
    EXPECT_TRUE(content_enabled.find("Should appear") != std::string::npos);

    // Now disable and test
    logger.SetLoggingEnabled(false);
    logger.LogInfo("Should not appear", "Test");

    auto content_disabled = ReadLogFile();
    EXPECT_FALSE(content_disabled.find("Should not appear") != std::string::npos);
}

