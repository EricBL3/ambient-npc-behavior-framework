#pragma once
#include <cstdint>
#include <string>

#include "utils/StateOperationType.h"

namespace AmbientCharacterBehavior {
/**
 * @brief Handles the mappings of different containers in the framework.
 */
class IFrameworkSchemaManager {

public:
    virtual ~IFrameworkSchemaManager() = default;
    virtual void LoadFrameworkSchema(const std::string& config_file_path) = 0;

    virtual int32_t GetStateKey(const std::string& state_name) = 0;
    virtual std::string GetStateName(int32_t state_key) = 0;

    virtual StateOperationType GetStateOperationTypeId(const std::string &name) = 0;
    virtual std::string GetStateOperationTypeName(StateOperationType id) = 0;

    virtual int32_t GetInterruptionKey(const std::string& interruption_name) = 0;
    virtual std::string GetInterruptionName(int32_t interruption_key) = 0;
};
}
