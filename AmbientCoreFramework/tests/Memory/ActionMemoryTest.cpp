//
// Created by Eric on 8/11/2025.
//

#include <gtest/gtest.h>
#include "Memory/ActionMemory.h"

TEST(ActionMemoryTest, ConstructorInitializesCorrectly) {
    ActionMemory memory(20, 3, 456);
    EXPECT_EQ(20, memory.GetActionId());
    EXPECT_EQ(3, memory.GetTargetEntityId());
    EXPECT_EQ(456, memory.GetLastUsedTime());
}

TEST(ActionMemoryTest, ConstructorHandlesEdgeCases) {
    ActionMemory zero_time(1, 1, 0);
    ActionMemory zero_action(0, 1, 100);
    ActionMemory zero_target_entity(1, 0, 100);
    
    EXPECT_EQ(0, zero_time.GetLastUsedTime());
    EXPECT_EQ(0, zero_action.GetActionId());
    EXPECT_EQ(0, zero_target_entity.GetTargetEntityId());
}

TEST(ActionMemoryTest, MatchesMemory) {
    ActionMemory memory1(5, 3, 100);
    ActionMemory memory2(5, 3, 200);
    
    EXPECT_TRUE(memory1.MatchesMemory(memory2));
    EXPECT_TRUE(memory2.MatchesMemory(memory1));  
}

TEST(ActionMemoryTest, DoesNotMatchDifferentActionId) {
    ActionMemory memory1(5, 2, 100);
    ActionMemory memory2(7, 2, 100);
    
    EXPECT_FALSE(memory1.MatchesMemory(memory2));
    EXPECT_FALSE(memory2.MatchesMemory(memory1));
}

TEST(ActionMemoryTest, DoesNotMatchDifferentTargetEntityId) {
    ActionMemory memory1(5, 5, 100);
    ActionMemory memory2(5, 2, 100);

    EXPECT_FALSE(memory1.MatchesMemory(memory2));
    EXPECT_FALSE(memory2.MatchesMemory(memory1));
}

TEST(ActionMemoryTest, MatchesItselfCorrectly) {
    ActionMemory memory(10, 2, 500);
    
    EXPECT_TRUE(memory.MatchesMemory(memory));
}

TEST(ActionMemoryTest, IsOlderThanWorksCorrectly) {
    ActionMemory older_memory(1, 2, 100);
    ActionMemory newer_memory(2, 3, 200);
    
    EXPECT_TRUE(older_memory.IsOlderThan(newer_memory));
    EXPECT_FALSE(newer_memory.IsOlderThan(older_memory));
}

TEST(ActionMemoryTest, EqualTimesAreNotOlder) {
    ActionMemory memory1(1, 3, 100);
    ActionMemory memory2(5, 4, 100);
    
    EXPECT_FALSE(memory1.IsOlderThan(memory2));
    EXPECT_FALSE(memory2.IsOlderThan(memory1));
}

TEST(ActionMemoryTest, IsNotOlderThanItself) {
    ActionMemory memory(10, 4, 500);
    
    EXPECT_FALSE(memory.IsOlderThan(memory));
}

TEST(ActionMemoryTest, ConstructorRejectsNegativeActionId) {
    EXPECT_THROW(ActionMemory(-1, 1, 100), std::invalid_argument);
    EXPECT_THROW(ActionMemory(-999, 1, 0), std::invalid_argument);
}

TEST(ActionMemoryTest, ConstructorRejectsNegativeTargetEntityId) {
    EXPECT_THROW(ActionMemory(1, -1, 100), std::invalid_argument);
    EXPECT_THROW(ActionMemory(999, -999, 0), std::invalid_argument);
}

TEST(ActionMemoryTest, ConstructorRejectsNegativeTime) {
    EXPECT_THROW(ActionMemory(1, 1, -1), std::invalid_argument);
    EXPECT_THROW(ActionMemory(0, 0, -500), std::invalid_argument);
}

TEST(ActionMemoryTest, ConstructorRejectsAllNegative) {
    EXPECT_THROW(ActionMemory(-1, -1, -1), std::invalid_argument);
}

TEST(ActionMemoryTest, ConstructorAcceptsBoundaryValues) {
    EXPECT_NO_THROW(ActionMemory(0, 0, 0));
    EXPECT_NO_THROW(ActionMemory(1, 0, 0));
    EXPECT_NO_THROW(ActionMemory(0, 1, 0));
    EXPECT_NO_THROW(ActionMemory(0, 0, 1));
    EXPECT_NO_THROW(ActionMemory(1, 1, 0));
    EXPECT_NO_THROW(ActionMemory(1, 1, 1));
}