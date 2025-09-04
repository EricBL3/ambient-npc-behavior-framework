/**
 * @file FrameworkLoggerTest.cpp
 * @brief Tests the FrameworkLogger
 * @author Eric Buitrón López
 * @date 9/1/2025
 *
 *
*/

#include <fstream>
#include <gtest/gtest.h>

#include "utils/FrameworkLogger.h"

using namespace AmbientCharacterBehavior;

class FrameworkLoggerTest : public testing::Test {
protected:

    std::string test_file_path;

    void SetUp() override
    {
        test_file_path = "test_log.txt";

        std::remove(test_file_path.c_str());
    }

    void TearDown() override
    {
        std::remove(test_file_path.c_str());
    }

    std::string ReadLogFile()
    {
        std::ifstream file(test_file_path);
        if (!file.good())
            return "";

        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

        return content;
    }
};

// =============================================================================
// INITIALIZATION TESTS
// =============================================================================

TEST_F(FrameworkLoggerTest, CanInitialize) {
    EXPECT_NO_THROW(FrameworkLogger::Initialize(test_file_path));

    std::ifstream file(test_file_path);
    EXPECT_TRUE(file.good());
}

// =============================================================================
// WRITING MESSAGE TESTS
// =============================================================================
TEST_F(FrameworkLoggerTest, CanWriteMessages) {
    FrameworkLogger::Initialize(test_file_path);

    FrameworkLogger::LogInfo("Info message", "TestComponent");
    FrameworkLogger::LogWarning("Warning message", "TestComponent");
    FrameworkLogger::LogError("Error message", "TestComponent");

    std::string content = ReadLogFile();

    EXPECT_FALSE(content.empty());
    EXPECT_TRUE(content.find("Info message") != std::string::npos);
    EXPECT_TRUE(content.find("Warning message") != std::string::npos);
    EXPECT_TRUE(content.find("Error message") != std::string::npos);
}

TEST_F(FrameworkLoggerTest, IncludesComponentName) {
    FrameworkLogger::Initialize(test_file_path);

    FrameworkLogger::LogInfo("Test message", "MySpecificComponent");

    std::string content = ReadLogFile();

    EXPECT_TRUE(content.find("MySpecificComponent") != std::string::npos);
    EXPECT_TRUE(content.find("Test message") != std::string::npos);
}

// =============================================================================
// LOG LEVEL FILTERING TESTS
// =============================================================================

TEST_F(FrameworkLoggerTest, LogLevelFiltering) {
    FrameworkLogger::Initialize(test_file_path);
    FrameworkLogger::SetLogLevel(FrameworkLogLevel::WARNING);

    FrameworkLogger::LogInfo("Info message", "Test");        // Should be filtered
    FrameworkLogger::LogWarning("Warning message", "Test");  // Should appear
    FrameworkLogger::LogError("Error message", "Test");      // Should appear

    std::string content = ReadLogFile();

    EXPECT_TRUE(content.find("Warning message") != std::string::npos);
    EXPECT_TRUE(content.find("Error message") != std::string::npos);
    EXPECT_TRUE(content.find("Info message") == std::string::npos);
}

// =============================================================================
// ENABLE/DISABLE LOG  TESTS
// =============================================================================
TEST_F(FrameworkLoggerTest, CanDisableLogging) {
    FrameworkLogger::Initialize(test_file_path);

    // First, verify logging works when enabled
    FrameworkLogger::LogInfo("Should appear", "Test");
    std::string content_enabled = ReadLogFile();
    EXPECT_TRUE(content_enabled.find("Should appear") != std::string::npos);

    // Now disable and test
    FrameworkLogger::SetLoggingEnabled(false);
    FrameworkLogger::LogInfo("Should not appear", "Test");

    std::string content_disabled = ReadLogFile();
    EXPECT_TRUE(content_disabled.find("Should not appear") == std::string::npos);
}

