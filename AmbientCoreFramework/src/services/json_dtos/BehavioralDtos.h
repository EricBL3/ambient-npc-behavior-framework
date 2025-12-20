#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace AmbientCharacterBehavior {

/// Needs custom parsing due to optional fields
struct SequenceNodeDto {

    int32_t node_id;
    std::string node_type;
    std::optional<int32_t> target_action_id;
    std::optional<int32_t> target_sequence_id;
};

struct StateOperationDto {
    std::string target_id_name;
    std::string state_key_name;
    std::string operation_name;
    int32_t value;
};

struct TransitionDto {
    int32_t transition_id;
    int32_t from_node_id;
    int32_t to_node_id;
    std::vector<StateOperationDto> preconditions;
};

/// Needs custom parsing due to use of other dto with custom parsing
struct SequenceDto {
    int32_t sequence_id;
    std::string sequence_name;
    int32_t entry_point_node_id;
    std::vector<SequenceNodeDto> nodes;
    std::vector<TransitionDto> transitions;
};

struct ActionDto {
    int32_t action_id;
    std::string action_name;
    std::vector<StateOperationDto> preconditions;
    std::vector<StateOperationDto> immediate_effects;
    std::vector<StateOperationDto> completion_effects;
    std::vector<StateOperationDto> interruption_effects;
    int64_t action_duration_ms;
    int64_t action_timeout_ms;
    std::string interruption_behavior_name;
};

}
