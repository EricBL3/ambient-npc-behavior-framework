//
// Created by Eric on 8/12/2025.
//

#include <gtest/gtest.h>
#include "Memory/MemorySystem.h"

class MemorySystemTest : public testing::Test {
protected:
    void SetUp() override {
        memory_system = std::make_unique<MemorySystem>(3, 3, 2);
    }

    std::unique_ptr<MemorySystem> memory_system;
};

// =============================================================================
// CONSTRUCTION AND CONFIGURATION TESTS
// =============================================================================

TEST_F(MemorySystemTest, ConstructorInitializesEmptyMemoryCounts) {
    MemorySystem system(5, 10, 3);

    EXPECT_EQ(0, system.GetTransitionMemoryCount());
    EXPECT_EQ(0, system.GetActionMemoryCount());
    EXPECT_EQ(0, system.GetInterruptionMemoryCount());
}

TEST_F(MemorySystemTest, ConstructorSetsTransitionCapacity) {
    MemorySystem system(5, 10, 3);
    EXPECT_EQ(5, system.GetMaxTransitionMemories());
}

TEST_F(MemorySystemTest, ConstructorSetsActionCapacity) {
    MemorySystem system(5, 10, 3);
    EXPECT_EQ(10, system.GetMaxActionMemories());
}

TEST_F(MemorySystemTest, ConstructorSetsInterruptionCapacity) {
    MemorySystem system(5, 10, 3);
    EXPECT_EQ(3, system.GetMaxInterruptionMemories());
}

// Configuration Tests
TEST_F(MemorySystemTest, SetMaxMemoriesRejectsZeroValues) {
    memory_system->SetMaxTransitionMemories(0);
    memory_system->SetMaxActionMemories(0);
    memory_system->SetMaxInterruptionMemories(0);

    // Should reject and keep original values
    EXPECT_EQ(3, memory_system->GetMaxTransitionMemories());
    EXPECT_EQ(3, memory_system->GetMaxActionMemories());
    EXPECT_EQ(2, memory_system->GetMaxInterruptionMemories());
}

TEST_F(MemorySystemTest, SetMaxMemoriesRejectsNegativeValues) {
    memory_system->SetMaxTransitionMemories(-5);
    memory_system->SetMaxActionMemories(-5);
    memory_system->SetMaxInterruptionMemories(-5);

    EXPECT_EQ(3, memory_system->GetMaxTransitionMemories());  // Should be unchanged
    EXPECT_EQ(3, memory_system->GetMaxActionMemories());  // Should be unchanged
    EXPECT_EQ(2, memory_system->GetMaxInterruptionMemories());  // Should be unchanged
}

TEST_F(MemorySystemTest, SetMaxTransitionMemoriesEnforcesNewCapacityImmediately) {
    // Fill with 3 memories
    memory_system->UpdateTransitionMemory(1, 100);
    memory_system->UpdateTransitionMemory(2, 200);
    memory_system->UpdateTransitionMemory(3, 300);

    // Reduce capacity to 2
    memory_system->SetMaxTransitionMemories(2);

    // Should immediately remove oldest
    EXPECT_EQ(2, memory_system->GetTransitionMemoryCount());
    EXPECT_EQ(nullptr, memory_system->FindTransitionMemory(1));  // Oldest removed
}

TEST_F(MemorySystemTest, SetMaxActionMemoriesEnforcesNewCapacityImmediately) {
    memory_system->UpdateActionMemory(1, 10, 100);
    memory_system->UpdateActionMemory(2, 20, 200);
    memory_system->UpdateActionMemory(3, 30, 300);

    memory_system->SetMaxActionMemories(2);

    EXPECT_EQ(2, memory_system->GetActionMemoryCount());
    EXPECT_EQ(nullptr, memory_system->FindActionMemory(1, 10));  // Oldest removed
}

TEST_F(MemorySystemTest, SetMaxInterruptionMemoriesEnforcesNewCapacityImmediately) {
    memory_system->UpdateInterruptionMemory(1, 10, 11, 20, 100);
    memory_system->UpdateInterruptionMemory(2, 10, 12, 21, 200);

    memory_system->SetMaxInterruptionMemories(1);

    EXPECT_EQ(1, memory_system->GetInterruptionMemoryCount());
    EXPECT_EQ(nullptr, memory_system->FindInterruptionMemory(1, 10, 11));  // Oldest removed
}

// =============================================================================
// FIND MEMORY TESTS
// =============================================================================

// Transition Memory Find Tests
TEST_F(MemorySystemTest, FindTransitionMemoryReturnsCorrectMemory) {
    memory_system->UpdateTransitionMemory(5, 100);

    TransitionMemory* found = memory_system->FindTransitionMemory(5);
    ASSERT_NE(nullptr, found);
    EXPECT_EQ(5, found->GetTargetNodeId());
    EXPECT_EQ(100, found->GetLastUsedTime());
}

TEST_F(MemorySystemTest, FindTransitionMemoryReturnsNullptrWhenNotFound) {
    EXPECT_EQ(nullptr, memory_system->FindTransitionMemory(999));
}

TEST_F(MemorySystemTest, FindTransitionMemoryMatchesOnlyNodeId) {
    memory_system->UpdateTransitionMemory(5, 100);
    memory_system->UpdateTransitionMemory(7, 200);

    EXPECT_NE(nullptr, memory_system->FindTransitionMemory(5));
    EXPECT_NE(nullptr, memory_system->FindTransitionMemory(7));
    EXPECT_EQ(nullptr, memory_system->FindTransitionMemory(9));  // Not found
}

// Action Memory Find Tests
TEST_F(MemorySystemTest, FindActionMemoryReturnsCorrectMemory) {
    memory_system->UpdateActionMemory(3, 10, 100);

    ActionMemory* found = memory_system->FindActionMemory(3, 10);
    ASSERT_NE(nullptr, found);
    EXPECT_EQ(3, found->GetActionId());
    EXPECT_EQ(10, found->GetTargetEntityId());
}

TEST_F(MemorySystemTest, FindActionMemoryReturnsNullptrWhenNotFound) {
    EXPECT_EQ(nullptr, memory_system->FindActionMemory(999, 888));
}

TEST_F(MemorySystemTest, FindActionMemoryRequiresBothKeysToMatch) {
    memory_system->UpdateActionMemory(3, 10, 100);

    EXPECT_NE(nullptr, memory_system->FindActionMemory(3, 10));   // Both match
    EXPECT_EQ(nullptr, memory_system->FindActionMemory(3, 11));   // Different entity
    EXPECT_EQ(nullptr, memory_system->FindActionMemory(4, 10));   // Different action
}

// Interruption Memory Find Tests
TEST_F(MemorySystemTest, FindInterruptionMemoryReturnsCorrectMemory) {
    memory_system->UpdateInterruptionMemory(5, 10, 15, 20, 100);

    InterruptionMemory* found = memory_system->FindInterruptionMemory(5, 10, 15);
    ASSERT_NE(nullptr, found);
    EXPECT_EQ(5, found->GetInterruptedActionId());
    EXPECT_EQ(10, found->GetInterruptedSequenceId());
    EXPECT_EQ(15, found->GetInterruptedSequenceNodeId());
}

TEST_F(MemorySystemTest, FindInterruptionMemoryReturnsNullptrWhenNotFound) {
    EXPECT_EQ(nullptr, memory_system->FindInterruptionMemory(999, 888, 777));
}

TEST_F(MemorySystemTest, FindInterruptionMemoryRequiresAllThreeKeysToMatch) {
    memory_system->UpdateInterruptionMemory(5, 10, 15, 20, 100);

    EXPECT_NE(nullptr, memory_system->FindInterruptionMemory(5, 10, 15));  // All match
    EXPECT_EQ(nullptr, memory_system->FindInterruptionMemory(6, 10, 15));  // Different action
    EXPECT_EQ(nullptr, memory_system->FindInterruptionMemory(5, 11, 15));  // Different sequence
    EXPECT_EQ(nullptr, memory_system->FindInterruptionMemory(5, 10, 16));  // Different node
}

/*
// =============================================================================
// MEMORY UPDATE TESTS
// =============================================================================

// Transition Memory Updates
TEST_F(MemorySystemTest, UpdateTransitionMemoryCreatesNewEntryAndReturnsTrue) {
    EXPECT_TRUE(memory_system->UpdateTransitionMemory(5, 100));
    EXPECT_EQ(1, memory_system->GetTransitionMemoryCount());
}

TEST_F(MemorySystemTest, UpdateTransitionMemoryStoresCorrectData) {
    memory_system->UpdateTransitionMemory(5, 100);

    TransitionMemory* found = memory_system->FindTransitionMemory(5);
    ASSERT_NE(nullptr, found);
    EXPECT_EQ(5, found->GetTargetNodeId());
    EXPECT_EQ(100, found->GetLastUsedTime());
}

TEST_F(MemorySystemTest, UpdateTransitionMemoryReplacesExistingEntry) {
    memory_system->UpdateTransitionMemory(5, 100);
    memory_system->UpdateTransitionMemory(5, 200);

    TransitionMemory* found = memory_system->FindTransitionMemory(5);
    ASSERT_NE(nullptr, found);

    EXPECT_EQ(1, memory_system->GetTransitionMemoryCount());
    EXPECT_EQ(200, found->GetLastUsedTime());
}

TEST_F(MemorySystemTest, TransitionMemoryRespectsCapacityLimit) {
    memory_system->UpdateTransitionMemory(1, 100);
    memory_system->UpdateTransitionMemory(2, 200);
    memory_system->UpdateTransitionMemory(3, 300);
    memory_system->UpdateTransitionMemory(4, 400);  // Should remove oldest

    EXPECT_EQ(3, memory_system->GetTransitionMemoryCount());
    EXPECT_EQ(nullptr, memory_system->FindTransitionMemory(1));
    EXPECT_NE(nullptr, memory_system->FindTransitionMemory(4));
}

TEST_F(MemorySystemTest, InvalidUpdateDoesNotRemoveExistingMemoryAndReturnsFalse) {
    memory_system->UpdateTransitionMemory(5, 100);  // Valid memory
    EXPECT_EQ(1, memory_system->GetTransitionMemoryCount());

    // Invalid update returns false
    EXPECT_FALSE(memory_system->UpdateTransitionMemory(5, -1));  // Invalid time

    // Original memory should still exist (validation failed before removal)
    EXPECT_EQ(1, memory_system->GetTransitionMemoryCount());
    TransitionMemory* found = memory_system->FindTransitionMemory(5);
    ASSERT_NE(nullptr, found);
    EXPECT_EQ(100, found->GetLastUsedTime());  // Original data preserved
}

// Action Memory Updates
TEST_F(MemorySystemTest, UpdateActionMemoryCreatesNewEntry) {
    EXPECT_TRUE(memory_system->UpdateActionMemory(3, 10, 100));
    EXPECT_EQ(1, memory_system->GetActionMemoryCount());
}

TEST_F(MemorySystemTest, UpdateActionMemoryStoresCorrectData) {
    memory_system->UpdateActionMemory(3, 10, 100);

    ActionMemory* found = memory_system->FindActionMemory(3, 10);
    ASSERT_NE(nullptr, found);
    EXPECT_EQ(3, found->GetActionId());
    EXPECT_EQ(10, found->GetTargetEntityId());
    EXPECT_EQ(100, found->GetLastUsedTime());
}

TEST_F(MemorySystemTest, UpdateActionMemoryReplacesExistingEntry) {
    memory_system->UpdateActionMemory(3, 10, 100);
    memory_system->UpdateActionMemory(3, 10, 200);  // Same action+entity, different time

    ActionMemory* found = memory_system->FindActionMemory(3, 10);
    ASSERT_NE(nullptr, found);

    EXPECT_EQ(1, memory_system->GetActionMemoryCount());
    EXPECT_EQ(200, found->GetLastUsedTime());
}

TEST_F(MemorySystemTest, ActionMemoryRespectsCapacityLimit) {
    memory_system->UpdateActionMemory(1, 10, 100);
    memory_system->UpdateActionMemory(2, 20, 200);
    memory_system->UpdateActionMemory(3, 30, 300);
    memory_system->UpdateActionMemory(4, 40, 400);

    EXPECT_EQ(3, memory_system->GetActionMemoryCount());
    EXPECT_EQ(nullptr, memory_system->FindActionMemory(1, 10));  // Oldest removed
    EXPECT_NE(nullptr, memory_system->FindActionMemory(4, 40));  // Newest kept
}

TEST_F(MemorySystemTest, InvalidActionUpdateDoesNotRemoveExistingMemoryAndReturnsFalse) {
    memory_system->UpdateActionMemory(3, 10, 100);  // Valid memory
    EXPECT_EQ(1, memory_system->GetActionMemoryCount());

    // Invalid update returns false
    EXPECT_FALSE(memory_system->UpdateActionMemory(3, 10, -1));  // Invalid time

    // Original memory should still exist (validation failed before removal)
    EXPECT_EQ(1, memory_system->GetActionMemoryCount());
    ActionMemory* found = memory_system->FindActionMemory(3, 10);
    ASSERT_NE(nullptr, found);
    EXPECT_EQ(100, found->GetLastUsedTime());  // Original data preserved
}

// Interruption Memory Updates
TEST_F(MemorySystemTest, UpdateInterruptionMemoryCreatesNewEntryAndReturnsTrue) {
    EXPECT_TRUE(memory_system->UpdateInterruptionMemory(5, 10, 15, 20, 100));
    EXPECT_EQ(1, memory_system->GetInterruptionMemoryCount());
}

TEST_F(MemorySystemTest, UpdateInterruptionMemoryStoresCorrectData) {
    memory_system->UpdateInterruptionMemory(5, 10, 15, 20, 100);

    InterruptionMemory* found = memory_system->FindInterruptionMemory(5, 10, 15);
    ASSERT_NE(nullptr, found);
    EXPECT_EQ(5, found->GetInterruptedActionId());
    EXPECT_EQ(10, found->GetInterruptedSequenceId());
    EXPECT_EQ(15, found->GetInterruptedSequenceNodeId());
    EXPECT_EQ(20, found->GetInterruptedTargetEntityId());
    EXPECT_EQ(100, found->GetLastUsedTime());
}

TEST_F(MemorySystemTest, UpdateInterruptionMemoryReplacesExistingEntry) {
    memory_system->UpdateInterruptionMemory(5, 10, 15, 20, 100);
    memory_system->UpdateInterruptionMemory(5, 10, 15, 25, 200);  // Same keys, different entity+time

    InterruptionMemory* found = memory_system->FindInterruptionMemory(5, 10, 15);
    ASSERT_NE(nullptr, found);

    EXPECT_EQ(1, memory_system->GetInterruptionMemoryCount());
    EXPECT_EQ(25, found->GetInterruptedTargetEntityId());  // Entity updated
    EXPECT_EQ(200, found->GetLastUsedTime());              // Time updated
}

TEST_F(MemorySystemTest, InterruptionMemoryRespectsCapacityLimit) {
    memory_system->UpdateInterruptionMemory(1, 10, 11, 20, 100);
    memory_system->UpdateInterruptionMemory(2, 10, 12, 21, 200);
    memory_system->UpdateInterruptionMemory(3, 10, 13, 22, 300);  // At capacity (2)

    // This should remove the oldest (first entry)
    EXPECT_EQ(2, memory_system->GetInterruptionMemoryCount());
    EXPECT_EQ(nullptr, memory_system->FindInterruptionMemory(1, 10, 11));  // Oldest removed
    EXPECT_NE(nullptr, memory_system->FindInterruptionMemory(3, 10, 13));  // Newest kept
}

TEST_F(MemorySystemTest, InvalidInterruptionUpdateDoesNotRemoveExistingMemoryAndReturnsFalse) {
    memory_system->UpdateInterruptionMemory(5, 10, 15, 20, 100);  // Valid memory
    EXPECT_EQ(1, memory_system->GetInterruptionMemoryCount());

    // Invalid update returns false (invalid entity_id)
    EXPECT_FALSE(memory_system->UpdateInterruptionMemory(5, 10, 15, -2, 200));  // -2 not allowed

    // Original memory should still exist (validation failed before removal)
    EXPECT_EQ(1, memory_system->GetInterruptionMemoryCount());
    InterruptionMemory* found = memory_system->FindInterruptionMemory(5, 10, 15);
    ASSERT_NE(nullptr, found);
    EXPECT_EQ(20, found->GetInterruptedTargetEntityId());  // Original data preserved
    EXPECT_EQ(100, found->GetLastUsedTime());              // Original time preserved
}

// =============================================================================
// RECENCY SELECTION TESTS
// =============================================================================

// Transition Memory Recency Tests
TEST_F(MemorySystemTest, GetLeastRecentlyVisitedNodePrefersNeverUsed) {
    memory_system->UpdateTransitionMemory(5, 100);
    memory_system->UpdateTransitionMemory(7, 200);

    std::vector<int> node_ids = {5, 7, 9};  // 9 never used
    int selected = memory_system->GetLeastRecentlyVisitedNode(node_ids);

    EXPECT_EQ(9, selected);
}

TEST_F(MemorySystemTest, GetLeastRecentlyVisitedNodeSelectsOldestTimestamp) {
    memory_system->UpdateTransitionMemory(5, 100);  // Older
    memory_system->UpdateTransitionMemory(7, 200);  // Newer

    std::vector<int> node_ids = {5, 7};
    int selected = memory_system->GetLeastRecentlyVisitedNode(node_ids);

    EXPECT_EQ(5, selected);
}

TEST_F(MemorySystemTest, GetLeastRecentlyVisitedNodeReturnsMinusOneForEmptyInput) {
    std::vector<int> empty_list;
    int selected = memory_system->GetLeastRecentlyVisitedNode(empty_list);

    EXPECT_EQ(-1, selected);
}

TEST_F(MemorySystemTest, GetLeastRecentlyVisitedNodeHandlesEqualTimestamps) {
    memory_system->UpdateTransitionMemory(5, 100);
    memory_system->UpdateTransitionMemory(7, 100);  // Same timestamp so should pick randomly

    std::vector<int> node_ids = {5, 7};
    int selected = memory_system->GetLeastRecentlyVisitedNode(node_ids);

    EXPECT_TRUE(selected == 5 || selected == 7);
}

// Action Memory Recency Tests
TEST_F(MemorySystemTest, GetLeastRecentlyUsedEntityPrefersNeverUsed) {
    memory_system->UpdateActionMemory(3, 10, 100);
    memory_system->UpdateActionMemory(3, 11, 200);

    std::vector<int> entity_ids = {10, 11, 12};  // 12 never used
    int selected = memory_system->GetLeastRecentlyUsedEntityForAction(3, entity_ids);

    EXPECT_EQ(12, selected);
}

TEST_F(MemorySystemTest, GetLeastRecentlyUsedEntitySelectsOldestTimestamp) {
    memory_system->UpdateActionMemory(3, 10, 100);  // Older
    memory_system->UpdateActionMemory(3, 11, 200);  // Newer

    std::vector<int> entity_ids = {10, 11};
    int selected = memory_system->GetLeastRecentlyUsedEntityForAction(3, entity_ids);

    EXPECT_EQ(10, selected);
}

TEST_F(MemorySystemTest, GetLeastRecentlyUsedEntityReturnsMinusOneForEmptyInput) {
    std::vector<int> empty_list;
    int selected = memory_system->GetLeastRecentlyUsedEntityForAction(3, empty_list);

    EXPECT_EQ(-1, selected);
}

TEST_F(MemorySystemTest, GetLeastRecentlyUsedEntityHandlesEqualTimestamps) {
    memory_system->UpdateActionMemory(3, 10, 100);
    memory_system->UpdateActionMemory(3, 11, 100); // Same timestamp so should pick randomly

    std::vector<int> entity_ids = {10, 11};
    int selected = memory_system->GetLeastRecentlyUsedEntityForAction(3, entity_ids);

    EXPECT_TRUE(selected == 10 || selected == 11);
}

// =============================================================================
// CLEANUP TESTS
// =============================================================================
TEST_F(MemorySystemTest, ClearAllMemoriesResetsMemories) {
    memory_system->UpdateTransitionMemory(5, 100);
    memory_system->UpdateActionMemory(5, 1, 100);
    memory_system->UpdateInterruptionMemory(2, 1, 2, 1, 200);
    memory_system->ClearAllMemories();

    EXPECT_EQ(0, memory_system->GetTransitionMemoryCount());
    EXPECT_EQ(0, memory_system->GetActionMemoryCount());
    EXPECT_EQ(0, memory_system->GetInterruptionMemoryCount());
}

TEST_F(MemorySystemTest, ClearInterruptionMemoriesRemovesAllFromSequence) {
    memory_system->UpdateInterruptionMemory(5, 10, 15, 20, 100);  // Sequence 10
    memory_system->UpdateInterruptionMemory(6, 10, 16, 21, 200);  // Sequence 10
    memory_system->UpdateInterruptionMemory(7, 12, 17, 22, 300);  // Sequence 12

    memory_system->ClearInterruptionMemories(10);

    EXPECT_EQ(1, memory_system->GetInterruptionMemoryCount());
    EXPECT_EQ(nullptr, memory_system->FindInterruptionMemory(5, 10, 15));
    EXPECT_EQ(nullptr, memory_system->FindInterruptionMemory(6, 10, 16));
    EXPECT_NE(nullptr, memory_system->FindInterruptionMemory(7, 12, 17));
}

TEST_F(MemorySystemTest, ClearInterruptionMemoriesDoesNothingWhenNoneFound) {
    memory_system->UpdateInterruptionMemory(5, 10, 15, 20, 100);

    memory_system->ClearInterruptionMemories(999);  // Non-existent sequence

    EXPECT_EQ(1, memory_system->GetInterruptionMemoryCount());  // No change
}

TEST_F(MemorySystemTest, RemoveInterruptionMemoryReturnsTrueWhenSuccessful) {
    memory_system->UpdateInterruptionMemory(5, 10, 15, 20, 100);

    EXPECT_TRUE(memory_system->RemoveInterruptionMemory(5, 10, 15));
    EXPECT_EQ(0, memory_system->GetInterruptionMemoryCount());
}

TEST_F(MemorySystemTest, RemoveInterruptionMemoryReturnsFalseWhenNotSuccessful) {
    memory_system->UpdateInterruptionMemory(5, 10, 15, 20, 100);

    EXPECT_FALSE(memory_system->RemoveInterruptionMemory(2, 10, 15));
    EXPECT_EQ(1, memory_system->GetInterruptionMemoryCount());
}
*/