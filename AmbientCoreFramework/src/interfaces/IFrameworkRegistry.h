#pragma once
#include <memory>
#include <string>

#include "behavior/Action.h"
#include "behavior/Sequence.h"
#include "entity/FrameworkEntity.h"

namespace AmbientCharacterBehavior {
class IFrameworkRegistry {
public:
    virtual ~IFrameworkRegistry() = default;
    virtual void RegisterSequences(const std::string& config_file_path) = 0;
    virtual void RegisterActions(const std::string& config_file_path) = 0;
    virtual void RegisterEntity(void* entity_handle, const std::string& config_file_path) = 0;
    virtual void UnregisterEntity(void* entity_handle) = 0;

    virtual bool HasSequence(int32_t sequence_id) const = 0;
    virtual std::shared_ptr<Sequence> GetSequenceById(int32_t sequence_id) const = 0;
    virtual bool HasAction(int32_t action_id) const = 0;
    virtual std::shared_ptr<Action> GetActionById(int32_t action_id) const = 0;

    virtual bool HasFrameworkEntity(int32_t entity_id) const = 0;
    virtual FrameworkEntity* GetFrameworkEntityById(int32_t entity_id) const = 0;

    virtual bool HasBehavioralEntity(int32_t entity_id) const = 0;
    virtual BehavioralEntity* GetBehavioralEntityById(int32_t entity_id) const = 0;
    virtual BehavioralEntity* GetBehavioralEntityByHandle(void* entity_handle) const = 0;

    virtual std::vector<BehavioralEntity*> GetBehavioralEntitiesRange(int32_t start_index, int32_t count) const = 0;
    virtual int32_t GetBehavioralEntityCount() const = 0;
};
}
