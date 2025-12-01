#include <gtest/gtest.h>

#include "../../mocks/MockEnvironmentalConditionManager.h"
#include "../../mocks/MockJsonLoader.h"
#include "../../mocks/MockLogger.h"
#include "../../mocks/MockFrameworkSchemaManager.h"
#include "../../../src/services/registry/FrameworkRegistry.h"
#include "../../mocks/MockStartCharacterActionProvider.h"
#include "../../mocks/MockTimeManager.h"
#include "../../mocks/MockStateOperationEvaluator.h"

using namespace AmbientCharacterBehavior;

class FrameworkRegistryTest : public testing::Test {
protected:
    std::unique_ptr<MockLogger> mock_logger;
    std::unique_ptr<MockTimeManager> mock_time_manager;
    std::unique_ptr<MockStartCharacterActionProvider> mock_action_provider;
    std::unique_ptr<MockJsonLoader> mock_json_loader;
    std::unique_ptr<MockFrameworkSchemaManager> mock_schema;
    std::unique_ptr<MockEnvironmentalConditionManager> mock_environment_manager;
    std::unique_ptr<MockStateOperationEvaluator> mock_state_operation_evaluator;

    std::unique_ptr<FrameworkRegistry> registry;
    int framework_entity_handle = 100;
    int behavioral_entity_handle = 200;

    void SetUp() override {
        mock_logger = std::make_unique<MockLogger>();
        mock_action_provider = std::make_unique<MockStartCharacterActionProvider>();
        mock_json_loader = std::make_unique<MockJsonLoader>();
        mock_schema = std::make_unique<MockFrameworkSchemaManager>();
        mock_environment_manager = std::make_unique<MockEnvironmentalConditionManager>();

        registry = std::make_unique<FrameworkRegistry>(*mock_logger, *mock_time_manager, *mock_action_provider, *mock_json_loader,
            *mock_schema, *mock_environment_manager, *mock_state_operation_evaluator);
    }

    SequenceDto CreateBasicSequenceDto(int32_t sequence_id)
    {
        SequenceDto sequence_dto;
        sequence_dto.sequence_id = sequence_id;
        sequence_dto.sequence_name = "test_sequence";
        sequence_dto.entry_point_node_id = 0;

        SequenceNodeDto node_dto;
        node_dto.node_id = 0;
        node_dto.node_type = "END";

        sequence_dto.nodes.push_back(node_dto);

        return sequence_dto;
    }

    SequenceDto CreateSequenceDtoWithSelfTransition(int32_t sequence_id) {
        auto sequence_dto = CreateBasicSequenceDto(sequence_id);

        SequenceNodeDto action_node;
        action_node.node_id = 10;
        action_node.node_type = "ACTION";
        action_node.target_action_id = 1;
        sequence_dto.nodes.push_back(action_node);
        sequence_dto.entry_point_node_id = 10;

        // Add transition with SELF state reference
        TransitionDto transition;
        transition.transition_id = 0;
        transition.from_node_id = 10;
        transition.to_node_id = 0; // END node

        StateOperationDto precondition;
        precondition.target_id_name = "SELF";
        precondition.state_key_name = "ENERGY";
        precondition.operation_name = "EQUALS";
        precondition.value = 100;

        transition.preconditions.push_back(precondition);
        sequence_dto.transitions.push_back(transition);

        return sequence_dto;
    }

    ActionDto CreateBasicActionDto(int32_t action_id)
    {
        ActionDto action_dto;
        action_dto.action_id = action_id;
        action_dto.action_name = "test_action";
        action_dto.requires_target_entity = true;
        action_dto.max_duration_ms = 1000;
        action_dto.interruption_behavior_name = "RESUMABLE";

        return action_dto;
    }

    ActionDto CreateActionDtoWithEntityPrecondition(int32_t action_id)
    {
        auto action_dto = CreateBasicActionDto(action_id);

        StateOperationDto precondition;
        precondition.target_id_name = "ENTITY";
        precondition.state_key_name = "AVAILABLE_SEATS";
        precondition.operation_name = "GREATER_THAN";
        precondition.value = 0;

        action_dto.preconditions.push_back(precondition);

        return action_dto;
    }

    ActionDto CreateActionDtoWithEnvironmentPrecondition(int32_t action_id)
    {
        auto action_dto = CreateBasicActionDto(action_id);

        StateOperationDto precondition;
        precondition.target_id_name = "ENVIRONMENT";
        precondition.state_key_name = "WEATHER";
        precondition.operation_name = "EQUALS";
        precondition.value = 0;

        action_dto.preconditions.push_back(precondition);

        return action_dto;
    }

    void * FrameworkEntityHandle() { return &framework_entity_handle; }
    void * BehavioralEntityHandle() { return &behavioral_entity_handle; }

    EntityDtoResult CreateBasicFrameworkEntityDto(int32_t entity_id)
    {
        EntityDtoResult entity_dto;
        entity_dto.entity_type = "FRAMEWORK";

        FrameworkEntityDto framework_entity_dto;
        framework_entity_dto.entity_id = entity_id;
        framework_entity_dto.entity_name = "test_entity";

        entity_dto.framework_entity = framework_entity_dto;

        return entity_dto;
    }

    EntityDtoResult CreateComplexFrameworkEntityDto(int32_t entity_id)
    {
        auto base_entity = CreateBasicFrameworkEntityDto(entity_id);

        base_entity.framework_entity->accepted_actions_ids = {0};
        base_entity.framework_entity->initial_state = {
            {"AVAILABLE_SEATS", 3},
        };

        return base_entity;
    }

    EntityDtoResult CreateBasicBehavioralEntityDto(int32_t entity_id)
    {
        EntityDtoResult entity_dto;
        entity_dto.entity_type = "BEHAVIORAL";

        auto base_entity = CreateBasicFrameworkEntityDto(entity_id);

        MemoryLimitsDto memory_limits;
        memory_limits.max_action_memories = 15;
        memory_limits.max_transition_memories = 10;
        memory_limits.max_interruption_memories = 5;

        BehavioralEntityDto behavioral_entity_dto;
        behavioral_entity_dto.base_properties = base_entity.framework_entity.value();
        behavioral_entity_dto.memory_limits = memory_limits;
        behavioral_entity_dto.main_sequence_id = 0;

        entity_dto.behavioral_entity = behavioral_entity_dto;

        return entity_dto;
    }

    EntityDtoResult CreateComplexBehavioralEntityDto(int32_t entity_id)
    {
        auto base_entity = CreateBasicBehavioralEntityDto(entity_id);

        base_entity.behavioral_entity->base_properties.accepted_actions_ids = {0};
        base_entity.behavioral_entity->base_properties.initial_state = {
            {"AVAILABLE_SEATS", 3},
        };

        base_entity.behavioral_entity->fallback_sequences = {1};
        base_entity.behavioral_entity->interruption_handlers = {
            {"RAINING", 2},
        };

        return base_entity;
    }
};

// Constructor test
TEST_F(FrameworkRegistryTest, Constructor_ValidServices_CreatesFrameworkRegistry) {
    EXPECT_NO_THROW(FrameworkRegistry framework_registry(*mock_logger, *mock_time_manager, *mock_action_provider,
        *mock_json_loader, *mock_schema, *mock_environment_manager, *mock_state_operation_evaluator));
}

// REGISTER SEQUENCES TESTS
TEST_F(FrameworkRegistryTest, RegisterSequences_AddsSequenceToRegistry)
{
    auto sequence_id = 0;
    auto sequence_dto = CreateBasicSequenceDto(sequence_id);
    EXPECT_CALL(*mock_json_loader, ProcessSequencesConfigFile("test.json"))
        .WillOnce(testing::Return(std::vector{sequence_dto}));

    EXPECT_CALL(*mock_logger, LogWarning(testing::_, testing::_))
        .Times(0);

    EXPECT_CALL(*mock_logger, LogError(testing::_, testing::_))
        .Times(0);

    registry->RegisterSequences("test.json");

    auto sequence = registry->GetSequenceById(sequence_id);

    EXPECT_EQ(1, registry->GetSequencesCount());
    EXPECT_EQ(sequence_id, sequence->GetSequenceId());
    EXPECT_EQ("test_sequence", sequence->GetSequenceName());
    EXPECT_EQ(0, sequence->GetEntryPointNodeId());
    EXPECT_EQ(1, sequence->GetNodeCount());
}

TEST_F(FrameworkRegistryTest, RegisterSequences_EmptyConfigFile_LogsWarningAndReturns) {
    EXPECT_CALL(*mock_json_loader, ProcessSequencesConfigFile("empty.json"))
        .WillOnce(testing::Return(std::vector<SequenceDto>{}));

    EXPECT_CALL(*mock_logger, LogWarning(
        testing::HasSubstr("did not contain any valid sequences"),
        "FrameworkRegistry"))
        .Times(1);

    registry->RegisterSequences("empty.json");

    EXPECT_EQ(0, registry->GetSequencesCount());
}

TEST_F(FrameworkRegistryTest, RegisterSequences_DuplicateSequenceId_LogsWarningAndSkipsSecond) {
    auto dto1 = CreateBasicSequenceDto(1);
    auto dto2 = CreateBasicSequenceDto(1); // Same ID
    dto2.sequence_name = "duplicate_sequence";

    EXPECT_CALL(*mock_json_loader, ProcessSequencesConfigFile("test.json"))
        .WillOnce(testing::Return(std::vector{dto1, dto2}));

    EXPECT_CALL(*mock_logger, LogWarning(
        testing::HasSubstr("was not added to the registry"),
        "FrameworkRegistry"))
        .Times(1);

    registry->RegisterSequences("test.json");

    EXPECT_EQ(1, registry->GetSequencesCount());
    // Should keep the first one
    EXPECT_EQ("test_sequence", registry->GetSequenceById(1)->GetSequenceName());
}

TEST_F(FrameworkRegistryTest, RegisterSequences_StateReference_CallsStateSchema) {
    auto sequence_dto = CreateSequenceDtoWithSelfTransition(1);

    EXPECT_CALL(*mock_json_loader, ProcessSequencesConfigFile("test.json"))
        .WillOnce(testing::Return(std::vector{sequence_dto}));

    EXPECT_CALL(*mock_schema, GetStateKey("ENERGY"))
         .WillOnce(testing::Return(42));

    EXPECT_CALL(*mock_logger, LogWarning(testing::_, testing::_))
        .Times(0);

    EXPECT_CALL(*mock_logger, LogError(testing::_, testing::_))
        .Times(0);

    registry->RegisterSequences("test.json");

    auto sequence = registry->GetSequenceById(1);
    auto transitions = sequence->FindTransitionsFrom(10);
    EXPECT_EQ(1, transitions.size());

    auto preconditions = transitions[0].GetPreconditionsForTarget(StateOperationTarget::SELF);
    EXPECT_EQ(1, preconditions->size());
    EXPECT_EQ(StateOperationTarget::SELF, preconditions->at(0).GetTarget());
    EXPECT_EQ(42, preconditions->at(0).GetStateKey());  // From StateSchema
    EXPECT_EQ(StateOperationType::EQUALS, preconditions->at(0).GetOperationType());
    EXPECT_EQ(100, preconditions->at(0).GetValue());
}

// REGISTER ACTIONS TESTS
TEST_F(FrameworkRegistryTest, RegisterActions_AddsActionToRegistry)
{
    auto action_id = 0;
    auto action_dto = CreateBasicActionDto(action_id);
    EXPECT_CALL(*mock_json_loader, ProcessActionsConfigFile("test.json"))
        .WillOnce(testing::Return(std::vector{action_dto}));

    EXPECT_CALL(*mock_logger, LogWarning(testing::_, testing::_))
        .Times(0);

    EXPECT_CALL(*mock_logger, LogError(testing::_, testing::_))
        .Times(0);

    registry->RegisterActions("test.json");

    auto action = registry->GetActionById(action_id);

    EXPECT_EQ(1, registry->GetActionsCount());
    EXPECT_EQ(action_id, action->GetActionId());
    EXPECT_EQ("test_action", action->GetActionName());
    EXPECT_TRUE(action->GetRequiresTargetEntity());
    EXPECT_EQ(1000, action->GetMaxDuration());
    EXPECT_EQ(InterruptionBehaviorType::RESUMABLE, action->GetInterruptionBehavior());
}

TEST_F(FrameworkRegistryTest, RegisterActions_EmptyConfigFile_LogsWarningAndReturns) {
    EXPECT_CALL(*mock_json_loader, ProcessActionsConfigFile("empty.json"))
        .WillOnce(testing::Return(std::vector<ActionDto>{}));

    EXPECT_CALL(*mock_logger, LogWarning(
        testing::HasSubstr("did not contain any valid actions"),
        "FrameworkRegistry"))
        .Times(1);

    registry->RegisterActions("empty.json");

    EXPECT_EQ(0, registry->GetActionsCount());
}

TEST_F(FrameworkRegistryTest, RegisterActions_DuplicateActionId_LogsWarningAndSkipsSecond) {
    auto dto1 = CreateBasicActionDto(1);
    auto dto2 = CreateBasicActionDto(1); // Same ID
    dto2.action_name = "duplicate_action";

    EXPECT_CALL(*mock_json_loader, ProcessActionsConfigFile("test.json"))
        .WillOnce(testing::Return(std::vector{dto1, dto2}));

    EXPECT_CALL(*mock_logger, LogWarning(
        testing::HasSubstr("was not added to the registry"),
        "FrameworkRegistry"))
        .Times(1);

    registry->RegisterActions("test.json");

    EXPECT_EQ(1, registry->GetActionsCount());
    // Should keep the first one
    EXPECT_EQ("test_action", registry->GetActionById(1)->GetActionName());
}

TEST_F(FrameworkRegistryTest, RegisterActions_StateReference_CallsStateSchema) {
    auto action_dto = CreateActionDtoWithEntityPrecondition(1);

    EXPECT_CALL(*mock_json_loader, ProcessActionsConfigFile("test.json"))
        .WillOnce(testing::Return(std::vector{action_dto}));

    EXPECT_CALL(*mock_schema, GetStateKey("AVAILABLE_SEATS"))
         .WillOnce(testing::Return(3));

    EXPECT_CALL(*mock_schema, GetStateOperationTypeId("GREATER_THAN"))
         .WillOnce(testing::Return(StateOperationType::GREATER_THAN));

    EXPECT_CALL(*mock_logger, LogWarning(testing::_, testing::_))
        .Times(0);

    EXPECT_CALL(*mock_logger, LogError(testing::_, testing::_))
        .Times(0);

    registry->RegisterActions("test.json");

    auto action = registry->GetActionById(1);
    auto preconditions = action->GetPreconditionsForTarget(StateOperationTarget::ENTITY);
    EXPECT_EQ(1, preconditions->size());
    EXPECT_EQ(StateOperationTarget::ENTITY, preconditions->at(0).GetTarget());
    EXPECT_EQ(3, preconditions->at(0).GetStateKey());  // From StateSchema
    EXPECT_EQ(StateOperationType::GREATER_THAN, preconditions->at(0).GetOperationType());
    EXPECT_EQ(0, preconditions->at(0).GetValue());
}

TEST_F(FrameworkRegistryTest, RegisterActions_EnvironmentReference_CallsEnvironmentSchema) {
    auto sequence_dto = CreateActionDtoWithEnvironmentPrecondition(1);

    EXPECT_CALL(*mock_json_loader, ProcessActionsConfigFile("test.json"))
        .WillOnce(testing::Return(std::vector{sequence_dto}));

    EXPECT_CALL(*mock_environment_manager, GetEnvironmentalConditionKey("WEATHER"))
         .WillOnce(testing::Return(3));

    EXPECT_CALL(*mock_logger, LogWarning(testing::_, testing::_))
        .Times(0);

    EXPECT_CALL(*mock_logger, LogError(testing::_, testing::_))
        .Times(0);

    registry->RegisterActions("test.json");

    auto action = registry->GetActionById(1);
    auto preconditions = action->GetPreconditionsForTarget(StateOperationTarget::ENVIRONMENT);
    EXPECT_EQ(1, preconditions->size());
    EXPECT_EQ(StateOperationTarget::ENVIRONMENT, preconditions->at(0).GetTarget());
    EXPECT_EQ(3, preconditions->at(0).GetStateKey());  // From StateSchema
    EXPECT_EQ(StateOperationType::EQUALS, preconditions->at(0).GetOperationType());
    EXPECT_EQ(0, preconditions->at(0).GetValue());
}

// REGISTER ENTITY TESTS
TEST_F(FrameworkRegistryTest, RegisterEntity_AddsFrameworkEntityToRegistry)
{
    auto entity_dto = CreateBasicFrameworkEntityDto(0);

    EXPECT_CALL(*mock_json_loader, ProcessSingleEntityConfigFile("test.json"))
        .WillOnce(testing::Return(std::optional{entity_dto}));

    EXPECT_CALL(*mock_logger, LogWarning(testing::_, testing::_))
        .Times(0);

    EXPECT_CALL(*mock_logger, LogError(testing::_, testing::_))
        .Times(0);

    registry->RegisterEntity(FrameworkEntityHandle(), "test.json");

    auto framework_entity = registry->GetFrameworkEntityById(0);

    EXPECT_EQ(1, registry->GetFrameworkEntitiesCount());
    EXPECT_EQ(0, registry->GetBehavioralEntitiesCount());
    EXPECT_EQ(0, framework_entity->GetEntityId());
    EXPECT_EQ(FrameworkEntityHandle(), framework_entity->GetEntityHandle());
    EXPECT_EQ("test_entity", framework_entity->GetName());
}

TEST_F(FrameworkRegistryTest, RegisterEntity_CreatesFrameworkEntityMapping)
{
    auto entity_dto = CreateBasicFrameworkEntityDto(0);

    EXPECT_CALL(*mock_json_loader, ProcessSingleEntityConfigFile("test.json"))
        .WillOnce(testing::Return(std::optional{entity_dto}));

    EXPECT_CALL(*mock_logger, LogWarning(testing::_, testing::_))
        .Times(0);

    EXPECT_CALL(*mock_logger, LogError(testing::_, testing::_))
        .Times(0);

    registry->RegisterEntity(FrameworkEntityHandle(), "test.json");

    auto framework_entity = registry->GetFrameworkEntityById(0);

    EXPECT_EQ(framework_entity->GetEntityHandle(), registry->GetHandleFromFrameworkId(framework_entity->GetEntityId()));
    EXPECT_EQ(framework_entity->GetEntityId(), registry->GetFrameworkIdFromHandle(framework_entity->GetEntityHandle()));
}

TEST_F(FrameworkRegistryTest, RegisterEntity_AddsBehavioralEntityToRegistry)
{
    auto sequence_dto_0 = CreateBasicSequenceDto(0);
    EXPECT_CALL(*mock_json_loader, ProcessSequencesConfigFile("test.json"))
        .WillOnce(testing::Return(std::vector{sequence_dto_0}));

    registry->RegisterSequences("test.json");

    auto entity_dto = CreateBasicBehavioralEntityDto(0);

    EXPECT_CALL(*mock_json_loader, ProcessSingleEntityConfigFile("test.json"))
        .WillOnce(testing::Return(std::optional{entity_dto}));

    EXPECT_CALL(*mock_logger, LogWarning(testing::_, testing::_))
        .Times(0);

    EXPECT_CALL(*mock_logger, LogError(testing::_, testing::_))
        .Times(0);

    registry->RegisterEntity(BehavioralEntityHandle(), "test.json");

    auto entity = registry->GetBehavioralEntityById(0);

    EXPECT_EQ(0, registry->GetFrameworkEntitiesCount());
    EXPECT_EQ(1, registry->GetBehavioralEntitiesCount());
    EXPECT_EQ(0, entity->GetEntityId());
    EXPECT_EQ(BehavioralEntityHandle(), entity->GetEntityHandle());
    EXPECT_EQ("test_entity", entity->GetName());
    EXPECT_EQ(15, entity->GetMemorySystem().GetMaxActionMemories());
    EXPECT_EQ(10, entity->GetMemorySystem().GetMaxTransitionMemories());
    EXPECT_EQ(5, entity->GetMemorySystem().GetMaxInterruptionMemories());
}

TEST_F(FrameworkRegistryTest, RegisterEntity_CreatesBehavioralEntityMapping)
{
    auto sequence_dto_0 = CreateBasicSequenceDto(0);
    EXPECT_CALL(*mock_json_loader, ProcessSequencesConfigFile("test.json"))
        .WillOnce(testing::Return(std::vector{sequence_dto_0}));

    registry->RegisterSequences("test.json");

    auto entity_dto = CreateBasicBehavioralEntityDto(0);

    EXPECT_CALL(*mock_json_loader, ProcessSingleEntityConfigFile("test.json"))
        .WillOnce(testing::Return(std::optional{entity_dto}));

    EXPECT_CALL(*mock_logger, LogWarning(testing::_, testing::_))
        .Times(0);

    EXPECT_CALL(*mock_logger, LogError(testing::_, testing::_))
        .Times(0);

    registry->RegisterEntity(BehavioralEntityHandle(), "test.json");

    auto entity = registry->GetBehavioralEntityById(0);

    EXPECT_EQ(entity->GetEntityHandle(), registry->GetHandleFromBehavioralId(entity->GetEntityId()));
    EXPECT_EQ(entity->GetEntityId(), registry->GetBehavioralIdFromHandle(entity->GetEntityHandle()));
}

TEST_F(FrameworkRegistryTest, RegisterEntity_InvalidEntityType_LogsWarningAndReturns)
{
    auto entity_dto = CreateBasicBehavioralEntityDto(0);
    entity_dto.entity_type = "OTHER";

    EXPECT_CALL(*mock_json_loader, ProcessSingleEntityConfigFile("test.json"))
        .WillOnce(testing::Return(std::optional{entity_dto}));

    EXPECT_CALL(*mock_logger, LogWarning(
        testing::HasSubstr("does not exist"),
        "FrameworkRegistry"))
        .Times(1);

    registry->RegisterEntity(BehavioralEntityHandle(), "test.json");

    EXPECT_EQ(0, registry->GetFrameworkEntitiesCount());
    EXPECT_EQ(0, registry->GetBehavioralEntitiesCount());
}

TEST_F(FrameworkRegistryTest, RegisterEntity_EmptyConfigFile_LogsWarningAndReturns)
{
    EXPECT_CALL(*mock_json_loader, ProcessSingleEntityConfigFile("test.json"))
        .WillOnce(testing::Return(std::nullopt));

    EXPECT_CALL(*mock_logger, LogWarning(
        testing::HasSubstr("did not contain any valid entity"),
        "FrameworkRegistry"))
        .Times(1);

    registry->RegisterEntity(BehavioralEntityHandle(), "test.json");

    EXPECT_EQ(0, registry->GetFrameworkEntitiesCount());
    EXPECT_EQ(0, registry->GetBehavioralEntitiesCount());
}

TEST_F(FrameworkRegistryTest, RegisterFrameworkEntity_DuplicateEntityId_LogsWarningAndSkipsSecond)
{
    auto entity_dto = CreateBasicFrameworkEntityDto(0);

    EXPECT_CALL(*mock_json_loader, ProcessSingleEntityConfigFile("test.json"))
        .WillOnce(testing::Return(std::optional{entity_dto}));

    registry->RegisterEntity(FrameworkEntityHandle(), "test.json");

    auto repeated_entity_dto = CreateBasicFrameworkEntityDto(0);
    repeated_entity_dto.framework_entity->entity_name = "repeated_entity";

    EXPECT_CALL(*mock_json_loader, ProcessSingleEntityConfigFile("test.json"))
        .WillOnce(testing::Return(std::optional{repeated_entity_dto}));

    EXPECT_CALL(*mock_logger, LogWarning(
       testing::HasSubstr("The framework entity was not generated."),
       "FrameworkRegistry"))
       .Times(1);

    registry->RegisterEntity(BehavioralEntityHandle(), "test.json");

    auto framework_entity = registry->GetFrameworkEntityById(0);

    EXPECT_EQ(1, registry->GetFrameworkEntitiesCount());
    EXPECT_EQ(0, registry->GetBehavioralEntitiesCount());
    EXPECT_EQ(0, framework_entity->GetEntityId());
    EXPECT_EQ(FrameworkEntityHandle(), framework_entity->GetEntityHandle());
    EXPECT_EQ("test_entity", framework_entity->GetName());
}

TEST_F(FrameworkRegistryTest, RegisterFrameworkEntity_DuplicateEntityHandle_LogsWarningAndSkipsSecond)
{
    auto entity_dto = CreateBasicFrameworkEntityDto(0);

    EXPECT_CALL(*mock_json_loader, ProcessSingleEntityConfigFile("test.json"))
        .WillOnce(testing::Return(std::optional{entity_dto}));

    registry->RegisterEntity(FrameworkEntityHandle(), "test.json");

    auto repeated_entity_dto = CreateBasicFrameworkEntityDto(1);
    repeated_entity_dto.framework_entity->entity_name = "repeated_entity";

    EXPECT_CALL(*mock_json_loader, ProcessSingleEntityConfigFile("test.json"))
        .WillOnce(testing::Return(std::optional{repeated_entity_dto}));

    EXPECT_CALL(*mock_logger, LogWarning(
       testing::HasSubstr("The framework entity was not generated."),
       "FrameworkRegistry"))
       .Times(1);

    // repeated handle
    registry->RegisterEntity(FrameworkEntityHandle(), "test.json");

    auto framework_entity = registry->GetFrameworkEntityById(0);

    EXPECT_EQ(1, registry->GetFrameworkEntitiesCount());
    EXPECT_EQ(0, registry->GetBehavioralEntitiesCount());
    EXPECT_EQ(0, framework_entity->GetEntityId());
    EXPECT_EQ(FrameworkEntityHandle(), framework_entity->GetEntityHandle());
    EXPECT_EQ("test_entity", framework_entity->GetName());
}

TEST_F(FrameworkRegistryTest, RegisterBehavioralEntity_DuplicateEntityId_LogsWarningAndSkipsSecond)
{
    auto entity_dto = CreateBasicBehavioralEntityDto(0);

    EXPECT_CALL(*mock_json_loader, ProcessSingleEntityConfigFile("test.json"))
        .WillOnce(testing::Return(std::optional{entity_dto}));

    registry->RegisterEntity(FrameworkEntityHandle(), "test.json");

    auto repeated_entity_dto = CreateBasicBehavioralEntityDto(0);
    repeated_entity_dto.behavioral_entity->base_properties.entity_name = "repeated_entity";

    EXPECT_CALL(*mock_json_loader, ProcessSingleEntityConfigFile("test.json"))
        .WillOnce(testing::Return(std::optional{repeated_entity_dto}));

    EXPECT_CALL(*mock_logger, LogWarning(
       testing::HasSubstr("The framework entity was not generated."),
       "FrameworkRegistry"))
       .Times(1);

    registry->RegisterEntity(BehavioralEntityHandle(), "test.json");

    auto framework_entity = registry->GetBehavioralEntityById(0);

    EXPECT_EQ(0, registry->GetFrameworkEntitiesCount());
    EXPECT_EQ(1, registry->GetBehavioralEntitiesCount());
    EXPECT_EQ(0, framework_entity->GetEntityId());
    EXPECT_EQ(FrameworkEntityHandle(), framework_entity->GetEntityHandle());
    EXPECT_EQ("test_entity", framework_entity->GetName());
}

TEST_F(FrameworkRegistryTest, RegisterBehavioralEntity_DuplicateEntityHandle_LogsWarningAndSkipsSecond)
{
    auto entity_dto = CreateBasicBehavioralEntityDto(0);

    EXPECT_CALL(*mock_json_loader, ProcessSingleEntityConfigFile("test.json"))
        .WillOnce(testing::Return(std::optional{entity_dto}));

    registry->RegisterEntity(BehavioralEntityHandle(), "test.json");

    auto repeated_entity_dto = CreateBasicBehavioralEntityDto(1);
    repeated_entity_dto.behavioral_entity->base_properties.entity_name = "repeated_entity";

    EXPECT_CALL(*mock_json_loader, ProcessSingleEntityConfigFile("test.json"))
        .WillOnce(testing::Return(std::optional{repeated_entity_dto}));

    EXPECT_CALL(*mock_logger, LogWarning(
       testing::HasSubstr("The framework entity was not generated."),
       "FrameworkRegistry"))
       .Times(1);

    // repeated handle
    registry->RegisterEntity(BehavioralEntityHandle(), "test.json");

    auto framework_entity = registry->GetBehavioralEntityById(0);

    EXPECT_EQ(0, registry->GetFrameworkEntitiesCount());
    EXPECT_EQ(1, registry->GetBehavioralEntitiesCount());
    EXPECT_EQ(0, framework_entity->GetEntityId());
    EXPECT_EQ(BehavioralEntityHandle(), framework_entity->GetEntityHandle());
    EXPECT_EQ("test_entity", framework_entity->GetName());
}

// REGISTER COMPLEX FRAMEWORK ENTITY TESTS

TEST_F(FrameworkRegistryTest, RegisterEntity_ComplexFrameworkEntity_RegistersValidActionIdsAndGeneratesStateMap)
{
    auto action_dto = CreateBasicActionDto(0);
    EXPECT_CALL(*mock_json_loader, ProcessActionsConfigFile("test.json"))
        .WillOnce(testing::Return(std::vector{action_dto}));

    registry->RegisterActions("test.json");

    // Test supported_actions_ids matches the ones in the dto
    auto entity_dto = CreateComplexFrameworkEntityDto(0);

    EXPECT_CALL(*mock_json_loader, ProcessSingleEntityConfigFile("test.json"))
        .WillOnce(testing::Return(std::optional{entity_dto}));

    EXPECT_CALL(*mock_logger, LogWarning(testing::_, testing::_))
        .Times(0);

    EXPECT_CALL(*mock_logger, LogError(testing::_, testing::_))
        .Times(0);

    EXPECT_CALL(*mock_schema, GetStateKey("AVAILABLE_SEATS"))
         .WillOnce(testing::Return(0));

    registry->RegisterEntity(FrameworkEntityHandle(), "test.json");

    auto framework_entity = registry->GetFrameworkEntityById(0);

    EXPECT_EQ(1, registry->GetFrameworkEntitiesCount());
    EXPECT_EQ(0, registry->GetBehavioralEntitiesCount());
    EXPECT_EQ(1, framework_entity->GetSupportedActionsIds().size());
    EXPECT_EQ(3, framework_entity->GetStateValue(0));
}

TEST_F(FrameworkRegistryTest, RegisterEntity_ComplexFrameworkEntity_LogsWarningAndSkipsInvalidActionIds)
{
    // Test supported_actions_ids matches the ones in the dto
    auto entity_dto = CreateComplexFrameworkEntityDto(0);

    EXPECT_CALL(*mock_json_loader, ProcessSingleEntityConfigFile("test.json"))
        .WillOnce(testing::Return(std::optional{entity_dto}));

    EXPECT_CALL(*mock_logger, LogWarning(
       testing::HasSubstr("does not exist."),
       "FrameworkRegistry"))
       .Times(1);

    EXPECT_CALL(*mock_schema, GetStateKey("AVAILABLE_SEATS"))
         .WillOnce(testing::Return(0));

    registry->RegisterEntity(FrameworkEntityHandle(), "test.json");

    auto framework_entity = registry->GetFrameworkEntityById(0);

    EXPECT_EQ(1, registry->GetFrameworkEntitiesCount());
    EXPECT_EQ(0, registry->GetBehavioralEntitiesCount());
    EXPECT_EQ(0, framework_entity->GetSupportedActionsIds().size());
    EXPECT_EQ(3, framework_entity->GetStateValue(0));
}

// REGISTER COMPLEX BEHAVIORAL ENTITY TESTS

TEST_F(FrameworkRegistryTest, RegisterEntity_ComplexBehavioralEntity_ConfiguresCorrectly)
{
    auto action_dto = CreateBasicActionDto(0);
    EXPECT_CALL(*mock_json_loader, ProcessActionsConfigFile("test.json"))
        .WillOnce(testing::Return(std::vector{action_dto}));

    registry->RegisterActions("test.json");

    auto sequence_dto_0 = CreateBasicSequenceDto(0);
    auto sequence_dto_1 = CreateBasicSequenceDto(1);
    auto sequence_dto_2 = CreateBasicSequenceDto(2);
    EXPECT_CALL(*mock_json_loader, ProcessSequencesConfigFile("test.json"))
        .WillOnce(testing::Return(std::vector{sequence_dto_0, sequence_dto_1, sequence_dto_2}));

    registry->RegisterSequences("test.json");

    auto entity_dto = CreateComplexBehavioralEntityDto(0);

    EXPECT_CALL(*mock_json_loader, ProcessSingleEntityConfigFile("test.json"))
        .WillOnce(testing::Return(std::optional{entity_dto}));

    EXPECT_CALL(*mock_logger, LogWarning(testing::_, testing::_))
        .Times(0);

    EXPECT_CALL(*mock_logger, LogError(testing::_, testing::_))
        .Times(0);

    EXPECT_CALL(*mock_schema, GetStateKey("AVAILABLE_SEATS"))
         .WillOnce(testing::Return(0));

    registry->RegisterEntity(BehavioralEntityHandle(), "test.json");

    auto framework_entity = registry->GetBehavioralEntityById(0);

    EXPECT_EQ(0, registry->GetFrameworkEntitiesCount());
    EXPECT_EQ(1, registry->GetBehavioralEntitiesCount());
    EXPECT_EQ(1, framework_entity->GetSupportedActionsIds().size());
    EXPECT_EQ(3, framework_entity->GetStateValue(0));
    EXPECT_EQ(0, framework_entity->GetMainSequence()->GetSequenceId());
    EXPECT_EQ(true, framework_entity->HasFallbackSequence(1));
    EXPECT_EQ(2, framework_entity->FindInterruptionHandler(0)->GetSequenceId());
}

TEST_F(FrameworkRegistryTest, RegisterEntity_ComplexBehavioralEntity_LogsWarningAndSkipsInvalidActionIds)
{
    auto sequence_dto_0 = CreateBasicSequenceDto(0);
    auto sequence_dto_1 = CreateBasicSequenceDto(1);
    auto sequence_dto_2 = CreateBasicSequenceDto(2);
    EXPECT_CALL(*mock_json_loader, ProcessSequencesConfigFile("test.json"))
        .WillOnce(testing::Return(std::vector{sequence_dto_0, sequence_dto_1, sequence_dto_2}));

    registry->RegisterSequences("test.json");

    // Test supported_actions_ids matches the ones in the dto
    auto entity_dto = CreateComplexBehavioralEntityDto(0);

    EXPECT_CALL(*mock_json_loader, ProcessSingleEntityConfigFile("test.json"))
        .WillOnce(testing::Return(std::optional{entity_dto}));

    EXPECT_CALL(*mock_logger, LogWarning(
       testing::HasSubstr("does not exist."),
       "FrameworkRegistry"))
       .Times(1);

    EXPECT_CALL(*mock_schema, GetStateKey("AVAILABLE_SEATS"))
         .WillOnce(testing::Return(0));

    registry->RegisterEntity(BehavioralEntityHandle(), "test.json");

    auto framework_entity = registry->GetBehavioralEntityById(0);

    EXPECT_EQ(0, registry->GetFrameworkEntitiesCount());
    EXPECT_EQ(1, registry->GetBehavioralEntitiesCount());
    EXPECT_EQ(0, framework_entity->GetSupportedActionsIds().size());
    EXPECT_EQ(3, framework_entity->GetStateValue(0));
}

TEST_F(FrameworkRegistryTest, RegisterEntity_ComplexBehavioralEntity_LogsWarningAndSkipsInvalidMainSequence)
{
    auto action_dto = CreateBasicActionDto(0);
    EXPECT_CALL(*mock_json_loader, ProcessActionsConfigFile("test.json"))
        .WillOnce(testing::Return(std::vector{action_dto}));

    registry->RegisterActions("test.json");

    auto sequence_dto_1 = CreateBasicSequenceDto(1);
    auto sequence_dto_2 = CreateBasicSequenceDto(2);
    EXPECT_CALL(*mock_json_loader, ProcessSequencesConfigFile("test.json"))
        .WillOnce(testing::Return(std::vector{sequence_dto_1, sequence_dto_2}));

    registry->RegisterSequences("test.json");

    auto entity_dto = CreateComplexBehavioralEntityDto(0);

    EXPECT_CALL(*mock_json_loader, ProcessSingleEntityConfigFile("test.json"))
        .WillOnce(testing::Return(std::optional{entity_dto}));

    EXPECT_CALL(*mock_logger, LogWarning(
       testing::HasSubstr("is not in the registry"),
       "FrameworkRegistry"))
       .Times(1);

    EXPECT_CALL(*mock_schema, GetStateKey("AVAILABLE_SEATS"))
         .WillOnce(testing::Return(0));

    registry->RegisterEntity(BehavioralEntityHandle(), "test.json");

    auto framework_entity = registry->GetBehavioralEntityById(0);

    EXPECT_EQ(0, registry->GetFrameworkEntitiesCount());
    EXPECT_EQ(1, registry->GetBehavioralEntitiesCount());
    EXPECT_EQ(1, framework_entity->GetSupportedActionsIds().size());
    EXPECT_EQ(3, framework_entity->GetStateValue(0));
    EXPECT_EQ(nullptr, framework_entity->GetMainSequence());
}

TEST_F(FrameworkRegistryTest, RegisterEntity_ComplexBehavioralEntity_LogsWarningAndSkipsInvalidFallbackSequences)
{
    auto action_dto = CreateBasicActionDto(0);
    EXPECT_CALL(*mock_json_loader, ProcessActionsConfigFile("test.json"))
        .WillOnce(testing::Return(std::vector{action_dto}));

    registry->RegisterActions("test.json");

    auto sequence_dto_0 = CreateBasicSequenceDto(0);
    auto sequence_dto_2 = CreateBasicSequenceDto(2);
    EXPECT_CALL(*mock_json_loader, ProcessSequencesConfigFile("test.json"))
        .WillOnce(testing::Return(std::vector{sequence_dto_0, sequence_dto_2}));

    registry->RegisterSequences("test.json");

    auto entity_dto = CreateComplexBehavioralEntityDto(0);

    EXPECT_CALL(*mock_json_loader, ProcessSingleEntityConfigFile("test.json"))
        .WillOnce(testing::Return(std::optional{entity_dto}));

    EXPECT_CALL(*mock_logger, LogWarning(
       testing::HasSubstr("is not in the registry"),
       "FrameworkRegistry"))
       .Times(1);

    EXPECT_CALL(*mock_schema, GetStateKey("AVAILABLE_SEATS"))
         .WillOnce(testing::Return(0));

    registry->RegisterEntity(BehavioralEntityHandle(), "test.json");

    auto framework_entity = registry->GetBehavioralEntityById(0);

    EXPECT_EQ(0, registry->GetFrameworkEntitiesCount());
    EXPECT_EQ(1, registry->GetBehavioralEntitiesCount());
    EXPECT_EQ(1, framework_entity->GetSupportedActionsIds().size());
    EXPECT_EQ(3, framework_entity->GetStateValue(0));
    EXPECT_EQ(false, framework_entity->HasFallbackSequence(1));
}

TEST_F(FrameworkRegistryTest, RegisterEntity_ComplexBehavioralEntity_LogsWarningAndSkipsInvalidInterruptionHandlerSequenceMapping)
{
    auto action_dto = CreateBasicActionDto(0);
    EXPECT_CALL(*mock_json_loader, ProcessActionsConfigFile("test.json"))
        .WillOnce(testing::Return(std::vector{action_dto}));

    registry->RegisterActions("test.json");

    auto sequence_dto_0 = CreateBasicSequenceDto(0);
    auto sequence_dto_1 = CreateBasicSequenceDto(1);
    EXPECT_CALL(*mock_json_loader, ProcessSequencesConfigFile("test.json"))
        .WillOnce(testing::Return(std::vector{sequence_dto_0, sequence_dto_1}));

    registry->RegisterSequences("test.json");

    auto entity_dto = CreateComplexBehavioralEntityDto(0);

    EXPECT_CALL(*mock_json_loader, ProcessSingleEntityConfigFile("test.json"))
        .WillOnce(testing::Return(std::optional{entity_dto}));

    EXPECT_CALL(*mock_logger, LogWarning(
       testing::HasSubstr("is not in the registry"),
       "FrameworkRegistry"))
       .Times(1);

    EXPECT_CALL(*mock_logger, LogWarning(
     testing::HasSubstr("not found"),
     testing::_))
     .Times(1);

    EXPECT_CALL(*mock_schema, GetStateKey("AVAILABLE_SEATS"))
         .WillOnce(testing::Return(0));

    registry->RegisterEntity(BehavioralEntityHandle(), "test.json");

    auto framework_entity = registry->GetBehavioralEntityById(0);

    EXPECT_EQ(0, registry->GetFrameworkEntitiesCount());
    EXPECT_EQ(1, registry->GetBehavioralEntitiesCount());
    EXPECT_EQ(1, framework_entity->GetSupportedActionsIds().size());
    EXPECT_EQ(3, framework_entity->GetStateValue(0));
    EXPECT_EQ(true, framework_entity->HasFallbackSequence(1));
    EXPECT_EQ(nullptr, framework_entity->FindInterruptionHandler(0));
}

// UNREGISTER ENTITY TESTS

TEST_F(FrameworkRegistryTest, UnregisterEntity_RemovesFrameworkEntity) {
    auto entity_dto = CreateBasicFrameworkEntityDto(0);

    EXPECT_CALL(*mock_json_loader, ProcessSingleEntityConfigFile("test.json"))
        .WillOnce(testing::Return(std::optional{entity_dto}));

    registry->RegisterEntity(FrameworkEntityHandle(), "test.json");

    EXPECT_EQ(1, registry->GetFrameworkEntitiesCount());

    EXPECT_CALL(*mock_logger, LogWarning(testing::_, testing::_))
    .Times(0);

    EXPECT_CALL(*mock_logger, LogError(testing::_, testing::_))
        .Times(0);

    registry->UnregisterEntity(FrameworkEntityHandle());

    EXPECT_EQ(0, registry->GetFrameworkEntitiesCount());
    EXPECT_EQ(-1, registry->GetFrameworkIdFromHandle(FrameworkEntityHandle()));
    EXPECT_EQ(nullptr, registry->GetHandleFromFrameworkId(0));
}

TEST_F(FrameworkRegistryTest, UnregisterEntity_RemovesBehavioralEntity) {
    auto sequence_dto_0 = CreateBasicSequenceDto(0);
    EXPECT_CALL(*mock_json_loader, ProcessSequencesConfigFile("test.json"))
        .WillOnce(testing::Return(std::vector{sequence_dto_0}));

    registry->RegisterSequences("test.json");

    auto entity_dto = CreateBasicBehavioralEntityDto(0);

    EXPECT_CALL(*mock_json_loader, ProcessSingleEntityConfigFile("test.json"))
        .WillOnce(testing::Return(std::optional{entity_dto}));

    registry->RegisterEntity(BehavioralEntityHandle(), "test.json");

    EXPECT_EQ(1, registry->GetBehavioralEntitiesCount());

    EXPECT_CALL(*mock_logger, LogWarning(testing::_, testing::_))
        .Times(0);

    EXPECT_CALL(*mock_logger, LogError(testing::_, testing::_))
        .Times(0);

    registry->UnregisterEntity(BehavioralEntityHandle());

    EXPECT_EQ(0, registry->GetBehavioralEntitiesCount());
    EXPECT_EQ(-1, registry->GetBehavioralIdFromHandle(BehavioralEntityHandle()));
    EXPECT_EQ(nullptr, registry->GetHandleFromBehavioralId(0));
}

TEST_F(FrameworkRegistryTest, UnregisterEntity_LogsWarningOnNullHandle) {
    auto entity_dto = CreateBasicFrameworkEntityDto(0);

    EXPECT_CALL(*mock_json_loader, ProcessSingleEntityConfigFile("test.json"))
        .WillOnce(testing::Return(std::optional{entity_dto}));

    registry->RegisterEntity(FrameworkEntityHandle(), "test.json");

    EXPECT_EQ(1, registry->GetFrameworkEntitiesCount());

    EXPECT_CALL(*mock_logger, LogWarning(
       testing::HasSubstr("Cannot unregister entity with null handle"),
       "FrameworkRegistry"))
       .Times(1);

    registry->UnregisterEntity(nullptr);

    EXPECT_EQ(1, registry->GetFrameworkEntitiesCount());
}

TEST_F(FrameworkRegistryTest, UnregisterEntity_LogsWarningOnInexistentHandle) {
    auto entity_dto = CreateBasicFrameworkEntityDto(0);

    EXPECT_CALL(*mock_json_loader, ProcessSingleEntityConfigFile("test.json"))
        .WillOnce(testing::Return(std::optional{entity_dto}));

    registry->RegisterEntity(FrameworkEntityHandle(), "test.json");

    EXPECT_EQ(1, registry->GetFrameworkEntitiesCount());

    EXPECT_CALL(*mock_logger, LogWarning(
       testing::HasSubstr("Entity with passed handle does not exist"),
       "FrameworkRegistry"))
       .Times(1);

    registry->UnregisterEntity(BehavioralEntityHandle());

    EXPECT_EQ(1, registry->GetFrameworkEntitiesCount());
}

TEST_F(FrameworkRegistryTest, UnregisterEntity_CanRegisterFrameworkEntityAgain) {
    auto entity_dto = CreateBasicFrameworkEntityDto(0);

    EXPECT_CALL(*mock_json_loader, ProcessSingleEntityConfigFile("test.json"))
        .WillOnce(testing::Return(std::optional{entity_dto}));

    registry->RegisterEntity(FrameworkEntityHandle(), "test.json");
    EXPECT_EQ(1, registry->GetFrameworkEntitiesCount());

    registry->UnregisterEntity(FrameworkEntityHandle());
    EXPECT_EQ(0, registry->GetFrameworkEntitiesCount());

    entity_dto.framework_entity->entity_name = "register_framework_entity_again";

    EXPECT_CALL(*mock_json_loader, ProcessSingleEntityConfigFile("test.json"))
        .WillOnce(testing::Return(std::optional{entity_dto}));

    EXPECT_CALL(*mock_logger, LogWarning(testing::_, testing::_))
    .Times(0);

    EXPECT_CALL(*mock_logger, LogError(testing::_, testing::_))
        .Times(0);

    registry->RegisterEntity(FrameworkEntityHandle(), "test.json");
    auto framework_entity = registry->GetFrameworkEntityById(0);

    EXPECT_EQ(1, registry->GetFrameworkEntitiesCount());
    EXPECT_EQ("register_framework_entity_again", framework_entity->GetName());

}

TEST_F(FrameworkRegistryTest, UnregisterEntity_CanRegisterBehavioralEntityAgain) {
    auto sequence_dto_0 = CreateBasicSequenceDto(0);
    EXPECT_CALL(*mock_json_loader, ProcessSequencesConfigFile("test.json"))
        .WillOnce(testing::Return(std::vector{sequence_dto_0}));

    registry->RegisterSequences("test.json");
    auto entity_dto = CreateBasicBehavioralEntityDto(0);

    EXPECT_CALL(*mock_json_loader, ProcessSingleEntityConfigFile("test.json"))
        .WillOnce(testing::Return(std::optional{entity_dto}));

    registry->RegisterEntity(BehavioralEntityHandle(), "test.json");
    EXPECT_EQ(1, registry->GetBehavioralEntitiesCount());

    registry->UnregisterEntity(BehavioralEntityHandle());
    EXPECT_EQ(0, registry->GetBehavioralEntitiesCount());

    entity_dto.behavioral_entity->base_properties.entity_name = "register_behavioral_entity_again";

    EXPECT_CALL(*mock_json_loader, ProcessSingleEntityConfigFile("test.json"))
        .WillOnce(testing::Return(std::optional{entity_dto}));

    EXPECT_CALL(*mock_logger, LogWarning(testing::_, testing::_))
        .Times(0);

    EXPECT_CALL(*mock_logger, LogError(testing::_, testing::_))
        .Times(0);

    registry->RegisterEntity(BehavioralEntityHandle(), "test.json");
    auto framework_entity = registry->GetBehavioralEntityById(0);

    EXPECT_EQ(1, registry->GetBehavioralEntitiesCount());
    EXPECT_EQ("register_behavioral_entity_again", framework_entity->GetName());
}