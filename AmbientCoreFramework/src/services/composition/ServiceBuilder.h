#pragma once
#include <memory>

#include "ApplicationContext.h"
#include "../include/BehaviorFrameworkInterface.h"
#include "integration/BehaviorFramework.h"

namespace AmbientCharacterBehavior {

class ServiceBuilder {
private:

public:

    static std::unique_ptr<ApplicationContext> CreateApplicationContext(
        QueryEnvironmentalConditionFn query_callback,
        StartCharacterActionFn start_action_callback,
        QueryEntityPositionFn query_position_callback
    );

    static std::unique_ptr<BehaviorFramework> CreateBehaviorFramework(
        QueryEnvironmentalConditionFn query_callback,
        StartCharacterActionFn start_action_callback,
        QueryEntityPositionFn query_position_callback
    );
};
}
