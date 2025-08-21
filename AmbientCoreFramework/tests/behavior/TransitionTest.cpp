/**
 * @file TransitionTest.cpp
 * @brief Unit tests for the Transition clas
 * @author Eric Buitrón López
 * @date 8/19/2025
 *
 *
*/

#include <gtest/gtest.h>
#include "behavior/EndSequenceNode.h"
#include "behavior/Transition.h"

using namespace AmbientCharacterBehavior;

// =============================================================================
// CONSTRUCTOR TESTS
// =============================================================================
TEST(TransitionTest, SimpleConstructorInitializesCorrectly)
{
    Transition transition(0,  1, 5);

    EXPECT_EQ(0, transition.GetTransitionId());
    EXPECT_EQ(5, transition.GetPreconditions().capacity());
    EXPECT_EQ(0, transition.GetPreconditions().size());
    EXPECT_EQ(1, transition.GetToNodeIndex());
}

TEST(TransitionTest, ConstructorWithToNodeInitializesCorrectly)
{
    Transition transition(0, 0 , 5);

    EXPECT_EQ(0, transition.GetTransitionId());
    EXPECT_EQ(5, transition.GetPreconditions().capacity());
    EXPECT_EQ(0, transition.GetToNodeIndex());
}

TEST(TransitionTest, ConstructorRejectsNegativeTransitionId)
{
    EXPECT_THROW(Transition(-1, 0), std::invalid_argument);
    EXPECT_THROW(Transition(0, -4), std::invalid_argument);
}


// =============================================================================
// ADD PRECONDITIONS TESTS
// =============================================================================

TEST(TransitionTest, AddPreconditionWorksCorrectly)
{
    Transition transition(0, 5);

    EXPECT_EQ(0, transition.GetPreconditions().size());

    StateOperation stateOperation(0, 0, 0, {1});
    transition.AddPrecondition(stateOperation);

    EXPECT_EQ(1, transition.GetPreconditions().size());
}
