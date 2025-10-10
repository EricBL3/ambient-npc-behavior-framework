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
    EXPECT_EQ(456, memory.GetLastUsedTime());
}

TEST(ActionMemoryTest, RejectsNegativeValues) {
    EXPECT_THROW(ActionMemory(-1, 1, 100), std::invalid_argument);
    EXPECT_THROW(ActionMemory(1, 1, -1), std::invalid_argument);
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

