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

#include "Entity.h"
#include "StatefulEntity.h"
#include "behavior/Sequence.h"
#include "memory/MemorySystem.h"


namespace AmbientCharacterBehavior {
/**
 * @brief Entities that have decision-making capabilities and behavioral systems (Characters)
 */
class BehavioralEntity : public StatefulEntity {
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
     * @param entity_id The identifier being used as the handle of the entity.
     * @param current_location_id The identifier (also used as handle) of the current location
     * @param name The name of the entity
     *
     * @throw std::invalid_argument if entity_id or current_location_id < 0.
     */
    explicit BehavioralEntity(int32_t entity_id, int32_t current_location_id, std::string name = "") :
        StatefulEntity(entity_id, current_location_id, std::move(name)),
        main_sequence(nullptr), current_action_target_index(-1), is_processing(false) {}

};
} // AmbientCharacterBehavior
