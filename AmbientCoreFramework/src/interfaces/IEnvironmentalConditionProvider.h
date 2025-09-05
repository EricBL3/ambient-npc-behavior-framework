#pragma once
#include <cstdint>

class IEnvironmentalConditionProvider {
public:
    virtual ~IEnvironmentalConditionProvider() = default;

    virtual int32_t QueryEnvironmentalCondition(int32_t key) = 0;
};
