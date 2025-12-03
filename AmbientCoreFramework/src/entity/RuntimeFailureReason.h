#pragma once
namespace AmbientCharacterBehavior {
enum class RuntimeFailureReason {
    NODE_NOT_FOUND,
    ACTION_NOT_FOUND,
    ENTITY_NOT_FOUND,
    SEQUENCE_NOT_FOUND,
    NO_VALID_ENTITIES,
    PRECONDITIONS_FAILED,
    INVALID_NODE_TYPE,
    INTERRUPTION_NOT_FOUND,
};

struct RuntimeFailureContext {
    RuntimeFailureReason reason;
    int32_t action_id = -1;
    int32_t entity_id = -1;
    int32_t node_id = -1;
    int32_t sequence_id = -1;
    int32_t interruption_id = -1;
    std::string additional_info;
    bool should_stop_processing = false;
};
}
