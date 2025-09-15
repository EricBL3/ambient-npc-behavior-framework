
#pragma once
#include <stack>
#include <unordered_map>

#include "FrameworkEntity.h"
#include "behavior/Sequence.h"
#include "memory/MemorySystem.h"


namespace AmbientCharacterBehavior {
/**
 * @brief Entities that have decision-making capabilities and behavioral systems (Characters)
 */
class BehavioralEntity : public FrameworkEntity {
private:
    MemorySystem memory;

    std::shared_ptr<Sequence> main_sequence;

    std::stack<std::shared_ptr<Sequence>> sequences;

    std::vector<std::shared_ptr<Sequence>> fallback_sequences;

    std::unordered_map<int, std::shared_ptr<Sequence>> interruption_handlers;

    int32_t current_action_target_index;

    bool is_processing;

public:
    /**
     * @throw std::invalid_argument if entity_id or current_location_id < 0.
     */
    explicit BehavioralEntity(void* entity_handle, int32_t entity_id, int32_t max_transition_memories,
        int32_t max_action_memories, int32_t max_interruption_memories, std::string name = "") :
        FrameworkEntity(entity_handle, entity_id, std::move(name)),
        memory(max_transition_memories, max_action_memories, max_interruption_memories),
        main_sequence(nullptr), current_action_target_index(-1), is_processing(false) {}

    const MemorySystem& GetMemorySystem() const { return memory; }
    void SetMainSequence(const std::shared_ptr<Sequence> &new_sequence) { main_sequence = new_sequence; }
    const std::shared_ptr<Sequence>& GetMainSequence() const { return main_sequence; }
    void AddFallbackSequence(const std::shared_ptr<Sequence> &new_sequence) { fallback_sequences.emplace_back(new_sequence); }

    bool HasFallbackSequence(int32_t sequence_id) const;
    std::shared_ptr<Sequence> GetFallbackSequenceById(int32_t sequence_id) const;

};
}
