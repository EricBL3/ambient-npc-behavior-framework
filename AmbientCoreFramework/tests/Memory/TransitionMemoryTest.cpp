#include <gtest/gtest.h>
#include "../../src/Memory/TransitionMemory.h"

TEST(TransitionMemoryTest, ConstructorInitializesCorrectly)
{
    TransitionMemory memory(20, 456);
    EXPECT_EQ(20, memory.GetTargetNodeId());
    EXPECT_EQ(456, memory.GetLastUsedTime());
}

TEST(TransitionMemoryTest, ConstructorHandlesEdgeCases) {
    TransitionMemory zero_time(1, 0);
    TransitionMemory zero_node(0, 100);
    
    EXPECT_EQ(0, zero_time.GetLastUsedTime());
    EXPECT_EQ(0, zero_node.GetTargetNodeId());
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

TEST(TransitionMemoryTest, MatchesItselfCorrectly) {
    TransitionMemory memory(10, 500);
    
    EXPECT_TRUE(memory.MatchesMemory(memory));
}

TEST(TransitionMemoryTest, IsOlderThanWorksCorrectly) {
    TransitionMemory older_memory(1, 100);
    TransitionMemory newer_memory(2, 200);
    
    EXPECT_TRUE(older_memory.IsOlderThan(newer_memory));
    EXPECT_FALSE(newer_memory.IsOlderThan(older_memory));
}

TEST(TransitionMemoryTest, EqualTimesAreNotOlder) {
    TransitionMemory memory1(1, 100);
    TransitionMemory memory2(5, 100);  // Same time, different nodes
    
    EXPECT_FALSE(memory1.IsOlderThan(memory2));
    EXPECT_FALSE(memory2.IsOlderThan(memory1));
}

TEST(TransitionMemoryTest, IsNotOlderThanItself) {
    TransitionMemory memory(10, 500);
    
    EXPECT_FALSE(memory.IsOlderThan(memory));
}

TEST(TransitionMemoryTest, ConstructorRejectsNegativeNodeId) {
    EXPECT_THROW(TransitionMemory(-1, 100), std::invalid_argument);
    EXPECT_THROW(TransitionMemory(-999, 0), std::invalid_argument);
}

TEST(TransitionMemoryTest, ConstructorRejectsNegativeTime) {
    EXPECT_THROW(TransitionMemory(1, -1), std::invalid_argument);
    EXPECT_THROW(TransitionMemory(0, -500), std::invalid_argument);
}

TEST(TransitionMemoryTest, ConstructorRejectsBothNegative) {
    EXPECT_THROW(TransitionMemory(-1, -1), std::invalid_argument);
}

TEST(TransitionMemoryTest, ConstructorAcceptsBoundaryValues) {
    // These should work fine
    EXPECT_NO_THROW(TransitionMemory(0, 0));
    EXPECT_NO_THROW(TransitionMemory(1, 0));
    EXPECT_NO_THROW(TransitionMemory(0, 1));
}