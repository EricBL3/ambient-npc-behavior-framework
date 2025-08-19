/**
 * @file Action.cpp
 * @brief 
 * @author Eric Buitrón López
 * @date 8/18/2025
 *
 *
*/

#include "Action.h"
#include <stdexcept>

using namespace AmbientCharacterBehavior;

Action::Action(int action_id, int max_duration, InterruptionBehaviorType interruption_behavior,
               size_t initial_preconditions_count, size_t initial_immediate_effects_count, size_t initial_completion_effects_count) :
    action_id(action_id), max_duration(max_duration), interruption_behavior(interruption_behavior)
{
    if (action_id < 0)
    {
        throw std::invalid_argument("Action: action_id cannot be negative");
    }

    if (max_duration < 0)
    {
        throw std::invalid_argument("Action: max_duration cannot be negative");
    }

    preconditions.reserve(initial_preconditions_count);
    immediate_effects.reserve(initial_immediate_effects_count);
    completion_effects.reserve(initial_completion_effects_count);
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
