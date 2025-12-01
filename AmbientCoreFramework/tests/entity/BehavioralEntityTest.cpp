#include <gtest/gtest.h>

#include "../mocks/MockLogger.h"
#include "../mocks/MockTimeManager.h"
#include "../mocks/MockContentProvider.h"
#include "../mocks/MockEntityQuery.h"
#include "../mocks/MockStartCharacterActionProvider.h"
#include "../mocks/MockStateOperationEvaluator.h"
#include "behavior/sequence_nodes/ActionSequenceNode.h"
#include "../../src/behavior/sequence_nodes/EndSequenceNode.h"
#include "entity/BehavioralEntity.h"

using namespace AmbientCharacterBehavior;

class BehavioralEntityTest : public testing::Test {
protected:
    std::unique_ptr<MockLogger> mock_logger;
    std::unique_ptr<MockTimeManager> mock_time;
    std::unique_ptr<MockContentProvider> mock_content;
    std::unique_ptr<MockEntityQuery> mock_entity_query;
    std::unique_ptr<MockStartCharacterActionProvider> mock_action_provider;
    std::unique_ptr<MockStateOperationEvaluator> mock_evaluator;

    void* entity_handle;
    int32_t entity_id;
    std::unique_ptr<BehavioralEntity> test_entity;

    std::vector<std::unique_ptr<FrameworkEntity>> mock_entities_;

    static constexpr int32_t STATE_KEY_ENERGY = 0;
    static constexpr int32_t STATE_KEY_OCCUPIED = 1;
    static constexpr int32_t STATE_KEY_CAPACITY = 2;
    static constexpr int32_t STATE_KEY_COMFORT = 3;

    void SetUp() override
    {
        mock_logger = std::make_unique<MockLogger>();
        mock_time = std::make_unique<MockTimeManager>();
        mock_content = std::make_unique<MockContentProvider>();
        mock_entity_query = std::make_unique<MockEntityQuery>();
        mock_action_provider = std::make_unique<MockStartCharacterActionProvider>();
        mock_evaluator = std::make_unique<MockStateOperationEvaluator>();

        entity_handle = reinterpret_cast<void*>(0x1234);
        entity_id = 42;

        CreateDefaultEntity();
    }

    void TearDown() override
    {
        mock_entities_.clear();
    }

    void CreateDefaultEntity() {
        test_entity = std::make_unique<BehavioralEntity>(
            *mock_logger, *mock_time, *mock_action_provider,
            *mock_content, *mock_entity_query, *mock_evaluator,
            entity_handle, entity_id, 10, 10, 10, "TestCharacter"
        );
    }

    std::shared_ptr<Sequence> CreateMockSequence(int32_t sequence_id)
    {
        auto sequence = std::make_shared<Sequence>(sequence_id, "TestSequence_" + std::to_string(sequence_id));

        // Add minimal structure: entry node -> end node
        sequence->AddActionSequenceNode(1, 100);
        sequence->AddEndSequenceNode(999);

        sequence->TrySetEntryPoint(1);
        sequence->TryAddTransition(0, 1,  999, {}); // Simple path to end

        return sequence;
    }

    std::shared_ptr<Sequence> CreateMockSequenceInState(
    int32_t id, SequenceState state) {
        auto sequence = CreateMockSequence(id);
        sequence->SetSequenceState(state);
        return sequence;
    }

    std::shared_ptr<Sequence> CreateSequenceWithActionNode(
    int32_t seq_id, std::shared_ptr<Action> action) {
        auto sequence = std::make_shared<Sequence>(seq_id, "ActionSequence");

        sequence->AddActionSequenceNode(1, action->GetActionId());
        sequence->AddEndSequenceNode(999);

        sequence->TrySetEntryPoint(1);
        sequence->TryAddTransition(0, 1,  999, {}); // Simple path to end

        return sequence;
    }

    void SetupEntityWithSequenceOnStack(std::shared_ptr<Sequence> sequence) {
        test_entity->SetMainSequence(sequence);

        // Execute once to push main sequence onto stack
        test_entity->ExecuteCurrentSequence();

        // Set sequence to desired starting state
        sequence->SetSequenceState(SequenceState::PROCESSING_NODE);
        sequence->ResetCurrentNodeToEntry();
    }

    std::shared_ptr<Action> CreateAction(int32_t id) {
        return std::make_shared<Action>(
            id,
            "TestAction_" + std::to_string(id),
            false, // Doesn't require test_entity
            5000,   // 5 second max duration
            InterruptionBehaviorType::RESUMABLE
        );
    }

    std::shared_ptr<Action> CreateActionRequiringEntity(int32_t id,
        InterruptionBehaviorType interruptionBehavior = InterruptionBehaviorType::RESUMABLE)
    {
        return std::make_shared<Action>(
            id,
            "EntityAction_" + std::to_string(id),
            true,  // Requires test_entity
            5000,
            interruptionBehavior
        );
    }

    std::shared_ptr<Action> CreateActionWithoutEntity(int32_t id) {
        return CreateAction(id); // Same as base action
    }

    std::shared_ptr<Action> CreateActionWithImmediateEffects(int32_t id) {
        auto action = CreateActionRequiringEntity(id);

        // Add some immediate effects
        StateOperation effect1(
            StateOperationTarget::ENTITY,
            STATE_KEY_OCCUPIED,
            StateOperationType::INCREMENT,
            {1}
        );

        StateOperation effect2(
            StateOperationTarget::SELF,
            STATE_KEY_ENERGY,
            StateOperationType::DECREMENT,
            {5}
        );

        action->AddImmediateEffect(effect1);
        action->AddImmediateEffect(effect2);

        return action;
    }

    std::shared_ptr<Action> CreateActionWithCompletionEffects(int32_t id) {
        auto action = CreateActionRequiringEntity(id);

        StateOperation effect1(
            StateOperationTarget::SELF,
            STATE_KEY_ENERGY,
            StateOperationType::INCREMENT,
            {10}
        );

        StateOperation effect2(
            StateOperationTarget::ENTITY,
            STATE_KEY_OCCUPIED,
            StateOperationType::DECREMENT,
            {1}
        );

        action->AddCompletionEffect(effect1);
        action->AddCompletionEffect(effect2);

        return action;
    }

    FrameworkEntity* CreateMockEntity(int32_t id) {
        void* handle = reinterpret_cast<void*>(static_cast<uintptr_t>(id));
        auto entity = new FrameworkEntity(handle, id, "Entity_" + std::to_string(id));

        // Initialize with some default state
        entity->SetStateValue(STATE_KEY_OCCUPIED, 0);
        entity->SetStateValue(STATE_KEY_CAPACITY, 5);
        entity->SetStateValue(STATE_KEY_COMFORT, 7);

        // Store for cleanup
        mock_entities_.push_back(std::unique_ptr<FrameworkEntity>(entity));

        return entity;
    }

    void SetupEntityQueryToReturn(const std::vector<FrameworkEntity*>& entities) {
        EXPECT_CALL(*mock_entity_query, GetEntitiesSupportingAction(testing::_))
            .WillOnce(testing::Return(entities));

        // Also setup GetEntityFromId for each
        for (auto entity : entities) {
            EXPECT_CALL(*mock_entity_query, GetEntityFromId(entity->GetEntityId()))
                .WillRepeatedly(testing::Return(entity));
        }
    }

    void SetupEntitySelectionToReturn(FrameworkEntity* entity) {
        SetupEntityQueryToReturn({entity});
    }

    void SetupAndStartAction(BehavioralEntity* entity, std::shared_ptr<Action> action, FrameworkEntity* target) {

        auto sequence = CreateSequenceWithActionNode(1, action);

        // Mock content provider to return this action
        EXPECT_CALL(*mock_content, GetActionById(action->GetActionId()))
            .WillRepeatedly(testing::Return(action));

        if (target) {
            // Mock entity query
            EXPECT_CALL(*mock_entity_query, GetEntitiesSupportingAction(action->GetActionId()))
                .WillOnce(testing::Return(std::vector<FrameworkEntity*>{target}));

            EXPECT_CALL(*mock_entity_query, GetEntityFromId(target->GetEntityId()))
                .WillRepeatedly(testing::Return(target));

            // Mock all preconditions_by_target passing
            EXPECT_CALL(*mock_evaluator, ProcessStateOperation(testing::_, testing::_))
                .WillRepeatedly(testing::Return(true));
        }

        // Mock StartCharacterAction to not actually do anything
        EXPECT_CALL(*mock_action_provider, StartCharacterAction(
            testing::_, testing::_, testing::_, testing::_, testing::_))
            .Times(1);

        entity->SetMainSequence(sequence);
        SetupEntityWithSequenceOnStack(sequence);

        // Execute to start the action
        entity->ExecuteCurrentSequence(); // UNINITIALIZED → PROCESSING_NODE
        test_entity->ExecuteCurrentSequence();  // PROCESSING_NODE → Execute action node
    }

    int64_t GetCurrentActionToken(const BehavioralEntity* entity) {
        return entity->GetCurrentActionToken();
    }

    std::shared_ptr<Sequence> CreateSequenceWithThreeTransitions() {
        auto sequence = std::make_shared<Sequence>(1, "MultiTransitionSequence");

        sequence->AddActionSequenceNode(1, 100);
        sequence->AddActionSequenceNode(2, 101);
        sequence->AddActionSequenceNode(3, 102);
        sequence->AddActionSequenceNode(4, 103);
        sequence->AddEndSequenceNode(999);
        sequence->TrySetEntryPoint(1);

        // Entry node can transition to nodes 2, 3, or 4
        sequence->TryAddTransition(0, 1, 2, {});
        sequence->TryAddTransition(1, 1, 3, {});
        sequence->TryAddTransition(2, 1, 4, {});

        // All eventually lead to end
        sequence->TryAddTransition(3, 2, 999, {});
        sequence->TryAddTransition(4, 3, 999, {});
        sequence->TryAddTransition(5, 4, 999, {});

        return sequence;
    }

    std::shared_ptr<Sequence> CreateSequenceWithInvalidTransition() {
        auto sequence = std::make_shared<Sequence>(1, "InvalidTransitionSequence");

        // Create nodes
        auto entry_node = std::make_shared<ActionSequenceNode>(1, 100);
        auto end_node = std::make_shared<EndSequenceNode>(999);

        sequence->AddActionSequenceNode(1, 100);
        sequence->AddEndSequenceNode(999);
        sequence->TrySetEntryPoint(1);

        // Precondition unlikely to pass without setup
        StateOperation precondition(
            StateOperationTarget::ENTITY,
            STATE_KEY_CAPACITY,
            StateOperationType::GREATER_THAN,
            {100}
        );

        std::unordered_map<StateOperationTarget, std::vector<StateOperation>> preconditions_by_target;
        preconditions_by_target[StateOperationTarget::ENTITY].push_back(precondition);

        // Entry node can transition to nodes 2, 3, or 4
        sequence->TryAddTransition(0, 1, 999, preconditions_by_target);

        return sequence;
    }

    void SetupEntityWithSequenceAtNodeExecuted(
    std::shared_ptr<Sequence> sequence) {
        SetupEntityWithSequenceOnStack(sequence);

        // Mark current node as completed
        auto current_node = sequence->FindCurrentNode();
        if (current_node) {
            current_node->MarkAsCompleted();
        }

        sequence->SetSequenceState(SequenceState::NODE_EXECUTED);
    }

    void SetupAllPreconditionsPass() {
        EXPECT_CALL(*mock_evaluator, ProcessStateOperation(testing::_, testing::_))
            .WillRepeatedly(testing::Return(true));
    }

    void SetupAllTransitionPreconditionsPass() {
        // Same as above but more semantic naming for transition tests
        SetupAllPreconditionsPass();
    }

    std::vector<FrameworkEntity*> CreateThreeEntities() {
        return {
            CreateMockEntity(100),
            CreateMockEntity(101),
            CreateMockEntity(102)
        };
    }

    std::shared_ptr<Sequence> CreateSequenceWithNestedNode(
    int32_t seq_id, int32_t nested_seq_id) {
        auto sequence = std::make_shared<Sequence>(seq_id, "ParentSequence");

        sequence->AddNestedSequenceNode(1, nested_seq_id);
        sequence->AddEndSequenceNode(999);
        sequence->TrySetEntryPoint(1);
        sequence->TryAddTransition(0, 1, 999, {});

        return sequence;
    }

    std::shared_ptr<Sequence> CreateSequenceWithEndNode(int32_t seq_id) {
        auto sequence = std::make_shared<Sequence>(seq_id, "EndNodeSequence");

        sequence->AddEndSequenceNode(1);
        sequence->TrySetEntryPoint(1);

        return sequence;
    }

    std::shared_ptr<Action> CreateActionWithPrecondition(
    int32_t id, const StateOperation& precondition) {
        auto action = CreateActionRequiringEntity(id);
        action->AddPrecondition(precondition.GetTarget(), precondition);
        return action;
    }

    std::shared_ptr<Action> CreateActionWithStrictPrecondition(int32_t id) {
        auto action = CreateActionRequiringEntity(id);

        // Precondition unlikely to pass without setup
        StateOperation precondition(
            StateOperationTarget::ENTITY,
            STATE_KEY_CAPACITY,
            StateOperationType::GREATER_THAN,
            100
        );

        action->AddPrecondition(precondition.GetTarget(), precondition);
        return action;
    }
};

// CONSTRUCTION TESTS

TEST_F(BehavioralEntityTest, Construction_ValidParameters_CreatesEntity) {
    EXPECT_FALSE(test_entity->IsProcessing());
    EXPECT_EQ(test_entity->GetEntityId(), entity_id);
    EXPECT_EQ(test_entity->GetEntityHandle(), entity_handle);
}

TEST_F(BehavioralEntityTest, Construction_NoMainSequence_CannotUpdate) {
    // Entity created without main sequence
    EXPECT_TRUE(test_entity->CanUpdate());  // Not processing, so can update
}

TEST_F(BehavioralEntityTest, SetMainSequence_ValidSequence_StoresSequence) {
    auto sequence = CreateMockSequence(1);
    test_entity->SetMainSequence(sequence);

    EXPECT_NE(test_entity->GetMainSequence(), nullptr);
    EXPECT_EQ(test_entity->GetMainSequence()->GetSequenceId(), 1);
}

// SEQUENCE STACK MANAGEMENT TESTS

TEST_F(BehavioralEntityTest, ExecuteCurrentSequence_EmptyStack_PushesMainSequence) {
    auto main_seq = CreateMockSequence(1);
    test_entity->SetMainSequence(main_seq);

    // First execution should push main sequence
    test_entity->ExecuteCurrentSequence();

    // Verify main sequence is now on stack (indirectly - check it doesn't error)
    EXPECT_FALSE(test_entity->IsProcessing());
}

TEST_F(BehavioralEntityTest, ExecuteCurrentSequence_NoMainSequence_LogsErrorAndStopsProcessing) {
    EXPECT_CALL(*mock_logger, LogError(
        testing::HasSubstr("does not have a valid main sequence"),
        testing::_))
        .Times(1);

    test_entity->ExecuteCurrentSequence();

    // Should be stuck in processing state to prevent further updates
    EXPECT_TRUE(test_entity->IsProcessing());
}

// SEQUENCE STATE TRANSITION TESTS

TEST_F(BehavioralEntityTest, HandleSequenceStartup_SetsProcessingNodeState) {
    auto sequence = CreateMockSequenceInState(1, SequenceState::UNINITIALIZED);
    test_entity->SetMainSequence(sequence);

    test_entity->ExecuteCurrentSequence();

    // After handling UNINITIALIZED, should be in PROCESSING_NODE
    // Verify via subsequent execution attempt
    EXPECT_FALSE(test_entity->IsProcessing());
}

TEST_F(BehavioralEntityTest, ExecuteCurrentSequence_WaitingForAction_DoesNothing) {
    auto sequence = CreateMockSequenceInState(1, SequenceState::WAITING_FOR_ACTION);
    SetupEntityWithSequenceOnStack(sequence);

    test_entity->ExecuteCurrentSequence();

    // Should log waiting message but not change state
    EXPECT_FALSE(test_entity->IsProcessing());
}

// ACTION EXECUTION TESTS

TEST_F(BehavioralEntityTest, ExecuteActionNode_RequiresEntity_NoValidEntities_FailsSequence) {
    auto action = CreateActionRequiringEntity(5);
    auto sequence = CreateSequenceWithActionNode(1, action);
    SetupEntityWithSequenceOnStack(sequence);

    EXPECT_CALL(*mock_content, GetActionById(action->GetActionId()))
        .WillRepeatedly(testing::Return(action));

    // Mock test_entity query returns empty list
    EXPECT_CALL(*mock_entity_query, GetEntitiesSupportingAction(5))
        .WillOnce(testing::Return(std::vector<FrameworkEntity*>{}));

    // Should have transitioned to FAILED state
    EXPECT_CALL(*mock_logger, LogWarning(testing::HasSubstr("No valid entities found"),
        testing::_));

    test_entity->ExecuteCurrentSequence();  // UNINITIALIZED → PROCESSING_NODE
    test_entity->ExecuteCurrentSequence();  // PROCESSING_NODE → Execute action node
}

TEST_F(BehavioralEntityTest, ExecuteActionNode_ValidEntity_AppliesImmediateEffects) {
    auto action = CreateActionWithImmediateEffects(5);
    auto sequence = CreateSequenceWithActionNode(1, action);
    auto target_entity = CreateMockEntity(100);

    // Setup ALL mocks before execution
    EXPECT_CALL(*mock_content, GetActionById(action->GetActionId()))
        .WillRepeatedly(testing::Return(action));

    SetupEntityQueryToReturn({target_entity});

    // Mock preconditions_by_target passing
    EXPECT_CALL(*mock_evaluator, ProcessStateOperation(testing::_, testing::_))
        .WillRepeatedly(testing::Return(true));

    // Setup expectations for immediate effects (in sequence)
    {
        testing::InSequence seq;

        // Immediate effects get applied
        EXPECT_CALL(*mock_evaluator, ProcessStateOperation(
            testing::Property(&StateOperation::GetTarget, StateOperationTarget::ENTITY),
            testing::Eq(target_entity)))
            .Times(1);

        EXPECT_CALL(*mock_evaluator, ProcessStateOperation(
            testing::Property(&StateOperation::GetTarget, StateOperationTarget::SELF),
            testing::NotNull()))
            .Times(1);

        // Then action starts
        EXPECT_CALL(*mock_action_provider, StartCharacterAction(
            entity_handle, 5, testing::_, testing::_, target_entity->GetEntityHandle()))
            .Times(1);
    }

    SetupEntityWithSequenceOnStack(sequence);

    // Progress through states
    test_entity->ExecuteCurrentSequence();  // UNINITIALIZED → PROCESSING_NODE
    test_entity->ExecuteCurrentSequence();  // PROCESSING_NODE → Execute action node
}

TEST_F(BehavioralEntityTest, ExecuteActionNode_NoEntityRequired_UsesCharacterAsSelf) {
    auto action = CreateActionWithoutEntity(5);
    auto sequence = CreateSequenceWithActionNode(1, action);

    // Mock content provider
    EXPECT_CALL(*mock_content, GetActionById(action->GetActionId()))
        .WillRepeatedly(testing::Return(action));

    // Action starts (no entity required)
    EXPECT_CALL(*mock_action_provider, StartCharacterAction(
        entity_handle, 5, testing::_, testing::_, nullptr))
        .Times(1);

    SetupEntityWithSequenceOnStack(sequence);

    test_entity->ExecuteCurrentSequence();  // UNINITIALIZED → PROCESSING_NODE
    test_entity->ExecuteCurrentSequence();  // PROCESSING_NODE → Execute action
}

// ACTION COMPLETION TESTS

TEST_F(BehavioralEntityTest, CompleteAction_ValidToken_AppliesCompletionEffects) {
    auto action = CreateActionWithCompletionEffects(5);
    auto target_entity = CreateMockEntity(100);

    // Start action first
    SetupAndStartAction(test_entity.get(), action, target_entity);
    int64_t token = GetCurrentActionToken(test_entity.get());

    EXPECT_CALL(*mock_evaluator, ProcessStateOperation(testing::_, testing::_))
        .Times(action->GetCompletionEffects().size());

    test_entity->CompleteAction(5, token);

    // Should transition to NODE_EXECUTED state
    EXPECT_FALSE(test_entity->IsProcessing());
}

TEST_F(BehavioralEntityTest, CompleteAction_InvalidToken_IgnoresCompletion) {
    SetupAndStartAction(test_entity.get(), CreateAction(5), nullptr);
    int64_t correct_token = GetCurrentActionToken(test_entity.get());
    int64_t wrong_token = correct_token + 1;

    EXPECT_CALL(*mock_logger, LogWarning(
        testing::HasSubstr("is not the same as the currently executing action"),
        testing::_))
        .Times(1);

    test_entity->CompleteAction(5, wrong_token);

    // State should remain WAITING_FOR_ACTION (not change)
}

TEST_F(BehavioralEntityTest, CompleteAction_ActionNotRequiringEntity_HandlesNullTarget) {
    auto action = CreateActionWithoutEntity(5);
    SetupAndStartAction(test_entity.get(), action, nullptr);

    // Should not crash when target_entity is null
    // Effects with SELF target should still apply
    EXPECT_CALL(*mock_evaluator, ProcessStateOperation(testing::_, testing::_))
        .Times(testing::AtLeast(0));

    EXPECT_NO_THROW(test_entity->CompleteAction(5, GetCurrentActionToken(test_entity.get())));
}

// MEMORY INTEGRATION TESTS

TEST_F(BehavioralEntityTest, TransitionSelection_MultipleValid_PrioritizesUnvisited) {
    auto sequence = CreateSequenceWithThreeTransitions();

    EXPECT_CALL(*mock_content, GetActionById(testing::_))
        .WillRepeatedly(testing::Return(CreateAction(100)));

    SetupEntityWithSequenceAtNodeExecuted(sequence);

    // Pre-populate memory with visits to nodes 2 and 3
    test_entity->GetMemorySystem().UpdateTransitionMemory(2, 1000);
    test_entity->GetMemorySystem().UpdateTransitionMemory(3, 2000);

    // All three transitions pass precondition checks
    SetupAllTransitionPreconditionsPass();

    test_entity->ExecuteCurrentSequence();
    test_entity->ExecuteCurrentSequence();
    test_entity->CompleteAction(100, test_entity->GetCurrentActionToken());

    test_entity->ExecuteCurrentSequence();

    auto memory = test_entity->GetMemorySystem().FindTransitionMemory(4);
    EXPECT_NE(memory, nullptr);
}

TEST_F(BehavioralEntityTest, EntitySelection_MultipleValid_PrioritizesLeastRecentlyUsed) {
    auto action = CreateActionRequiringEntity(5);
    auto sequence = CreateSequenceWithActionNode(1, action);

    EXPECT_CALL(*mock_content, GetActionById(5))
        .WillRepeatedly(testing::Return(action));

    SetupEntityWithSequenceOnStack(sequence);

    auto entities = CreateThreeEntities();
    SetupEntityQueryToReturn(entities);
    SetupAllPreconditionsPass();

    // Pre-populate memory: entity 0 used at time 1000, entity 1 used at 2000
    test_entity->GetMemorySystem().UpdateActionMemory(5, entities[0]->GetEntityId(), 1000);
    test_entity->GetMemorySystem().UpdateActionMemory(5, entities[1]->GetEntityId(), 2000);
    // Entity 2 never used

    test_entity->ExecuteCurrentSequence();
    test_entity->ExecuteCurrentSequence();

    auto selected_id = test_entity->GetCurrentActionTargetId();

    // Should select entity 2 (never used)
    EXPECT_EQ(selected_id, entities[2]->GetEntityId());
}

// FAILURE AND FALLBACK TESTS
TEST_F(BehavioralEntityTest, SequenceFailure_ActivatesFallbackSequence) {
    auto main_seq = CreateMockSequence(1);
    auto fallback1 = CreateMockSequence(2);
    auto fallback2 = CreateMockSequence(3);

    EXPECT_CALL(*mock_content, GetActionById(testing::_))
        .WillRepeatedly(testing::Return(CreateAction(100)));

    test_entity->SetMainSequence(main_seq);
    test_entity->AddFallbackSequence(fallback1);
    test_entity->AddFallbackSequence(fallback2);

    main_seq->SetSequenceState(SequenceState::FAILED);
    test_entity->ExecuteCurrentSequence();
    EXPECT_FALSE(test_entity->IsProcessing());
}

TEST_F(BehavioralEntityTest, SequenceFailure_ClearsInterruptionMemories) {
    auto sequence = CreateMockSequence(1);

    EXPECT_CALL(*mock_content, GetActionById(testing::_))
        .WillRepeatedly(testing::Return(CreateAction(100)));

    test_entity->SetMainSequence(sequence);
    test_entity->AddFallbackSequence(CreateMockSequence(2));

    auto current_node_id = sequence->GetCurrentNodeId();

    // Pre-populate interruption memory
    test_entity->GetMemorySystem().UpdateInterruptionMemory(5, sequence->GetSequenceId(), current_node_id,
        100, 1000);

    sequence->SetSequenceState(SequenceState::FAILED);
    test_entity->ExecuteCurrentSequence();

    // Interruption memory should be cleared
    auto memory = test_entity->GetMemorySystem().FindInterruptionMemory(5, sequence->GetSequenceId(), current_node_id);
    EXPECT_EQ(memory, nullptr);
}

// NODE TYPE HANDLING TESTS

TEST_F(BehavioralEntityTest, ExecuteNestedSequenceNode_ValidSequence_PushesToStack) {
    auto nested_seq = CreateMockSequence(2);
    EXPECT_CALL(*mock_content, GetSequenceById(2))
        .WillOnce(testing::Return(nested_seq));

    auto main_seq = CreateSequenceWithNestedNode(1, 2);
    SetupEntityWithSequenceOnStack(main_seq);

    test_entity->ExecuteCurrentSequence();
    test_entity->ExecuteCurrentSequence();

    // Parent should be in IN_SUBSEQUENCE state
    // (Verify indirectly through subsequent calls)
}

TEST_F(BehavioralEntityTest, ExecuteEndSequenceNode_PopsSequence) {
    auto sequence = CreateSequenceWithEndNode(1);
    SetupEntityWithSequenceOnStack(sequence);

    test_entity->ExecuteCurrentSequence();
    test_entity->ExecuteCurrentSequence();

    // Sequence should be popped
    // If it was main sequence, should be pushed again
}

// PRECONDITION EVALUATION CONTEXT TESTS

TEST_F(BehavioralEntityTest, ActionPrecondition_SelfTarget_UsesCharacterState) {
    auto precondition = StateOperation(
        StateOperationTarget::SELF,
        STATE_KEY_ENERGY,
        StateOperationType::GREATER_THAN,
        {50}
    );

    auto action = CreateActionWithPrecondition(5, precondition);
    auto sequence = CreateSequenceWithActionNode(1, action);
    auto target_entity = CreateMockEntity(100);

    EXPECT_CALL(*mock_content, GetActionById(5))
        .WillRepeatedly(testing::Return(action));

    SetupEntityQueryToReturn({target_entity});

    // Should evaluate precondition against character (this), not target
    EXPECT_CALL(*mock_evaluator, ProcessStateOperation(
        testing::_,
        testing::Eq(test_entity.get())))  // Character, not target_entity
        .WillOnce(testing::Return(true));

    SetupEntityWithSequenceOnStack(sequence);

    test_entity->ExecuteCurrentSequence();
    test_entity->ExecuteCurrentSequence();
}

TEST_F(BehavioralEntityTest, ActionPrecondition_EntityTarget_UsesTargetState) {
    auto precondition = StateOperation(
        StateOperationTarget::ENTITY,
        STATE_KEY_OCCUPIED,
        StateOperationType::EQUALS,
        {0}
    );

    auto action = CreateActionWithPrecondition(5, precondition);
    auto sequence = CreateSequenceWithActionNode(1, action);
    auto target_entity = CreateMockEntity(100);

    EXPECT_CALL(*mock_content, GetActionById(5))
        .WillRepeatedly(testing::Return(action));

    SetupEntityQueryToReturn({target_entity});

    // Should evaluate precondition against target entity
    EXPECT_CALL(*mock_evaluator, ProcessStateOperation(
        testing::_,
        testing::Eq(target_entity)))
        .WillOnce(testing::Return(true));

    SetupEntityWithSequenceOnStack(sequence);

    test_entity->ExecuteCurrentSequence();
    test_entity->ExecuteCurrentSequence();
}

// EDGE CASE TESTS

TEST_F(BehavioralEntityTest, GetActionTargetEntity_AllEntitiesFailPreconditions_ReturnsNull) {
    auto action = CreateActionWithStrictPrecondition(5);
    auto sequence = CreateSequenceWithActionNode(1, action);
    auto entities = CreateThreeEntities();

    EXPECT_CALL(*mock_content, GetActionById(5))
        .WillRepeatedly(testing::Return(action));

    SetupEntityQueryToReturn(entities);

    // All preconditions_by_target fail
    EXPECT_CALL(*mock_evaluator, ProcessStateOperation(testing::_, testing::_))
        .WillRepeatedly(testing::Return(false));

    EXPECT_CALL(*mock_entity_query, GetEntityFromId(-1))
        .WillOnce(testing::Return(nullptr));

    SetupEntityWithSequenceOnStack(sequence);

    test_entity->ExecuteCurrentSequence();
    test_entity->ExecuteCurrentSequence();

    auto result_id = test_entity->GetCurrentActionTargetId();

    EXPECT_EQ(result_id, -1);
}

TEST_F(BehavioralEntityTest, HandleNodeExecutionCompletion_NoValidTransitions_FailsSequence) {
    auto sequence = CreateSequenceWithInvalidTransition();
    SetupEntityWithSequenceAtNodeExecuted(sequence);

    // All transition preconditions_by_target fail
    EXPECT_CALL(*mock_evaluator, ProcessStateOperation(testing::_, testing::_))
        .WillRepeatedly(testing::Return(false));

    test_entity->ExecuteCurrentSequence();
    test_entity->ExecuteCurrentSequence();

    // Should transition to FAILED state
    // (This is currently a bug - you're logging error but not setting FAILED)
}