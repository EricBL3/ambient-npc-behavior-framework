#pragma once
#include <cstdint>
#include <stdexcept>

#include "../../../include/BehaviorFrameworkInterface.h"
#include "services/interfaces/IEnvironmentalConditionProvider.h"

namespace AmbientCharacterBehavior {
class EnvironmentalConditionProvider : public IEnvironmentalConditionProvider {
private:
    QueryEnvironmentalConditionCallback query_callback;

public:

    explicit EnvironmentalConditionProvider(QueryEnvironmentalConditionCallback query_callback) : query_callback(query_callback)
    {
        if (!query_callback)
        {
            throw std::invalid_argument("EnvironmentalConditionProvider: Query function cannot be null");
        }
    }

    int32_t QueryEnvironmentalCondition(int32_t condition_key) override
    {
        return query_callback(condition_key);
    }
};
}
