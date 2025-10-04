#pragma once
#include <optional>

#include "interfaces/IEnvironmentalConditionManager.h"
#include "interfaces/IFrameworkSchemaManager.h"
#include "interfaces/ILogger.h"
#include "interfaces/IStateOperationEvaluator.h"

namespace AmbientCharacterBehavior {
class StateOperationEvaluator : public IStateOperationEvaluator {
private:
    ILogger& logger;
    IFrameworkSchemaManager& schema_manager;
    IEnvironmentalConditionManager& environment_manager;

public:
    explicit StateOperationEvaluator(ILogger& logger, IFrameworkSchemaManager& schema_manager,
        IEnvironmentalConditionManager& environment_manager) :
        logger(logger), schema_manager(schema_manager), environment_manager(environment_manager) {}

    /**
     * @return For comparisons, the condition result. For modifications, the success status.
     */
    bool ProcessStateOperation(StateOperation state_operation, FrameworkEntity* target_entity = nullptr) override;

private:
    bool IsValidStateOperation(StateOperationTarget target, StateOperationType operation_type, const std::vector<int32_t>& parameters,
        const FrameworkEntity* target_entity) const;

    bool IsComparisonOperation(StateOperationType operation) const;
    bool IsModificationOperation(StateOperationType operation) const;
    bool RequiresTargetEntity(StateOperationTarget target) const;

    std::optional<int32_t> GetStateOperationValue(StateOperationTarget target, int32_t state_key, FrameworkEntity* target_entity) const;

    bool EvaluateStateOperation(StateOperationType operation_type, int32_t state_value, const std::vector<int32_t>& parameters,
        FrameworkEntity* target_entity, int32_t state_key);

    int32_t SafeAdd(int32_t a, int32_t b);
    int32_t SafeSubtract(int32_t a, int32_t b);
};
}