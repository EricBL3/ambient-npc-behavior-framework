#include <gtest/gtest.h>
#include "behavior/Sequence.h"

using namespace AmbientCharacterBehavior;

class SequenceTest : public testing::Test {
protected:
    void SetUp() override {
        sequence = std::make_unique<Sequence>(0, "test_sequence");
    }

    std::unique_ptr<Sequence> sequence;
};

// =============================================================================
// CONSTRUCTOR TESTS
// =============================================================================

TEST_F(SequenceTest, ConstructorWorksCorrectly)
{
    EXPECT_EQ(sequence->GetSequenceId(), 0);
    EXPECT_EQ(sequence->GetSequenceName(), "test_sequence");
    EXPECT_EQ(sequence->GetEntryPointNodeId(), -1);
    EXPECT_EQ(sequence->HasEntryPoint(), false);
    EXPECT_EQ(sequence->GetCurrentNodeId(), -1);
    EXPECT_EQ(sequence->HasCurrentNode(), false);
    EXPECT_EQ(sequence->GetSequenceState(), SequenceState::NORMAL);
}

TEST_F(SequenceTest, ConstructorRejectsNegativeSequenceId)
{
    EXPECT_THROW(Sequence(-1, ""), std::invalid_argument);
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
    sequence->AddActionSequenceNode(0, 10);

    EXPECT_EQ(1, sequence->GetNodeCount());
    EXPECT_TRUE(sequence->HasNode(0));
}

TEST_F(SequenceTest, AddNestedSequenceWorksCorrectly)
{
    sequence->AddNestedSequenceNode(0, 10);

    EXPECT_EQ(1, sequence->GetNodeCount());
    EXPECT_TRUE(sequence->HasNode(0));
}

TEST_F(SequenceTest, AddEndSequenceNodeWorksCorrectly)
{
    sequence->AddEndSequenceNode(0);

    EXPECT_EQ(1, sequence->GetNodeCount());
    EXPECT_TRUE(sequence->HasNode(0));
}

// =============================================================================
// ADD TRANSITION TESTS
// =============================================================================

TEST_F(SequenceTest, TryAddTransitionWorksCorrectly)
{
    sequence->AddActionSequenceNode(0, 0);
    sequence->AddActionSequenceNode(1, 0);
    auto success = sequence->TryAddTransition(0, 0, 1, {});

    EXPECT_TRUE(success);
    EXPECT_EQ(1, sequence->FindTransitionsFrom(0).size());

}

TEST_F(SequenceTest, TryAddTransitionDoesNothingOnFailure)
{
    sequence->AddActionSequenceNode(0, 0);
    auto invalid_from_node_transition_id = sequence->TryAddTransition(0, 5, 0, {});
    auto invalid_to_node_transition_id = sequence->TryAddTransition(1, 0, 5, {});
    auto invalid_transition_id = sequence->TryAddTransition(2, 5, 5, {});

    EXPECT_FALSE(invalid_from_node_transition_id);
    EXPECT_FALSE(invalid_to_node_transition_id);
    EXPECT_FALSE(invalid_transition_id);

    auto transitions = sequence->FindTransitionsFrom(0);
    EXPECT_TRUE(transitions.empty());
}

// =============================================================================
// GET TRANSITIONS FROM NODE TESTS
// =============================================================================

TEST_F(SequenceTest, FindTransitionsFromWorksCorrectly)
{
    sequence->AddActionSequenceNode(0, 0);
    sequence->AddActionSequenceNode(1, 0);
    [[maybe_unused]] auto res1 = sequence->TryAddTransition(0, 0, 1, {});
    [[maybe_unused]] auto res2 = sequence->TryAddTransition(1, 0, 1, {});

    auto transitions = sequence->FindTransitionsFrom(0);

    EXPECT_EQ(2, transitions.size());
}

TEST_F(SequenceTest, FindTransitionsFromReturnsEmptyOnFailure)
{
    auto transitions = sequence->FindTransitionsFrom(1);

    EXPECT_EQ(transitions.size(), 0);
}

// =============================================================================
// ENTRY POINT TESTS
// =============================================================================

TEST_F(SequenceTest, TrySetEntryPointReturnsTrueOnSuccess)
{
    sequence->AddEndSequenceNode(0);
    auto res = sequence->TrySetEntryPoint(0);

    EXPECT_EQ(true, res);
    EXPECT_EQ(0, sequence->GetEntryPointNodeId());
    EXPECT_EQ(true, sequence->HasEntryPoint());
}

TEST_F(SequenceTest, TrySetEntryPointReturnsFalseOnFailure)
{
    auto res = sequence->TrySetEntryPoint(1);

    EXPECT_EQ(false, res);
    EXPECT_EQ(-1, sequence->GetEntryPointNodeId());
    EXPECT_EQ(false, sequence->HasEntryPoint());
}

TEST_F(SequenceTest, FindEntryPointWorksIfNodeFound)
{
    sequence->AddEndSequenceNode(0);
    [[maybe_unused]] auto res = sequence->TrySetEntryPoint(0);

    auto entry_point = sequence->FindEntryPointNode();
    auto exists = entry_point != nullptr;

    EXPECT_EQ(true, exists);
}

TEST_F(SequenceTest, FindEntryPointReturnsNullIfNodeNotFound)
{
    auto entry_point = sequence->FindEntryPointNode();
    auto exists = entry_point != nullptr;

    EXPECT_EQ(false, exists);
}

// =============================================================================
// CURRENT NODE TESTS
// =============================================================================

TEST_F(SequenceTest, TrySetCurrentNodeReturnsTrueOnSuccess)
{
    sequence->AddEndSequenceNode(0);
    auto res = sequence->TrySetCurrentNode(0);

    EXPECT_EQ(true, res);
    EXPECT_EQ(0, sequence->GetCurrentNodeId());
    EXPECT_EQ(true, sequence->HasCurrentNode());
}

TEST_F(SequenceTest, TrySetCurrentNodeReturnsFalseOnFailure)
{
    auto res = sequence->TrySetEntryPoint(1);

    EXPECT_EQ(false, res);
    EXPECT_EQ(-1, sequence->GetCurrentNodeId());
    EXPECT_EQ(false, sequence->HasCurrentNode());
}

TEST_F(SequenceTest, FindCurrentNodeWorksIfNodeFound)
{
    sequence->AddEndSequenceNode(0);
    [[maybe_unused]] auto res = sequence->TrySetCurrentNode(0);

    auto current_node = sequence->FindCurrentNode();
    auto exists = current_node != nullptr;

    EXPECT_EQ(true, exists);
}

TEST_F(SequenceTest, FindCurrentNodeReturnsNullIfNodeNotFound)
{
    auto current_node = sequence->FindCurrentNode();
    auto exists = current_node != nullptr;

    EXPECT_EQ(false, exists);
}

// =============================================================================
// RESET TO ENTRY TESTS
// =============================================================================

TEST_F(SequenceTest, ResetCurrentNodeToEntryWorksIfHasEntryPoint)
{
    sequence->AddEndSequenceNode(0);
    [[maybe_unused]] auto res = sequence->TrySetEntryPoint(0);
    sequence->AddEndSequenceNode(1);
    [[maybe_unused]] auto res2 = sequence->TrySetCurrentNode(1);

    EXPECT_EQ(1, sequence->GetCurrentNodeId());

    sequence->ResetCurrentNodeToEntry();
    EXPECT_EQ(0, sequence->GetCurrentNodeId());
}

TEST_F(SequenceTest, ResetCurrentNodeToEntryDoesNothingIfNoEntryPoint)
{
    sequence->AddEndSequenceNode(0);
    [[maybe_unused]] auto res = sequence->TrySetCurrentNode(0);

    EXPECT_EQ(0, sequence->GetCurrentNodeId());

    sequence->ResetCurrentNodeToEntry();

    EXPECT_EQ(0, sequence->GetCurrentNodeId());
}

// =============================================================================
// Get Transition To Node TESTS
// =============================================================================

TEST_F(SequenceTest, FindTransitionDestinationWorksCorrectly)
{
    auto action_node_id = 0;
    sequence->AddActionSequenceNode(action_node_id, 0);
    auto second_node_id = 1;
    sequence->AddActionSequenceNode(second_node_id, 0);
    auto second_node = sequence->FindNodeById(second_node_id);
    [[maybe_unused]] auto res1 = sequence->TryAddTransition(0, action_node_id, second_node_id, {});

    auto transition = sequence->FindTransitionsFrom(action_node_id).front();

    auto to_node = sequence->FindTransitionDestination(transition);

    EXPECT_EQ(second_node, to_node);
    EXPECT_EQ(second_node->GetNodeId(), to_node->GetNodeId());
}

// Other tests
TEST_F(SequenceTest, HasNodeWorksCorrectly)
{
    EXPECT_FALSE(sequence->HasNode(0));

    sequence->AddEndSequenceNode(0);
    EXPECT_TRUE(sequence->HasNode(0));
    EXPECT_FALSE(sequence->HasNode(1));
}

TEST_F(SequenceTest, FindNodeByIdWorksCorrectly)
{
    EXPECT_EQ(nullptr, sequence->FindNodeById(0));

    sequence->AddActionSequenceNode(5, 10);
    auto node = sequence->FindNodeById(5);
    EXPECT_NE(nullptr, node);
    EXPECT_EQ(5, node->GetNodeId());

    EXPECT_EQ(nullptr, sequence->FindNodeById(99));
}

TEST_F(SequenceTest, SupportsNonSequentialNodeIds)
{
    sequence->AddActionSequenceNode(10, 1);
    sequence->AddActionSequenceNode(25, 2);
    sequence->AddEndSequenceNode(100);

    EXPECT_EQ(3, sequence->GetNodeCount());
    EXPECT_TRUE(sequence->HasNode(10));
    EXPECT_TRUE(sequence->HasNode(25));
    EXPECT_TRUE(sequence->HasNode(100));
    EXPECT_FALSE(sequence->HasNode(0));
    EXPECT_FALSE(sequence->HasNode(11));
}