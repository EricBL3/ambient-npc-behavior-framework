#pragma once

#include <gmock/gmock.h>

#include "interfaces/IStateSchemaManager.h"

namespace AmbientCharacterBehavior {
class MockStateSchemaManager : public IStateSchemaManager {
public:
    MOCK_METHOD(void, LoadStateSchema, (const std::string& config_file_path), (override));
    MOCK_METHOD((int32_t), GetStateKey, (const std::string& state_name), (override));
    MOCK_METHOD((std::string), GetStateName, (int32_t state_key), (override));
    MOCK_METHOD((StateOperationType), GetStateOperationTypeId, (const std::string& state_name), (override));
    MOCK_METHOD((std::string), GetStateOperationTypeName, (StateOperationType id), (override));
};
}