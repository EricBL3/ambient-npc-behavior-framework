#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace AmbientCharacterBehavior {
/**
 * @brief Interactive entities that maintain state and support actions.
 *
 */
class FrameworkEntity {
protected:
    /**
     * @invariant entity_handle != nullptr
     */
    void* entity_handle;

    /**
     * @invariant entity_id  >= 0
     */
    int32_t entity_id;

    std::string name;
    std::unordered_map<int32_t, int32_t> state;

public:
    /**
     * @brief Creates a framework entity
     * @param entity_handle The handle of the entity (must not be nulltpr)
     * @param entity_id The identifier for the entity (must be >= 0)
     * @param name (Optional) The name of the entity. Used mostly for logging purposes.
     * @throw std::invalid_argument if entity_id < 0.
     */
    explicit FrameworkEntity(void* entity_handle, int32_t entity_id, std::string name = "");

    [[nodiscard]]
    void* GetEntityHandle() const { return entity_handle; }

    [[nodiscard]]
    int32_t GetEntityId() const { return entity_id; }

    [[nodiscard]]
    std::string_view GetName() const { return name; }

    /**
     * @return The state value that is paired with the passed key.
     * @throw std::runtime_error if the state_key was not found
     */
    int32_t GetStateValue(int32_t state_key);

    /**
     * @brief Assigns the value to the state key.
     * @param state_key
     * @param value
     */
    void SetStateValue(int32_t state_key, int32_t value);
};
}
