
#include <gtest/gtest.h>
#include "memory/MemorySystem.h"
#include "../mocks/MockLogger.h"

using namespace AmbientCharacterBehavior;

class MemorySystemTest : public testing::Test {
protected:
    std::unique_ptr<MockLogger> mock_logger;


    void SetUp() override {
        mock_logger = std::make_unique<MockLogger>();

        memory_system = std::make_unique<MemorySystem>(3, 3, 2, *mock_logger);
    }

    std::unique_ptr<MemorySystem> memory_system;
};

// =============================================================================
// CONSTRUCTION AND CONFIGURATION TESTS
// =============================================================================

TEST_F(MemorySystemTest, ConstructorInitializesEmptyMemoryCounts) {
    MemorySystem system(5, 10, 3, *mock_logger);

    EXPECT_EQ(0, system.GetTransitionMemoryCount());
    EXPECT_EQ(0, system.GetActionMemoryCount());
    EXPECT_EQ(0, system.GetInterruptionMemoryCount());
}

TEST_F(MemorySystemTest, ConstructorSetsTransitionCapacity) {
    MemorySystem system(5, 10, 3, *mock_logger);
    EXPECT_EQ(5, system.GetMaxTransitionMemories());
}

TEST_F(MemorySystemTest, ConstructorSetsActionCapacity) {
    MemorySystem system(5, 10, 3, *mock_logger);
    EXPECT_EQ(10, system.GetMaxActionMemories());
}

TEST_F(MemorySystemTest, ConstructorSetsInterruptionCapacity) {
    MemorySystem system(5, 10, 3, *mock_logger);
    EXPECT_EQ(3, system.GetMaxInterruptionMemories());
}

// Configuration Tests
TEST_F(MemorySystemTest, SetMaxMemoriesRejectsZeroValues) {
    memory_system->SetAndEnforceMaxTransitionMemories(0);
    memory_system->SetAndEnforceMaxActionMemories(0);
    memory_system->SetAndEnforceMaxInterruptionMemories(0);

    // Should reject and keep original values
    EXPECT_EQ(3, memory_system->GetMaxTransitionMemories());
    EXPECT_EQ(3, memory_system->GetMaxActionMemories());
    EXPECT_EQ(2, memory_system->GetMaxInterruptionMemories());
}

TEST_F(MemorySystemTest, SetMaxMemoriesRejectsNegativeValues) {
    memory_system->SetAndEnforceMaxTransitionMemories(-5);
    memory_system->SetAndEnforceMaxActionMemories(-5);
    memory_system->SetAndEnforceMaxInterruptionMemories(-5);

    EXPECT_EQ(3, memory_system->GetMaxTransitionMemories());  // Should be unchanged
    EXPECT_EQ(3, memory_system->GetMaxActionMemories());  // Should be unchanged
    EXPECT_EQ(2, memory_system->GetMaxInterruptionMemories());  // Should be unchanged
}

TEST_F(MemorySystemTest, SetMaxTransitionMemoriesEnforcesNewCapacityImmediately) {
    // Fill with 3 memories
    [[maybe_unused]] auto res = memory_system->UpdateTransitionMemory(1, 100);
    [[maybe_unused]] auto res1 = memory_system->UpdateTransitionMemory(2, 200);
    [[maybe_unused]] auto res2 = memory_system->UpdateTransitionMemory(3, 300);

    // Reduce capacity to 2
    memory_system->SetAndEnforceMaxTransitionMemories(2);

    // Should immediately remove oldest
    EXPECT_EQ(2, memory_system->GetTransitionMemoryCount());
    EXPECT_EQ(nullptr, memory_system->FindTransitionMemory(1));  // Oldest removed
}

TEST_F(MemorySystemTest, SetMaxActionMemoriesEnforcesNewCapacityImmediately) {
    [[maybe_unused]] auto res = memory_system->UpdateActionMemory(1, 10, 100);
    [[maybe_unused]] auto res1 = memory_system->UpdateActionMemory(2, 20, 200);
    [[maybe_unused]] auto res2 = memory_system->UpdateActionMemory(3, 30, 300);

    memory_system->SetAndEnforceMaxActionMemories(2);

    EXPECT_EQ(2, memory_system->GetActionMemoryCount());
    EXPECT_EQ(nullptr, memory_system->FindActionMemory(1, 10));  // Oldest removed
}

TEST_F(MemorySystemTest, SetMaxInterruptionMemoriesEnforcesNewCapacityImmediately) {
    [[maybe_unused]] auto res = memory_system->UpdateInterruptionMemory(1, 10, 11, 20, 100);
    [[maybe_unused]] auto res1 = memory_system->UpdateInterruptionMemory(2, 10, 12, 21, 200);

    memory_system->SetAndEnforceMaxInterruptionMemories(1);

    EXPECT_EQ(1, memory_system->GetInterruptionMemoryCount());
    EXPECT_EQ(nullptr, memory_system->FindInterruptionMemory(1, 10, 11));  // Oldest removed
}

// =============================================================================
// FIND MEMORY TESTS
// =============================================================================

// Transition Memory Find Tests
TEST_F(MemorySystemTest, FindTransitionMemoryReturnsCorrectMemory) {
   [[maybe_unused]] auto res = memory_system->UpdateTransitionMemory(5, 100);

    const TransitionMemory* found = memory_system->FindTransitionMemory(5);
    ASSERT_NE(nullptr, found);
    EXPECT_EQ(5, found->GetTargetNodeId());
    EXPECT_EQ(100, found->GetLastUsedTime());
}

TEST_F(MemorySystemTest, FindTransitionMemoryReturnsNullptrWhenNotFound) {
    EXPECT_EQ(nullptr, memory_system->FindTransitionMemory(999));
}

TEST_F(MemorySystemTest, FindTransitionMemoryMatchesOnlyNodeId) {
    [[maybe_unused]] auto res = memory_system->UpdateTransitionMemory(5, 100);
    [[maybe_unused]] auto res1 = memory_system->UpdateTransitionMemory(7, 200);

    EXPECT_NE(nullptr, memory_system->FindTransitionMemory(5));
    EXPECT_NE(nullptr, memory_system->FindTransitionMemory(7));
    EXPECT_EQ(nullptr, memory_system->FindTransitionMemory(9));  // Not found
}

// Action Memory Find Tests
TEST_F(MemorySystemTest, FindActionMemoryReturnsCorrectMemory) {
   [[maybe_unused]] auto res =  memory_system->UpdateActionMemory(3, 10, 100);

    const ActionMemory* found = memory_system->FindActionMemory(3, 10);
    ASSERT_NE(nullptr, found);
    EXPECT_EQ(3, found->GetActionId());
    EXPECT_EQ(10, found->GetTargetEntityId());
}

TEST_F(MemorySystemTest, FindActionMemoryReturnsNullptrWhenNotFound) {
    EXPECT_EQ(nullptr, memory_system->FindActionMemory(999, 888));
}

TEST_F(MemorySystemTest, FindActionMemoryRequiresBothKeysToMatch) {
   [[maybe_unused]] auto res =  memory_system->UpdateActionMemory(3, 10, 100);

    EXPECT_NE(nullptr, memory_system->FindActionMemory(3, 10));   // Both match
    EXPECT_EQ(nullptr, memory_system->FindActionMemory(3, 11));   // Different entity
    EXPECT_EQ(nullptr, memory_system->FindActionMemory(4, 10));   // Different action
}

// Interruption Memory Find Tests
TEST_F(MemorySystemTest, FindInterruptionMemoryReturnsCorrectMemory) {
   [[maybe_unused]] auto res =  memory_system->UpdateInterruptionMemory(5, 10, 15, 20, 100);

    const InterruptionMemory* found = memory_system->FindInterruptionMemory(5, 10, 15);
    ASSERT_NE(nullptr, found);
    EXPECT_EQ(5, found->GetInterruptedActionId());
    EXPECT_EQ(10, found->GetInterruptedSequenceId());
    EXPECT_EQ(15, found->GetInterruptedSequenceNodeId());
}

TEST_F(MemorySystemTest, FindInterruptionMemoryReturnsNullptrWhenNotFound) {
    EXPECT_EQ(nullptr, memory_system->FindInterruptionMemory(999, 888, 777));
}

TEST_F(MemorySystemTest, FindInterruptionMemoryRequiresAllThreeKeysToMatch) {
   [[maybe_unused]] auto res =  memory_system->UpdateInterruptionMemory(5, 10, 15, 20, 100);

    EXPECT_NE(nullptr, memory_system->FindInterruptionMemory(5, 10, 15));  // All match
    EXPECT_EQ(nullptr, memory_system->FindInterruptionMemory(6, 10, 15));  // Different action
    EXPECT_EQ(nullptr, memory_system->FindInterruptionMemory(5, 11, 15));  // Different sequence
    EXPECT_EQ(nullptr, memory_system->FindInterruptionMemory(5, 10, 16));  // Different node
}

// =============================================================================
// MEMORY UPDATE TESTS
// =============================================================================

// Transition Memory Updates
TEST_F(MemorySystemTest, UpdateTransitionMemoryCreatesNewEntryAndReturnsTrue) {
    EXPECT_TRUE(memory_system->UpdateTransitionMemory(5, 100));
    EXPECT_EQ(1, memory_system->GetTransitionMemoryCount());
}

TEST_F(MemorySystemTest, UpdateTransitionMemoryStoresCorrectData) {
   [[maybe_unused]] auto res =  memory_system->UpdateTransitionMemory(5, 100);

    const TransitionMemory* found = memory_system->FindTransitionMemory(5);
    ASSERT_NE(nullptr, found);
    EXPECT_EQ(5, found->GetTargetNodeId());
    EXPECT_EQ(100, found->GetLastUsedTime());
}

TEST_F(MemorySystemTest, UpdateTransitionMemoryReplacesExistingEntry) {
   [[maybe_unused]] auto res =  memory_system->UpdateTransitionMemory(5, 100);
   [[maybe_unused]] auto res1 =  memory_system->UpdateTransitionMemory(5, 200);

    const TransitionMemory* found = memory_system->FindTransitionMemory(5);
    ASSERT_NE(nullptr, found);

    EXPECT_EQ(1, memory_system->GetTransitionMemoryCount());
    EXPECT_EQ(200, found->GetLastUsedTime());
}

TEST_F(MemorySystemTest, TransitionMemoryRespectsCapacityLimit) {
   [[maybe_unused]] auto res =  memory_system->UpdateTransitionMemory(1, 100);
   [[maybe_unused]] auto res1 =  memory_system->UpdateTransitionMemory(2, 200);
   [[maybe_unused]] auto res2 =  memory_system->UpdateTransitionMemory(3, 300);
   [[maybe_unused]] auto res3 =  memory_system->UpdateTransitionMemory(4, 400);  // Should remove oldest

    EXPECT_EQ(3, memory_system->GetTransitionMemoryCount());
    EXPECT_EQ(nullptr, memory_system->FindTransitionMemory(1));
    EXPECT_NE(nullptr, memory_system->FindTransitionMemory(4));
}

TEST_F(MemorySystemTest, InvalidUpdateDoesNotRemoveExistingMemoryAndReturnsFalse) {
   [[maybe_unused]] auto res =  memory_system->UpdateTransitionMemory(5, 100);  // Valid memory
    EXPECT_EQ(1, memory_system->GetTransitionMemoryCount());

    // Invalid update returns false
    EXPECT_FALSE(memory_system->UpdateTransitionMemory(5, -1));  // Invalid time

    // Original memory should still exist (validation failed before removal)
    EXPECT_EQ(1, memory_system->GetTransitionMemoryCount());
    const TransitionMemory* found = memory_system->FindTransitionMemory(5);
    ASSERT_NE(nullptr, found);
    EXPECT_EQ(100, found->GetLastUsedTime());  // Original data preserved
}

// Action Memory Updates
TEST_F(MemorySystemTest, UpdateActionMemoryCreatesNewEntry) {
    EXPECT_TRUE(memory_system->UpdateActionMemory(3, 10, 100));
    EXPECT_EQ(1, memory_system->GetActionMemoryCount());
}

TEST_F(MemorySystemTest, UpdateActionMemoryStoresCorrectData) {
   [[maybe_unused]] auto res =  memory_system->UpdateActionMemory(3, 10, 100);

    const ActionMemory* found = memory_system->FindActionMemory(3, 10);
    ASSERT_NE(nullptr, found);
    EXPECT_EQ(3, found->GetActionId());
    EXPECT_EQ(10, found->GetTargetEntityId());
    EXPECT_EQ(100, found->GetLastUsedTime());
}

TEST_F(MemorySystemTest, UpdateActionMemoryReplacesExistingEntry) {
   [[maybe_unused]] auto res =  memory_system->UpdateActionMemory(3, 10, 100);
   [[maybe_unused]] auto res1 =  memory_system->UpdateActionMemory(3, 10, 200);  // Same action+entity, different time

    const ActionMemory* found = memory_system->FindActionMemory(3, 10);
    ASSERT_NE(nullptr, found);

    EXPECT_EQ(1, memory_system->GetActionMemoryCount());
    EXPECT_EQ(200, found->GetLastUsedTime());
}

TEST_F(MemorySystemTest, ActionMemoryRespectsCapacityLimit) {
   [[maybe_unused]] auto res =  memory_system->UpdateActionMemory(1, 10, 100);
   [[maybe_unused]] auto res1 =  memory_system->UpdateActionMemory(2, 20, 200);
   [[maybe_unused]] auto res2 =  memory_system->UpdateActionMemory(3, 30, 300);
   [[maybe_unused]] auto res3 =  memory_system->UpdateActionMemory(4, 40, 400);

    EXPECT_EQ(3, memory_system->GetActionMemoryCount());
    EXPECT_EQ(nullptr, memory_system->FindActionMemory(1, 10));  // Oldest removed
    EXPECT_NE(nullptr, memory_system->FindActionMemory(4, 40));  // Newest kept
}

TEST_F(MemorySystemTest, InvalidActionUpdateDoesNotRemoveExistingMemoryAndReturnsFalse) {
   [[maybe_unused]] auto res =  memory_system->UpdateActionMemory(3, 10, 100);  // Valid memory
    EXPECT_EQ(1, memory_system->GetActionMemoryCount());

    // Invalid update returns false
    EXPECT_FALSE(memory_system->UpdateActionMemory(3, 10, -1));  // Invalid time

    // Original memory should still exist (validation failed before removal)
    EXPECT_EQ(1, memory_system->GetActionMemoryCount());
    const ActionMemory* found = memory_system->FindActionMemory(3, 10);
    ASSERT_NE(nullptr, found);
    EXPECT_EQ(100, found->GetLastUsedTime());  // Original data preserved
}

// Interruption Memory Updates
TEST_F(MemorySystemTest, UpdateInterruptionMemoryCreatesNewEntryAndReturnsTrue) {
    EXPECT_TRUE(memory_system->UpdateInterruptionMemory(5, 10, 15, 20, 100));
    EXPECT_EQ(1, memory_system->GetInterruptionMemoryCount());
}

TEST_F(MemorySystemTest, UpdateInterruptionMemoryStoresCorrectData) {
   [[maybe_unused]] auto res =  memory_system->UpdateInterruptionMemory(5, 10, 15, 20, 100);

    const InterruptionMemory* found = memory_system->FindInterruptionMemory(5, 10, 15);
    ASSERT_NE(nullptr, found);
    EXPECT_EQ(5, found->GetInterruptedActionId());
    EXPECT_EQ(10, found->GetInterruptedSequenceId());
    EXPECT_EQ(15, found->GetInterruptedSequenceNodeId());
    EXPECT_EQ(20, found->GetInterruptedTargetEntityId());
    EXPECT_EQ(100, found->GetLastUsedTime());
}

TEST_F(MemorySystemTest, UpdateInterruptionMemoryReplacesExistingEntry) {
   [[maybe_unused]] auto res =  memory_system->UpdateInterruptionMemory(5, 10, 15, 20, 100);
   [[maybe_unused]] auto res1 =  memory_system->UpdateInterruptionMemory(5, 10, 15, 25, 200);  // Same keys, different entity+time

    const InterruptionMemory* found = memory_system->FindInterruptionMemory(5, 10, 15);
    ASSERT_NE(nullptr, found);

    EXPECT_EQ(1, memory_system->GetInterruptionMemoryCount());
    EXPECT_EQ(25, found->GetInterruptedTargetEntityId());  // Entity updated
    EXPECT_EQ(200, found->GetLastUsedTime());              // Time updated
}

TEST_F(MemorySystemTest, InterruptionMemoryRespectsCapacityLimit) {
   [[maybe_unused]] auto res =  memory_system->UpdateInterruptionMemory(1, 10, 11, 20, 100);
   [[maybe_unused]] auto res1 =  memory_system->UpdateInterruptionMemory(2, 10, 12, 21, 200);
   [[maybe_unused]] auto res2 =  memory_system->UpdateInterruptionMemory(3, 10, 13, 22, 300);  // At capacity (2)

    // This should remove the oldest (first entry)
    EXPECT_EQ(2, memory_system->GetInterruptionMemoryCount());
    EXPECT_EQ(nullptr, memory_system->FindInterruptionMemory(1, 10, 11));  // Oldest removed
    EXPECT_NE(nullptr, memory_system->FindInterruptionMemory(3, 10, 13));  // Newest kept
}

TEST_F(MemorySystemTest, InvalidInterruptionUpdateDoesNotRemoveExistingMemoryAndReturnsFalse) {
   [[maybe_unused]] auto res =  memory_system->UpdateInterruptionMemory(5, 10, 15, 20, 100);  // Valid memory
    EXPECT_EQ(1, memory_system->GetInterruptionMemoryCount());

    // Invalid update returns false (invalid entity_id)
    EXPECT_FALSE(memory_system->UpdateInterruptionMemory(5, 10, 15, -2, 200));  // -2 not allowed

    // Original memory should still exist (validation failed before removal)
    EXPECT_EQ(1, memory_system->GetInterruptionMemoryCount());
    const InterruptionMemory* found = memory_system->FindInterruptionMemory(5, 10, 15);
    ASSERT_NE(nullptr, found);
    EXPECT_EQ(20, found->GetInterruptedTargetEntityId());  // Original data preserved
    EXPECT_EQ(100, found->GetLastUsedTime());              // Original time preserved
}

// =============================================================================
// RECENCY SELECTION TESTS
// =============================================================================

// Transition Memory Recency Tests
TEST_F(MemorySystemTest, GetLeastRecentlyVisitedNodePrefersNeverUsed) {
   [[maybe_unused]] auto res =  memory_system->UpdateTransitionMemory(5, 100);
   [[maybe_unused]] auto res1 =  memory_system->UpdateTransitionMemory(7, 200);

    std::vector<int> node_ids = {5, 7, 9};  // 9 never used
    int selected = memory_system->GetLeastRecentlyVisitedNodeId(node_ids);

    EXPECT_EQ(9, selected);
}

TEST_F(MemorySystemTest, GetLeastRecentlyVisitedNodeSelectsOldestTimestamp) {
   [[maybe_unused]] auto res =  memory_system->UpdateTransitionMemory(5, 100);  // Older
   [[maybe_unused]] auto res1 =  memory_system->UpdateTransitionMemory(7, 200);  // Newer

    std::vector<int> node_ids = {5, 7};
    int selected = memory_system->GetLeastRecentlyVisitedNodeId(node_ids);

    EXPECT_EQ(5, selected);
}

TEST_F(MemorySystemTest, GetLeastRecentlyVisitedNodeReturnsMinusOneForEmptyInput) {
    std::vector<int> empty_list;
    int selected = memory_system->GetLeastRecentlyVisitedNodeId(empty_list);

    EXPECT_EQ(-1, selected);
}

TEST_F(MemorySystemTest, GetLeastRecentlyVisitedNodeHandlesEqualTimestamps) {
   [[maybe_unused]] auto res =  memory_system->UpdateTransitionMemory(5, 100);
   [[maybe_unused]] auto res1 =  memory_system->UpdateTransitionMemory(7, 100);  // Same timestamp so should pick randomly

    std::vector<int> node_ids = {5, 7};
    int selected = memory_system->GetLeastRecentlyVisitedNodeId(node_ids);

    EXPECT_TRUE(selected == 5 || selected == 7);
}

// Action Memory Recency Tests
TEST_F(MemorySystemTest, GetLeastRecentlyUsedEntityPrefersNeverUsed) {
   [[maybe_unused]] auto res =  memory_system->UpdateActionMemory(3, 10, 100);
   [[maybe_unused]] auto res1 =  memory_system->UpdateActionMemory(3, 11, 200);

    std::vector<int> entity_ids = {10, 11, 12};  // 12 never used
    int selected = memory_system->GetLeastRecentlyUsedEntityIdForAction(3, entity_ids);

    EXPECT_EQ(12, selected);
}

TEST_F(MemorySystemTest, GetLeastRecentlyUsedEntitySelectsOldestTimestamp) {
   [[maybe_unused]] auto res =  memory_system->UpdateActionMemory(3, 10, 100);  // Older
   [[maybe_unused]] auto res1 =  memory_system->UpdateActionMemory(3, 11, 200);  // Newer

    std::vector<int> entity_ids = {10, 11};
    int selected = memory_system->GetLeastRecentlyUsedEntityIdForAction(3, entity_ids);

    EXPECT_EQ(10, selected);
}

TEST_F(MemorySystemTest, GetLeastRecentlyUsedEntityReturnsMinusOneForEmptyInput) {
    std::vector<int> empty_list;
    int selected = memory_system->GetLeastRecentlyUsedEntityIdForAction(3, empty_list);

    EXPECT_EQ(-1, selected);
}

TEST_F(MemorySystemTest, GetLeastRecentlyUsedEntityHandlesEqualTimestamps) {
   [[maybe_unused]] auto res =  memory_system->UpdateActionMemory(3, 10, 100);
   [[maybe_unused]] auto res1 =  memory_system->UpdateActionMemory(3, 11, 100); // Same timestamp so should pick randomly

    std::vector<int> entity_ids = {10, 11};
    int selected = memory_system->GetLeastRecentlyUsedEntityIdForAction(3, entity_ids);

    EXPECT_TRUE(selected == 10 || selected == 11);
}


// =============================================================================
// CLEANUP TESTS
// =============================================================================
TEST_F(MemorySystemTest, ClearAllMemoriesResetsMemories) {
   [[maybe_unused]] auto res =  memory_system->UpdateTransitionMemory(5, 100);
   [[maybe_unused]] auto res1 =  memory_system->UpdateActionMemory(5, 1, 100);
   [[maybe_unused]] auto res2 =  memory_system->UpdateInterruptionMemory(2, 1, 2, 1, 200);
    memory_system->ClearAllMemories();

    EXPECT_EQ(0, memory_system->GetTransitionMemoryCount());
    EXPECT_EQ(0, memory_system->GetActionMemoryCount());
    EXPECT_EQ(0, memory_system->GetInterruptionMemoryCount());
}

TEST_F(MemorySystemTest, ClearInterruptionMemoriesRemovesAllFromSequence) {
   [[maybe_unused]] auto res =  memory_system->UpdateInterruptionMemory(5, 10, 15, 20, 100);  // Sequence 10
   [[maybe_unused]] auto res1 =  memory_system->UpdateInterruptionMemory(6, 10, 16, 21, 200);  // Sequence 10
   [[maybe_unused]] auto res2 =  memory_system->UpdateInterruptionMemory(7, 12, 17, 22, 300);  // Sequence 12

    memory_system->ClearSequenceInterruptionMemories(10);

    EXPECT_EQ(1, memory_system->GetInterruptionMemoryCount());
    EXPECT_EQ(nullptr, memory_system->FindInterruptionMemory(5, 10, 15));
    EXPECT_EQ(nullptr, memory_system->FindInterruptionMemory(6, 10, 16));
    EXPECT_NE(nullptr, memory_system->FindInterruptionMemory(7, 12, 17));
}

TEST_F(MemorySystemTest, ClearInterruptionMemoriesDoesNothingWhenNoneFound) {
   [[maybe_unused]] auto res =  memory_system->UpdateInterruptionMemory(5, 10, 15, 20, 100);

    memory_system->ClearSequenceInterruptionMemories(999);  // Non-existent sequence

    EXPECT_EQ(1, memory_system->GetInterruptionMemoryCount());  // No change
}

TEST_F(MemorySystemTest, RemoveInterruptionMemoryReturnsTrueWhenSuccessful) {
   [[maybe_unused]] auto res =  memory_system->UpdateInterruptionMemory(5, 10, 15, 20, 100);

    EXPECT_TRUE(memory_system->RemoveInterruptionMemory(5, 10, 15));
    EXPECT_EQ(0, memory_system->GetInterruptionMemoryCount());
}

TEST_F(MemorySystemTest, RemoveInterruptionMemoryReturnsFalseWhenNotSuccessful) {
   [[maybe_unused]] auto res =  memory_system->UpdateInterruptionMemory(5, 10, 15, 20, 100);

    EXPECT_FALSE(memory_system->RemoveInterruptionMemory(2, 10, 15));
    EXPECT_EQ(1, memory_system->GetInterruptionMemoryCount());
}
