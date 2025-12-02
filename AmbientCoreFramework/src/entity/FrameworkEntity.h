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
    std::unordered_map<int32_t, int32_t> state;

public:

    /**
     * @throw std::invalid_argument if entity_id < 0.
     */
    explicit FrameworkEntity(void* entity_handle, int32_t entity_id, std::string name = "");

    void* GetEntityHandle() const { return entity_handle; }

    int32_t GetEntityId() const { return entity_id; }

    std::string_view GetName() const { return name; }

    int32_t GetStateValue(int32_t state_key);
    void SetStateValue(int32_t state_key, int32_t value);

};
}
