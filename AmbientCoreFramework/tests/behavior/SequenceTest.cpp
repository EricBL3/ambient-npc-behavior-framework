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

using namespace AmbientCharacterBehavior;

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
    auto action_node_id = sequence->AddActionSequenceNode(0);

    EXPECT_EQ(0, action_node_id);
    EXPECT_EQ(1, sequence->GetTransitions().capacity());
    EXPECT_EQ(1, sequence->GetNodes().size());
}

TEST_F(SequenceTest, AddNestedSequenceWorksCorrectly)
{
    auto subsequence_node_id = sequence->AddNestedSequenceNode(0);

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

// =============================================================================
// ADD TRANSITION TESTS
// =============================================================================

TEST_F(SequenceTest, AddTransitionWorksCorrectly)
{
    Action action(0, 10, InterruptionBehaviorType::RESUMABLE);
    auto action_node_id = sequence->AddActionSequenceNode(0);
    auto second_node_id = sequence->AddActionSequenceNode(1);
    auto transition_id = sequence->AddTransition(action_node_id, second_node_id);

    EXPECT_EQ(0, transition_id);
    EXPECT_EQ(1, sequence->GetTransitions()[action_node_id].size());

}

TEST_F(SequenceTest, AddTransitionDoesNothingOnFailure)
{
    auto valid_action_node_id = sequence->AddActionSequenceNode(0);

    auto invalid_from_node_transition_id = sequence->AddTransition(5, valid_action_node_id);
    auto invalid_to_node_transition_id = sequence->AddTransition(valid_action_node_id, 5);
    auto invalid_transition_id = sequence->AddTransition(5, 5);

    EXPECT_EQ(-1, invalid_from_node_transition_id);
    EXPECT_EQ(-1, invalid_to_node_transition_id);
    EXPECT_EQ(-1, invalid_transition_id);

    for (auto const& bucket : sequence->GetTransitions()) {
        EXPECT_TRUE(bucket.empty());
    }
}

// =============================================================================
// GET TRANSITIONS FROM NODE TESTS
// =============================================================================

TEST_F(SequenceTest, GetTransitionsFromNodeWorksCorrectly)
{
    auto action_node_id = sequence->AddActionSequenceNode(0);
    auto second_node_id = sequence->AddActionSequenceNode(1);

    [[maybe_unused]] auto res1 = sequence->AddTransition(action_node_id, second_node_id);
    [[maybe_unused]] auto res2 = sequence->AddTransition(action_node_id, second_node_id);

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
    [[maybe_unused]] auto res = sequence->SetEntryPoint(end_node_id);

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
    [[maybe_unused]] auto res = sequence->SetCurrentNode(end_node_id);

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
    [[maybe_unused]] auto res = sequence->SetEntryPoint(end_node_id);
    auto other_end_node_id = sequence->AddEndSequenceNode();
    [[maybe_unused]] auto res2 = sequence->SetCurrentNode(other_end_node_id);

    EXPECT_EQ(other_end_node_id, sequence->GetCurrentNodeIndex());

    sequence->ResetToEntry();
    EXPECT_EQ(end_node_id, sequence->GetCurrentNodeIndex());
}

TEST_F(SequenceTest, ResetToEntryDoesNothingIfNoEntryPoint)
{
    auto other_end_node_id = sequence->AddEndSequenceNode();
    [[maybe_unused]] auto res = sequence->SetCurrentNode(other_end_node_id);

    EXPECT_EQ(other_end_node_id, sequence->GetCurrentNodeIndex());

    sequence->ResetToEntry();

    EXPECT_EQ(other_end_node_id, sequence->GetCurrentNodeIndex());
}

// =============================================================================
// Get Transition To Node TESTS
// =============================================================================

TEST_F(SequenceTest, GetTransitionToNodeWorksCorrectly)
{
    auto action_node_id = sequence->AddActionSequenceNode(0);
    auto second_node_id = sequence->AddActionSequenceNode(1);
    auto second_node = sequence->GetNode(second_node_id);
    [[maybe_unused]] auto res1 = sequence->AddTransition(action_node_id, second_node_id);

    auto transition = sequence->GetTransitionsFromNode(action_node_id).front();

    auto to_node = sequence->GetTransitionToNode(transition);

    EXPECT_EQ(second_node, to_node);
    EXPECT_EQ(second_node->GetNodeId(), to_node->GetNodeId());
}

TEST_F(SequenceTest, GetTransitionToNodeReturnsNullptrIfNodeNotValid)
{
    auto action_node_id = sequence->AddActionSequenceNode(0);
    auto res = sequence->AddTransition(action_node_id, 5);

    EXPECT_EQ(-1, res);
}