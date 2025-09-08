#pragma once
#include <string>

namespace AmbientCharacterBehavior {
class IFrameworkRegistry {
public:
    virtual ~IFrameworkRegistry() = default;
    virtual void RegisterSequences(const std::string& config_file_path) = 0;
    virtual void RegisterActions(const std::string& config_file_path) = 0;
    virtual void RegisterEntity(void* entity_handle, const std::string& config_file_path) = 0;
    virtual void UnregisterEntity(void* entity_handle) = 0;
};
}
