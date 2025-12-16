#include <gtest/gtest.h>
#include "memory/ActionMemory.h"

using namespace AmbientCharacterBehavior;

// =============================================================================
// CONSTRUCTION TESTS
// =============================================================================

TEST(ActionMemoryTest, ConstructorInitializesCorrectly) {
    ActionMemory memory(20, 3, 456);
    EXPECT_EQ(20, memory.GetActionId());
    EXPECT_EQ(3, memory.GetTargetEntityId());
    EXPECT_EQ(456, memory.GetCreationTime());
}

TEST(ActionMemoryTest, RejectsNegativeActionIdAndTime) {
    EXPECT_THROW(ActionMemory(-1, 1, 100), std::invalid_argument);  // Negative action
    EXPECT_THROW(ActionMemory(1, 1, -1), std::invalid_argument);    // Negative time
}

TEST(ActionMemoryTest, AcceptsMinusOneForNullEntity) {
    EXPECT_NO_THROW(ActionMemory(1, -1, 100));  // -1 is valid for entity
}

TEST(ActionMemoryTest, RejectsInvalidEntityIds) {
    EXPECT_THROW(ActionMemory(1, -2, 100), std::invalid_argument);   // -2 not allowed
    EXPECT_THROW(ActionMemory(1, -999, 100), std::invalid_argument); // Other negatives
}

TEST(ActionMemoryTest, AcceptsZeroValues) {
    EXPECT_NO_THROW(ActionMemory(0, 0, 0));
}

// =============================================================================
// MEMORY MATCHING TESTS
// =============================================================================

TEST(ActionMemoryTest, MatchesActionMemory) {
    ActionMemory memory1(5, 3, 100);
    ActionMemory memory2(5, 3, 200);
    
    EXPECT_TRUE(memory1.MatchesMemory(memory2));
    EXPECT_TRUE(memory2.MatchesMemory(memory1));  
}

TEST(ActionMemoryTest, RequiresBothActionAndEntityToMatch) {
    ActionMemory base_memory(5, 3, 100);
    ActionMemory different_action(7, 3, 100);
    ActionMemory different_entity(5, 7, 100);

    EXPECT_FALSE(base_memory.MatchesMemory(different_action));
    EXPECT_FALSE(base_memory.MatchesMemory(different_entity));
}

