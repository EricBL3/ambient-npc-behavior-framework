/**
 * @file SequenceNodeTest.cpp
 * @brief Unit tests for all implementations of SequenceNode
 * @author Eric Buitrón López
 * @date 8/18/2025
 *
 *
*/

#include <gtest/gtest.h>

#include "behavior/Action.h"
#include "behavior/ActionSequenceNode.h"
#include "behavior/EndSequenceNode.h"
#include "behavior/NestedSequenceNode.h"
#include "behavior/Sequence.h"

TEST(SequenceNodeTest, ActionSequenceNodeConstructorInitializesCorrectly)
{
    Action action{};
    ActionSequenceNode action_node(0, &action);

    EXPECT_EQ(0, action_node.GetNodeId());
    EXPECT_FALSE(action_node.GetHasCompleted());
    EXPECT_EQ(&action, action_node.GetTargetAction());
    EXPECT_EQ(SequenceNodeType::ACTION_NODE, action_node.GetNodeType());
}

TEST(SequenceNodeTest, NestedSequenceNodeConstructorInitializesCorrectly)
{
    Sequence sequence{};
    NestedSequenceNode sequence_node(1, &sequence);

    EXPECT_EQ(1, sequence_node.GetNodeId());
    EXPECT_FALSE(sequence_node.GetHasCompleted());
    EXPECT_EQ(&sequence, sequence_node.GetTargetSequence());
    EXPECT_EQ(SequenceNodeType::NESTED_SEQUENCE_NODE, sequence_node.GetNodeType());
}

TEST(SequenceNodeTest, EndSequenceNodeConstructorInitializesCorrectly)
{
    EndSequenceNode end_node(2);

    EXPECT_EQ(2, end_node.GetNodeId());
    EXPECT_FALSE(end_node.GetHasCompleted());
    EXPECT_EQ(SequenceNodeType::END_SEQUENCE_NODE, end_node.GetNodeType());
}

TEST(SequenceNodeTest, ConstructorRejectsNegativeNodeId)
{
    Action action{};
    Sequence sequence{};

    EXPECT_THROW(ActionSequenceNode(-1, &action), std::invalid_argument);
    EXPECT_THROW(NestedSequenceNode(-50, &sequence), std::invalid_argument);
    EXPECT_THROW(EndSequenceNode(-999), std::invalid_argument);
}

TEST(SequenceNodeTest, CanModifyHasCompleted)
{
    EndSequenceNode end_node(2);

    end_node.MarkAsCompleted();

    EXPECT_TRUE(end_node.GetHasCompleted());

    end_node.ResetCompletion();

    EXPECT_FALSE(end_node.GetHasCompleted());
}
