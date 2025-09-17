#pragma once
#include "services/composition/ApplicationContext.h"

namespace AmbientCharacterBehavior {
class BehaviorFramework {
private:
    std::unique_ptr<ApplicationContext> app_context;
    bool is_initialized;

public:
    explicit BehaviorFramework(std::unique_ptr<ApplicationContext> context) :
        app_context(std::move(context)), is_initialized(false) {}

    ApplicationContext& GetServices() const { return *app_context; }
};
}
