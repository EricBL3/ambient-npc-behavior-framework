#include "StateOperationEvaluator.h"

#include <algorithm>
#include <limits>

namespace AmbientCharacterBehavior {
bool StateOperationEvaluator::ProcessStateOperation(StateOperation state_operation, StateOperationContext context)
{
    auto state_key = state_operation.GetStateKey();
    auto target = state_operation.GetTarget();
    auto operation_type = state_operation.GetOperationType();
    auto value = state_operation.GetValue();

    if (!IsValidStateOperation(target, operation_type, context))
    {
        return false;
    }

    // Get the state value
    auto state_value = GetStateOperationValue(target, state_key, context);

    if (!state_value.has_value())
    {
        return false;
    }

    FrameworkEntity* context_entity = nullptr;

    if (IsModificationOperation(operation_type))
    {
        if (RequiresTargetEntity(state_operation.GetTarget()))
        {
            context_entity = context.target_entity;
        }
        else
        {
            context_entity = context.self_entity;
        }
    }

    // Evaluate operation
    return EvaluateStateOperation(operation_type, state_value.value(), value, context_entity, state_key);
}

bool StateOperationEvaluator::IsValidStateOperation(StateOperationTarget target, StateOperationType operation_type,
    StateOperationContext context) const
{
    if (target == StateOperationTarget::ENVIRONMENT && IsModificationOperation(operation_type))
    {
        logger.LogWarning("Environment State operations can only be of comparison. The state operation will not be processed. "
                          "Attempted operation_type: " + schema_manager.GetStateOperationTypeName(operation_type),
                          "StateOperationEvaluator");

        return false;
    }

    if (target == StateOperationTarget::DISTANCE_TO_ENTITY && IsModificationOperation(operation_type))
    {
        logger.LogWarning("Distance to entity state operations can only be of comparison. The state operation will "
            "not be processed. Attempted operation_type: " + schema_manager.GetStateOperationTypeName(operation_type),
            "StateOperationEvaluator");

        return false;
    }

    if (target == StateOperationTarget::SELF && context.self_entity == nullptr)
    {
        logger.LogWarning("No self entity was passed for the evaluation. The state operation will not be processed.",
              "StateOperationEvaluator");

        return false;
    }

    if (RequiresTargetEntity(target) && context.target_entity == nullptr)
    {
        logger.LogWarning("No target entity was passed for the evaluation. The state operation will not be processed.",
            "StateOperationEvaluator");

        return false;
    }

    return true;
}

bool StateOperationEvaluator::IsComparisonOperation(StateOperationType operation) const
{
    return operation == StateOperationType::EQUALS ||
        operation == StateOperationType::NOT_EQUALS ||
        operation == StateOperationType::GREATER_THAN ||
        operation == StateOperationType::LESS_THAN;
}

bool StateOperationEvaluator::IsModificationOperation(StateOperationType operation) const
{
    return operation == StateOperationType::SET ||
        operation == StateOperationType::INCREMENT ||
        operation == StateOperationType::DECREMENT;
}

bool StateOperationEvaluator::RequiresTargetEntity(StateOperationTarget target) const
{
    return target == StateOperationTarget::ENTITY || target == StateOperationTarget::DISTANCE_TO_ENTITY;
}

std::optional<int32_t> StateOperationEvaluator::GetStateOperationValue(StateOperationTarget target, int32_t state_key,
    StateOperationContext context) const
{
    try
    {
        switch (target)
        {
            case StateOperationTarget::ENVIRONMENT:
                return environment_manager.GetEnvironmentalConditionValue(state_key);
            case StateOperationTarget::SELF:
                return context.self_entity->GetStateValue(state_key);
            case StateOperationTarget::ENTITY:
                return context.target_entity->GetStateValue(state_key);
            case StateOperationTarget::DISTANCE_TO_ENTITY:
                return entity_position_manager.CalculateDistance(
                    context.self_entity->GetEntityHandle(),
                    context.target_entity->GetEntityHandle()
                );
        }
    }
    catch (std::exception& e)
    {
        logger.LogError("Error while obtaining the state value for state_key: " + std::to_string(state_key) +
            " for target_key: " + std::to_string(static_cast<int>(target)) + ". " + e.what(),
            "StateOperationEvaluator");

        return std::nullopt;
    }

    return std::nullopt;
}

bool StateOperationEvaluator::EvaluateStateOperation(StateOperationType operation_type, int32_t state_value,
    int32_t value, FrameworkEntity* context_entity, int32_t state_key)
{
    int32_t new_value;
    bool res = false;
    switch (operation_type)
    {
        case StateOperationType::EQUALS:
            res = state_value == value;
            logger.LogInfo("Evaluating state_value: " + std::to_string(state_value) + " == " + std::to_string(value),
                "EvaluateStateOperation");
            break;
        case StateOperationType::NOT_EQUALS:
            res = state_value != value;
            logger.LogInfo("Evaluating state_value: " + std::to_string(state_value) + " != " + std::to_string(value),
                "EvaluateStateOperation");
            break;
        case StateOperationType::GREATER_THAN:
            res = state_value > value;
            logger.LogInfo("Evaluating state_value: " + std::to_string(state_value) + " > " + std::to_string(value),
                "EvaluateStateOperation");
            break;
        case StateOperationType::LESS_THAN:
            res = state_value < value;
            logger.LogInfo("Evaluating state_value: " + std::to_string(state_value) + " < " + std::to_string(value),
                "EvaluateStateOperation");
            break;
        case StateOperationType::SET:
            new_value = value;
            break;
        case StateOperationType::INCREMENT:
            new_value = SafeAdd(state_value, value);
            break;
        case StateOperationType::DECREMENT:
            new_value = SafeSubtract(state_value, value);
            break;
        default:
            logger.LogWarning("The operation type: " + schema_manager.GetStateOperationTypeName(operation_type) + " is "
                "currently not supported. The state operation will not be processed", "StateOperationEvaluator");
            return false;
    }

    if (IsComparisonOperation(operation_type))
    {
        return res;
    }

    if (!context_entity)
    {
        logger.LogError("Modification operation called without a valid context entity",
            "EvaluateStateOperation");

        return false;
    }

    logger.LogInfo("Setting state_key: " + std::to_string(state_key) + " from: " + std::to_string(state_value) +
        " to: " + std::to_string(new_value) + " for entity: " + std::to_string(context_entity->GetEntityId()),
                "EvaluateStateOperation");
    context_entity->SetStateValue(state_key, new_value);


    return true;
}

int32_t StateOperationEvaluator::SafeAdd(int32_t a, int32_t b) {
    if (b > 0 && a > std::numeric_limits<int32_t>::max() - b) {
        logger.LogWarning("Addition would overflow, clamping to maximum", "StateOperationEvaluator");
        return std::numeric_limits<int32_t>::max();
    }
    if (b < 0 && a < std::numeric_limits<int32_t>::min() - b) {
        logger.LogWarning("Addition would underflow, clamping to minimum", "StateOperationEvaluator");
        return std::numeric_limits<int32_t>::min();
    }
    return a + b;
}

int32_t StateOperationEvaluator::SafeSubtract(int32_t a, int32_t b) {
    if (b > 0 && a < std::numeric_limits<int32_t>::min() + b) {
        logger.LogWarning("Subtraction would underflow, clamping to minimum", "StateOperationEvaluator");
        return std::numeric_limits<int32_t>::min();
    }
    if (b < 0 && a > std::numeric_limits<int32_t>::max() + b) {
        logger.LogWarning("Subtraction would overflow, clamping to maximum", "StateOperationEvaluator");
        return std::numeric_limits<int32_t>::max();
    }
    return a - b;
}

}
