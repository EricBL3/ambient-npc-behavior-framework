/**
 * @file BehavioralEntity.h
 * @brief 
 * @author Eric Buitrón López
 * @date 8/19/2025
 *
 * TODO: Missing state management
*/

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
     *
     * @param entity_handle The handle of the entity.
     * @param entity_id The identifier being used as the handle of the entity.
     * @param name The name of the entity
     *
     * @throw std::invalid_argument if entity_id or current_location_id < 0.
     */
    explicit BehavioralEntity(void* entity_handle, int32_t entity_id, std::string name = "") :
        FrameworkEntity(entity_handle, entity_id, std::move(name)),
        main_sequence(nullptr), current_action_target_index(-1), is_processing(false) {}

};
} // AmbientCharacterBehavior
