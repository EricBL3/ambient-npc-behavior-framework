/**
 * @file Action.h
 * @brief 
 * @author Eric Buitrón López
 * @date 8/18/2025
 *
 *
*/

#pragma once
#include <vector>

#include "integration/StateOperation.h"
#include "utils/InterruptionBehaviorType.h"

/**
 * @brief An atomic behavior that characters can perform.
 */
class Action {
private:

    /**
     * @brief The unique identifier of the action
     */
    int action_id;

    /**
     * @brief The preconditions that must be satisfied for the action to be able to execute.
     */
    std::vector<StateOperation> preconditions;

    /**
     * @brief The state changes that will be applied when the action begins.
     */
    std::vector<StateOperation> immediate_effects;

    /**
     * @brief The state changes that will be applied when the action completes.
     */
    std::vector<StateOperation> completion_effects;

    /**
     * @brief The maximum amount of time the framework will wait before it automatically completes the action.
     */
    int max_duration;

    /**
     * @brief The resumption capability of the action.
     */
    InterruptionBehaviorType interruption_behavior;

public:
    /**
     *
     * @param action_id The unique identifier of the action
     * @param max_duration The maximum amount of time the framework will wait before it automatically completes the action.
     * @param interruption_behavior The resumption capability of the action.
     * @param initial_preconditions_count The initial number of preconditions for this action (default is 0).
     * @param initial_immediate_effects_count The initial number of immediate effects for this action (default is 0).
     * @param initial_completion_effects_count The initial number of completion effects for this action (default is 0).
     *
     * @throw std::invalid_argument if any parameter < 0.
     */
    explicit Action(int action_id, int max_duration, InterruptionBehaviorType interruption_behavior,
                    size_t initial_preconditions_count = 0, size_t initial_immediate_effects_count = 0,
                    size_t initial_completion_effects_count = 0);

    /**
     * @brief Adds a new precondition to the transition.
     * @param precondition The StateOperation that will be used as a precondition for the transition
     */
    void AddPrecondition(const StateOperation& precondition);

    /**
     * @brief Adds a new immediate effect
     * @param effect The StateOperation that represents the effect to be applied.
     */
    void AddImmediateEffect(const StateOperation& effect);

    /**
     * Adds a new completion effect
     * @param effect The StateOperation that represents the effect to be applied.
     */
    void AddCompletionEffect(const StateOperation& effect);

    int GetActionId() const { return action_id; }

    int GetMaxDuration() const { return max_duration; }

    InterruptionBehaviorType GetInterruptionBehavior() const { return interruption_behavior; }

    const std::vector<StateOperation>& GetPreconditions() const { return preconditions; }

    const std::vector<StateOperation>& GetImmediateEffects() const { return immediate_effects; }

    const std::vector<StateOperation>& GetCompletionEffects() const { return completion_effects; }

};
