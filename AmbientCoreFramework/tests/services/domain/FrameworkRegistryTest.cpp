#include <gtest/gtest.h>

#include "../../mocks/MockJsonLoader.h"
#include "../../mocks/MockLogger.h"
#include "../../mocks/MockStateSchemaManager.h"
#include "services/domain/FrameworkRegistry.h"

using namespace AmbientCharacterBehavior;

class FrameworkRegistryTest : public testing::Test {
protected:
    std::unique_ptr<MockLogger> mock_logger;
    std::unique_ptr<MockJsonLoader> mock_json_loader;
    std::unique_ptr<MockStateSchemaManager> mock_state_schema;

    std::unique_ptr<FrameworkRegistry> registry;

    void SetUp() override {
        mock_logger = std::make_unique<MockLogger>();
        mock_json_loader = std::make_unique<MockJsonLoader>();
        mock_state_schema = std::make_unique<MockStateSchemaManager>();

        registry = std::make_unique<FrameworkRegistry>(*mock_logger, *mock_json_loader, *mock_state_schema);
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
        precondition.parameters = {100};

        transition.preconditions.push_back(precondition);
        sequence_dto.transitions.push_back(transition);

        return sequence_dto;
    }

    ActionDto CreateBasicActionDto(int32_t action_id)
    {
        ActionDto action_dto;
        action_dto.action_id = action_id;
        action_dto.action_name = "test_action";
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
        precondition.parameters = {0};

        action_dto.preconditions.push_back(precondition);

        return action_dto;
    }
};

// Constructor test
TEST_F(FrameworkRegistryTest, Constructor_ValidServices_CreatesFrameworkRegistry) {
    EXPECT_NO_THROW(FrameworkRegistry framework_registry(*mock_logger, *mock_json_loader, *mock_state_schema));
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

    EXPECT_CALL(*mock_state_schema, GetStateKey("ENERGY"))
         .WillOnce(testing::Return(42));

    EXPECT_CALL(*mock_logger, LogWarning(testing::_, testing::_))
        .Times(0);

    EXPECT_CALL(*mock_logger, LogError(testing::_, testing::_))
        .Times(0);

    registry->RegisterSequences("test.json");

    auto sequence = registry->GetSequenceById(1);
    auto transitions = sequence->FindTransitionsFrom(10);
    EXPECT_EQ(1, transitions.size());

    auto preconditions = transitions[0].GetPreconditions();
    EXPECT_EQ(1, preconditions.size());
    EXPECT_EQ(-1, preconditions[0].GetTargetId()); // SELF -> -1
    EXPECT_EQ(42, preconditions[0].GetStateKey());  // From StateSchema
    EXPECT_EQ(0, preconditions[0].GetOperation());
    EXPECT_EQ(100, preconditions[0].GetParameters().front());
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
    EXPECT_EQ("test_sequence", registry->GetActionById(1)->GetActionName());
}

TEST_F(FrameworkRegistryTest, RegisterActions_StateReference_CallsStateSchema) {
    auto sequence_dto = CreateActionDtoWithEntityPrecondition(1);

    EXPECT_CALL(*mock_json_loader, ProcessActionsConfigFile("test.json"))
        .WillOnce(testing::Return(std::vector{sequence_dto}));

    EXPECT_CALL(*mock_state_schema, GetStateKey("AVAILABLE_SEATS"))
         .WillOnce(testing::Return(3));

    EXPECT_CALL(*mock_logger, LogWarning(testing::_, testing::_))
        .Times(0);

    EXPECT_CALL(*mock_logger, LogError(testing::_, testing::_))
        .Times(0);

    registry->RegisterActions("test.json");

    auto action = registry->GetActionById(1);
    auto preconditions = action->GetPreconditions();
    EXPECT_EQ(1, preconditions.size());
    EXPECT_EQ(0, preconditions[0].GetTargetId()); // ENTITY -> 0
    EXPECT_EQ(3, preconditions[0].GetStateKey());  // From StateSchema
    EXPECT_EQ(1, preconditions[0].GetOperation()); // GREATER_THAN
    EXPECT_EQ(0, preconditions[0].GetParameters().front());
}