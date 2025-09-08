#include <gtest/gtest.h>

#include "../../mocks/MockJsonLoader.h"
#include "../../mocks/MockLogger.h"
#include "services/domain/FrameworkRegistry.h"

using namespace AmbientCharacterBehavior;

class FrameworkRegistryTest : public testing::Test {
protected:
    std::unique_ptr<MockLogger> mock_logger;
    std::unique_ptr<MockJsonLoader> mock_json_loader;
    std::unique_ptr<FrameworkRegistry> registry;

    void SetUp() override {
        mock_logger = std::make_unique<MockLogger>();
        mock_json_loader = std::make_unique<MockJsonLoader>();

        registry = std::make_unique<FrameworkRegistry>(*mock_logger, *mock_json_loader);
    }
};

// Constructor test
TEST_F(FrameworkRegistryTest, Constructor_ValidServices_CreatesFrameworkRegistry) {
    EXPECT_NO_THROW(FrameworkRegistry framework_registry(*mock_logger, *mock_json_loader));
}