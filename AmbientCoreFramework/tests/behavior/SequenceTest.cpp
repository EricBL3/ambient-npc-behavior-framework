/**
 * @file SequenceTest.cpp
 * @brief Unit tests for the sequence class
 * @author Eric Buitrón López
 * @date 8/19/2025
 *
 *
*/

#include <gtest/gtest.h>

#include "behavior/Sequence.h"

class SequenceTest : public testing::Test {
protected:
    void SetUp() override {
        sequence = std::make_unique<Sequence>(0);
    }

    std::unique_ptr<Sequence> sequence;
};

// =============================================================================
// CONSTRUCTOR TESTS
// =============================================================================

TEST_F(SequenceTest, ConstructorWorksCorrectly)
{
    EXPECT_EQ(sequence->GetSequenceId(), 0);
    EXPECT_EQ(sequence->GetEntryPointIndex(), -1);
    EXPECT_EQ(sequence->GetHasEntryPoint(), false);
    EXPECT_EQ(sequence->GetCurrentNodeIndex(), -1);
    EXPECT_EQ(sequence->GetHasCurrentNode(), false);
    EXPECT_EQ(sequence->GetSequenceState(), SequenceState::NORMAL);
}

TEST_F(SequenceTest, ConstructorRejectsNegativeSequenceId)
{
    EXPECT_THROW(Sequence(-1), std::invalid_argument);
}

// =============================================================================
// SET SEQUENCE STATE TESTS
// =============================================================================

TEST_F(SequenceTest, SetSequenceStateWorksCorrectly)
{
    EXPECT_EQ(sequence->GetSequenceState(), SequenceState::NORMAL);
    sequence->SetSequenceState(SequenceState::INTERRUPTED);
    EXPECT_EQ(sequence->GetSequenceState(), SequenceState::INTERRUPTED);
}

// =============================================================================
// ADD SEQUENCE NODE TESTS
// =============================================================================

TEST_F(SequenceTest, AddActionSequenceNodeWorksCorrectly)
{
    Action action(0, 10, InterruptionBehaviorType::RESUMABLE);
    auto action_node_id = sequence->AddActionSequenceNode(&action);

    EXPECT_EQ(0, action_node_id);
    EXPECT_EQ(1, sequence->GetTransitions().capacity());
    EXPECT_EQ(1, sequence->GetNodes().size());
}

TEST_F(SequenceTest, AddNestedSequenceWorksCorrectly)
{
    Sequence subsequence(0);
    auto subsequence_node_id = sequence->AddNestedSequenceNode(&subsequence);

    EXPECT_EQ(0, subsequence_node_id);
    EXPECT_EQ(1, sequence->GetTransitions().capacity());
    EXPECT_EQ(1, sequence->GetNodes().size());
}

TEST_F(SequenceTest, AddEndSequenceNodeWorksCorrectly)
{
    auto end_node_id = sequence->AddEndSequenceNode();

    EXPECT_EQ(0, end_node_id);
    EXPECT_EQ(1, sequence->GetTransitions().capacity());
    EXPECT_EQ(1, sequence->GetNodes().size());
}

TEST_F(SequenceTest, AddNodeFunctionsRejectNullptr)
{
    EXPECT_THROW(sequence->AddActionSequenceNode(nullptr), std::invalid_argument);
    EXPECT_THROW(sequence->AddNestedSequenceNode(nullptr), std::invalid_argument);

    EXPECT_EQ(0, sequence->GetTransitions().capacity());
    EXPECT_EQ(0, sequence->GetNodes().size());
}

// =============================================================================
// ADD TRANSITION TESTS
// =============================================================================

TEST_F(SequenceTest, AddTransitionWorksCorrectly)
{
    Action action(0, 10, InterruptionBehaviorType::RESUMABLE);
    auto action_node_id = sequence->AddActionSequenceNode(&action);
    auto transition_id = sequence->AddTransition(action_node_id);

    EXPECT_EQ(0, transition_id);
    EXPECT_EQ(1, sequence->GetTransitions()[action_node_id].size());

}

TEST_F(SequenceTest, AddTransitionDoesNothingOnFailure)
{
    auto transition_id = sequence->AddTransition(0);

    EXPECT_EQ(-1, transition_id);
    EXPECT_EQ(0, sequence->GetTransitions().size());

}

// =============================================================================
// GET TRANSITIONS FROM NODE TESTS
// =============================================================================

TEST_F(SequenceTest, GetTransitionsFromNodeWorksCorrectly)
{
    Action action(0, 10, InterruptionBehaviorType::RESUMABLE);
    auto action_node_id = sequence->AddActionSequenceNode(&action);
    sequence->AddTransition(action_node_id);
    sequence->AddTransition(action_node_id);

    auto transitions = sequence->GetTransitionsFromNode(action_node_id);

    EXPECT_EQ(2, transitions.size());
}

TEST_F(SequenceTest, GetTransitionsFromNodeReturnsEmptyOnFailure)
{
    auto transitions = sequence->GetTransitionsFromNode(1);

    EXPECT_EQ(transitions.size(), 0);
}

// =============================================================================
// ENTRY POINT TESTS
// =============================================================================

TEST_F(SequenceTest, SetEntryPointReturnsTrueOnSuccess)
{
    auto end_node_id = sequence->AddEndSequenceNode();
    auto res = sequence->SetEntryPoint(end_node_id);

    EXPECT_EQ(true, res);
    EXPECT_EQ(end_node_id, sequence->GetEntryPointIndex());
    EXPECT_EQ(true, sequence->GetHasEntryPoint());
}

TEST_F(SequenceTest, SetEntryPointReturnsFalseOnFailure)
{
    auto res = sequence->SetEntryPoint(1);

    EXPECT_EQ(false, res);
    EXPECT_EQ(-1, sequence->GetEntryPointIndex());
    EXPECT_EQ(false, sequence->GetHasEntryPoint());
}

TEST_F(SequenceTest, GetEntryPointWorksIfNodeFound)
{
    auto end_node_id = sequence->AddEndSequenceNode();
    sequence->SetEntryPoint(end_node_id);

    auto entry_point = sequence->GetEntryPoint();
    auto exists = entry_point != nullptr;

    EXPECT_EQ(true, exists);
}

TEST_F(SequenceTest, GetEntryPointReturnsNullIfNodeNotFound)
{
    auto entry_point = sequence->GetEntryPoint();
    auto exists = entry_point != nullptr;

    EXPECT_EQ(false, exists);
}

// =============================================================================
// ENTRY POINT TESTS
// =============================================================================

TEST_F(SequenceTest, SetCurrentNodeReturnsTrueOnSuccess)
{
    auto end_node_id = sequence->AddEndSequenceNode();
    auto res = sequence->SetCurrentNode(end_node_id);

    EXPECT_EQ(true, res);
    EXPECT_EQ(end_node_id, sequence->GetCurrentNodeIndex());
    EXPECT_EQ(true, sequence->GetHasCurrentNode());
}

TEST_F(SequenceTest, SetCurrentNodeReturnsFalseOnFailure)
{
    auto res = sequence->SetEntryPoint(1);

    EXPECT_EQ(false, res);
    EXPECT_EQ(-1, sequence->GetCurrentNodeIndex());
    EXPECT_EQ(false, sequence->GetHasCurrentNode());
}

TEST_F(SequenceTest, GetCurrentNodeWorksIfNodeFound)
{
    auto end_node_id = sequence->AddEndSequenceNode();
    sequence->SetCurrentNode(end_node_id);

    auto current_node = sequence->GetCurrentNode();
    auto exists = current_node != nullptr;

    EXPECT_EQ(true, exists);
}

TEST_F(SequenceTest, GetCurrentNodeReturnsNullIfNodeNotFound)
{
    auto current_node = sequence->GetCurrentNode();
    auto exists = current_node != nullptr;

    EXPECT_EQ(false, exists);
}

// =============================================================================
// RESET TO ENTRY TESTS
// =============================================================================

TEST_F(SequenceTest, ResetToEntryWorksIfHasEntryPoint)
{
    auto end_node_id = sequence->AddEndSequenceNode();
    sequence->SetEntryPoint(end_node_id);
    auto other_end_node_id = sequence->AddEndSequenceNode();
    sequence->SetCurrentNode(other_end_node_id);

    EXPECT_EQ(other_end_node_id, sequence->GetCurrentNodeIndex());

    sequence->ResetToEntry();
    EXPECT_EQ(end_node_id, sequence->GetCurrentNodeIndex());
}

TEST_F(SequenceTest, ResetToEntryDoesNothingIfNoEntryPoint)
{
    auto other_end_node_id = sequence->AddEndSequenceNode();
    sequence->SetCurrentNode(other_end_node_id);

    EXPECT_EQ(other_end_node_id, sequence->GetCurrentNodeIndex());

    sequence->ResetToEntry();

    EXPECT_EQ(other_end_node_id, sequence->GetCurrentNodeIndex());
}