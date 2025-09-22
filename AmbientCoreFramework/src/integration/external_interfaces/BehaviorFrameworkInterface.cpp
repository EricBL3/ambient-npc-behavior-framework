#include "../include/BehaviorFrameworkInterface.h"
#include <stdexcept>

static QueryEnvironmentalConditionFn query_callback = nullptr;

extern "C" {
    AmbientCoreFramework_API void RegisterQueryEnvironmentalConditionFunction(QueryEnvironmentalConditionFn fn)
    {
        query_callback = fn;
    }
}

namespace AmbientCharacterBehavior {
    int32_t QueryEnvironmentalCondition(int32_t condition_key)
    {
        if (!query_callback)
        {
            throw std::runtime_error("QueryEnvironmentalCondition: Callback not registered");
        }

        return query_callback(condition_key);
    }
}