/**
 * @file ActionTest.cpp
 * @brief Unit tests for the action class
 * @author Eric Buitrón López
 * @date 8/19/2025
 *
 *
*/

#include <gtest/gtest.h>

#include "behavior/Action.h"

class ActionTest : public testing::Test {
protected:
    void SetUp() override {
        action = std::make_unique<Action>(0, 10, InterruptionBehaviorType::NON_RESUMABLE,
            3, 3, 3);
    }

    std::unique_ptr<Action> action;
};

// =============================================================================
// CONSTRUCTOR TESTS
// =============================================================================

TEST_F(ActionTest, ConstructorWorksCorrectly)
{

    EXPECT_EQ(0, action->GetActionId());
    EXPECT_EQ(10, action->GetMaxDuration());
    EXPECT_EQ(InterruptionBehaviorType::NON_RESUMABLE, action->GetInterruptionBehavior());
    EXPECT_EQ(3, action->GetPreconditions().capacity());
    EXPECT_EQ(3, action->GetImmediateEffects().capacity());
    EXPECT_EQ(3, action->GetCompletionEffects().capacity());
}

TEST_F(ActionTest, ConstructorRejectsNegativeValues)
{
    EXPECT_THROW(Action(-1, 0, InterruptionBehaviorType::NON_RESUMABLE), std::invalid_argument);
    EXPECT_THROW(Action(0, -1, InterruptionBehaviorType::RESUMABLE), std::invalid_argument);
}

// =============================================================================
// ADD PRECONDITIONS TESTS
// =============================================================================

TEST_F(ActionTest, AddPreconditionsWorksCorrectly)
{

    StateOperation stateOperation(0, 0, 0, {1});
    action->AddPrecondition(stateOperation);

    EXPECT_EQ(1, action->GetPreconditions().size());
}

// =============================================================================
// ADD IMMEDIATE EFFECTS TESTS
// =============================================================================

TEST_F(ActionTest, AddImmediateEffectsWorksCorrectly)
{

    StateOperation stateOperation(0, 0, 0, {1});
    action->AddImmediateEffect(stateOperation);

    EXPECT_EQ(1, action->GetImmediateEffects().size());
}

// =============================================================================
// ADD COMPLETION EFFECTS TESTS
// =============================================================================

TEST_F(ActionTest, AddCompletionEffectsWorksCorrectly)
{

    StateOperation stateOperation(0, 0, 0, {1});
    action->AddCompletionEffect(stateOperation);

    EXPECT_EQ(1, action->GetCompletionEffects().size());
}