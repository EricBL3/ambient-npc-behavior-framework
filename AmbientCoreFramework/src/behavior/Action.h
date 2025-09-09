#pragma once
#include <cstdint>
#include <string>
#include <vector>

#include "StateOperation.h"
#include "utils/InterruptionBehaviorType.h"

namespace AmbientCharacterBehavior {

/**
 * @brief An atomic behavior that characters can perform.
 */
class Action {
private:


    int32_t action_id;
    std::string action_name;
    std::vector<StateOperation> preconditions;
    std::vector<StateOperation> immediate_effects;
    std::vector<StateOperation> completion_effects;
    int64_t max_duration_ms;
    InterruptionBehaviorType interruption_behavior;

public:
    /**
     * @throws std::invalid_argument if action_id or max_duration < 0.
     */
    explicit Action(int32_t action_id, std::string action_name, int64_t max_duration, InterruptionBehaviorType interruption_behavior);
    
    void AddPrecondition(const StateOperation& precondition);
    void AddImmediateEffect(const StateOperation& effect);
    void AddCompletionEffect(const StateOperation& effect);

    int32_t GetActionId() const { return action_id; }

    std::string GetActionName() const { return action_name; }

    int64_t GetMaxDuration() const { return max_duration_ms; }

    InterruptionBehaviorType GetInterruptionBehavior() const { return interruption_behavior; }

    const std::vector<StateOperation>& GetPreconditions() const { return preconditions; }

    const std::vector<StateOperation>& GetImmediateEffects() const { return immediate_effects; }

    const std::vector<StateOperation>& GetCompletionEffects() const { return completion_effects; }

};

}