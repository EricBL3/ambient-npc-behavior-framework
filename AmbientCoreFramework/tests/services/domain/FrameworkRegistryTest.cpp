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
    EXPECT_EQ(0, sequence->GetEntryPointIndex());
    EXPECT_EQ(1, sequence->GetNodes().size());
}