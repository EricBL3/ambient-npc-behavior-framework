/**
 * @file Entity
 * @brief Implements the Entity class
 * @author Eric Buitrón López
 * @date 8/19/2025
 *
 *
*/

#include "Entity.h"

#include <stdexcept>

using namespace AmbientCharacterBehavior;

Entity::Entity(int entity_id, int current_location_id, std::string name) : entity_id(entity_id),
    current_location_id(current_location_id), name(std::move(name))
{
    if (entity_id < 0)
    {
        throw std::invalid_argument("Entity: entity_id cannot be negative");
    }

    if (current_location_id < 0)
    {
        throw std::invalid_argument("Entity: current_location_id cannot be negative");
    }
}

void Entity::SetCurrentLocationId(int new_location_id)
{
    if (new_location_id < 0)
    {
        throw std::invalid_argument("Entity: new_location_id cannot be negative");
    }

    current_location_id = new_location_id;
}
