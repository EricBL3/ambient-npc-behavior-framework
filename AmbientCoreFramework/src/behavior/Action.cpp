#include "Action.h"
#include <stdexcept>
#include <utility>

using namespace AmbientCharacterBehavior;

Action::Action(int32_t action_id, std::string action_name, int64_t max_duration,
    InterruptionBehaviorType interruption_behavior) : action_id(action_id), action_name(std::move(action_name)),
    max_duration_ms(max_duration), interruption_behavior(interruption_behavior)
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

void Action::AddPrecondition(StateOperationTarget target, const StateOperation &precondition)
{
    preconditions_by_target[target].emplace_back(precondition);
}

void Action::AddImmediateEffect(const StateOperation &effect)
{
    immediate_effects.emplace_back(effect);
}

void Action::AddCompletionEffect(const StateOperation &effect)
{
    completion_effects.emplace_back(effect);
}

void Action::AddInterruptionEffect(const StateOperation &effect)
{
    interruption_effects.emplace_back(effect);
}

const std::vector<StateOperation> * Action::GetPreconditionsForTarget(StateOperationTarget target) const
{
    auto iterator = preconditions_by_target.find(target);

    return iterator == preconditions_by_target.end() ? nullptr : &iterator->second;
}

bool Action::GetRequiresTargetEntity() const
{
    bool requires_target = false;

    const auto entity_preconditions = GetPreconditionsForTarget(StateOperationTarget::ENTITY);
    if (entity_preconditions && !entity_preconditions->empty())
    {
        requires_target =  true;
    }

    const auto entity_distance_preconditions = GetPreconditionsForTarget(StateOperationTarget::DISTANCE_TO_ENTITY);
    if (entity_distance_preconditions && !entity_distance_preconditions->empty())
    {
        requires_target = true;
    }

    return requires_target;
}
