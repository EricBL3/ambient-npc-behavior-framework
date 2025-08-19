/*
* TransitionMemoryTest.cpp
 *
 * Unit tests for the TransitionMemory class.
 * Validates transition decision tracking and memory matching behavior.
 *
 * Author: Eric Buitrón López
 * Created: 8/11/2025
 */

#include <gtest/gtest.h>
#include "memory/TransitionMemory.h"

using namespace AmbientCharacterBehavior;

// =============================================================================
// CONSTRUCTION TESTS
// =============================================================================

TEST(TransitionMemoryTest, ConstructorInitializesCorrectly)
{
    TransitionMemory memory(20, 456);
    EXPECT_EQ(20, memory.GetTargetNodeId());
    EXPECT_EQ(456, memory.GetLastUsedTime());
}

TEST(TransitionMemoryTest, RejectsNegativeValues) {
    EXPECT_THROW(TransitionMemory(-1, 100), std::invalid_argument);
    EXPECT_THROW(TransitionMemory(1, -1), std::invalid_argument);
}

TEST(TransitionMemoryTest, AcceptsZeroValues) {
    EXPECT_NO_THROW(TransitionMemory(0, 0));
}

// =============================================================================
// MEMORY MATCHING TESTS
// =============================================================================

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