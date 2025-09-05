
#include "EnvironmentalCondition.h"
#include <stdexcept>

using namespace AmbientCharacterBehavior;

EnvironmentalCondition::EnvironmentalCondition(int32_t condition_key, std::string name, int64_t update_frequency_ms) :
    condition_key(condition_key),
    name(std::move(name)),
    value{},
    update_frequency_ms(update_frequency_ms),
    last_update_ms{},
    is_initialized(false)
{
    if (condition_key < 0)
    {
        throw std::invalid_argument("EnvironmentalCondition: condition_key cannot be negative, got " +
            std::to_string(condition_key));
    }

    if (update_frequency_ms < 0)
    {
        throw std::invalid_argument("EnvironmentalCondition: update_frequency_ms cannot be negative, got " +
            std::to_string(update_frequency_ms));
    }
}

bool EnvironmentalCondition::NeedsToBeUpdated() const
{
    //TODO: Use timer when services is available
    return !is_initialized; //|| TimeManager::GetCurrentTime() - last_update_ms >= update_frequency_ms;
}
