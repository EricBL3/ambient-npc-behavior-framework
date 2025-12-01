#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "StateOperation.h"
#include "enums/InterruptionBehaviorType.h"

namespace AmbientCharacterBehavior {

/**
 * @brief An atomic behavior that characters can perform.
 */
class Action {
private:


    int32_t action_id;
    std::string action_name;
    bool requires_target_entity;
    std::unordered_map<StateOperationTarget, std::vector<StateOperation>> preconditions_by_target;
    std::vector<StateOperation> immediate_effects;
    std::vector<StateOperation> completion_effects;
    std::vector<StateOperation> interruption_effects;
    int64_t max_duration_ms;
    InterruptionBehaviorType interruption_behavior;

public:
    /**
     * @throws std::invalid_argument if action_id or max_duration < 0.
     */
    explicit Action(int32_t action_id, std::string action_name, bool requires_target_entity, int64_t max_duration,
        InterruptionBehaviorType interruption_behavior);
    
    void AddPrecondition(StateOperationTarget target, const StateOperation& precondition);
    void AddImmediateEffect(const StateOperation& effect);
    void AddCompletionEffect(const StateOperation& effect);
    void AddInterruptionEffect(const StateOperation& effect);

    const std::vector<StateOperation>* GetPreconditionsForTarget(StateOperationTarget target) const;

    int32_t GetActionId() const { return action_id; }

    std::string GetActionName() const { return action_name; }

    bool GetRequiresTargetEntity() const { return requires_target_entity; }

    int64_t GetMaxDuration() const { return max_duration_ms; }

    std::unordered_map<StateOperationTarget, std::vector<StateOperation>> GetAllPreconditions() const { return preconditions_by_target; }

    InterruptionBehaviorType GetInterruptionBehavior() const { return interruption_behavior; }

    const std::vector<StateOperation>& GetImmediateEffects() const { return immediate_effects; }

    const std::vector<StateOperation>& GetCompletionEffects() const { return completion_effects; }

    const std::vector<StateOperation>& GetInterruptionEffects() const { return interruption_effects; }

};

}