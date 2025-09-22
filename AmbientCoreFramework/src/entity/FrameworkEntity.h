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
    std::unordered_map<int32_t, int32_t> state;

public:

    /**
     * @throw std::invalid_argument if entity_id < 0.
     */
    explicit FrameworkEntity(void* entity_handle, int32_t entity_id, std::string name = "");

    void* GetEntityHandle() const { return entity_handle; }

    int32_t GetEntityId() const { return entity_id; }

    std::string_view GetName() const { return name; }

    const std::unordered_set<int32_t>& GetSupportedActionsIds() const { return supported_actions_ids; }

    void AddSupportedAction(int32_t action_id);

    void RemoveSupportedAction(int32_t action_id) { supported_actions_ids.erase(action_id); }

    bool SupportsAction(int32_t action_id) const { return supported_actions_ids.find(action_id) != supported_actions_ids.end(); }

    int32_t GetStateValue(int32_t state_key);
    void SetStateValue(int32_t state_key, int32_t value);

};
}
