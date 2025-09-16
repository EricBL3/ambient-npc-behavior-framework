#pragma once
#include <cstdint>
#include <string>

#include "utils/StateOperationType.h"

namespace AmbientCharacterBehavior {
/**
 * @brief Handles the mappings of state names and keys.
 */
class IStateSchemaManager {

public:
    virtual ~IStateSchemaManager() = default;
    virtual void LoadStateSchema(const std::string& config_file_path) = 0;
    virtual int32_t GetStateKey(const std::string& state_name) = 0;
    virtual std::string GetStateName(int32_t state_key) = 0;

    virtual StateOperationType GetStateOperationTypeId(const std::string &name) = 0;
    virtual std::string GetStateOperationTypeName(StateOperationType id) = 0;
};
}
