
#include <gtest/gtest.h>

#include "behavior/ActionSequenceNode.h"
#include "behavior/EndSequenceNode.h"
#include "behavior/NestedSequenceNode.h"

using namespace AmbientCharacterBehavior;

// =============================================================================
// CONSTRUCTOR TESTS
// =============================================================================

TEST(SequenceNodeTest, ActionSequenceNodeConstructorInitializesCorrectly)
{
    ActionSequenceNode action_node(0, 0);

    EXPECT_EQ(0, action_node.GetNodeId());
    EXPECT_FALSE(action_node.HasCompletedExecution());
    EXPECT_EQ(0, action_node.GetTargetActionId());
    EXPECT_EQ(SequenceNodeType::ACTION_NODE, action_node.GetNodeType());
}

TEST(SequenceNodeTest, NestedSequenceNodeConstructorInitializesCorrectly)
{
    NestedSequenceNode sequence_node(1, 0);

    EXPECT_EQ(1, sequence_node.GetNodeId());
    EXPECT_FALSE(sequence_node.HasCompletedExecution());
    EXPECT_EQ(0, sequence_node.GetTargetSequenceId());
    EXPECT_EQ(SequenceNodeType::NESTED_SEQUENCE_NODE, sequence_node.GetNodeType());
}

TEST(SequenceNodeTest, EndSequenceNodeConstructorInitializesCorrectly)
{
    EndSequenceNode end_node(2);

    EXPECT_EQ(2, end_node.GetNodeId());
    EXPECT_FALSE(end_node.HasCompletedExecution());
    EXPECT_EQ(SequenceNodeType::END_SEQUENCE_NODE, end_node.GetNodeType());
}

TEST(SequenceNodeTest, ConstructorRejectsNegativeNodeId)
{

    EXPECT_THROW(ActionSequenceNode(-1, 0), std::invalid_argument);
    EXPECT_THROW(NestedSequenceNode(-50, 0), std::invalid_argument);
    EXPECT_THROW(EndSequenceNode(-999), std::invalid_argument);
}

// =============================================================================
// HAS COMPLETED TESTS
// =============================================================================

TEST(SequenceNodeTest, CanModifyHasCompleted)
{
    EndSequenceNode end_node(2);

    end_node.MarkAsCompleted();

    EXPECT_TRUE(end_node.HasCompletedExecution());

    end_node.ResetCompletion();

    EXPECT_FALSE(end_node.HasCompletedExecution());
}
