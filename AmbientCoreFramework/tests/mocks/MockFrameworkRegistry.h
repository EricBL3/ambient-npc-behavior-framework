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
};
}
