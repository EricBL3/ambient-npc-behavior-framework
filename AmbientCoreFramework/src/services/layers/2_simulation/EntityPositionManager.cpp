#include "EntityPositionManager.h"
#include <tracy/Tracy.hpp>
#include <stdexcept>

using namespace AmbientCharacterBehavior;

void EntityPositionManager::RegisterEntityPosition(void* entity_handle, Position3D position, bool is_static,
    int64_t update_frequency_ms)
{
    if (entity_position_cache.contains(entity_handle))
    {
        logger.LogWarning("The entity already has registered a position. "
            "Overwriting with new values.", "RegisterEntityPosition");
    }

    EntityPosition entity_position {
        position,
        time_manager.GetCurrentTime(),
        update_frequency_ms,
        is_static
    };

    entity_position_cache[entity_handle] = entity_position;

    logger.LogInfo("Registered position for the entity at (" + std::to_string(position.x) + ", " +
        std::to_string(position.y) + ", " + std::to_string(position.z) + ")" +
        (is_static ? " [STATIC]" : " [DYNAMIC, update_freq=" + std::to_string(update_frequency_ms) + "ms]"),
        "RegisterEntityPosition");
}

void EntityPositionManager::UnregisterEntityPosition(void* entity_handle)
{
    auto iterator = entity_position_cache.find(entity_handle);
    if (iterator == entity_position_cache.end())
    {
        logger.LogWarning("Attempted to unregister position for an entity with no registered position",
            "UnregisterEntityPosition");

        return;
    }

    entity_position_cache.erase(iterator);

    logger.LogInfo("Unregistered position for entity","UnregisterEntityPosition");
}

int32_t EntityPositionManager::CalculateDistance(void* entity_a_handle, void* entity_b_handle)
{
    ZoneScoped;

    // Validate entities exist
    auto iterator_a = entity_position_cache.find(entity_a_handle);
    if (iterator_a == entity_position_cache.end())
    {
        logger.LogError("Entity A has no registered position. Cannot calculate distance.",
            "CalculateDistance");

        throw std::runtime_error("Entity A position not found");
    }

    auto iterator_b = entity_position_cache.find(entity_b_handle);
    if (iterator_b == entity_position_cache.end())
    {
        logger.LogError("Entity B has no registered position. Cannot calculate distance.",
            "CalculateDistance");

        throw std::runtime_error("Entity B position not found");
    }

    // Update positions if needed
    auto current_time = time_manager.GetCurrentTime();

    if (iterator_a->second.NeedsUpdate(current_time))
    {
        UpdateEntityPosition(entity_a_handle);
    }

    if (iterator_b->second.NeedsUpdate(current_time))
    {
        UpdateEntityPosition(entity_b_handle);
    }

    const auto& pos_a = entity_position_cache[entity_a_handle];
    const auto& pos_b = entity_position_cache[entity_b_handle];

    logger.LogInfo("Pos A: (" + std::to_string(pos_a.value.x) + ", " +
               std::to_string(pos_a.value.y) + ", " +
               std::to_string(pos_a.value.z) + ")",
               "CalculateDistance");

    logger.LogInfo("Pos B: (" + std::to_string(pos_b.value.x) + ", " +
                   std::to_string(pos_b.value.y) + ", " +
                   std::to_string(pos_b.value.z) + ")",
                   "CalculateDistance");

    // Calculate Manhattan distance
    auto distance_x = std::abs(pos_a.value.x - pos_b.value.x);
    auto distance_y = std::abs(pos_a.value.y - pos_b.value.y);
    auto distance_z = std::abs(pos_a.value.z - pos_b.value.z);
    auto distance = distance_x + distance_y + distance_z;
    logger.LogInfo("Distance between entity A and entity B = " + std::to_string(distance),
        "CalculateDistance");

    return distance;
}

void EntityPositionManager::UpdateEntityPosition(void* entity_handle)
{
    ZoneScoped;

    auto iterator = entity_position_cache.find(entity_handle);
    if (iterator == entity_position_cache.end())
    {
        logger.LogError("Cannot update position for entity - not registered.",
            "UpdateEntityPosition");

        return;
    }

    auto& position = iterator->second;
    if (position.is_static)
    {
        logger.LogWarning("Attempted to update position of static entity ",
            "UpdateEntityPosition");

        return;
    }

    auto position_query = position_provider.QueryEntityPosition(entity_handle);
    if (!position_query.success)
    {
        logger.LogWarning("Updating position for entity failed.",
            "UpdateEntityPosition");

        position.last_update_ms = time_manager.GetCurrentTime();
        return;
    }

    logger.LogInfo("Updating position to x: " + std::to_string(position_query.position.x) +
        "y: " + std::to_string(position_query.position.y) + "z: " + std::to_string(position_query.position.z),
        "UpdateEntityPosition");

    position.value = position_query.position;
    position.last_update_ms = time_manager.GetCurrentTime();
}