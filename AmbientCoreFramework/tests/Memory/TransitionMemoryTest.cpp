//
// Created by Eric on 8/11/2025.
//

#include <gtest/gtest.h>
#include "Memory/TransitionMemory.h"

TEST(TransitionMemoryTest, ConstructorInitializesCorrectly)
{
    TransitionMemory memory(20, 456);
    EXPECT_EQ(20, memory.GetTargetNodeId());
    EXPECT_EQ(456, memory.GetLastUsedTime());
}

TEST(TransitionMemoryTest, MatchesMemoryWithSameNodeId) {
    TransitionMemory memory1(5, 100);
    TransitionMemory memory2(5, 200);  
    
    EXPECT_TRUE(memory1.MatchesMemory(memory2));
    EXPECT_TRUE(memory2.MatchesMemory(memory1));  
}

TEST(TransitionMemoryTest, DoesNotMatchDifferentNodeId) {
    TransitionMemory memory1(5, 100);
    TransitionMemory memory2(7, 100);  
    
    EXPECT_FALSE(memory1.MatchesMemory(memory2));
    EXPECT_FALSE(memory2.MatchesMemory(memory1));
}
TEST(TransitionMemoryTest, RejectsNegativeValues) {
    EXPECT_THROW(TransitionMemory(-1, 100), std::invalid_argument);
    EXPECT_THROW(TransitionMemory(1, -1), std::invalid_argument);
}

TEST(TransitionMemoryTest, AcceptsZeroValues) {
    EXPECT_NO_THROW(TransitionMemory(0, 0));
}