#include <gtest/gtest.h>
#include "memory/TransitionMemory.h"

using namespace AmbientCharacterBehavior;

// =============================================================================
// CONSTRUCTION TESTS
// =============================================================================

TEST(TransitionMemoryTest, ConstructorInitializesCorrectly)
{
    TransitionMemory memory(0, 20, 456);
    EXPECT_EQ(20, memory.GetTargetNodeId());
    EXPECT_EQ(456, memory.GetCreationTime());
}

TEST(TransitionMemoryTest, RejectsNegativeValues) {
    EXPECT_THROW(TransitionMemory(0, -1, 100), std::invalid_argument);
    EXPECT_THROW(TransitionMemory(0, 1, -1), std::invalid_argument);
    EXPECT_THROW(TransitionMemory(-1, 1, 10), std::invalid_argument);
}

TEST(TransitionMemoryTest, AcceptsZeroValues) {
    EXPECT_NO_THROW(TransitionMemory(0, 0, 0));
}

// =============================================================================
// MEMORY MATCHING TESTS
// =============================================================================

TEST(TransitionMemoryTest, MatchesMemoryWithSameNodeId) {
    TransitionMemory memory1(0, 5, 100);
    TransitionMemory memory2(0, 5, 200);
    
    EXPECT_TRUE(memory1.MatchesMemory(memory2));
    EXPECT_TRUE(memory2.MatchesMemory(memory1));  
}

TEST(TransitionMemoryTest, DoesNotMatchDifferentNodeId) {
    TransitionMemory memory1(0, 5, 100);
    TransitionMemory memory2(0, 7, 100);
    
    EXPECT_FALSE(memory1.MatchesMemory(memory2));
    EXPECT_FALSE(memory2.MatchesMemory(memory1));
}

TEST(TransitionMemoryTest, RequiresBothSequenceAndNodeToMatch) {
    TransitionMemory seq1_node5(1, 5, 100);  // Sequence 1, Node 5
    TransitionMemory seq2_node5(2, 5, 100);  // Sequence 2, Node 5 (different sequence!)
    TransitionMemory seq1_node7(1, 7, 100);  // Sequence 1, Node 7 (different node)

    // Same node in different sequences should NOT match
    EXPECT_FALSE(seq1_node5.MatchesMemory(seq2_node5));
    EXPECT_FALSE(seq2_node5.MatchesMemory(seq1_node5));

    // Different nodes in same sequence should NOT match
    EXPECT_FALSE(seq1_node5.MatchesMemory(seq1_node7));

    // Only exact match should match
    TransitionMemory duplicate(1, 5, 200);  // Same sequence and node, different time
    EXPECT_TRUE(seq1_node5.MatchesMemory(duplicate));
}