#pragma once
#include "interfaces/IFrameworkRegistry.h"
#include <gmock/gmock.h>

namespace AmbientCharacterBehavior {
class MockFrameworkRegistry : public IFrameworkRegistry {
public:
    MOCK_METHOD(bool, RegisterSequences, (const std::string& config_file_path), (override));
    MOCK_METHOD(bool, RegisterActions, (const std::string& config_file_path), (override));

    MOCK_METHOD(void, QueueEntityRegistration, (void* entity_handle,const std::string& config_file_path), (override));
    MOCK_METHOD(void, QueueEntityUnregistration, (void* entity_handle), (override));
    MOCK_METHOD(size_t, ProcessPendingEntityCommands, (), (override));
    MOCK_METHOD(size_t, GetPendingCommandCount, (), (const, override));
    MOCK_METHOD(void, ClearPendingCommands, (), (override));

    MOCK_METHOD(bool, HasSequence, (int32_t sequence_id), (const, override));
    MOCK_METHOD(std::shared_ptr<Sequence>, GetSequenceById, (int32_t sequence_id), (const, override));
    MOCK_METHOD(bool, HasAction, (int32_t action_id), (const, override));
    MOCK_METHOD(std::shared_ptr<Action>, GetActionById, (int32_t action_id), (const, override));

    MOCK_METHOD(bool, HasFrameworkEntity, (int32_t entity_id), (const, override));
    MOCK_METHOD(FrameworkEntity*, GetFrameworkEntityById, (int32_t entity_id), (const, override));

    MOCK_METHOD(bool, HasBehavioralEntity, (int32_t entity_id), (const, override));
    MOCK_METHOD(BehavioralEntity*, GetBehavioralEntityById, (int32_t entity_id), (const, override));
    MOCK_METHOD(BehavioralEntity*, GetBehavioralEntityByHandle, (void* entity_handle), (const, override));

    MOCK_METHOD(std::vector<BehavioralEntity*>, GetBehavioralEntitiesRange, (int32_t start_index, int32_t count), (const, override));
    MOCK_METHOD(int32_t, GetBehavioralEntityCount, (), (const, override));
};
}