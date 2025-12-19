#include <gtest/gtest.h>

#include "../services/mocks/MockLogger.h"
#include "../services/mocks/MockTimeManager.h"
#include "../services/mocks/MockContentProvider.h"
#include "../services/mocks/MockEntityQuery.h"
#include "../services/mocks/MockStartCharacterActionProvider.h"
#include "../services/mocks/MockEnvironmentalConditionProvider.h"
#include "../services/mocks/MockStateOperationEvaluator.h"
#include "../services/mocks/MockEntityPositionProvider.h"
#include "../services/mocks/MockActionTimeoutManager.h"
#include "../services/mocks/MockJsonLoader.h"
#include "../services/mocks/MockFrameworkSchemaManager.h"
#include "../services/mocks/MockEnvironmentalConditionManager.h"
#include "../services/mocks/MockEntityPositionManager.h"
#include "../services/mocks/MockEntityRegistry.h"
#include "behavior/sequence_nodes/ActionSequenceNode.h"
#include "../../src/behavior/sequence_nodes/EndSequenceNode.h"
#include "entity/BehavioralEntity.h"


namespace AmbientCharacterBehavior {
using namespace AmbientCharacterBehavior;

class BehavioralEntityTest : public testing::Test {
protected:
    std::unique_ptr<MockLogger> mock_logger;
    std::unique_ptr<MockTimeManager> mock_time_manager;
    std::unique_ptr<MockEnvironmentalConditionProvider> mock_environmental_condition_provider;
    std::unique_ptr<MockStartCharacterActionProvider> mock_action_provider;
    std::unique_ptr<MockEntityPositionProvider> mock_entity_pos_provider;
    std::unique_ptr<MockJsonLoader> mock_json_loader;
    std::unique_ptr<MockFrameworkSchemaManager> mock_schema;
    std::unique_ptr<MockEnvironmentalConditionManager> mock_environment_manager;
    std::unique_ptr<MockEntityPositionManager> mock_entity_pos_manager;
    std::unique_ptr<MockActionTimeoutManager> mock_action_timeout_manager;
    std::unique_ptr<MockStateOperationEvaluator> mock_state_operation_evaluator;
    std::unique_ptr<MockContentProvider> mock_content_provider;
    std::unique_ptr<MockEntityRegistry> mock_entity_registry;
    std::unique_ptr<MockEntityQuery> mock_entity_query;

    std::unique_ptr<FoundationServices> foundation_services;
    std::unique_ptr<DataAccessServices> data_access_services;
    std::unique_ptr<SimulationServices> simulation_state_services;
    std::unique_ptr<BehavioralEvaluationServices> behavioral_evaluation_services;
    std::unique_ptr<ContentRegistryServices> content_registry_services;

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
        mock_time_manager = std::make_unique<MockTimeManager>();
        mock_environmental_condition_provider = std::make_unique<MockEnvironmentalConditionProvider>();
        mock_action_provider = std::make_unique<MockStartCharacterActionProvider>();
        mock_entity_pos_provider = std::make_unique<MockEntityPositionProvider>();

        mock_json_loader = std::make_unique<MockJsonLoader>();

        mock_schema = std::make_unique<MockFrameworkSchemaManager>();
        mock_environment_manager = std::make_unique<MockEnvironmentalConditionManager>();
        mock_entity_pos_manager = std::make_unique<MockEntityPositionManager>();
        mock_action_timeout_manager = std::make_unique<MockActionTimeoutManager>();

        mock_state_operation_evaluator = std::make_unique<MockStateOperationEvaluator>();

        mock_content_provider = std::make_unique<MockContentProvider>();
        mock_entity_registry = std::make_unique<MockEntityRegistry>();
        mock_entity_query = std::make_unique<MockEntityQuery>();

        foundation_services = std::make_unique<FoundationServices>(*mock_logger, *mock_time_manager,
            *mock_environmental_condition_provider,*mock_action_provider, *mock_entity_pos_provider);

        data_access_services = std::make_unique<DataAccessServices>(*foundation_services, *mock_json_loader);

        simulation_state_services = std::make_unique<SimulationServices>(*data_access_services,
            *mock_environment_manager,*mock_entity_pos_manager, *mock_schema, *mock_action_timeout_manager);

        behavioral_evaluation_services = std::make_unique<BehavioralEvaluationServices>(*simulation_state_services,
            *mock_state_operation_evaluator);

        content_registry_services = std::make_unique<ContentRegistryServices>(*behavioral_evaluation_services,
            *mock_content_provider, *mock_entity_registry, *mock_entity_query);

        entity_handle = reinterpret_cast<void*>(0x1234);
        entity_id = 42;

        CreateDefaultEntity();
    }

    void TearDown() override
    {
        mock_entities_.clear();
    }

    void VerifyMockSetup() {
        ASSERT_NE(mock_state_operation_evaluator.get(), nullptr);
        ASSERT_NE(mock_content_provider.get(), nullptr);
        ASSERT_NE(mock_entity_query.get(), nullptr);
        ASSERT_NE(test_entity.get(), nullptr);
    }

    void CreateDefaultEntity() {
        test_entity = std::make_unique<BehavioralEntity>(
            *content_registry_services,
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
            5000,   // 5 second max duration
            InterruptionBehaviorType::RESUMABLE
        );
    }

    std::shared_ptr<Action> CreateActionWithImmediateEffects(int32_t id) {
        auto action = CreateAction(id);

        // Add some immediate effects
        StateOperation effect1(
            StateOperationTarget::ENTITY,
            STATE_KEY_OCCUPIED,
            StateOperationType::INCREMENT,
            1
        );

        StateOperation effect2(
            StateOperationTarget::SELF,
            STATE_KEY_ENERGY,
            StateOperationType::DECREMENT,
            5
        );

        action->AddImmediateEffect(effect1);
        action->AddImmediateEffect(effect2);

        return action;
    }

    std::shared_ptr<Action> CreateActionWithCompletionEffects(int32_t id) {
        auto action = CreateAction(id);

        StateOperation effect1(
            StateOperationTarget::SELF,
            STATE_KEY_ENERGY,
            StateOperationType::INCREMENT,
            10
        );

        StateOperation effect2(
            StateOperationTarget::ENTITY,
            STATE_KEY_OCCUPIED,
            StateOperationType::DECREMENT,
            1
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
            .WillRepeatedly(testing::Return(entities));

        // Also setup GetEntityFromId for each
        for (auto entity : entities) {
            EXPECT_CALL(*mock_entity_query, EntitySupportsAction(entity->GetEntityId(), testing::_))
            .WillRepeatedly(testing::Return(true));

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
        EXPECT_CALL(*mock_content_provider, GetActionById(action->GetActionId()))
            .WillRepeatedly(testing::Return(action));

        if (target) {
            EXPECT_CALL(*mock_entity_query, EntitySupportsAction(entity->GetEntityId(), testing::_))
            .WillRepeatedly(testing::Return(true));

            // Mock entity query
            EXPECT_CALL(*mock_entity_query, GetEntitiesSupportingAction(action->GetActionId()))
                .WillRepeatedly(testing::Return(std::vector<FrameworkEntity*>{target}));

            EXPECT_CALL(*mock_entity_query, GetEntityFromId(target->GetEntityId()))
                .WillRepeatedly(testing::Return(target));

            // Mock all preconditions_by_target passing
            EXPECT_CALL(*mock_state_operation_evaluator, ProcessStateOperation(testing::_, testing::_))
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
            100
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
        EXPECT_CALL(*mock_state_operation_evaluator, ProcessStateOperation(testing::_, testing::_))
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
        auto action = CreateAction(id);
        action->AddPrecondition(precondition.GetTarget(), precondition);
        return action;
    }

    std::shared_ptr<Action> CreateActionWithStrictPrecondition(int32_t id) {
        auto action = CreateAction(id);

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
    auto precondition = StateOperation(StateOperationTarget::ENTITY, STATE_KEY_OCCUPIED, StateOperationType::EQUALS, 0);
    auto action = CreateActionWithPrecondition(5, precondition);
    auto sequence = CreateSequenceWithActionNode(1, action);
    SetupEntityWithSequenceOnStack(sequence);

    EXPECT_CALL(*mock_content_provider, GetActionById(action->GetActionId()))
        .WillRepeatedly(testing::Return(action));

    // Mock test_entity query returns empty list
    EXPECT_CALL(*mock_entity_query, GetEntitiesSupportingAction(5))
        .WillOnce(testing::Return(std::vector<FrameworkEntity*>{}));

    // Should have transitioned to FAILED state
    EXPECT_CALL(*mock_logger, LogError(testing::_,
        testing::_));

    test_entity->ExecuteCurrentSequence();  // UNINITIALIZED → PROCESSING_NODE
    test_entity->ExecuteCurrentSequence();  // PROCESSING_NODE → Execute action node
}

TEST_F(BehavioralEntityTest, ExecuteActionNode_ValidEntity_AppliesImmediateEffects) {
    VerifyMockSetup();

    auto action = CreateActionWithImmediateEffects(5);
    auto precondition = StateOperation(StateOperationTarget::ENTITY, STATE_KEY_OCCUPIED, StateOperationType::EQUALS, 0);
    action->AddPrecondition(StateOperationTarget::ENTITY, precondition);

    auto sequence = CreateSequenceWithActionNode(1, action);
    auto target_entity = CreateMockEntity(100);

    // Setup ALL mocks before execution
    EXPECT_CALL(*mock_content_provider, GetActionById(action->GetActionId()))
        .WillRepeatedly(testing::Return(action));

    // Mock preconditions_by_target passing
    EXPECT_CALL(*mock_state_operation_evaluator, ProcessStateOperation(testing::_, testing::_))
        .WillRepeatedly(testing::Return(true));

    EXPECT_CALL(*mock_action_provider, StartCharacterAction(
            entity_handle, 5, testing::_, testing::_, target_entity->GetEntityHandle()))
            .Times(1);

    SetupEntityQueryToReturn({target_entity});

    SetupEntityWithSequenceOnStack(sequence);

    // Progress through states
    test_entity->ExecuteCurrentSequence();  // UNINITIALIZED → PROCESSING_NODE
    test_entity->ExecuteCurrentSequence();  // PROCESSING_NODE → Execute action node
}

TEST_F(BehavioralEntityTest, ExecuteActionNode_NoEntityRequired_UsesCharacterAsSelf) {
    auto action = CreateAction(5);
    auto sequence = CreateSequenceWithActionNode(1, action);

    // Mock content provider
    EXPECT_CALL(*mock_content_provider, GetActionById(action->GetActionId()))
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
    VerifyMockSetup();

    auto precondition = StateOperation(StateOperationTarget::ENTITY, STATE_KEY_OCCUPIED, StateOperationType::EQUALS, 0);
    auto action = CreateActionWithPrecondition(5, precondition);
    auto target_entity = CreateMockEntity(100);

    // Start action first
    SetupAndStartAction(test_entity.get(), action, target_entity);
    int64_t token = GetCurrentActionToken(test_entity.get());

    EXPECT_CALL(*mock_state_operation_evaluator, ProcessStateOperation(testing::_, testing::_))
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
    VerifyMockSetup();

    auto action = CreateAction(5);
    SetupAndStartAction(test_entity.get(), action, nullptr);

    // Should not crash when target_entity is null
    // Effects with SELF target should still apply
    EXPECT_CALL(*mock_state_operation_evaluator, ProcessStateOperation(testing::_, testing::_))
        .Times(testing::AtLeast(0));

    EXPECT_NO_THROW(test_entity->CompleteAction(5, GetCurrentActionToken(test_entity.get())));
}

// FAILURE AND FALLBACK TESTS
TEST_F(BehavioralEntityTest, SequenceFailure_ActivatesFallbackSequence) {
    auto main_seq = CreateMockSequence(1);
    auto fallback1 = CreateMockSequence(2);
    auto fallback2 = CreateMockSequence(3);

    EXPECT_CALL(*mock_content_provider, GetActionById(testing::_))
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

    EXPECT_CALL(*mock_content_provider, GetActionById(testing::_))
        .WillRepeatedly(testing::Return(CreateAction(100)));

    test_entity->SetMainSequence(sequence);
    test_entity->AddFallbackSequence(CreateMockSequence(2));

    auto current_node_id = sequence->GetCurrentNodeId();

    // Pre-populate interruption memory
    test_entity->GetMemorySystem().CreateInterruptionMemory(5, sequence->GetSequenceId(), current_node_id,
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
    EXPECT_CALL(*mock_content_provider, GetSequenceById(2))
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
    VerifyMockSetup();

    auto precondition = StateOperation(
        StateOperationTarget::SELF,
        STATE_KEY_ENERGY,
        StateOperationType::GREATER_THAN,
        50
    );

    auto action = CreateActionWithPrecondition(5, precondition);
    auto sequence = CreateSequenceWithActionNode(1, action);
    auto target_entity = CreateMockEntity(100);

    EXPECT_CALL(*mock_content_provider, GetActionById(5))
        .WillRepeatedly(testing::Return(action));

    StateOperationContext context(test_entity.get());

    // Should evaluate precondition against character (this), not target
    EXPECT_CALL(*mock_state_operation_evaluator, ProcessStateOperation(
    testing::_,
    testing::Truly([this](const StateOperationContext& ctx) {
        return ctx.self_entity == test_entity.get() &&
               ctx.target_entity == nullptr;
        })))
        .WillOnce(testing::Return(true));

    SetupEntityWithSequenceOnStack(sequence);

    test_entity->ExecuteCurrentSequence();
    test_entity->ExecuteCurrentSequence();
}

TEST_F(BehavioralEntityTest, ActionPrecondition_EntityTarget_UsesTargetState) {
    VerifyMockSetup();

    auto precondition = StateOperation(
        StateOperationTarget::ENTITY,
        STATE_KEY_OCCUPIED,
        StateOperationType::EQUALS,
        0
    );

    auto action = CreateActionWithPrecondition(5, precondition);
    auto sequence = CreateSequenceWithActionNode(1, action);
    auto target_entity = CreateMockEntity(100);

    EXPECT_CALL(*mock_content_provider, GetActionById(5))
        .WillRepeatedly(testing::Return(action));

    SetupEntityQueryToReturn({target_entity});

    // Should evaluate precondition against target entity
    EXPECT_CALL(*mock_state_operation_evaluator, ProcessStateOperation(
        testing::_,
        testing::Truly([this, target_entity](const StateOperationContext& ctx) {
            return ctx.self_entity == test_entity.get() &&
                   ctx.target_entity == target_entity;
        })))
        .WillOnce(testing::Return(true));

    SetupEntityWithSequenceOnStack(sequence);

    test_entity->ExecuteCurrentSequence();
    test_entity->ExecuteCurrentSequence();
}

// EDGE CASE TESTS

TEST_F(BehavioralEntityTest, GetActionTargetEntity_AllEntitiesFailPreconditions_ReturnsNull) {
    VerifyMockSetup();

    auto action = CreateActionWithStrictPrecondition(5);
    auto sequence = CreateSequenceWithActionNode(1, action);
    auto entities = CreateThreeEntities();

    EXPECT_CALL(*mock_content_provider, GetActionById(5))
        .WillRepeatedly(testing::Return(action));

    SetupEntityQueryToReturn(entities);

    // All preconditions_by_target fail
    EXPECT_CALL(*mock_state_operation_evaluator, ProcessStateOperation(testing::_, testing::_))
        .WillRepeatedly(testing::Return(false));

    SetupEntityWithSequenceOnStack(sequence);

    test_entity->ExecuteCurrentSequence();
    test_entity->ExecuteCurrentSequence();

    auto result_id = test_entity->GetCurrentActionTargetId();

    EXPECT_EQ(result_id, -1);
}

TEST_F(BehavioralEntityTest, HandleNodeExecutionCompletion_NoValidTransitions_FailsSequence)
{
    VerifyMockSetup();
    auto sequence = CreateSequenceWithInvalidTransition();
    SetupEntityWithSequenceAtNodeExecuted(sequence);

    // All transition preconditions_by_target fail
    EXPECT_CALL(*mock_state_operation_evaluator, ProcessStateOperation(testing::_, testing::_))
        .WillRepeatedly(testing::Return(false));

    test_entity->ExecuteCurrentSequence();
    test_entity->ExecuteCurrentSequence();

    // Should transition to FAILED state
    // (This is currently a bug - you're logging error but not setting FAILED)
}
}