#pragma once
#include "interfaces/IFrameworkRegistry.h"
#include <gmock/gmock.h>

namespace AmbientCharacterBehavior {
class MockFrameworkRegistry : public IFrameworkRegistry {
public:
    MOCK_METHOD(void, RegisterSequences, (const std::string& config_file_path), (override));
    MOCK_METHOD(void, RegisterActions, (const std::string& config_file_path), (override));
    MOCK_METHOD(void, RegisterEntity, (void* entity_handle,const std::string& config_file_path), (override));
    MOCK_METHOD(void, UnregisterEntity, (void* entity_handle), (override));

    MOCK_METHOD(bool, HasSequence, (int32_t sequence_id), (const, override));
    MOCK_METHOD(std::shared_ptr<Sequence>, GetSequenceById, (int32_t sequence_id), (const, override));
    MOCK_METHOD(bool, HasAction, (int32_t action_id), (const, override));
    MOCK_METHOD(std::shared_ptr<Action>, GetActionById, (int32_t action_id), (const, override));

};
}