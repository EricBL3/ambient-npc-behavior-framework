#pragma once
#include <unordered_map>

#include "EntityPosition.h"
#include "services/interfaces/IEntityPositionManager.h"
#include "services/interfaces/IEntityPositionProvider.h"
#include "services/interfaces/ILogger.h"
#include "services/interfaces/ITimeManager.h"

namespace AmbientCharacterBehavior {
class EntityPositionManager : public IEntityPositionManager {
    std::pmr::unordered_map<void*, EntityPosition> entity_position_cache;

    ILogger& logger;
    ITimeManager& time_manager;
    IEntityPositionProvider& position_provider;

public:
    explicit EntityPositionManager(ILogger& logger, ITimeManager& time_manager, IEntityPositionProvider& position_provider) :
        logger(logger), time_manager(time_manager), position_provider(position_provider) {}

    // Should be called during entity registration
    void RegisterEntityPosition(void* entity_handle, Position3D position, bool is_static,
        int64_t update_frequency_ms) override;

    // Should be called during entity unregistration
    void UnregisterEntityPosition(void* entity_handle) override;

    // Returns the Manhattan distance between 2 entities. Handles the position updates internally if needed.
    int32_t CalculateDistance(void* entity_a_handle, void* entity_b_handle) override;

    void UpdateEntityPosition(void* entity_handle) override;
};
}
