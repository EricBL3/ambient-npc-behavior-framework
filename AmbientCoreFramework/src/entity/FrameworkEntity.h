/**
 * @file FrameworkEntity.h
 * @brief 
 * @author Eric Buitrón López
 * @date 8/19/2025
 *
 * TODO: Missing state management support
*/

#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace AmbientCharacterBehavior {
/**
 * @brief Interactive entities that maintain state and support actions
 */
class FrameworkEntity {
protected:

    void* entity_handle;

    int32_t entity_id;

    std::string name;

    std::unordered_set<int32_t> supported_actions_ids;

    /**
     * @brief Maps state keys to their values.
     */
    std::unordered_map<int32_t, int32_t> state;

public:

    /**
     *
     * @param entity_handle The handle of the entity.
     * @param entity_id The identifier of the entity.
     * @param name The name of the entity
     *
     * @throw std::invalid_argument if entity_id < 0.
     */
    explicit FrameworkEntity(void* entity_handle, int32_t entity_id, std::string name = "");

    void* GetEntityHandle() const { return entity_handle; }

    int32_t GetEntityId() const { return entity_id; }

    std::string_view GetName() const { return name; }

    const std::unordered_set<int32_t>& GetSupportedActionsIds() const { return supported_actions_ids; }

    /**
     *
     * @param action_id The identifier of the action to add support for.
     */
    void AddSupportedAction(int32_t action_id);

    /**
     * @param action_id The identifier of the action to remove support for.
     */
    void RemoveSupportedAction(int32_t action_id) { supported_actions_ids.erase(action_id); }

    /**
     * @brief Checks if the stateful entity accepts a specific action
     * @param action_id The identifier of the action to look for
     * @return True if the action is supported, false if not.
     */
    bool SupportsAction(int32_t action_id) const { return supported_actions_ids.find(action_id) != supported_actions_ids.end(); }

    /**
     * @brief Gets the value of a state property of the entity
     * @param state_key The key for the value to get
     * @return The value of the state
     * @throw std::runtime_error if the state doesn't exist.
     */
    int32_t GetStateValue(int32_t state_key);

    void SetStateValue(int32_t state_key, int32_t value);

};
} // AmbientCharacterBehavior
