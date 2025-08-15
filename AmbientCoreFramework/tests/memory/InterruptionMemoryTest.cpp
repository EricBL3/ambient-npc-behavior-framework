/*
* InterruptionMemoryTest.cpp
 *
 * Unit tests for the InterruptionMemory class.
 * Validates interruption context storage and three-key matching behavior
 * for action resumption in ambient characters.
 *
 * Author: Eric Buitrón López
 * Created: 8/12/2025
 */

#include <gtest/gtest.h>
#include "memory/InterruptionMemory.h"

// =============================================================================
// CONSTRUCTION TESTS
// =============================================================================

TEST(InterruptionMemoryTest, ConstructorInitializesCorrectly)
{
    InterruptionMemory memory(5, 10, 15, 20, 456);
    EXPECT_EQ(5, memory.GetInterruptedActionId());
    EXPECT_EQ(10, memory.GetInterruptedSequenceId());
    EXPECT_EQ(15, memory.GetInterruptedSequenceNodeId());
    EXPECT_EQ(20, memory.GetInterruptedTargetEntityId());
    EXPECT_EQ(456, memory.GetLastUsedTime());
}

TEST(InterruptionMemoryTest, ConstructorHandlesNullEntity) {
    InterruptionMemory memory(5, 10, 15, -1, 456);  // -1 indicates null entity
    EXPECT_EQ(-1, memory.GetInterruptedTargetEntityId());
}

TEST(InterruptionMemoryTest, RejectsNegativeActionId) {
    EXPECT_THROW(InterruptionMemory(-1, 10, 15, 20, 100), std::invalid_argument);
}

TEST(InterruptionMemoryTest, RejectsNegativeSequenceId) {
    EXPECT_THROW(InterruptionMemory(5, -1, 15, 20, 100), std::invalid_argument);
}

TEST(InterruptionMemoryTest, RejectsNegativeNodeId) {
    EXPECT_THROW(InterruptionMemory(5, 10, -1, 20, 100), std::invalid_argument);
}

TEST(InterruptionMemoryTest, RejectsInvalidEntityIds) {
    EXPECT_THROW(InterruptionMemory(5, 10, 15, -2, 100), std::invalid_argument);  // -2 not allowed
    EXPECT_THROW(InterruptionMemory(5, 10, 15, -999, 100), std::invalid_argument); // Other negatives not allowed
}

TEST(InterruptionMemoryTest, RejectsNegativeTime) {
    EXPECT_THROW(InterruptionMemory(5, 10, 15, 20, -1), std::invalid_argument);
}

// =============================================================================
// MEMORY MATCHING TESTS
// =============================================================================

TEST(InterruptionMemoryTest, MatchesRequiresAllThreeKeys) {
    InterruptionMemory base_memory(5, 10, 15, 20, 100);
    InterruptionMemory same_context(5, 10, 15, 25, 200);  // Different entity, same context

    EXPECT_TRUE(base_memory.MatchesMemory(same_context));
    EXPECT_TRUE(same_context.MatchesMemory(base_memory));
}

TEST(InterruptionMemoryTest, DoesNotMatchDifferentAction) {
    InterruptionMemory memory1(5, 10, 15, 20, 100);
    InterruptionMemory memory2(7, 10, 15, 20, 100);  // Different action

    EXPECT_FALSE(memory1.MatchesMemory(memory2));
    EXPECT_FALSE(memory2.MatchesMemory(memory1));
}

TEST(InterruptionMemoryTest, DoesNotMatchDifferentSequence) {
    InterruptionMemory memory1(5, 10, 15, 20, 100);
    InterruptionMemory memory2(5, 12, 15, 20, 100);  // Different sequence

    EXPECT_FALSE(memory1.MatchesMemory(memory2));
    EXPECT_FALSE(memory2.MatchesMemory(memory1));
}

TEST(InterruptionMemoryTest, DoesNotMatchDifferentNode) {
    InterruptionMemory memory1(5, 10, 15, 20, 100);
    InterruptionMemory memory2(5, 10, 17, 20, 100);  // Different node

    EXPECT_FALSE(memory1.MatchesMemory(memory2));
    EXPECT_FALSE(memory2.MatchesMemory(memory1));
}

TEST(InterruptionMemoryTest, EntityIdDoesNotAffectMatching) {
    InterruptionMemory memory1(5, 10, 15, 20, 100);   // Entity 20
    InterruptionMemory memory2(5, 10, 15, 25, 200);   // Entity 25
    InterruptionMemory memory3(5, 10, 15, -1, 300);   // Null entity

    // All should match because action/sequence/node are the same
    EXPECT_TRUE(memory1.MatchesMemory(memory2));
    EXPECT_TRUE(memory1.MatchesMemory(memory3));
    EXPECT_TRUE(memory2.MatchesMemory(memory3));
}