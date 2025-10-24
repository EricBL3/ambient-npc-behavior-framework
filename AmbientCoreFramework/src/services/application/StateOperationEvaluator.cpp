#include "StateOperationEvaluator.h"

#include <algorithm>
#include <limits>

namespace AmbientCharacterBehavior {
bool StateOperationEvaluator::ProcessStateOperation(StateOperation state_operation, FrameworkEntity* context_entity)
{
    auto state_key = state_operation.GetStateKey();
    auto target = state_operation.GetTarget();
    auto operation_type = state_operation.GetOperationType();
    auto parameters = state_operation.GetParameters();

    if (!IsValidStateOperation(target, operation_type, parameters, context_entity))
    {
        return false;
    }

    // Get the state value
    auto state_value = GetStateOperationValue(target, state_key, context_entity);

    if (!state_value.has_value())
    {
        return false;
    }

    // Evaluate operation
    return EvaluateStateOperation(operation_type, state_value.value(), parameters, context_entity, state_key);
}

bool StateOperationEvaluator::IsValidStateOperation(StateOperationTarget target, StateOperationType operation_type,
    const std::vector<int32_t>& parameters, const FrameworkEntity* context_entity) const
{
    if (target == StateOperationTarget::ENVIRONMENT && IsModificationOperation(operation_type))
    {
        logger.LogWarning("Environment State operations can only be of comparison. The state operation will not be processed. "
                          "Attempted operation_type: " + schema_manager.GetStateOperationTypeName(operation_type),
                          "StateOperationEvaluator");

        return false;
    }

    if (parameters.empty())
    {
        logger.LogWarning("There are no parameters to evaluate. The state operation will not be processed",
            "StateOperationEvaluator");

        return false;
    }

    if (RequiresTargetEntity(target) && context_entity == nullptr)
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
    return target == StateOperationTarget::SELF || target == StateOperationTarget::ENTITY;
}

std::optional<int32_t> StateOperationEvaluator::GetStateOperationValue(StateOperationTarget target, int32_t state_key,
    FrameworkEntity* context_entity) const
{
    try
    {
        switch (target)
        {
            case StateOperationTarget::ENVIRONMENT:
                return environment_manager.GetEnvironmentalConditionValue(state_key);
                break;
            case StateOperationTarget::SELF:
            case StateOperationTarget::ENTITY:
                return context_entity->GetStateValue(state_key);
                break;
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
    const std::vector<int32_t>& parameters, FrameworkEntity* context_entity, int32_t state_key)
{
    int32_t new_value;
    bool res = false;
    switch (operation_type)
    {
        case StateOperationType::EQUALS:
            res = state_value == parameters[0];
            logger.LogInfo("Evaluating state_value: " + std::to_string(state_value) + " == " + std::to_string(parameters[0]),
                "EvaluateStateOperation");
            break;
        case StateOperationType::NOT_EQUALS:
            res = state_value != parameters[0];
            logger.LogInfo("Evaluating state_value: " + std::to_string(state_value) + " != " + std::to_string(parameters[0]),
                "EvaluateStateOperation");
            break;
        case StateOperationType::GREATER_THAN:
            res = state_value > parameters[0];
            logger.LogInfo("Evaluating state_value: " + std::to_string(state_value) + " > " + std::to_string(parameters[0]),
                "EvaluateStateOperation");
            break;
        case StateOperationType::LESS_THAN:
            res = state_value < parameters[0];
            logger.LogInfo("Evaluating state_value: " + std::to_string(state_value) + " < " + std::to_string(parameters[0]),
                "EvaluateStateOperation");
            break;
        case StateOperationType::SET:
            new_value = parameters[0];
            break;
        case StateOperationType::INCREMENT:
            new_value = SafeAdd(state_value, parameters[0]);
            break;
        case StateOperationType::DECREMENT:
            new_value = SafeSubtract(state_value, parameters[0]);
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

    context_entity->SetStateValue(state_key, new_value);
    logger.LogInfo("Setting state_key: " + std::to_string(state_key) + " to value " + std::to_string(new_value),
                "EvaluateStateOperation");

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
