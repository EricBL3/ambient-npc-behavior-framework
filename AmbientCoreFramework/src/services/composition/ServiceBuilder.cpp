#include "ServiceBuilder.h"
#include "integration/BehaviorFramework.h"

using namespace AmbientCharacterBehavior;

std::unique_ptr<ApplicationContext> ServiceBuilder::CreateApplicationContext(
    QueryEnvironmentalConditionFn query_callback,
    StartCharacterActionFn start_action_callback,
    QueryEntityPositionFn query_position_callback
)
{
    if (!query_callback || !start_action_callback || !query_position_callback) {
        throw std::invalid_argument(
            "CreateApplicationContext: Callbacks cannot be null");
    }

    return ApplicationContext::Create(query_callback, start_action_callback, query_position_callback);
}

std::unique_ptr<BehaviorFramework> ServiceBuilder::CreateBehaviorFramework(
    QueryEnvironmentalConditionFn query_callback,
    StartCharacterActionFn start_action_callback,
    QueryEntityPositionFn query_position_callback)
{
    return std::make_unique<BehaviorFramework>(
        CreateApplicationContext(query_callback, start_action_callback, query_position_callback));
}
