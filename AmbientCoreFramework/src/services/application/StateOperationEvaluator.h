#pragma once
#include "interfaces/IEnvironmentalConditionManager.h"
#include "interfaces/IFrameworkRegistry.h"
#include "interfaces/IFrameworkSchemaManager.h"
#include "interfaces/ILogger.h"
#include "interfaces/IStateOperationEvaluator.h"

namespace AmbientCharacterBehavior {
class StateOperationEvaluator : public IStateOperationEvaluator {
private:
    ILogger& logger;
    IFrameworkSchemaManager& schema_manager;
    IEnvironmentalConditionManager& environment_manager;
    IFrameworkRegistry& framework_registry;

public:
    explicit StateOperationEvaluator(ILogger& logger, IFrameworkSchemaManager& schema_manager,
        IEnvironmentalConditionManager& environment_manager, IFrameworkRegistry& framework_registry) :
        logger(logger), schema_manager(schema_manager), environment_manager(environment_manager), framework_registry(framework_registry) {}

    /**
     * @return True if the state operation could be processed, false if not.
     */
    bool ProcessStateOperation(StateOperation state_operation) override;
};
}