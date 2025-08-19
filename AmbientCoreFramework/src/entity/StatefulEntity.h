/**
 * @file StatefulEntity.h
 * @brief 
 * @author Eric Buitrón López
 * @date 8/19/2025
 *
 * TODO: Missing state management support
*/

#pragma once
#include <unordered_set>

#include "Entity.h"

namespace AmbientCharacterBehavior {
/**
 * @brief Interactive objects that maintain state and support actions
 */
class StatefulEntity : public Entity {
private:
    std::unordered_set<int> accepted_actions_ids;

public:
    /**
     *
     * @param entity_id The identifier being used as the handle of the entity.
     * @param current_location_id The identifier (also used as handle) of the current location
     * @param name The name of the entity
     *
     * @throw std::invalid_argument if entity_id or current_location_id < 0.
     */
    explicit StatefulEntity(int entity_id, int current_location_id, std::string name = "") :
        Entity(entity_id, current_location_id, name) {}

    /**
     * @brief Checks if the stateful entity accepts a specific action
     * @param action_id The identifier of the action to look for
     * @return True if the action is supported, false if not.
     */
    bool SupportsAction(int action_id) const { return accepted_actions_ids.find(action_id) != accepted_actions_ids.end(); }

    /**
     *
     * @param action_id The identifier of the action to add support for.
     */
    void AddSupportedAction(int action_id) { accepted_actions_ids.insert(action_id); }

    /**
     * @param action_id The identifier of the action to remove support for.
     */
    void RemoveSupportedAction(int action_id) { accepted_actions_ids.erase(action_id); }

    const std::unordered_set<int>& GetSupportedActions() const { return accepted_actions_ids; }
};
} // AmbientCharacterBehavior
