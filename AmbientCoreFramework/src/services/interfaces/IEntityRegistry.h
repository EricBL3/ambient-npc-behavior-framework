#pragma once
#include <cstdint>
#include <string>

#include "entity/BehavioralEntity.h"
#include "entity/FrameworkEntity.h"
#include "services/layers/2_simulation/EntityPosition.h"

namespace AmbientCharacterBehavior {
class IEntityRegistry {
public:
    virtual ~IEntityRegistry() {}

    virtual void QueueEntityRegistration(void* handle, const std::string& path, Position3D position) = 0;
    virtual void QueueEntityUnregistration(void* handle) = 0;
    virtual size_t ProcessPendingEntityCommands(int32_t batch_size) = 0;
    virtual size_t GetPendingCommandCount() const = 0;
    virtual void ClearPendingCommands() = 0;



    virtual bool HasFrameworkEntity(int32_t entity_id) const = 0;
    virtual FrameworkEntity* GetFrameworkEntityById(int32_t entity_id) const = 0;

    virtual bool HasBehavioralEntity(int32_t entity_id) const = 0;
    virtual BehavioralEntity* GetBehavioralEntityById(int32_t entity_id) const = 0;
    virtual BehavioralEntity* GetBehavioralEntityByHandle(void* entity_handle) const = 0;

    virtual std::vector<BehavioralEntity*> GetBehavioralEntitiesRange(int32_t start_index, int32_t count) const = 0;
    virtual int32_t GetBehavioralEntityCount() const = 0;
};
}
