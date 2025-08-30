/**
 * @file Entity.h
 * @author Eric Buitrón López
 * @date 8/19/2025
 *
 *
*/

#pragma once
#include <cstdint>
#include <string>
#include <cstdint>

namespace AmbientCharacterBehavior {
/**
 * @brief The foundation for all interactive elements in the simulation.
 */
class Entity {
private:
    int32_t entity_id;

    int32_t current_location_id;

    std::string name;

public:
    /**
     *
     * @param entity_id The identifier being used as the handle of the entity.
     * @param current_location_id The identifier (also used as handle) of the current location
     * @param name The name of the entity
     *
     * @throw std::invalid_argument if entity_id or current_location_id < 0.
     */
    explicit Entity(int32_t entity_id, int32_t current_location_id, std::string name = "");

    int32_t GetEntityId() const { return entity_id; }

    /**
     *
     * @param new_location_id The identifier of the new location
     *
     * @throw std::invalid_argument if new_location_id < 0.
     */
    void SetCurrentLocationId(int32_t new_location_id);

    int32_t GetCurrentLocationId() const { return current_location_id; }

    std::string GetName() const { return name; }

};

}
