/**
 * @file Action.cpp
 * @brief 
 * @author Eric Buitrón López
 * @date 8/18/2025
 *
 *
*/

#include "Action.h"

Action::Action(int action_id, int max_duration, InterruptionBehaviorType interruption_behavior,
    int initial_preconditions_count, int initial_immediate_effects_count, int initial_completion_effects_count) :
    action_id(action_id), max_duration(max_duration), interruption_behavior(interruption_behavior)
{
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
