#pragma once
#include <memory>

#include "services/interfaces/IEnvironmentalConditionManager.h"
#include "services/interfaces/IEnvironmentalConditionProvider.h"
#include "services/interfaces/IFrameworkSchemaManager.h"
#include "services/interfaces/IJsonLoader.h"
#include "services/interfaces/ILogger.h"
#include "services/interfaces/IStateOperationEvaluator.h"
#include "services/interfaces/ITimeManager.h"
#include "ServiceBundles.h"
#include "services/interfaces/IEntityPositionManager.h"
#include "services/interfaces/IEntityPositionProvider.h"
#include "services/layers/4_content_registry/FrameworkRegistry.h"


namespace AmbientCharacterBehavior {
class ApplicationContext {
private:
    // Service ownership (order matters for destruction)

    std::unique_ptr<ILogger> logger;
    std::unique_ptr<ITimeManager> time_manager;
    std::unique_ptr<IEnvironmentalConditionProvider> environmental_condition_provider;
    std::unique_ptr<IStartCharacterActionProvider> start_character_action_provider;
    std::unique_ptr<IEntityPositionProvider> entity_position_provider;
    std::unique_ptr<IJsonLoader> json_loader;
    std::unique_ptr<IEnvironmentalConditionManager> environmental_condition_manager;
    std::unique_ptr<IEntityPositionManager> entity_position_manager;
    std::unique_ptr<IFrameworkSchemaManager> schema_manager;
    std::unique_ptr<IActionTimeoutManager> action_timeout_manager;
    std::unique_ptr<IStateOperationEvaluator> state_operation_evaluator;
    std::unique_ptr<FrameworkRegistry> registry;

    // Service bundles

    FoundationServices foundation_services;
    DataAccessServices data_access_services;
    SimulationServices simulation_state_services;
    BehavioralEvaluationServices behavioral_evaluation_services;
    std::unique_ptr<ContentRegistryServices> content_registry_services;

public:
    // Factory method that creates all services
    static std::unique_ptr<ApplicationContext> Create(
        QueryEnvironmentalConditionCallback query_env_callback,
        StartCharacterActionCallback start_action_callback,
        QueryEntityPositionCallback query_position_callback
    );

    // Accessor methods

    FoundationServices& Foundation() { return foundation_services; }
    DataAccessServices& DataAccess() { return data_access_services; }
    SimulationServices& SimulationState() { return simulation_state_services; }
    BehavioralEvaluationServices& BehavioralEvaluation() { return behavioral_evaluation_services; }
    ContentRegistryServices& ContentRegistry() const { return *content_registry_services; }

private:
    // Private constructor, forcing the use of the factory method
    ApplicationContext(
        std::unique_ptr<ILogger> logger,
        std::unique_ptr<ITimeManager> time_manager,
        std::unique_ptr<IEnvironmentalConditionProvider> environmental_condition_provider,
        std::unique_ptr<IStartCharacterActionProvider> start_character_action_provider,
        std::unique_ptr<IEntityPositionProvider> entity_position_provider,
        std::unique_ptr<IJsonLoader> json_loader,
        std::unique_ptr<IEnvironmentalConditionManager> environmental_condition_manager,
        std::unique_ptr<IEntityPositionManager> entity_position_manager,
        std::unique_ptr<IFrameworkSchemaManager> schema_manager,
        std::unique_ptr<IActionTimeoutManager> action_timeout_manager,
        std::unique_ptr<IStateOperationEvaluator> state_operation_evaluator);

    // Two phase initialization for registry
    void InitializeRegistry();

};
}
