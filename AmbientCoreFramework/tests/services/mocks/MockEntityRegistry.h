#pragma once
#include <gmock/gmock.h>
#include "services/interfaces/IEntityRegistry.h"

namespace AmbientCharacterBehavior {
class MockEntityRegistry : public IEntityRegistry {
    MOCK_METHOD(void, QueueEntityRegistration, (void* entity_handle,const std::string& config_file_path, Position3D position), (override));
    MOCK_METHOD(void, QueueEntityUnregistration, (void* entity_handle), (override));
    MOCK_METHOD(size_t, ProcessPendingEntityCommands, (int32_t character_batch_size), (override));
    MOCK_METHOD(size_t, GetPendingCommandCount, (), (const, override));
    MOCK_METHOD(void, ClearPendingCommands, (), (override));

    MOCK_METHOD(bool, HasFrameworkEntity, (int32_t entity_id), (const, override));
    MOCK_METHOD(FrameworkEntity*, GetFrameworkEntityById, (int32_t entity_id), (const, override));

    MOCK_METHOD(bool, HasBehavioralEntity, (int32_t entity_id), (const, override));
    MOCK_METHOD(BehavioralEntity*, GetBehavioralEntityById, (int32_t entity_id), (const, override));
    MOCK_METHOD(BehavioralEntity*, GetBehavioralEntityByHandle, (void* entity_handle), (const, override));

    MOCK_METHOD(std::vector<BehavioralEntity*>, GetBehavioralEntitiesRange, (int32_t start_index, int32_t count), (const, override));
    MOCK_METHOD(int32_t, GetBehavioralEntityCount, (), (const, override));

};
}
