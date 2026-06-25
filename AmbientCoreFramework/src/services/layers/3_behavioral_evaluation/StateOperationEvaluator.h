#pragma once
#include <optional>
#include "services/interfaces/IEntityPositionManager.h"
#include "services/interfaces/IEnvironmentalConditionManager.h"
#include "services/interfaces/IFrameworkSchemaManager.h"
#include "services/interfaces/ILogger.h"
#include "services/interfaces/IStateOperationEvaluator.h"

namespace AmbientCharacterBehavior {
class StateOperationEvaluator : public IStateOperationEvaluator {
private:
    ILogger& logger;
    IFrameworkSchemaManager& schema_manager;
    IEnvironmentalConditionManager& environment_manager;
    IEntityPositionManager& entity_position_manager;

public:
    explicit StateOperationEvaluator(ILogger& logger, IFrameworkSchemaManager& schema_manager,
        IEnvironmentalConditionManager& environment_manager, IEntityPositionManager& entity_position_manager) :
        logger(logger), schema_manager(schema_manager), environment_manager(environment_manager),
        entity_position_manager(entity_position_manager) {}

    /**
     * @return For comparisons, the condition result. For modifications, the success status.
     */
    bool ProcessStateOperation(StateOperation state_operation, StateOperationContext context) override;

    bool IsComparisonOperation(StateOperationType operation) override;
    bool IsModificationOperation(StateOperationType operation) override;

private:
    bool IsValidStateOperation(StateOperationTarget target, StateOperationType operation_type,
        StateOperationContext context);


    bool RequiresTargetEntity(StateOperationTarget target) const;

    std::optional<int32_t> GetStateOperationValue(StateOperationTarget target, int32_t state_key,
        StateOperationContext context) const;

    bool EvaluateStateOperation(StateOperationType operation_type, int32_t state_value, int32_t value,
        FrameworkEntity* context_entity, int32_t state_key);

    int32_t SafeAdd(int32_t a, int32_t b);
    int32_t SafeSubtract(int32_t a, int32_t b);
};
}