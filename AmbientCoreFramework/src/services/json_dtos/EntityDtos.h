#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace AmbientCharacterBehavior {

struct FrameworkEntityDto {
    int32_t entity_id;
    std::string entity_name;
    std::vector<int32_t> accepted_actions_ids;
    std::unordered_map<std::string, int32_t> initial_state;
};

struct MemoryLimitsDto {
    int32_t max_transition_memories;
    int32_t max_action_memories;
    int32_t max_interruption_memories;
};

struct BehavioralEntityDto {
    FrameworkEntityDto base_properties;
    int32_t main_sequence_id;
    std::vector<int32_t> fallback_sequences;
    std::unordered_map<std::string, int32_t> interruption_handlers;
    MemoryLimitsDto memory_limits;
};

/// Requires custom parsing
struct EntityDtoResult {
    std::string entity_type;
    std::optional<FrameworkEntityDto> framework_entity;
    std::optional<BehavioralEntityDto> behavioral_entity;
};

}
