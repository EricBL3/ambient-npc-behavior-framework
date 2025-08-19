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

// =============================================================================
// CONSTRUCTOR TESTS
// =============================================================================
TEST(TransitionTest, SimpleConstructorInitializesCorrectly)
{
    Transition transition(0, 5);

    EXPECT_EQ(0, transition.GetTransitionId());
    EXPECT_EQ(5, transition.GetPreconditions().capacity());
    EXPECT_EQ(0, transition.GetPreconditions().size());
    EXPECT_EQ(nullptr, transition.GetToNode());
}

TEST(TransitionTest, ConstructorWithToNodeInitializesCorrectly)
{
    EndSequenceNode node(0);
    Transition transition(0, &node , 5);

    EXPECT_EQ(0, transition.GetTransitionId());
    EXPECT_EQ(5, transition.GetPreconditions().capacity());
    EXPECT_EQ(&node, transition.GetToNode());
}

TEST(TransitionTest, ConstructorRejectsNegativeTransitionId)
{
    EXPECT_THROW(Transition(-1), std::invalid_argument);
}

// =============================================================================
// SET TO NODE TESTS
// =============================================================================

TEST(TransitionTest, SetToNodeWorksCorrectly)
{
    Transition transition(0, 5);

    EXPECT_EQ(nullptr, transition.GetToNode());

    EndSequenceNode node(0);
    transition.SetToNode(&node);

    EXPECT_EQ(&node, transition.GetToNode());
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
