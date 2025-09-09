#include "Action.h"
#include <stdexcept>
#include <utility>

using namespace AmbientCharacterBehavior;

Action::Action(int32_t action_id, std::string action_name, int64_t max_duration, InterruptionBehaviorType interruption_behavior) :
    action_id(action_id), action_name(std::move(action_name)), max_duration_ms(max_duration), interruption_behavior(interruption_behavior)
{
    if (action_id < 0)
    {
        throw std::invalid_argument("Action: action_id cannot be negative");
    }

    if (max_duration < 0)
    {
        throw std::invalid_argument("Action: max_duration_ms cannot be negative");
    }
}

void Action::AddPrecondition(const StateOperation &precondition)
{
    preconditions.emplace_back(precondition);
}

void Action::AddImmediateEffect(const StateOperation &effect)
{
    immediate_effects.emplace_back(effect);
}

void Action::AddCompletionEffect(const StateOperation &effect)
{
    completion_effects.emplace_back(effect);
}
