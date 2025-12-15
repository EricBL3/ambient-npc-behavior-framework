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
    EXPECT_EQ(456, memory.GetLastUsedTime());
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