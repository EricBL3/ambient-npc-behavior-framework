
#include <gtest/gtest.h>

#include "behavior/Action.h"

using namespace AmbientCharacterBehavior;

class ActionTest : public testing::Test {
protected:
    void SetUp() override {
        action = std::make_unique<Action>(0, "test_action", true, 10,
            InterruptionBehaviorType::NON_RESUMABLE);
    }

    std::unique_ptr<Action> action;
};

// =============================================================================
// CONSTRUCTOR TESTS
// =============================================================================

TEST_F(ActionTest, ConstructorWorksCorrectly)
{

    EXPECT_EQ(0, action->GetActionId());
    EXPECT_EQ("test_action", action->GetActionName());
    EXPECT_EQ(10, action->GetMaxDuration());
    EXPECT_EQ(InterruptionBehaviorType::NON_RESUMABLE, action->GetInterruptionBehavior());
}

TEST_F(ActionTest, ConstructorRejectsNegativeValues)
{
    EXPECT_THROW(Action(-1, "test", true,0,
        InterruptionBehaviorType::NON_RESUMABLE), std::invalid_argument);

    EXPECT_THROW(Action(0, "test", true, -1,
        InterruptionBehaviorType::RESUMABLE), std::invalid_argument);
}

// =============================================================================
// ADD PRECONDITIONS TESTS
// =============================================================================

TEST_F(ActionTest, AddPreconditionsWorksCorrectly)
{

    StateOperation stateOperation(StateOperationTarget::ENTITY, 0, StateOperationType::EQUALS, 1);
    action->AddPrecondition(stateOperation);

    EXPECT_EQ(1, action->GetPreconditions().size());
}

// =============================================================================
// ADD IMMEDIATE EFFECTS TESTS
// =============================================================================

TEST_F(ActionTest, AddImmediateEffectsWorksCorrectly)
{

    StateOperation stateOperation(StateOperationTarget::ENTITY, 0, StateOperationType::EQUALS, 1);
    action->AddImmediateEffect(stateOperation);

    EXPECT_EQ(1, action->GetImmediateEffects().size());
}

// =============================================================================
// ADD COMPLETION EFFECTS TESTS
// =============================================================================

TEST_F(ActionTest, AddCompletionEffectsWorksCorrectly)
{

    StateOperation stateOperation(StateOperationTarget::ENTITY, 0, StateOperationType::EQUALS, 1);
    action->AddCompletionEffect(stateOperation);

    EXPECT_EQ(1, action->GetCompletionEffects().size());
}

TEST_F(ActionTest, AddInterruptionEffectsWorksCorrectly)
{
    StateOperation stateOperation(StateOperationTarget::ENTITY, 0, StateOperationType::EQUALS, 1);
    action->AddInterruptionEffect(stateOperation);

    EXPECT_EQ(1, action->GetInterruptionEffects().size());
}