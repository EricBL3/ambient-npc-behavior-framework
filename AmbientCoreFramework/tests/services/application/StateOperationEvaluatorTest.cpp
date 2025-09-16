#include <gtest/gtest.h>

#include "../../mocks/MockEnvironmentalConditionManager.h"
#include "../../mocks/MockFrameworkSchemaManager.h"
#include "../../mocks/MockLogger.h"
#include "../../mocks/MockFrameworkRegistry.h"
#include "services/application/StateOperationEvaluator.h"


using namespace AmbientCharacterBehavior;

class StateOperationEvaluatorTest : public testing::Test {
protected:
    std::unique_ptr<MockLogger> mock_logger;
    std::unique_ptr<MockFrameworkSchemaManager> mock_schema;
    std::unique_ptr<MockEnvironmentalConditionManager> mock_environment_manager;
    std::unique_ptr<MockFrameworkRegistry> mock_registry;

    std::unique_ptr<StateOperationEvaluator> state_operation_evaluator;

    void SetUp() override
    {
        mock_logger = std::make_unique<MockLogger>();
        mock_schema = std::make_unique<MockFrameworkSchemaManager>();
        mock_environment_manager = std::make_unique<MockEnvironmentalConditionManager>();
        mock_registry = std::make_unique<MockFrameworkRegistry>();

        state_operation_evaluator = std::make_unique<StateOperationEvaluator>(*mock_logger, *mock_schema,
            *mock_environment_manager, *mock_registry);
    }
};

// Constructor test
TEST_F(StateOperationEvaluatorTest, Constructor_ValidServices_CreatesStateOperationEvaluator) {
    EXPECT_NO_THROW(StateOperationEvaluator evaluator(*mock_logger, *mock_schema,
        *mock_environment_manager, *mock_registry));
}