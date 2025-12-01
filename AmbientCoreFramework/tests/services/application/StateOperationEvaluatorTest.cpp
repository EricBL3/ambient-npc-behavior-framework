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

    std::unique_ptr<StateOperationEvaluator> state_operation_evaluator;

    void SetUp() override
    {
        mock_logger = std::make_unique<MockLogger>();
        mock_schema = std::make_unique<MockFrameworkSchemaManager>();
        mock_environment_manager = std::make_unique<MockEnvironmentalConditionManager>();

        state_operation_evaluator = std::make_unique<StateOperationEvaluator>(*mock_logger, *mock_schema,
            *mock_environment_manager);
    }
};

TEST_F(StateOperationEvaluatorTest, Constructor_ValidServices_CreatesStateOperationEvaluator) {
    EXPECT_NO_THROW(StateOperationEvaluator evaluator(*mock_logger, *mock_schema,
        *mock_environment_manager));
}

TEST_F(StateOperationEvaluatorTest, ProcessStateOperation_InvalidEnvironmentOperation_LogsWarningAndReturnsFalse) {
    StateOperation set_state_operation(StateOperationTarget::ENVIRONMENT, 0, StateOperationType::SET, {0});
    StateOperation increment_state_operation(StateOperationTarget::ENVIRONMENT, 0, StateOperationType::INCREMENT, {0});
    StateOperation decrement_state_operation(StateOperationTarget::ENVIRONMENT, 0, StateOperationType::DECREMENT, {0});

    EXPECT_CALL(*mock_logger, LogWarning(
        testing::HasSubstr("Environment State operations can only be of comparison"),
        "StateOperationEvaluator"))
        .Times(3);

    auto set_res = state_operation_evaluator->ProcessStateOperation(set_state_operation);
    auto increment_res = state_operation_evaluator->ProcessStateOperation(increment_state_operation);
    auto decrement_res = state_operation_evaluator->ProcessStateOperation(decrement_state_operation);

    EXPECT_FALSE(set_res);
    EXPECT_FALSE(increment_res);
    EXPECT_FALSE(decrement_res);
}

TEST_F(StateOperationEvaluatorTest, ProcessStateOperation_RequiredEntityTargetIsNull_LogsWarningAndReturnsFalse) {
    StateOperation entity_state_operation(StateOperationTarget::ENTITY, 0, StateOperationType::EQUALS, {1});
    StateOperation self_state_operation(StateOperationTarget::SELF, 0, StateOperationType::EQUALS, {1});

    EXPECT_CALL(*mock_logger, LogWarning(
        testing::HasSubstr("No target entity was passed for the evaluation"),
        "StateOperationEvaluator"))
        .Times(2);

    auto entity_res = state_operation_evaluator->ProcessStateOperation(entity_state_operation);
    auto self_res = state_operation_evaluator->ProcessStateOperation(self_state_operation);

    EXPECT_FALSE(entity_res);
    EXPECT_FALSE(self_res);
}

TEST_F(StateOperationEvaluatorTest, ProcessStateOperation_EnvironmentTarget_CallsEnvironmentManager) {
    StateOperation state_operation(StateOperationTarget::ENVIRONMENT, 0, StateOperationType::EQUALS, {0});

    EXPECT_CALL(*mock_environment_manager, GetEnvironmentalConditionValue(0))
        .WillOnce(testing::Return(0));

    auto res = state_operation_evaluator->ProcessStateOperation(state_operation);

    EXPECT_TRUE(res);
}


TEST_F(StateOperationEvaluatorTest, ProcessStateOperation_NonExistentState_LogsErrorAndReturnsFalse) {
    int handle = 100;
    FrameworkEntity entity(&handle, 0);
    StateOperation state_operation(StateOperationTarget::SELF, 0, StateOperationType::EQUALS, {0});

    EXPECT_CALL(*mock_logger, LogError(
        testing::HasSubstr("Error while obtaining the state value"),
        "StateOperationEvaluator"))
        .Times(1);

    auto res = state_operation_evaluator->ProcessStateOperation(state_operation, &entity);

    EXPECT_FALSE(res);
}

TEST_F(StateOperationEvaluatorTest, ProcessStateOperation_EvaluatesEqualOperation) {
    StateOperation state_operation(StateOperationTarget::ENVIRONMENT, 0, StateOperationType::EQUALS, {0});

    EXPECT_CALL(*mock_environment_manager, GetEnvironmentalConditionValue(0))
        .WillOnce(testing::Return(0));

    auto res = state_operation_evaluator->ProcessStateOperation(state_operation);

    EXPECT_TRUE(res);
}

TEST_F(StateOperationEvaluatorTest, ProcessStateOperation_EvaluatesNotEqualOperation) {
    StateOperation state_operation(StateOperationTarget::ENVIRONMENT, 0, StateOperationType::NOT_EQUALS, {1});

    EXPECT_CALL(*mock_environment_manager, GetEnvironmentalConditionValue(0))
        .WillOnce(testing::Return(0));

    auto res = state_operation_evaluator->ProcessStateOperation(state_operation);

    EXPECT_TRUE(res);
}

TEST_F(StateOperationEvaluatorTest, ProcessStateOperation_EvaluatesGreaterThanOperation) {
    StateOperation state_operation(StateOperationTarget::ENVIRONMENT, 0, StateOperationType::GREATER_THAN, {0});

    EXPECT_CALL(*mock_environment_manager, GetEnvironmentalConditionValue(0))
        .WillOnce(testing::Return(1));

    auto res = state_operation_evaluator->ProcessStateOperation(state_operation);

    EXPECT_TRUE(res);
}

TEST_F(StateOperationEvaluatorTest, ProcessStateOperation_EvaluatesLessThanOperation) {
    StateOperation state_operation(StateOperationTarget::ENVIRONMENT, 0, StateOperationType::LESS_THAN, {1});

    EXPECT_CALL(*mock_environment_manager, GetEnvironmentalConditionValue(0))
        .WillOnce(testing::Return(0));

    auto res = state_operation_evaluator->ProcessStateOperation(state_operation);

    EXPECT_TRUE(res);
}

TEST_F(StateOperationEvaluatorTest, ProcessStateOperation_EvaluatesSetOperation) {
    int handle = 100;
    FrameworkEntity entity(&handle, 0);
    entity.SetStateValue(0, 1);

    StateOperation state_operation(StateOperationTarget::SELF, 0, StateOperationType::SET, {0});

    auto res = state_operation_evaluator->ProcessStateOperation(state_operation, &entity);

    EXPECT_TRUE(res);
    EXPECT_EQ(0, entity.GetStateValue(0));
}

TEST_F(StateOperationEvaluatorTest, ProcessStateOperation_EvaluatesIncrementOperation) {
    int handle = 100;
    FrameworkEntity entity(&handle, 0);
    entity.SetStateValue(0, 1);

    StateOperation state_operation(StateOperationTarget::SELF, 0, StateOperationType::INCREMENT, {1});

    auto res = state_operation_evaluator->ProcessStateOperation(state_operation, &entity);

    EXPECT_TRUE(res);
    EXPECT_EQ(2, entity.GetStateValue(0));
}

TEST_F(StateOperationEvaluatorTest, ProcessStateOperation_EvaluatesIncrementOperation_LogsWarningAndClampsOnOverflow) {
    int handle = 100;
    FrameworkEntity entity(&handle, 0);
    entity.SetStateValue(0, std::numeric_limits<int32_t>::max() - 1);

    StateOperation state_operation(StateOperationTarget::SELF, 0, StateOperationType::INCREMENT, {5});

    EXPECT_CALL(*mock_logger, LogWarning(
       testing::HasSubstr("Addition would overflow"),
       "StateOperationEvaluator"))
       .Times(1);

    auto res = state_operation_evaluator->ProcessStateOperation(state_operation, &entity);

    EXPECT_TRUE(res);
    EXPECT_EQ(std::numeric_limits<int32_t>::max(), entity.GetStateValue(0));
}

TEST_F(StateOperationEvaluatorTest, ProcessStateOperation_EvaluatesIncrementOperation_LogsWarningAndClampsOnUnderflow) {
    int handle = 100;
    FrameworkEntity entity(&handle, 0);
    entity.SetStateValue(0, std::numeric_limits<int32_t>::min() + 1);

    StateOperation state_operation(StateOperationTarget::SELF, 0, StateOperationType::INCREMENT, {-5});

    EXPECT_CALL(*mock_logger, LogWarning(
       testing::HasSubstr("Addition would underflow"),
       "StateOperationEvaluator"))
       .Times(1);

    auto res = state_operation_evaluator->ProcessStateOperation(state_operation, &entity);

    EXPECT_TRUE(res);
    EXPECT_EQ(std::numeric_limits<int32_t>::min(), entity.GetStateValue(0));
}

TEST_F(StateOperationEvaluatorTest, ProcessStateOperation_EvaluatesDecrementOperation) {
    int handle = 100;
    FrameworkEntity entity(&handle, 0);
    entity.SetStateValue(0, 1);

    StateOperation state_operation(StateOperationTarget::SELF, 0, StateOperationType::DECREMENT, {1});

    auto res = state_operation_evaluator->ProcessStateOperation(state_operation, &entity);

    EXPECT_TRUE(res);
    EXPECT_EQ(0, entity.GetStateValue(0));
}

TEST_F(StateOperationEvaluatorTest, ProcessStateOperation_EvaluatesDecrementOperation_LogsWarningAndClampsOnOverflow) {
    int handle = 100;
    FrameworkEntity entity(&handle, 0);
    entity.SetStateValue(0, std::numeric_limits<int32_t>::max() - 1);

    StateOperation state_operation(StateOperationTarget::SELF, 0, StateOperationType::DECREMENT, {-5});

    EXPECT_CALL(*mock_logger, LogWarning(
       testing::HasSubstr("Subtraction would overflow"),
       "StateOperationEvaluator"))
       .Times(1);

    auto res = state_operation_evaluator->ProcessStateOperation(state_operation, &entity);

    EXPECT_TRUE(res);
    EXPECT_EQ(std::numeric_limits<int32_t>::max(), entity.GetStateValue(0));
}

TEST_F(StateOperationEvaluatorTest, ProcessStateOperation_EvaluatesDecrementOperation_LogsWarningAndClampsOnUnderflow) {
    int handle = 100;
    FrameworkEntity entity(&handle, 0);
    entity.SetStateValue(0, std::numeric_limits<int32_t>::min() + 1);

    StateOperation state_operation(StateOperationTarget::SELF, 0, StateOperationType::DECREMENT, {5});

    EXPECT_CALL(*mock_logger, LogWarning(
       testing::HasSubstr("Subtraction would underflow"),
       "StateOperationEvaluator"))
       .Times(1);

    auto res = state_operation_evaluator->ProcessStateOperation(state_operation, &entity);

    EXPECT_TRUE(res);
    EXPECT_EQ(std::numeric_limits<int32_t>::min(), entity.GetStateValue(0));
}

TEST_F(StateOperationEvaluatorTest, ProcessStateOperation_UnsupportedOperationType_LogsWarningAndReturnsFalse) {
    StateOperation state_operation(StateOperationTarget::ENVIRONMENT, 0, StateOperationType::EXTERNAL_OPERATIONS, {0});

    EXPECT_CALL(*mock_environment_manager, GetEnvironmentalConditionValue(0))
        .WillOnce(testing::Return(0));

    EXPECT_CALL(*mock_logger, LogWarning(
       testing::HasSubstr("currently not supported"),
       "StateOperationEvaluator"))
       .Times(1);

    auto res = state_operation_evaluator->ProcessStateOperation(state_operation);

    EXPECT_FALSE(res);
}