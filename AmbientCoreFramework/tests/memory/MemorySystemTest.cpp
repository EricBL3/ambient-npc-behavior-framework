#include <gtest/gtest.h>
#include "memory/MemorySystem.h"
#include "../services/mocks/MockLogger.h"
#include "../services/mocks/MockTimeManager.h"
#include "../services/mocks/MockEnvironmentalConditionProvider.h"
#include "../services/mocks/MockStartCharacterActionProvider.h"
#include "../services/mocks/MockEntityPositionProvider.h"

using namespace AmbientCharacterBehavior;

class MemorySystemTest : public testing::Test {
protected:
    std::unique_ptr<MockLogger> mock_logger;
    std::unique_ptr<MockTimeManager> mock_time_manager;
    std::unique_ptr<MockEnvironmentalConditionProvider> mock_environmental_condition_provider;
    std::unique_ptr<MockStartCharacterActionProvider> mock_action_provider;
    std::unique_ptr<MockEntityPositionProvider> mock_entity_pos_provider;

    std::unique_ptr<FoundationServices> services;


    void SetUp() override {
        mock_logger = std::make_unique<MockLogger>();
        mock_time_manager = std::make_unique<MockTimeManager>();
        mock_environmental_condition_provider = std::make_unique<MockEnvironmentalConditionProvider>();
        mock_action_provider = std::make_unique<MockStartCharacterActionProvider>();
        mock_entity_pos_provider = std::make_unique<MockEntityPositionProvider>();

        services = std::make_unique<FoundationServices>(*mock_logger, *mock_time_manager,
            *mock_environmental_condition_provider,*mock_action_provider, *mock_entity_pos_provider);

        memory_system = std::make_unique<MemorySystem>(3, 3, 2, *services);
    }

    std::unique_ptr<MemorySystem> memory_system;
};

// =============================================================================
// CONSTRUCTION AND CONFIGURATION TESTS
// =============================================================================

// Construction Tests

TEST_F(MemorySystemTest, ConstructorInitializesEmptyMemoryCounts) {
    MemorySystem system(5, 10, 3, *services);

    EXPECT_EQ(0, system.GetTransitionMemoryCount());
    EXPECT_EQ(0, system.GetActionMemoryCount());
    EXPECT_EQ(0, system.GetInterruptionMemoryCount());
}

TEST_F(MemorySystemTest, ConstructorSetsAllCapacities) {
    MemorySystem system(5, 10, 3, *services);

    EXPECT_EQ(5, system.GetMaxTransitionMemories());
    EXPECT_EQ(10, system.GetMaxActionMemories());
    EXPECT_EQ(3, system.GetMaxInterruptionMemories());

    // All memory counts should start at zero
    EXPECT_EQ(0, system.GetTransitionMemoryCount());
    EXPECT_EQ(0, system.GetActionMemoryCount());
    EXPECT_EQ(0, system.GetInterruptionMemoryCount());
}

// Configuration Tests

TEST_F(MemorySystemTest, SetMaxMemoriesRejectsZeroValues) {
    memory_system->ConfigureMaxTransitionMemories(0);
    memory_system->ConfigureMaxActionMemories(0);
    memory_system->ConfigureMaxInterruptionMemories(0);

    // Should reject and keep original values
    EXPECT_EQ(3, memory_system->GetMaxTransitionMemories());
    EXPECT_EQ(3, memory_system->GetMaxActionMemories());
    EXPECT_EQ(2, memory_system->GetMaxInterruptionMemories());
}

TEST_F(MemorySystemTest, SetMaxMemoriesRejectsNegativeValues) {
    memory_system->ConfigureMaxTransitionMemories(-5);
    memory_system->ConfigureMaxActionMemories(-5);
    memory_system->ConfigureMaxInterruptionMemories(-5);

    EXPECT_EQ(3, memory_system->GetMaxTransitionMemories());  // Should be unchanged
    EXPECT_EQ(3, memory_system->GetMaxActionMemories());  // Should be unchanged
    EXPECT_EQ(2, memory_system->GetMaxInterruptionMemories());  // Should be unchanged
}

TEST_F(MemorySystemTest, SetMaxTransitionMemoriesEnforcesNewCapacityImmediately) {
    // Fill with 3 memories
    ASSERT_TRUE(memory_system->CreateTransitionMemory(0, 1, 100));
    ASSERT_TRUE(memory_system->CreateTransitionMemory(0, 2, 200));
    ASSERT_TRUE(memory_system->CreateTransitionMemory(0, 3, 300));

    // Reduce capacity to 2
    memory_system->ConfigureMaxTransitionMemories(2);

    // Should immediately remove oldest
    EXPECT_EQ(2, memory_system->GetTransitionMemoryCount());
    EXPECT_EQ(nullptr, memory_system->FindTransitionMemory(0, 1));  // Oldest removed
}

TEST_F(MemorySystemTest, SetMaxActionMemoriesEnforcesNewCapacityImmediately) {
    ASSERT_TRUE(memory_system->CreateActionMemory(1, 10, 100));
    ASSERT_TRUE(memory_system->CreateActionMemory(2, 20, 200));
    ASSERT_TRUE(memory_system->CreateActionMemory(3, 30, 300));

    memory_system->ConfigureMaxActionMemories(2);

    EXPECT_EQ(2, memory_system->GetActionMemoryCount());
    EXPECT_EQ(nullptr, memory_system->FindActionMemory(1, 10));  // Oldest removed
}

TEST_F(MemorySystemTest, SetMaxInterruptionMemoriesEnforcesNewCapacityImmediately) {
    ASSERT_TRUE(memory_system->CreateInterruptionMemory(1, 10, 11, 20, 100));
    ASSERT_TRUE(memory_system->CreateInterruptionMemory(2, 10, 12, 21, 200));

    memory_system->ConfigureMaxInterruptionMemories(1);

    EXPECT_EQ(1, memory_system->GetInterruptionMemoryCount());
    EXPECT_EQ(nullptr, memory_system->FindInterruptionMemory(1, 10, 11));  // Oldest removed
}

// =============================================================================
// TRANSITION MEMORY OPERATIONS
// =============================================================================

// Find Operations

TEST_F(MemorySystemTest, FindTransitionMemoryReturnsCorrectMemory) {
    ASSERT_TRUE(memory_system->CreateTransitionMemory(0, 5, 100));

    const TransitionMemory* found = memory_system->FindTransitionMemory(0, 5);
    ASSERT_NE(nullptr, found);
    EXPECT_EQ(5, found->GetTargetNodeId());
    EXPECT_EQ(100, found->GetCreationTime());
}

TEST_F(MemorySystemTest, FindTransitionMemoryReturnsNullptrWhenNotFound) {
    EXPECT_EQ(nullptr, memory_system->FindTransitionMemory(0, 999));
}

TEST_F(MemorySystemTest, FindTransitionMemoryRequiresBothSequenceAndNodeToMatch) {
    memory_system->CreateTransitionMemory(1, 5, 100);  // Sequence 1, Node 5
    memory_system->CreateTransitionMemory(2, 7, 200);  // Sequence 2, Node 7

    // Find with correct sequence and node
    EXPECT_NE(nullptr, memory_system->FindTransitionMemory(1, 5));
    EXPECT_NE(nullptr, memory_system->FindTransitionMemory(2, 7));

    // Wrong combinations return nullptr
    EXPECT_EQ(nullptr, memory_system->FindTransitionMemory(1, 7));  // Right seq, wrong node
    EXPECT_EQ(nullptr, memory_system->FindTransitionMemory(2, 5));  // Wrong seq, right node
    EXPECT_EQ(nullptr, memory_system->FindTransitionMemory(3, 5));  // Non-existent sequence
}

// Create/Update Operations

TEST_F(MemorySystemTest, UpdateTransitionMemoryCreatesNewEntryAndReturnsTrue) {
    EXPECT_TRUE(memory_system->CreateTransitionMemory(0, 5, 100));
    EXPECT_EQ(1, memory_system->GetTransitionMemoryCount());
}

TEST_F(MemorySystemTest, UpdateTransitionMemoryStoresCorrectData) {
    ASSERT_TRUE(memory_system->CreateTransitionMemory(0, 5, 100));

    const TransitionMemory* found = memory_system->FindTransitionMemory(0, 5);
    ASSERT_NE(nullptr, found);
    EXPECT_EQ(5, found->GetTargetNodeId());
    EXPECT_EQ(100, found->GetCreationTime());
}

TEST_F(MemorySystemTest, UpdateTransitionMemoryReplacesExistingEntry) {
    ASSERT_TRUE(memory_system->CreateTransitionMemory(0, 5, 100));
    ASSERT_TRUE(memory_system->CreateTransitionMemory(0, 5, 200));

    const TransitionMemory* found = memory_system->FindTransitionMemory(0, 5);
    ASSERT_NE(nullptr, found);

    EXPECT_EQ(1, memory_system->GetTransitionMemoryCount());
    EXPECT_EQ(200, found->GetCreationTime());
}

TEST_F(MemorySystemTest, TransitionMemoryRespectsCapacityLimit) {
    // Should remove oldest
    ASSERT_TRUE(memory_system->CreateTransitionMemory(0, 1, 100));
    ASSERT_TRUE(memory_system->CreateTransitionMemory(0, 2, 200));
    ASSERT_TRUE(memory_system->CreateTransitionMemory(0, 3, 300));
    ASSERT_TRUE(memory_system->CreateTransitionMemory(0, 4, 400));

    EXPECT_EQ(3, memory_system->GetTransitionMemoryCount());
    EXPECT_EQ(nullptr, memory_system->FindTransitionMemory(0, 1));
    EXPECT_NE(nullptr, memory_system->FindTransitionMemory(0, 4));
}

TEST_F(MemorySystemTest, InvalidUpdateDoesNotRemoveExistingMemoryAndReturnsFalse) {
    ASSERT_TRUE(memory_system->CreateTransitionMemory(0, 5, 100));  // Valid memory
    EXPECT_EQ(1, memory_system->GetTransitionMemoryCount());

    // Invalid update returns false
    EXPECT_FALSE(memory_system->CreateTransitionMemory(0, 5, -1));  // Invalid time

    // Original memory should still exist (validation failed before removal)
    EXPECT_EQ(1, memory_system->GetTransitionMemoryCount());
    const TransitionMemory* found = memory_system->FindTransitionMemory(0, 5);
    ASSERT_NE(nullptr, found);
    EXPECT_EQ(100, found->GetCreationTime());  // Original data preserved
}

// Selection Operations

TEST_F(MemorySystemTest, GetLeastRecentlyVisitedNodePrefersNeverUsed) {
    ASSERT_TRUE(memory_system->CreateTransitionMemory(0, 5, 100));
    ASSERT_TRUE(memory_system->CreateTransitionMemory(0, 7, 200));

    std::vector<int> node_ids = {5, 7, 9};  // 9 never used
    SelectionAlgorithmInfo metric_info{
        .sequence_id = 0,
        .current_node_id = 0
    };
    auto selected = memory_system->SelectTransitionNodeId(node_ids, metric_info);

    EXPECT_EQ(9, selected);
}

TEST_F(MemorySystemTest, GetLeastRecentlyVisitedNodeSelectsOldestTimestamp) {
    ASSERT_TRUE(memory_system->CreateTransitionMemory(0, 5, 100));  // Older
    ASSERT_TRUE(memory_system->CreateTransitionMemory(0, 7, 200));  // Newer

    std::vector<int> node_ids = {5, 7};
    SelectionAlgorithmInfo metric_info{
        .sequence_id = 0,
        .current_node_id = 0
    };
    auto selected = memory_system->SelectTransitionNodeId(node_ids, metric_info);

    EXPECT_EQ(5, selected);
}

TEST_F(MemorySystemTest, GetLeastRecentlyVisitedNodeReturnsEmptyForEmptyInput) {
    std::vector<int> empty_list;
    SelectionAlgorithmInfo metric_info{
        .sequence_id = 0,
        .current_node_id = 0
    };
    auto selected = memory_system->SelectTransitionNodeId(empty_list, metric_info);

    EXPECT_FALSE(selected.has_value());
}

TEST_F(MemorySystemTest, GetLeastRecentlyVisitedNodeHandlesEqualTimestamps) {
    ASSERT_TRUE(memory_system->CreateTransitionMemory(0, 5, 100));
    // Same timestamp so should pick randomly
    ASSERT_TRUE(memory_system->CreateTransitionMemory(0, 7, 100));


    std::vector<int> node_ids = {5, 7};
    SelectionAlgorithmInfo metric_info{
        .sequence_id = 0,
        .current_node_id = 0
    };
    auto selected = memory_system->SelectTransitionNodeId(node_ids, metric_info);

    EXPECT_TRUE(selected == 5 || selected == 7);
}

TEST_F(MemorySystemTest, SelectTransitionNodeIdReturnsSingleElement) {
    std::vector<int32_t> single_node = {42};
    SelectionAlgorithmInfo metric_info{
        .sequence_id = 0,
        .current_node_id = 0
    };
    auto selected = memory_system->SelectTransitionNodeId(single_node, metric_info);

    ASSERT_TRUE(selected.has_value());
    EXPECT_EQ(42, *selected);  // Should return the only option
}

TEST_F(MemorySystemTest, UpdateRefreshesRecencyForExistingMemory) {
    ASSERT_TRUE(memory_system->CreateTransitionMemory(0, 5, 100));  // Older
    ASSERT_TRUE(memory_system->CreateTransitionMemory(0, 7, 200));  // Newer

    // Node 5 is older, should be selected
    std::vector<int32_t> nodes = {5, 7};
    SelectionAlgorithmInfo metric_info{
        .sequence_id = 0,
        .current_node_id = 0
    };
    EXPECT_EQ(5, memory_system->SelectTransitionNodeId(nodes, metric_info));

    // Update node 5 with newer timestamp
    ASSERT_TRUE(memory_system->CreateTransitionMemory(0, 5, 300));

    // Now node 7 is older, should be selected
    EXPECT_EQ(7, memory_system->SelectTransitionNodeId(nodes, metric_info));
}

TEST_F(MemorySystemTest, SelectionPrefersUnusedOverOldest) {
    ASSERT_TRUE(memory_system->CreateTransitionMemory(0, 5, 1));  // Very old timestamp

    std::vector<int32_t> nodes = {5, 7, 9};  // 5=ancient, 7&9=unused
    SelectionAlgorithmInfo metric_info{
        .sequence_id = 0,
        .current_node_id = 0
    };
    auto selected = memory_system->SelectTransitionNodeId(nodes, metric_info);

    // Should select from unused (7 or 9), NOT the very old used one (5)
    ASSERT_TRUE(selected.has_value());
    EXPECT_TRUE(*selected == 7 || *selected == 9);
    EXPECT_NE(5, *selected);
}

TEST_F(MemorySystemTest, SelectionIsUnbiasedForUnusedNodes) {
    constexpr int trials = 1000;
    std::map<int32_t, int> selection_counts;

    for (int i = 0; i < trials; ++i) {
        MemorySystem temp_system(10, 10, 10, *services);
        std::vector<int32_t> unused_nodes = {1, 2, 3};
        SelectionAlgorithmInfo metric_info{
            .sequence_id = 0,
            .current_node_id = 0
        };
        auto selected = temp_system.SelectTransitionNodeId(unused_nodes, metric_info);

        ASSERT_TRUE(selected.has_value());
        selection_counts[*selected]++;
    }

    // Each node should be selected roughly 1/3 of the time
    // Using 20% tolerance (33.3% ± 6.7% = [26.6%, 40%])
    for (auto node_id : {1, 2, 3}) {
        double percentage = (selection_counts[node_id] * 100.0) / trials;
        EXPECT_GT(percentage, 26.6) << "Node " << node_id << " under-selected";
        EXPECT_LT(percentage, 40.0) << "Node " << node_id << " over-selected";
    }
}

// =============================================================================
// ACTION MEMORY OPERATIONS
// =============================================================================

// Find Operations

TEST_F(MemorySystemTest, FindActionMemoryReturnsCorrectMemory) {
    ASSERT_TRUE(memory_system->CreateActionMemory(3, 10, 100));

    const ActionMemory* found = memory_system->FindActionMemory(3, 10);
    ASSERT_NE(nullptr, found);
    EXPECT_EQ(3, found->GetActionId());
    EXPECT_EQ(10, found->GetTargetEntityId());
}

TEST_F(MemorySystemTest, FindActionMemoryReturnsNullptrWhenNotFound) {
    EXPECT_EQ(nullptr, memory_system->FindActionMemory(999, 888));
}

TEST_F(MemorySystemTest, FindActionMemoryRequiresBothKeysToMatch) {
    ASSERT_TRUE(memory_system->CreateActionMemory(3, 10, 100));

    EXPECT_NE(nullptr, memory_system->FindActionMemory(3, 10));   // Both match
    EXPECT_EQ(nullptr, memory_system->FindActionMemory(3, 11));   // Different entity
    EXPECT_EQ(nullptr, memory_system->FindActionMemory(4, 10));   // Different action
}

// Create/Update Operations

TEST_F(MemorySystemTest, UpdateActionMemoryCreatesNewEntry) {
    EXPECT_TRUE(memory_system->CreateActionMemory(3, 10, 100));
    EXPECT_EQ(1, memory_system->GetActionMemoryCount());
}

TEST_F(MemorySystemTest, UpdateActionMemoryStoresCorrectData) {
    ASSERT_TRUE(memory_system->CreateActionMemory(3, 10, 100));

    const ActionMemory* found = memory_system->FindActionMemory(3, 10);
    ASSERT_NE(nullptr, found);
    EXPECT_EQ(3, found->GetActionId());
    EXPECT_EQ(10, found->GetTargetEntityId());
    EXPECT_EQ(100, found->GetCreationTime());
}

TEST_F(MemorySystemTest, UpdateActionMemoryReplacesExistingEntry) {
    ASSERT_TRUE(memory_system->CreateActionMemory(3, 10, 100));
    ASSERT_TRUE(memory_system->CreateActionMemory(3, 10, 200));  // Same action+entity, different time

    const ActionMemory* found = memory_system->FindActionMemory(3, 10);
    ASSERT_NE(nullptr, found);

    EXPECT_EQ(1, memory_system->GetActionMemoryCount());
    EXPECT_EQ(200, found->GetCreationTime());
}

TEST_F(MemorySystemTest, ActionMemoryRespectsCapacityLimit) {
    ASSERT_TRUE(memory_system->CreateActionMemory(1, 10, 100));
    ASSERT_TRUE(memory_system->CreateActionMemory(2, 20, 200));
    ASSERT_TRUE(memory_system->CreateActionMemory(3, 30, 300));
    ASSERT_TRUE(memory_system->CreateActionMemory(4, 40, 400));

    EXPECT_EQ(3, memory_system->GetActionMemoryCount());
    EXPECT_EQ(nullptr, memory_system->FindActionMemory(1, 10));  // Oldest removed
    EXPECT_NE(nullptr, memory_system->FindActionMemory(4, 40));  // Newest kept
}

TEST_F(MemorySystemTest, InvalidActionUpdateDoesNotRemoveExistingMemoryAndReturnsFalse) {
    ASSERT_TRUE(memory_system->CreateActionMemory(3, 10, 100));  // Valid memory
    EXPECT_EQ(1, memory_system->GetActionMemoryCount());

    // Invalid update returns false
    EXPECT_FALSE(memory_system->CreateActionMemory(3, 10, -1));  // Invalid time

    // Original memory should still exist (validation failed before removal)
    EXPECT_EQ(1, memory_system->GetActionMemoryCount());
    const ActionMemory* found = memory_system->FindActionMemory(3, 10);
    ASSERT_NE(nullptr, found);
    EXPECT_EQ(100, found->GetCreationTime());  // Original data preserved
}

// Selection Operations
TEST_F(MemorySystemTest, GetLeastRecentlyUsedEntityPrefersNeverUsed) {
    ASSERT_TRUE(memory_system->CreateActionMemory(3, 10, 100));
    ASSERT_TRUE(memory_system->CreateActionMemory(3, 11, 200));

    std::vector<int> entity_ids = {10, 11, 12};  // 12 never used
    SelectionAlgorithmInfo metric_info{
        .action_id = 3
    };
    auto selected = memory_system->SelectActionEntityId(entity_ids, metric_info);

    EXPECT_EQ(12, selected);
}

TEST_F(MemorySystemTest, GetLeastRecentlyUsedEntitySelectsOldestTimestamp) {
    ASSERT_TRUE(memory_system->CreateActionMemory(3, 10, 100));  // Older
    ASSERT_TRUE(memory_system->CreateActionMemory(3, 11, 200));  // Newer

    std::vector<int> entity_ids = {10, 11};
    SelectionAlgorithmInfo metric_info{
        .action_id = 3
    };
    auto selected = memory_system->SelectActionEntityId(entity_ids, metric_info);

    EXPECT_EQ(10, selected);
}

TEST_F(MemorySystemTest, GetLeastRecentlyUsedEntityReturnsMinusOneForEmptyInput) {
    std::vector<int> empty_list;
    SelectionAlgorithmInfo metric_info{
        .action_id = 3
    };
    auto selected = memory_system->SelectActionEntityId(empty_list, metric_info);

    EXPECT_FALSE(selected.has_value());
}

TEST_F(MemorySystemTest, GetLeastRecentlyUsedEntityHandlesEqualTimestamps) {
    ASSERT_TRUE(memory_system->CreateActionMemory(3, 10, 100));
    ASSERT_TRUE(memory_system->CreateActionMemory(3, 11, 100)); // Same timestamp so should pick randomly

    std::vector<int> entity_ids = {10, 11};
    SelectionAlgorithmInfo metric_info{
        .action_id = 3
    };
    auto selected = memory_system->SelectActionEntityId(entity_ids, metric_info);

    EXPECT_TRUE(selected == 10 || selected == 11);
}

TEST_F(MemorySystemTest, SelectActionEntityIdReturnsSingleElement) {
    std::vector<int32_t> single_entity = {99};
    SelectionAlgorithmInfo metric_info{
        .action_id = 5
    };

    auto selected = memory_system->SelectActionEntityId(single_entity, metric_info);

    ASSERT_TRUE(selected.has_value());
    EXPECT_EQ(99, *selected);
}

// =============================================================================
// INTERRUPTION MEMORY OPERATIONS
// =============================================================================

// Find Operations

TEST_F(MemorySystemTest, FindInterruptionMemoryReturnsCorrectMemory) {
    ASSERT_TRUE(memory_system->CreateInterruptionMemory(5, 10, 15, 20, 100));

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
    ASSERT_TRUE(memory_system->CreateInterruptionMemory(5, 10, 15, 20, 100));

    EXPECT_NE(nullptr, memory_system->FindInterruptionMemory(5, 10, 15));  // All match
    EXPECT_EQ(nullptr, memory_system->FindInterruptionMemory(6, 10, 15));  // Different action
    EXPECT_EQ(nullptr, memory_system->FindInterruptionMemory(5, 11, 15));  // Different sequence
    EXPECT_EQ(nullptr, memory_system->FindInterruptionMemory(5, 10, 16));  // Different node
}

// Create/Update Operations
TEST_F(MemorySystemTest, UpdateInterruptionMemoryCreatesNewEntryAndReturnsTrue) {
    EXPECT_TRUE(memory_system->CreateInterruptionMemory(5, 10, 15, 20, 100));
    EXPECT_EQ(1, memory_system->GetInterruptionMemoryCount());
}

TEST_F(MemorySystemTest, UpdateInterruptionMemoryStoresCorrectData) {
    ASSERT_TRUE(memory_system->CreateInterruptionMemory(5, 10, 15, 20, 100));

    const InterruptionMemory* found = memory_system->FindInterruptionMemory(5, 10, 15);
    ASSERT_NE(nullptr, found);
    EXPECT_EQ(5, found->GetInterruptedActionId());
    EXPECT_EQ(10, found->GetInterruptedSequenceId());
    EXPECT_EQ(15, found->GetInterruptedSequenceNodeId());
    EXPECT_EQ(20, found->GetInterruptedTargetEntityId());
    EXPECT_EQ(100, found->GetCreationTime());
}

TEST_F(MemorySystemTest, UpdateInterruptionMemoryReplacesExistingEntry) {
    ASSERT_TRUE(memory_system->CreateInterruptionMemory(5, 10, 15, 20, 100));
    // Same keys, different entity+time
    ASSERT_TRUE(memory_system->CreateInterruptionMemory(5, 10, 15, 25, 200));

    const InterruptionMemory* found = memory_system->FindInterruptionMemory(5, 10, 15);
    ASSERT_NE(nullptr, found);

    EXPECT_EQ(1, memory_system->GetInterruptionMemoryCount());
    EXPECT_EQ(25, found->GetInterruptedTargetEntityId());  // Entity updated
    EXPECT_EQ(200, found->GetCreationTime());              // Time updated
}

TEST_F(MemorySystemTest, InterruptionMemoryRespectsCapacityLimit) {
    ASSERT_TRUE(memory_system->CreateInterruptionMemory(1, 10, 11, 20, 100));
    ASSERT_TRUE(memory_system->CreateInterruptionMemory(2, 10, 12, 21, 200));
    ASSERT_TRUE(memory_system->CreateInterruptionMemory(3, 10, 13, 22, 300));

    // This should remove the oldest (first entry)
    EXPECT_EQ(2, memory_system->GetInterruptionMemoryCount());
    EXPECT_EQ(nullptr, memory_system->FindInterruptionMemory(1, 10, 11));  // Oldest removed
    EXPECT_NE(nullptr, memory_system->FindInterruptionMemory(3, 10, 13));  // Newest kept
}

TEST_F(MemorySystemTest, InvalidInterruptionUpdateDoesNotRemoveExistingMemoryAndReturnsFalse) {
    ASSERT_TRUE(memory_system->CreateInterruptionMemory(5, 10, 15, 20, 100));
    EXPECT_EQ(1, memory_system->GetInterruptionMemoryCount());

    // Invalid update returns false (invalid entity_id). -2 is not allowed
    EXPECT_FALSE(memory_system->CreateInterruptionMemory(5, 10, 15, -2, 200));

    // Original memory should still exist (validation failed before removal)
    EXPECT_EQ(1, memory_system->GetInterruptionMemoryCount());
    const InterruptionMemory* found = memory_system->FindInterruptionMemory(5, 10, 15);
    ASSERT_NE(nullptr, found);
    EXPECT_EQ(20, found->GetInterruptedTargetEntityId());  // Original data preserved
    EXPECT_EQ(100, found->GetCreationTime());              // Original time preserved
}

// =============================================================================
// CLEANUP OPERATIONS
//// =============================================================================
TEST_F(MemorySystemTest, ClearAllMemoriesResetsMemories) {
    ASSERT_TRUE(memory_system->CreateTransitionMemory(0, 5, 100));
    ASSERT_TRUE(memory_system->CreateActionMemory(5, 1, 100));
    ASSERT_TRUE(memory_system->CreateInterruptionMemory(2, 1, 2, 1, 200));
    memory_system->ClearAllMemories();

    EXPECT_EQ(0, memory_system->GetTransitionMemoryCount());
    EXPECT_EQ(0, memory_system->GetActionMemoryCount());
    EXPECT_EQ(0, memory_system->GetInterruptionMemoryCount());
}

TEST_F(MemorySystemTest, ClearInterruptionMemoriesRemovesAllFromSequence) {
    ASSERT_TRUE(memory_system->CreateInterruptionMemory(5, 10, 15, 20, 100));
    ASSERT_TRUE(memory_system->CreateInterruptionMemory(6, 10, 16, 21, 200));
    ASSERT_TRUE(memory_system->CreateInterruptionMemory(7, 12, 17, 22, 300));

    memory_system->ClearSequenceInterruptionMemories(10);

    EXPECT_EQ(1, memory_system->GetInterruptionMemoryCount());
    EXPECT_EQ(nullptr, memory_system->FindInterruptionMemory(5, 10, 15));
    EXPECT_EQ(nullptr, memory_system->FindInterruptionMemory(6, 10, 16));
    EXPECT_NE(nullptr, memory_system->FindInterruptionMemory(7, 12, 17));
}

TEST_F(MemorySystemTest, ClearInterruptionMemoriesDoesNothingWhenNoneFound) {
    ASSERT_TRUE(memory_system->CreateInterruptionMemory(5, 10, 15, 20, 100));

    memory_system->ClearSequenceInterruptionMemories(999);  // Non-existent sequence

    EXPECT_EQ(1, memory_system->GetInterruptionMemoryCount());  // No change
}

TEST_F(MemorySystemTest, RemoveInterruptionMemoryReturnsTrueWhenSuccessful) {
    ASSERT_TRUE(memory_system->CreateInterruptionMemory(5, 10, 15, 20, 100));

    EXPECT_TRUE(memory_system->RemoveInterruptionMemory(5, 10, 15));
    EXPECT_EQ(0, memory_system->GetInterruptionMemoryCount());
}

TEST_F(MemorySystemTest, RemoveInterruptionMemoryReturnsFalseWhenNotSuccessful) {
    ASSERT_TRUE(memory_system->CreateInterruptionMemory(5, 10, 15, 20, 100));

    EXPECT_FALSE(memory_system->RemoveInterruptionMemory(2, 10, 15));
    EXPECT_EQ(1, memory_system->GetInterruptionMemoryCount());
}

// =============================================================================
// OTHER TESTS
//// =============================================================================

TEST_F(MemorySystemTest, DifferentMemoryTypesDoNotMatch) {
    TransitionMemory transition(0, 5, 100);
    ActionMemory action(5, 10, 100);
    InterruptionMemory interruption(5, 0, 5,
        10, 100);

    // Different memory types should never match
    EXPECT_FALSE(transition.MatchesMemory(action));
    EXPECT_FALSE(action.MatchesMemory(transition));
    EXPECT_FALSE(transition.MatchesMemory(interruption));
    EXPECT_FALSE(interruption.MatchesMemory(action));
}